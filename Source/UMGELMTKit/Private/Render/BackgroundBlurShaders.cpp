// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Render/BackgroundBlurShaders.h"
#include "Rendering/RenderingCommon.h"
#include "PipelineStateCache.h"

IMPLEMENT_TYPE_LAYOUT(FBackgroundBlurElementPS);

IMPLEMENT_SHADER_TYPE(, FBackgroundBlurPostProcessBlurPS,         TEXT("/Plugin/UMGELMTKit/Private/BackgroundBlurPostProcessPixelShader.usf"), TEXT("GaussianBlurMain"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FBackgroundBlurPostProcessDownsamplePS,   TEXT("/Plugin/UMGELMTKit/Private/BackgroundBlurPostProcessPixelShader.usf"), TEXT("DownsampleMain"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FBackgroundBlurPostProcessCopysamplePS,   TEXT("/Plugin/UMGELMTKit/Private/BackgroundBlurPostProcessPixelShader.usf"), TEXT("CopysampleMain"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FBackgroundBlurScreenWithMaskPS,          TEXT("/Plugin/UMGELMTKit/Private/BackgroundBlurPostProcessPixelShader.usf"), TEXT("MaskMain"), SF_Pixel);

void FBackgroundBlurElementPS::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.HDR.Display.OutputDevice"));
	OutEnvironment.SetDefine(TEXT("USE_709"), CVar ? (CVar->GetValueOnGameThread() == 1) : 1);
}
