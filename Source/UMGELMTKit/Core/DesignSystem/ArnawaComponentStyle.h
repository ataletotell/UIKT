// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "Styling/SlateBrush.h"

#include "ArnawaComponentStyle.generated.h"

class UArnawaDesignTokens;

/** The box half of a component: what a CSS rule says about background, border and padding. */
USTRUCT(BlueprintType)
struct FArnawaBoxStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	FLinearColor BackgroundColor = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	FLinearColor BorderColor = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	float BorderWidth = 0.f;

	/** X = top left, Y = top right, Z = bottom right, W = bottom left, matching FSlateBrushOutlineSettings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	FVector4 CornerRadii = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	FMargin Padding = FMargin(0.f);

	/** 0 = size to content (the usual case); > 0 pins the box, for dots/pips/rings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box)
	FVector2D FixedSize = FVector2D::ZeroVector;

	/** Builds the RoundedBox brush this style describes. */
	FSlateBrush MakeBrush() const;
};

/** The text half of a component. Font.Size is a Slate point size (see FArnawaTypeRole). */
USTRUCT(BlueprintType)
struct FArnawaTextStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Text)
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Text)
	FLinearColor Color = FLinearColor::White;

	/** CSS line-height in px; 0 leaves Slate's natural line height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Text)
	float LineHeight = 0.f;

	/** CSS text-transform:uppercase, applied by the widget so the design system owns it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Text)
	bool bUppercase = false;
};

/**
 * One reusable component of the design system - a `.btn`, a `.windchip`, a `.pip` - generated
 * from that CSS class in the kitchen sink, with its modifier classes captured as named variants
 * (`.btn.alt` -> "alt", `.pip.on` -> "on").
 *
 * Widgets reference this asset instead of carrying their own copies of the numbers, so editing
 * the kitchen sink and re-running the extractor restyles every screen at once. Variants are a
 * TMap rather than separate assets so a component and all of its states stay one reviewable unit.
 */
UCLASS(BlueprintType)
class UMGELMTKIT_API UArnawaComponentStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The CSS class this was generated from, kept for traceability back to the mockup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Source)
	FString SourceSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Source)
	TObjectPtr<UArnawaDesignTokens> Tokens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FArnawaBoxStyle Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FArnawaTextStyle Text;

	/** Modifier class name -> the box style with that modifier applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TMap<FName, FArnawaBoxStyle> BoxVariants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TMap<FName, FArnawaTextStyle> TextVariants;

	/** Variant's box style if that modifier exists, otherwise the base one. */
	UFUNCTION(BlueprintPure, Category = Style)
	FArnawaBoxStyle GetBox(FName Variant) const;

	UFUNCTION(BlueprintPure, Category = Style)
	FArnawaTextStyle GetText(FName Variant) const;
};
