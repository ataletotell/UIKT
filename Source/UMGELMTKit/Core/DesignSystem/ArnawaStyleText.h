// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Components/TextBlock.h"

#include "ArnawaStyleText.generated.h"

class UArnawaComponentStyle;
class UArnawaDesignTokens;

/**
 * A TextBlock whose font, colour, line height and casing come from the design system: either a
 * component style's text half, or a named role from the token asset's type ramp (set TypeRole
 * and leave Style empty for plain prose that is not part of a specific component).
 *
 * Line height is applied the way Slate actually means it: LineHeightPercentage multiplies the
 * font's own max character height, not the font size the way CSS line-height does, so the
 * CSS px value is divided by the measured natural height at apply time.
 */
UCLASS(meta = (DisplayName = "Arnawa Style Text", Category = "Arnawa|Design System"))
class UMGELMTKIT_API UArnawaStyleText : public UTextBlock
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	TObjectPtr<UArnawaComponentStyle> Style;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	FName Variant;

	/** Used when Style is unset: a role from the token asset's type ramp ("h3", "body"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	TObjectPtr<UArnawaDesignTokens> Tokens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design System", meta = (ExposeOnSpawn = true))
	FName TypeRole;

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
