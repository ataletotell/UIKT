// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"

#include "ArnawaDesignTokens.generated.h"

/**
 * One entry of the type ramp: a named text role from the design system (a heading, body prose,
 * a small caps label), already resolved to the values Slate needs.
 *
 * Size is a Slate point size, NOT the CSS px number: FSlateFontInfo.Size is a point size at
 * Slate's fixed 96 DPI (FontCacheFreeType.cpp's ComputeFontPixelSize does Size * 96 / 72), so a
 * CSS px font-size is written here as px * 0.75. LetterSpacing likewise is Slate's 1/1000-of-size
 * tracking unit, not CSS px. The generator that writes these assets does both conversions.
 */
USTRUCT(BlueprintType)
struct FArnawaTypeRole
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type)
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type)
	FLinearColor Color = FLinearColor::White;

	/** CSS line-height in px. 0 = leave Slate's natural line height alone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type)
	float LineHeight = 0.f;
};

/**
 * The design system's shared vocabulary: the colours and text roles that every component style
 * refers to, generated from the kitchen sink's `:root` custom properties and type rules
 * (docs/technical/screens/00-kitchen-sink.html).
 *
 * This asset is the propagation point. Component styles and widgets hold a reference to it
 * rather than copies of its values, so re-running the extractor after editing the kitchen sink
 * updates every screen that uses the system without rebuilding any screen.
 */
UCLASS(BlueprintType)
class UMGELMTKIT_API UArnawaDesignTokens : public UDataAsset
{
	GENERATED_BODY()

public:
	/** CSS custom property name without the leading dashes ("gold", "ink-dim") -> colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tokens)
	TMap<FName, FLinearColor> Colors;

	/** Named text role ("h3", "body", "small-caps") -> resolved font/colour/line height. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tokens)
	TMap<FName, FArnawaTypeRole> TypeRoles;

	UFUNCTION(BlueprintPure, Category = Tokens)
	FLinearColor GetColor(FName TokenName, FLinearColor Fallback) const;

	UFUNCTION(BlueprintPure, Category = Tokens)
	bool GetTypeRole(FName RoleName, FArnawaTypeRole& OutRole) const;
};
