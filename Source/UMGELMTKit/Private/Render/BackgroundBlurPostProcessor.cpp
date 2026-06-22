// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Render/BackgroundBlurPostProcessor.h"
#include "Render/BackgroundBlurPostProcessResource.h"
#include "Render/BackgroundBlurShaders.h"
#include "ScreenRendering.h"
#include "SceneUtils.h"
#include "RendererInterface.h"
#include "StaticBoundShaderState.h"
#include "PipelineStateCache.h"
#include "CommonRenderResources.h"

DECLARE_CYCLE_STAT(TEXT("Slate PostProcessing RT"), STAT_BackgroundBlurPostProcessingRTTime, STATGROUP_Slate);
DECLARE_CYCLE_STAT(TEXT("Slate ColorDeficiency RT"), STAT_SlateColorDeficiencyRTTime, STATGROUP_Slate);

FBackgroundBlurPostProcessor::FBackgroundBlurPostProcessor()
{
	const int32 NumIntermediateTargets = 3;
	IntermediateTargets = new FBackgroundBlurPostProcessResource(NumIntermediateTargets);
	BeginInitResource(IntermediateTargets);
}

FBackgroundBlurPostProcessor::~FBackgroundBlurPostProcessor()
{
	IntermediateTargets->CleanUp();
}

void FBackgroundBlurPostProcessor::BlurRect(FRHICommandListImmediate& RHICmdList, IRendererModule& RendererModule, const FBlurRectParams& Params, const FPostProcessRectParams& RectParams)
{
	SCOPE_CYCLE_COUNTER(STAT_BackgroundBlurPostProcessingRTTime);
	check(RHICmdList.IsOutsideRenderPass());

	TArray<FVector4f> WeightsAndOffsets;
	const int32 SampleCount = ComputeBlurWeights(Params.KernelSize, Params.Strength, WeightsAndOffsets);

	const bool bDownsample = Params.DownsampleAmount > 0;

	FIntPoint DestRectSize = RectParams.DestRect.GetSize().IntPoint();
	FIntPoint RequiredSize = bDownsample
		? FIntPoint(FMath::DivideAndRoundUp(DestRectSize.X, Params.DownsampleAmount), FMath::DivideAndRoundUp(DestRectSize.Y, Params.DownsampleAmount))
		: DestRectSize;

	RequiredSize.X = FMath::Min(RequiredSize.X, RectParams.SourceTextureSize.X);
	RequiredSize.Y = FMath::Min(RequiredSize.Y, RectParams.SourceTextureSize.Y);

	SCOPED_DRAW_EVENTF(RHICmdList, BackgroundBlurPostProcess, TEXT("Slate Post Process Blur Background Kernel: %dx%d Size: %dx%d"), SampleCount, SampleCount, RequiredSize.X, RequiredSize.Y);

	const FIntPoint DownsampleSize = RequiredSize;

	IntermediateTargets->Update(RHICmdList, DestRectSize);

	if (bDownsample)
	{
		DownsampleRect(RHICmdList, RendererModule, RectParams, DownsampleSize);
	}

	CopysampleRect(RHICmdList, RendererModule, RectParams);

	FSamplerStateRHIRef BilinearClamp = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	check(ShaderMap);

	const int32 SrcTextureWidth  = RectParams.SourceTextureSize.X;
	const int32 SrcTextureHeight = RectParams.SourceTextureSize.Y;
	const int32 DestTextureWidth  = IntermediateTargets->GetWidth();
	const int32 DestTextureHeight = IntermediateTargets->GetHeight();

	const FSlateRect& DestRect = RectParams.DestRect;

	FVertexDeclarationRHIRef VertexDecl = GFilterVertexDeclaration.VertexDeclarationRHI;
	check(IsValidRef(VertexDecl));

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	GraphicsPSOInit.BlendState        = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);

	const FVector2f InvBufferSize(1.0f / DestTextureWidth, 1.0f / DestTextureHeight);
	const FVector2f HalfTexelOffset(0.5f / DestTextureWidth, 0.5f / DestTextureHeight);

	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	TShaderMapRef<FBackgroundBlurPostProcessBlurPS> PixelShader(ShaderMap);

	for (int32 PassIndex = 0; PassIndex < 2; ++PassIndex)
	{
		FTextureRHIRef SourceTexture = (PassIndex == 0)
			? (bDownsample ? IntermediateTargets->GetRenderTarget(0) : RectParams.SourceTexture)
			: IntermediateTargets->GetRenderTarget(1);
		FTextureRHIRef DestTexture = (PassIndex == 0)
			? IntermediateTargets->GetRenderTarget(1)
			: IntermediateTargets->GetRenderTarget(0);

		RHICmdList.Transition(FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(DestTexture,   ERHIAccess::Unknown, ERHIAccess::RTV));

		const FString PassName = (PassIndex == 0) ? TEXT("SlateBlurRectPass0") : TEXT("SlateBlurRect");
		FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
		RHICmdList.BeginRenderPass(RPInfo, *PassName);
		{
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI      = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI       = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			PixelShader->SetWeightsAndOffsets(RHICmdList, WeightsAndOffsets, SampleCount);
			PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);

			if (bDownsample)
			{
				PixelShader->SetUVBounds(RHICmdList, FVector4f(
					0.f, 0.f,
					(float)DownsampleSize.X / DestTextureWidth  - HalfTexelOffset.X,
					(float)DownsampleSize.Y / DestTextureHeight - HalfTexelOffset.Y));
				PixelShader->SetBufferSizeAndDirection(RHICmdList, InvBufferSize, (PassIndex == 0) ? FVector2f(1, 0) : FVector2f(0, 1));

				RendererModule.DrawRectangle(
					RHICmdList,
					0, 0, DownsampleSize.X, DownsampleSize.Y,
					0, 0, DownsampleSize.X, DownsampleSize.Y,
					FIntPoint(DestTextureWidth, DestTextureHeight),
					FIntPoint(DestTextureWidth, DestTextureHeight),
					VertexShader, EDRF_Default);
			}
			else
			{
				const FVector2f InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);
				const FVector2f UVStart(DestRect.Left * InvSrcTextureSize.X, DestRect.Top    * InvSrcTextureSize.Y);
				const FVector2f UVEnd  (DestRect.Right * InvSrcTextureSize.X, DestRect.Bottom * InvSrcTextureSize.Y);
				const FVector2f SizeUV = UVEnd - UVStart;

				PixelShader->SetUVBounds(RHICmdList, FVector4f(UVStart.X, UVStart.Y, UVEnd.X, UVEnd.Y));
				PixelShader->SetBufferSizeAndDirection(RHICmdList, InvSrcTextureSize, (PassIndex == 0) ? FVector2f(1, 0) : FVector2f(0, 1));

				RendererModule.DrawRectangle(
					RHICmdList,
					0, 0, RequiredSize.X, RequiredSize.Y,
					UVStart.X, UVStart.Y, SizeUV.X, SizeUV.Y,
					FIntPoint(DestTextureWidth, DestTextureHeight),
					FIntPoint(1, 1),
					VertexShader, EDRF_Default);
			}
		}
		RHICmdList.EndRenderPass();
	}

	UpsampleRect(RHICmdList, RendererModule, Params, RectParams, DownsampleSize, BilinearClamp);
}

