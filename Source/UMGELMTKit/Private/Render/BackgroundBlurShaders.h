// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "Rendering/RenderingCommon.h"
#include "RHIStaticStates.h"

const int32 MAX_BLUR_SAMPLES = 127;

class FBackgroundBlurElementPS : public FGlobalShader
{
	DECLARE_TYPE_LAYOUT(FBackgroundBlurElementPS, NonVirtual);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FBackgroundBlurElementPS() {}

	FBackgroundBlurElementPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		TextureParameter.Bind(Initializer.ParameterMap, TEXT("ElementTexture"));
		TextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("ElementTextureSampler"));
		ShaderParams.Bind(Initializer.ParameterMap, TEXT("ShaderParams"));
		GammaAndAlphaValues.Bind(Initializer.ParameterMap, TEXT("GammaAndAlphaValues"));
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), TextureParameter, TextureParameterSampler, SamplerState, InTexture);
	}

	void SetShaderParams(FRHICommandList& RHICmdList, const FVector4f& InShaderParams)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, ShaderParams, InShaderParams);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetDisplayGammaAndInvertAlphaAndContrast(FRHICommandList& RHICmdList, float InDisplayGamma, float bInvertAlpha, float InContrast)
	{
		FVector4f Values(2.2f / InDisplayGamma, 1.0f / InDisplayGamma, bInvertAlpha, InContrast);
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, GammaAndAlphaValues, Values);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, TextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, TextureParameterSampler);
	LAYOUT_FIELD(FShaderParameter, ShaderParams);
	LAYOUT_FIELD(FShaderParameter, GammaAndAlphaValues);
};


class FBackgroundBlurPostProcessBlurPS : public FBackgroundBlurElementPS
{
	DECLARE_SHADER_TYPE(FBackgroundBlurPostProcessBlurPS, Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FBackgroundBlurPostProcessBlurPS() {}

	FBackgroundBlurPostProcessBlurPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FBackgroundBlurElementPS(Initializer)
	{
		BufferSizeAndDirection.Bind(Initializer.ParameterMap, TEXT("BufferSizeAndDirection"));
		WeightAndOffsets.Bind(Initializer.ParameterMap, TEXT("WeightAndOffsets"));
		SampleCount.Bind(Initializer.ParameterMap, TEXT("SampleCount"));
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
	}

	void SetBufferSizeAndDirection(FRHICommandList& RHICmdList, const FVector2f& InBufferSize, const FVector2f& InDir)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, BufferSizeAndDirection, FVector4f(InBufferSize.X, InBufferSize.Y, InDir.X, InDir.Y));
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetWeightsAndOffsets(FRHICommandList& RHICmdList, const TArray<FVector4f>& InWeightsAndOffsets, int32 NumSamples)
	{
		check(InWeightsAndOffsets.Num() <= MAX_BLUR_SAMPLES);
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValueArray(BatchedParams, WeightAndOffsets, InWeightsAndOffsets.GetData(), InWeightsAndOffsets.Num());
		SetShaderValue(BatchedParams, SampleCount, NumSamples);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4f& InUVBounds)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, UVBounds, InUVBounds);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

private:
	LAYOUT_FIELD(FShaderParameter, BufferSizeAndDirection);
	LAYOUT_FIELD(FShaderParameter, WeightAndOffsets);
	LAYOUT_FIELD(FShaderParameter, SampleCount);
	LAYOUT_FIELD(FShaderParameter, UVBounds);
};


class FBackgroundBlurScreenWithMaskPS : public FBackgroundBlurElementPS
{
	DECLARE_SHADER_TYPE(FBackgroundBlurScreenWithMaskPS, Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FBackgroundBlurScreenWithMaskPS() {}

	FBackgroundBlurScreenWithMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FBackgroundBlurElementPS(Initializer)
	{
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
		MaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("MaskTexture"));
		MaskTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("MaskTextureSampler"));
		MaskTextureChannelParameter.Bind(Initializer.ParameterMap, TEXT("MaskTextureChannel"));
		MaskTransformParameter.Bind(Initializer.ParameterMap, TEXT("MaskTransform"));
		SourceTextureParameter.Bind(Initializer.ParameterMap, TEXT("SourceTexture"));
		SourceTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("SourceTextureSampler"));
		SourceUVBounds.Bind(Initializer.ParameterMap, TEXT("SourceUVBounds"));
	}

	void SetSourceTexture(FRHICommandList& RHICmdList, FRHITexture* InSourceTexture, const FSamplerStateRHIRef SourceSamplerState)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SourceTextureParameter, SourceTextureParameterSampler, SourceSamplerState, InSourceTexture);
	}

	void SetSourceUVBounds(FRHICommandList& RHICmdList, const FVector4f& InSourceUVBounds)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, SourceUVBounds, InSourceUVBounds);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4f& InUVBounds)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, UVBounds, InUVBounds);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetMaskTexture(FRHICommandList& RHICmdList, FRHITexture* InMaskTexture, const FSamplerStateRHIRef MaskSamplerState)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MaskTextureParameter, MaskTextureParameterSampler, MaskSamplerState, InMaskTexture);
	}

	void SetMaskTextureChannel(FRHICommandList& RHICmdList, int32 InMaskTextureChannel)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, MaskTextureChannelParameter, InMaskTextureChannel);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

	void SetMaskTransform(FRHICommandList& RHICmdList, FVector2f InMaskTransform)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, MaskTransformParameter, InMaskTransform);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

private:
	LAYOUT_FIELD(FShaderParameter, UVBounds);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameterSampler);
	LAYOUT_FIELD(FShaderParameter, MaskTextureChannelParameter);
	LAYOUT_FIELD(FShaderParameter, MaskTransformParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SourceTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SourceTextureParameterSampler);
	LAYOUT_FIELD(FShaderParameter, SourceUVBounds);
};


class FBackgroundBlurPostProcessDownsamplePS : public FBackgroundBlurElementPS
{
	DECLARE_SHADER_TYPE(FBackgroundBlurPostProcessDownsamplePS, Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FBackgroundBlurPostProcessDownsamplePS() {}

	FBackgroundBlurPostProcessDownsamplePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FBackgroundBlurElementPS(Initializer)
	{
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4f& InUVBounds)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, UVBounds, InUVBounds);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

private:
	LAYOUT_FIELD(FShaderParameter, UVBounds);
};


class FBackgroundBlurPostProcessCopysamplePS : public FBackgroundBlurElementPS
{
	DECLARE_SHADER_TYPE(FBackgroundBlurPostProcessCopysamplePS, Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FBackgroundBlurPostProcessCopysamplePS() {}

	FBackgroundBlurPostProcessCopysamplePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FBackgroundBlurElementPS(Initializer)
	{
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4f& InUVBounds)
	{
		FRHIBatchedShaderParameters& BatchedParams = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParams, UVBounds, InUVBounds);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParams);
	}

private:
	LAYOUT_FIELD(FShaderParameter, UVBounds);
};
