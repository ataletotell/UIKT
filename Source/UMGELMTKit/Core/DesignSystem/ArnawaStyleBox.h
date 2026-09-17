// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Components/Border.h"

#include "ArnawaStyleBox.generated.h"

class UArnawaComponentStyle;

/**
 * A Border that takes its whole look from a design-system component style rather than from
 * per-instance values, so a screen never carries its own copy of a colour or a radius.
 *
 * Set Style + Variant; background, border, corner radii and padding all come from the asset.
 * Editing the style asset (or re-running the kitchen sink extractor over it) restyles every
 * instance in every screen at once - that is the point of the whole design system.
 */
UCLASS(meta = (DisplayName = "Arnawa Style Box", Category = "Arnawa|Design System"))
class UMGELMTKIT_API UArnawaStyleBox : public UBorder
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	TObjectPtr<UArnawaComponentStyle> Style;

	/** Modifier class from the mockup ("alt", "off", "on"); None uses the component's base look. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	FName Variant;

	UFUNCTION(BlueprintCallable, Category = "Design System")
	void SetStyleAsset(UArnawaComponentStyle* NewStyle, FName NewVariant);

	//~ UWidget
	virtual void SynchronizeProperties() override;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End UWidget

private:
	void ApplyStyle();
};