void FBackgroundBlurPostProcessor::ReleaseRenderTargets()
{
	check(IsInGameThread());
	BeginReleaseResource(IntermediateTargets);
}

void FBackgroundBlurPostProcessor::CopysampleRect(FRHICommandListImmediate& RHICmdList, IRendererModule& RendererModule, const FPostProcessRectParams& Params)
{
	const int32 SrcTextureWidth  = Params.SourceTextureSize.X;
	const int32 SrcTextureHeight = Params.SourceTextureSize.Y;
	const int32 DestTextureWidth  = IntermediateTargets->GetWidth();
	const int32 DestTextureHeight = IntermediateTargets->GetHeight();

	const FSlateRect& DestRect     = Params.DestRect;
	const FIntPoint   DestRectSize = Params.DestRect.GetSize().IntPoint();

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	TShaderMapRef<FBackgroundBlurPostProcessCopysamplePS> PixelShader(ShaderMap);

	FSamplerStateRHIRef BilinearClamp = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FTextureRHIRef DestTexture = IntermediateTargets->GetRenderTarget(2);

	RHICmdList.Transition(FRHITransitionInfo(Params.SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(DestTexture,          ERHIAccess::Unknown, ERHIAccess::RTV));

	const FVector2f InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);
	const FVector2f UVStart(DestRect.Left * InvSrcTextureSize.X, DestRect.Top    * InvSrcTextureSize.Y);
	const FVector2f UVEnd  (DestRect.Right * InvSrcTextureSize.X, DestRect.Bottom * InvSrcTextureSize.Y);
	const FVector2f SizeUV = UVEnd - UVStart;

	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);
	RHICmdList.SetScissorRect(false, 0, 0, 0, 0);

	FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("CopysampleRect"));
	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState        = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI      = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI       = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		PixelShader->SetTexture(RHICmdList, Params.SourceTexture, BilinearClamp);

		RendererModule.DrawRectangle(
			RHICmdList,
			0, 0, DestRectSize.X, DestRectSize.Y,
			UVStart.X, UVStart.Y, SizeUV.X, SizeUV.Y,
			FIntPoint(DestTextureWidth, DestTextureHeight),
			FIntPoint(1, 1),
			VertexShader, EDRF_Default);
	}
	RHICmdList.EndRenderPass();
}

