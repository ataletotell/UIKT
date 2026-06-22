// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Render/BackgroundBlurDrawer.h"
#include "RenderingThread.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "EngineModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Render/BackgroundBlurPostProcessor.h"
#include "Render/BackgroundBlurShaders.h"
#include "Render/BackgroundBlurRenderer.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphEvent.h"

#define INVALID_LAYER_ID UINT_MAX

static const FName RendererModuleName("Renderer");

FBackgroundBlurDrawer::FBackgroundBlurDrawer()
	: LayerID(INVALID_LAYER_ID)
	, bApplyColorDeficiencyCorrection(true)
{
}

FBackgroundBlurDrawer::~FBackgroundBlurDrawer()
{
}

static bool ShouldCullWidget(const FSlateWindowElementList& ElementList)
{
	const FSlateClippingManager& ClippingManager = ElementList.GetClippingManager();
	const int32 CurrentIndex = ClippingManager.GetClippingIndex();
	if (CurrentIndex != INDEX_NONE)
	{
		const FSlateClippingState& ClippingState = ClippingManager.GetClippingStates()[CurrentIndex];
		return ClippingState.HasZeroArea();
	}
	return false;
}

bool FBackgroundBlurDrawer::InitializeBackgroundBlurParams(FSlateWindowElementList& ElementList, uint32 InLayer, const FPaintGeometry& PaintGeometry, const FVector4& Params, int32 DownsampleAmount, UTexture* InMaskTexture, EMaskTextureChannel InMaskTextureChannel, bool bInMaskRevertAlpha, float InEffectiveAlpha)
{
	PaintGeometry.CommitTransformsIfUsingLegacyConstructor();

	if (ShouldCullWidget(ElementList))
	{
		return false;
	}

	const FSlateRenderTransform& RenderTransform = PaintGeometry.GetAccumulatedRenderTransform();
	const FVector2D& LocalSize = PaintGeometry.GetLocalSize();

	FVector2D WorldTopLeft  = TransformPoint(RenderTransform, FVector2D::ZeroVector).RoundToVector();
	FVector2D WorldBotRight = TransformPoint(RenderTransform, FVector2D(LocalSize.X, LocalSize.Y)).RoundToVector();

	FVector2D WindowSize = ElementList.GetPaintWindow()->GetViewportSize();
	FVector2D SizeUV = (WorldBotRight - WorldTopLeft) / WindowSize;

	if (SizeUV.X <= 0 || SizeUV.Y <= 0)
	{
		return false;
	}

	FTextureResource* MaskTextureResource = InMaskTexture ? InMaskTexture->GetResource() : nullptr;

	RenderParams.KernelSize        = Params.X;
	RenderParams.ComputedStrength  = Params.Y;
	RenderParams.RenderTargetWidth = Params.Z;
	RenderParams.RenderTargetHeight = Params.W;
	RenderParams.DownsampleAmount  = DownsampleAmount;
	RenderParams.QuadPositionData  = FVector4(WorldTopLeft, WorldBotRight);

	RenderParams.MaskTextureResource = MaskTextureResource;
	RenderParams.MaskTextureChannel  = InMaskTextureChannel;
	RenderParams.bMaskRevertAlpha    = bInMaskRevertAlpha;
	RenderParams.ClippingState       = ResolveClippingState(ElementList);
	RenderParams.EffectiveAlpha      = InEffectiveAlpha;

	return true;
}

const FSlateClippingState* FBackgroundBlurDrawer::ResolveClippingState(FSlateWindowElementList& ElementList) const
{
	FClipStateHandle ClipHandle;
	ClipHandle.SetPreCachedClipIndex(ElementList.GetClippingIndex());

	const TArray<FSlateClippingState>* PrecachedClippingStates = &ElementList.GetClippingManager().GetClippingStates();

	if (ClipHandle.GetCachedClipState())
	{
		return ClipHandle.GetCachedClipState();
	}
	else if (PrecachedClippingStates->IsValidIndex(ClipHandle.GetPrecachedClipIndex()))
	{
		return &(*PrecachedClippingStates)[ClipHandle.GetPrecachedClipIndex()];
	}

	return nullptr;
}

static bool IsMemorylessTexture(const FTextureRHIRef& Tex)
{
	return Tex && EnumHasAnyFlags(Tex->GetFlags(), TexCreate_Memoryless);
}

