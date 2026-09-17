// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "Fonts/SlateFontInfo.h"
#include "ELMTPill.generated.h"

class UHorizontalBox;
class UTextBlock;

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTPillStyle : public UObject
{
	GENERATED_BODY()

public:
	/** In absolute pixels, NOT auto-clamped to the pill's actual rendered height. A radius that
	 *  vastly exceeds the box's own size renders NOTHING at all (no fill, no stroke) for a
	 *  RoundedBox brush under FixedRadius rounding - confirmed by actually rendering it, not
	 *  assumed - so don't reach for a huge value like 999 to "guarantee" a full pill shape; pick
	 *  a radius close to half the pill's real expected height instead. 14 is a reasonable default
	 *  for typical pill sizes (roughly a 24-30px tall pill), not a universal safe maximum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	float CornerRadius = 14.f;

	/** RoundedBox brushes render nothing at all (no fill, no stroke) at outline width 0 - keep
	 *  this above zero even for a pill that shouldn't show a visible ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	float OutlineWidth = 1.5f;

	/** Background fill used when the pill is in outline (not filled) mode - normally the
	 *  surrounding panel/card's own background color, so the pill reads as just a colored ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FLinearColor OutlineModeBackgroundColor = FLinearColor(0.06f, 0.04f, 0.03f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FLinearColor FilledTextColor = FLinearColor(1.f, 0.973f, 0.910f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FLinearColor OutlineTextColor = FLinearColor(0.925f, 0.898f, 0.847f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FMargin ContentPadding = FMargin(10.f, 3.f, 10.f, 3.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FSlateFontInfo LabelFont;
};

/**
 * A rounded pill badge: solid-filled or outline-only, an optional leading icon glyph, a text
 * label - matches the wind-chip "Timur"/"Barat" pills in the reference mockups. Distinct from
 * ELMTBadge (which stays content-agnostic - the author places a child in the Designer) because a
 * pill's icon+label composition is the same every time it's used, so it's built in here once
 * instead of requiring the author to re-assemble it by hand in every Widget Blueprint that wants one.
 */
UCLASS(meta=(DisplayName="ELMT Pill", Category="ELMT|Display"))
class UMGELMTKIT_API UELMTPill : public UBorder
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTPillStyle> PillStyle;

	/** Theme color for this pill - the fill color when Filled, the ring color when not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style, meta=(ExposeOnSpawn=true))
	FLinearColor AccentColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style, meta=(ExposeOnSpawn=true))
	bool bFilled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Content, meta=(ExposeOnSpawn=true))
	FText Label;

	/** Optional glyph shown before the label (e.g. a Unicode icon character). Left empty if the
	 *  project's imported fonts don't carry a usable glyph for it - a missing glyph renders as a
	 *  visible tofu box, not nothing, so an empty default is the safe choice. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Content, meta=(ExposeOnSpawn=true))
	FText IconGlyph;

	/** Overrides the PillStyle's LabelFont when set (i.e. when its FontObject is non-null) - lets
	 *  an instance (or a script constructing pills without a Style Blueprint asset at all) pick
	 *  its own typography without needing a whole style asset just to set a font. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Content, meta=(ExposeOnSpawn=true))
	FSlateFontInfo LabelFontOverride;

	/** Stable identifier for automated QA/test tooling to find this widget in the live tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Testing, meta=(ExposeOnSpawn=true))
	FName TestId;

	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Pill")
	void SetLabel(const FText& NewLabel);

	UFUNCTION(BlueprintCallable, Category = "Pill")
	void SetFilled(bool bNewFilled);

	UFUNCTION(BlueprintCallable, Category = "Pill")
	void SetAccentColor(FLinearColor NewColor);

private:
	void EnsureInternalContent();
	void RefreshVisuals();

	// Transient: built at runtime in SynchronizeProperties, not meant to be part of this widget's
	// own serialized/Designer-editable data (matches the "author never touches this" intent).
	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> InternalContent;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> IconTextBlock;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelTextBlock;
};
