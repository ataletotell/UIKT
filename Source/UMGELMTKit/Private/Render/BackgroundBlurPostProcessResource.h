// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Textures/SlateShaderResource.h"
#include "RenderResource.h"
#include "RenderingThread.h"

class FBackgroundBlurPostProcessResource : public FSlateShaderResource, public FRenderResource, private FDeferredCleanupInterface
{
public:
	FBackgroundBlurPostProcessResource(int32 InRenderTargetCount);
	~FBackgroundBlurPostProcessResource();

	FTextureRHIRef GetRenderTarget(int32 Index)
	{
		return RenderTargets[Index];
	}

	void Update(FRHICommandListBase& RHICmdList, const FIntPoint& NewSize);
	void CleanUp();

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;

	virtual uint32 GetWidth() const override { return RenderTargetSize.X; }
	virtual uint32 GetHeight() const override { return RenderTargetSize.Y; }
	virtual ESlateShaderResource::Type GetType() const override { return ESlateShaderResource::PostProcess; }

private:
	void ResizeTargets(FRHICommandListBase& RHICmdList, const FIntPoint& NewSize);

private:
	TArray<FTextureRHIRef, TInlineAllocator<3>> RenderTargets;
	EPixelFormat PixelFormat;
	FIntPoint RenderTargetSize;
	int32 RenderTargetCount;
};