void FBackgroundBlurPostProcessor::DownsampleRect(FRHICommandListImmediate& RHICmdList, IRendererModule& RendererModule, const FPostProcessRectParams& Params, const FIntPoint& DownsampleSize)
{
	SCOPED_DRAW_EVENT(RHICmdList, BackgroundBlurPostProcessDownsample);

	const int32 SrcTextureWidth  = Params.SourceTextureSize.X;
	const int32 SrcTextureHeight = Params.SourceTextureSize.Y;
	const int32 DestTextureWidth  = IntermediateTargets->GetWidth();
	const int32 DestTextureHeight = IntermediateTargets->GetHeight();

	const FSlateRect& DestRect = Params.DestRect;

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	TShaderMapRef<FBackgroundBlurPostProcessDownsamplePS> PixelShader(ShaderMap);

	FSamplerStateRHIRef BilinearClamp = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FTextureRHIRef DestTexture = IntermediateTargets->GetRenderTarget(0);

	RHICmdList.Transition(FRHITransitionInfo(Params.SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(DestTexture,          ERHIAccess::Unknown, ERHIAccess::RTV));

	const FVector2f InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);
	const FVector2f UVStart(DestRect.Left * InvSrcTextureSize.X, DestRect.Top    * InvSrcTextureSize.Y);
	const FVector2f UVEnd  (DestRect.Right * InvSrcTextureSize.X, DestRect.Bottom * InvSrcTextureSize.Y);
	const FVector2f SizeUV = UVEnd - UVStart;

	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);
	RHICmdList.SetScissorRect(false, 0, 0, 0, 0);

	FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DownsampleRect"));
	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState        = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI      = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI       = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		PixelShader->SetShaderParams(RHICmdList, FVector4f(InvSrcTextureSize.X, InvSrcTextureSize.Y, 0, 0));
		PixelShader->SetUVBounds(RHICmdList, FVector4f(UVStart.X, UVStart.Y, UVEnd.X, UVEnd.Y));
		PixelShader->SetTexture(RHICmdList, Params.SourceTexture, BilinearClamp);

		RendererModule.DrawRectangle(
			RHICmdList,
			0, 0, DownsampleSize.X, DownsampleSize.Y,
			UVStart.X, UVStart.Y, SizeUV.X, SizeUV.Y,
			FIntPoint(DestTextureWidth, DestTextureHeight),
			FIntPoint(1, 1),
			VertexShader, EDRF_Default);
	}
	RHICmdList.EndRenderPass();
}

