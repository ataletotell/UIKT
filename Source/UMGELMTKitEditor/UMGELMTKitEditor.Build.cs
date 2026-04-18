// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UMGELMTKitEditor : ModuleRules
{
    public UMGELMTKitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UnrealEd",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "RHI",
                "RenderCore",
                "UnrealEd",
                "UMGELMTKit"
			}
            );
    }
}
