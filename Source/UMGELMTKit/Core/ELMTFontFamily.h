// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "ELMTFontFamily.generated.h"

class UFont;

/**
 * Weight scale matches the standard CSS/OpenType numeric scale (100-900), named the same way so
 * a value read out of CSS (font-weight:400, 600, ...) maps onto a value here without translation.
 */
UENUM(BlueprintType)
enum class EELMTFontWeight : uint8
{
	Thin        = 0,	// 100
	ExtraLight  = 1,	// 200
	Light       = 2,	// 300
	Regular     = 3,	// 400
	Medium      = 4,	// 500
	SemiBold    = 5,	// 600
	Bold        = 6,	// 700
	ExtraBold   = 7,	// 800
	Black       = 8,	// 900
};

/**
 * One type family (e.g. "Fraunces" or "Work Sans") as a set of per-weight UFont assets, upright
 * and italic tracked separately since CSS treats font-weight and font-style as independent axes
 * (not every weight necessarily has a matching italic face authored/imported).
 *
 * Built after a real, repeated bug this session: a one-off script that imported "SemiBold" once
 * and reused that single UFont for every piece of text regardless of what weight the real CSS
 * asked for (choice-row titles rendered as SemiBold when the source CSS said font-weight:400).
 * A family asset makes "give me this weight, italic or not" the actual unit of authoring instead
 * of hand-tracking which UFont variable happens to hold which weight.
 */
UCLASS(BlueprintType, meta=(DisplayName="ELMT Font Family"))
class UMGELMTKIT_API UELMTFontFamily : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fonts)
	TMap<EELMTFontWeight, TObjectPtr<UFont>> UprightFonts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fonts)
	TMap<EELMTFontWeight, TObjectPtr<UFont>> ItalicFonts;

	/** Registers one weight's font, upright or italic. The one write path both the Designer
	 *  (editing UprightFonts/ItalicFonts directly) and script-driven asset construction go
	 *  through, so both stay consistent with each other. */
	UFUNCTION(BlueprintCallable, Category = "Font Family")
	void RegisterWeight(EELMTFontWeight Weight, bool bItalic, UFont* Font);

	/**
	 * Resolves the exact requested (Weight, bItalic) combination if present, otherwise falls
	 * back to the nearest lighter-or-equal weight in the same style, then Regular in the same
	 * style, then the opposite style at the same weight, so a caller never gets a null font just
	 * because e.g. Black-Italic was never imported for this family.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Font Family")
	UFont* GetFont(EELMTFontWeight Weight, bool bItalic) const;

	/** Convenience: GetFont(...) wrapped straight into an FSlateFontInfo at the given size. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Font Family")
	FSlateFontInfo MakeFontInfo(EELMTFontWeight Weight, bool bItalic, float Size) const;
};