void FBackgroundBlurPostProcessor::UpsampleRect(FRHICommandListImmediate& RHICmdList, IRendererModule& RendererModule, const FBlurRectParams& Params, const FPostProcessRectParams& RectParams, const FIntPoint& DownsampleSize, FSamplerStateRHIRef& Sampler)
{
	SCOPED_DRAW_EVENT(RHICmdList, BackgroundBlurPostProcessUpsample);

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	const bool bUseAlphaBlend = Params.EffectiveAlpha < 1.0f - KINDA_SMALL_NUMBER;
	GraphicsPSOInit.BlendState = bUseAlphaBlend
		? TStaticBlendState<CW_RGBA, BO_Add, BF_ConstantBlendFactor, BF_InverseConstantBlendFactor>::GetRHI()
		: TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	FTextureRHIRef DestTexture      = RectParams.SourceTexture;
	const int32 DestTextureWidth    = RectParams.SourceTextureSize.X;
	const int32 DestTextureHeight   = RectParams.SourceTextureSize.Y;
	const int32 DownsampledWidth    = DownsampleSize.X;
	const int32 DownsampledHeight   = DownsampleSize.Y;

	FTextureRHIRef BlurTexture    = IntermediateTargets->GetRenderTarget(0);
	FTextureRHIRef SrcTexture     = IntermediateTargets->GetRenderTarget(2);
	const int32 SrcTextureWidth   = IntermediateTargets->GetWidth();
	const int32 SrcTextureHeight  = IntermediateTargets->GetHeight();

	const FSlateRect& DestRect    = RectParams.DestRect;
	const FIntPoint   DestRectSize = DestRect.GetSize().IntPoint();

	FTextureRHIRef MaskTexture = Params.MaskTexture;
	FVector2f MaskTransform = (Params.MaskTextureChannel == 3 && Params.bMaskRevertAlpha)
		? FVector2f(-1.0f, 1.0f)
		: FVector2f(1.0f, 0.0f);

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);

	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);
	RHICmdList.Transition(FRHITransitionInfo(MaskTexture,  ERHIAccess::Unknown, ERHIAccess::SRVMask));
	RHICmdList.Transition(FRHITransitionInfo(BlurTexture,  ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(SrcTexture,   ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(DestTexture,  ERHIAccess::Unknown, ERHIAccess::RTV));

	FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("UpsampleRect"));
	{
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		if (RectParams.RestoreStateFunc)
		{
			RectParams.RestoreStateFunc(RHICmdList, GraphicsPSOInit);
		}

		TShaderMapRef<FBackgroundBlurScreenWithMaskPS> PixelShader(ShaderMap);
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI      = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI       = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		if (bUseAlphaBlend)
		{
			RHICmdList.SetBlendFactor(FLinearColor(Params.EffectiveAlpha, Params.EffectiveAlpha, Params.EffectiveAlpha, Params.EffectiveAlpha));
		}

		if (RectParams.RestoreStateFuncPostPipelineState)
		{
			RectParams.RestoreStateFuncPostPipelineState();
		}

		PixelShader->SetTexture(RHICmdList, BlurTexture, Sampler);
		PixelShader->SetMaskTexture(RHICmdList, MaskTexture, Sampler);
		PixelShader->SetMaskTextureChannel(RHICmdList, Params.MaskTextureChannel);
		PixelShader->SetMaskTransform(RHICmdList, MaskTransform);
		PixelShader->SetSourceTexture(RHICmdList, SrcTexture, Sampler);

		const float SourceSizeU = (DestRectSize.X == SrcTextureWidth)  ? 1.0f : (DestRectSize.X / (float)SrcTextureWidth)  - (1.0f / (float)SrcTextureWidth);
		const float SourceSizeV = (DestRectSize.Y == SrcTextureHeight) ? 1.0f : (DestRectSize.Y / (float)SrcTextureHeight) - (1.0f / (float)SrcTextureHeight);
		PixelShader->SetSourceUVBounds(RHICmdList, FVector4f(0.f, 0.f, SourceSizeU, SourceSizeV));

		const float SizeU = (DownsampledWidth  == SrcTextureWidth)  ? 1.0f : (DownsampledWidth  / (float)SrcTextureWidth)  - (1.0f / (float)SrcTextureWidth);
		const float SizeV = (DownsampledHeight == SrcTextureHeight) ? 1.0f : (DownsampledHeight / (float)SrcTextureHeight) - (1.0f / (float)SrcTextureHeight);
		PixelShader->SetUVBounds(RHICmdList, FVector4f(0.f, 0.f, SizeU, SizeV));

		RendererModule.DrawRectangle(RHICmdList,
			DestRect.Left, DestRect.Top,
			DestRect.Right - DestRect.Left, DestRect.Bottom - DestRect.Top,
			0, 0, SizeU, SizeV,
			RectParams.SourceTextureSize,
			FIntPoint(1, 1),
			VertexShader, EDRF_Default);
	}
	RHICmdList.EndRenderPass();
}

// Bilinear-filter Gaussian weight computation

static float GetWeight(float Dist, float Strength)
{
	const float Strength2 = Strength * Strength;
	return (1.0f / FMath::Sqrt(2 * PI * Strength2)) * FMath::Exp(-(Dist * Dist) / (2 * Strength2));
}

static FVector2f GetWeightAndOffset(float Dist, float Sigma)
{
	const float Weight1 = GetWeight(Dist,     Sigma);
	const float Weight2 = GetWeight(Dist + 1, Sigma);
	const float TotalWeight = Weight1 + Weight2;
	const float Offset = (TotalWeight > 0) ? (Weight1 * Dist + Weight2 * (Dist + 1)) / TotalWeight : 0.f;
	return FVector2f(TotalWeight, Offset);
}

static int32 ComputeWeights(int32 KernelSize, float Sigma, TArray<FVector4f>& OutWeightsAndOffsets)
{
	const int32 NumSamples = FMath::DivideAndRoundUp(KernelSize, 2);
	OutWeightsAndOffsets.AddUninitialized(NumSamples % 2 == 0 ? NumSamples / 2 : NumSamples / 2 + 1);

	const FVector2f WO1 = GetWeightAndOffset(1, Sigma);
	OutWeightsAndOffsets[0] = FVector4f(GetWeight(0, Sigma), 0.f, WO1.X, WO1.Y);

	int32 SampleIndex = 1;
	for (int32 X = 3; X < KernelSize; X += 4)
	{
		const FVector2f WOA = GetWeightAndOffset(X,     Sigma);
		const FVector2f WOB = GetWeightAndOffset(X + 2, Sigma);
		OutWeightsAndOffsets[SampleIndex] = FVector4f(WOA.X, WOA.Y, WOB.X, WOB.Y);
		++SampleIndex;
	}

	return NumSamples;
}

int32 FBackgroundBlurPostProcessor::ComputeBlurWeights(int32 KernelSize, float StdDev, TArray<FVector4f>& OutWeightsAndOffsets)
{
	return ComputeWeights(KernelSize, StdDev, OutWeightsAndOffsets);
}
