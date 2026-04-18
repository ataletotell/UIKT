/**
 * Ported from UAnimatedTexture5
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FUMGELMTKitEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OnPostEngineInit();
};
