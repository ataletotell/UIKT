// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGELMTKit.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Render/BackgroundBlurRenderer.h"

#define LOCTEXT_NAMESPACE "FUMGELMTKitModule"

void FUMGELMTKitModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("UMGELMTKit"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/UMGELMTKit"), PluginShaderDir);
}

void FUMGELMTKitModule::ShutdownModule()
{
	FBackgroundBlurRenderer::Get().FlushGeneratedResources();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUMGELMTKitModule, UMGELMTKit)