static bool UpdateScissorRect(
	FRHICommandList& RHICmdList,
	FIntPoint& BackBufferSize,
	const FSlateClippingState* ClippingState,
	FTextureRHIRef& ColorTarget,
	const FVector2f& ViewTranslation2D,
	bool bSwitchVerticalAxis,
	FGraphicsPipelineStateInitializer& InGraphicsPSOInit,
	bool bForceStateChange)
{
	check(RHICmdList.IsInsideRenderPass());
	bool bDidRestartRenderpass = false;

	if (ClippingState != nullptr || bForceStateChange)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_Slate_UpdateScissorRect);

		if (ClippingState)
		{
			const FSlateClippingState& ClipState = *ClippingState;
			if (ClipState.GetClippingMethod() == EClippingMethod::Scissor)
			{
				const FSlateClippingZone& ScissorRect = ClipState.ScissorRect.GetValue();

				const FVector2f ViewSize((float)BackBufferSize.X, (float)BackBufferSize.Y);
				const FVector2f TL_ = ScissorRect.TopLeft     + ViewTranslation2D;
				const FVector2f BR_ = ScissorRect.BottomRight + ViewTranslation2D;
				const FVector2f TopLeft    (FMath::Clamp(TL_.X, 0.0f, ViewSize.X), FMath::Clamp(TL_.Y, 0.0f, ViewSize.Y));
				const FVector2f BottomRight(FMath::Clamp(BR_.X, 0.0f, ViewSize.X), FMath::Clamp(BR_.Y, 0.0f, ViewSize.Y));

				if (bSwitchVerticalAxis)
				{
					const int32 MinY = (int32)(ViewSize.Y - BottomRight.Y);
					const int32 MaxY = (int32)(ViewSize.Y - TopLeft.Y);
					ensure(TopLeft.X <= BottomRight.X && MinY <= MaxY);
					if (TopLeft.X <= BottomRight.X && TopLeft.Y <= BottomRight.Y)
					{
						RHICmdList.SetScissorRect(true, TopLeft.X, MinY, BottomRight.X, MaxY);
					}
					else
					{
						RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
					}
				}
				else
				{
					if (TopLeft.X <= BottomRight.X && TopLeft.Y <= BottomRight.Y)
					{
						RHICmdList.SetScissorRect(true, TopLeft.X, TopLeft.Y, BottomRight.X, BottomRight.Y);
					}
					else
					{
						RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
					}
				}

				InGraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			}

			RHICmdList.ApplyCachedRenderTargets(InGraphicsPSOInit);
		}
		else
		{
			RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
			InGraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		}
	}

	return bDidRestartRenderpass;
}

void FBackgroundBlurDrawer::Draw_RenderThread(FRDGBuilder& GraphBuilder, const FDrawPassInputs& Inputs)
{
	if (!Inputs.OutputTexture)
	{
		return;
	}

	IRendererModule& RendererModule = FModuleManager::GetModuleChecked<IRendererModule>(RendererModuleName);
	FBackgroundBlurRenderParams LocalParams = RenderParams;
	FRDGTexture* OutputRDGTexture = Inputs.OutputTexture;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("BackgroundBlurWithMask"),
		ERDGPassFlags::None,
		[OutputRDGTexture, LocalParams, &RendererModule](FRHICommandList& RHICmdListBase) mutable
		{
			FRHICommandListImmediate& RHICmdList = static_cast<FRHICommandListImmediate&>(RHICmdListBase);

			FRHITexture* OutputRHI = OutputRDGTexture->GetRHI();
			if (!OutputRHI)
			{
				return;
			}

			FTextureRHIRef PostProcessTexture(OutputRHI);
			FTextureRHIRef ColorTarget(OutputRHI);

			FVector4 QuadPositionData          = LocalParams.QuadPositionData;
			FIntPoint BackBufferSize            = PostProcessTexture->GetSizeXY();
			const FSlateClippingState* ClipState = LocalParams.ClippingState;

			FVector2f ViewTranslation2D = FVector2f::ZeroVector;
			bool bSwitchVerticalAxis    = false;

			FPostProcessRectParams RectParams;
			RectParams.SourceTexture     = PostProcessTexture;
			RectParams.SourceRect        = FSlateRect(0, 0, PostProcessTexture->GetSizeX(), PostProcessTexture->GetSizeY());
			RectParams.DestRect          = FSlateRect(QuadPositionData.X, QuadPositionData.Y, QuadPositionData.Z, QuadPositionData.W);
			RectParams.SourceTextureSize = PostProcessTexture->GetSizeXY();

			RectParams.RestoreStateFunc = [&](FRHICommandListImmediate& InRHICmdList, FGraphicsPipelineStateInitializer& InGraphicsPSOInit)
			{
				return UpdateScissorRect(InRHICmdList, BackBufferSize, ClipState, ColorTarget,
					ViewTranslation2D, bSwitchVerticalAxis, InGraphicsPSOInit, true);
			};

			RectParams.RestoreStateFuncPostPipelineState = [&]()
			{
				RHICmdList.SetStencilRef(0);
			};

			FBlurRectParams BlurParams;
			BlurParams.KernelSize         = LocalParams.KernelSize;
			BlurParams.Strength           = LocalParams.ComputedStrength;
			BlurParams.DownsampleAmount   = LocalParams.DownsampleAmount;
			BlurParams.MaskTextureChannel = (int32)LocalParams.MaskTextureChannel;
			BlurParams.bMaskRevertAlpha   = LocalParams.bMaskRevertAlpha;
			BlurParams.EffectiveAlpha     = LocalParams.EffectiveAlpha;
			if (LocalParams.MaskTextureResource)
			{
				BlurParams.MaskTexture = LocalParams.MaskTextureResource->TextureRHI;
			}

			FBackgroundBlurRenderer::Get().GetPostProcessor()->BlurRect(RHICmdList, RendererModule, BlurParams, RectParams);
		}
	);
}
