// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Render/BackgroundBlurPostProcessResource.h"
#include "RenderUtils.h"

DECLARE_MEMORY_STAT(TEXT("PostProcess RenderTargets"), STAT_SLATEPPRenderTargetMem, STATGROUP_SlateMemory);

FBackgroundBlurPostProcessResource::FBackgroundBlurPostProcessResource(int32 InRenderTargetCount)
	: RenderTargetSize(FIntPoint::ZeroValue)
	, RenderTargetCount(InRenderTargetCount)
{
}

FBackgroundBlurPostProcessResource::~FBackgroundBlurPostProcessResource()
{
}

void FBackgroundBlurPostProcessResource::Update(FRHICommandListBase& RHICmdList, const FIntPoint& NewSize)
{
	if (NewSize.X > RenderTargetSize.X || NewSize.Y > RenderTargetSize.Y || RenderTargetSize == FIntPoint::ZeroValue || RenderTargets.Num() == 0)
	{
		if (!IsInitialized())
		{
			InitResource(RHICmdList);
		}
		ResizeTargets(RHICmdList, NewSize);
	}
}

void FBackgroundBlurPostProcessResource::ResizeTargets(FRHICommandListBase& RHICmdList, const FIntPoint& NewSize)
{
	check(IsInRenderingThread() || IsInRHIThread() || IsInParallelRenderingThread());

	RenderTargets.Empty();
	RenderTargetSize = NewSize;
	PixelFormat = PF_B8G8R8A8;

	if (RenderTargetSize.X > 0 && RenderTargetSize.Y > 0)
	{
		for (int32 TexIndex = 0; TexIndex < RenderTargetCount; ++TexIndex)
		{
			const FRHITextureCreateDesc Desc =
				FRHITextureCreateDesc::Create2D(TEXT("BackgroundBlurRT"), RenderTargetSize.X, RenderTargetSize.Y, PixelFormat)
				.SetFlags(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource);
			RenderTargets.Add(RHICmdList.CreateTexture(Desc));
		}
	}

	STAT(int64 TotalMemory = RenderTargetCount * GPixelFormats[PixelFormat].BlockBytes * RenderTargetSize.X * RenderTargetSize.Y);
	SET_MEMORY_STAT(STAT_SLATEPPRenderTargetMem, TotalMemory);
}

void FBackgroundBlurPostProcessResource::CleanUp()
{
	BeginReleaseResource(this);
	BeginCleanup(this);
}

void FBackgroundBlurPostProcessResource::InitRHI(FRHICommandListBase& RHICmdList)
{
}

void FBackgroundBlurPostProcessResource::ReleaseRHI()
{
	SET_MEMORY_STAT(STAT_SLATEPPRenderTargetMem, 0);
	RenderTargetSize = FIntPoint::ZeroValue;
	RenderTargets.Empty();
}
