// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Render/BackgroundBlurRenderer.h"
#include "RenderUtils.h"
#include "Render/BackgroundBlurPostProcessor.h"

FBackgroundBlurRenderer::FBackgroundBlurRenderer()
	: PostProcessor(new FBackgroundBlurPostProcessor)
{
	;
}

FBackgroundBlurRenderer::~FBackgroundBlurRenderer()
{
	;
}

FBackgroundBlurRenderer& FBackgroundBlurRenderer::Get()
{
	static FBackgroundBlurRenderer Renderer;
	return Renderer;
}

TSharedRef<FBackgroundBlurPostProcessor> FBackgroundBlurRenderer::GetPostProcessor()
{
	return PostProcessor;
}

void FBackgroundBlurRenderer::ResetBatches()
{
	NumPostProcessPasses = 0;
}

void FBackgroundBlurRenderer::FlushGeneratedResources()
{
	PostProcessor->ReleaseRenderTargets();
}
