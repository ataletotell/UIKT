/**
 * Ported from UAnimatedTexture5
 */

#include "UMGELMTKitEditorModule.h"
#include "ElementAnimatedTextureThumbnailRenderer.h"
#include "ElementAnimatedTexture.h"
#include "Misc/CoreDelegates.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "FUMGELMTKitEditorModule"

void FUMGELMTKitEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUMGELMTKitEditorModule::OnPostEngineInit);
}

void FUMGELMTKitEditorModule::OnPostEngineInit()
{
	UThumbnailManager::Get().RegisterCustomRenderer(UElementAnimatedTexture::StaticClass(), UElementAnimatedTextureThumbnailRenderer::StaticClass());
}

void FUMGELMTKitEditorModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UElementAnimatedTexture::StaticClass());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUMGELMTKitEditorModule, UMGELMTKitEditor)
