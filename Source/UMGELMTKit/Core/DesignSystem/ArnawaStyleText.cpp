// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArnawaStyleText.h"

#include "ArnawaComponentStyle.h"
#include "ArnawaDesignTokens.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"

void UArnawaStyleText::SetStyleAsset(UArnawaComponentStyle* NewStyle, FName NewVariant)
{
	Style = NewStyle;
	Variant = NewVariant;
	ApplyStyle();
}

void UArnawaStyleText::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyStyle();
}

void UArnawaStyleText::ApplyStyle()
{
	FSlateFontInfo FontInfo;
	FLinearColor Color = FLinearColor::White;
	float LineHeightPx = 0.f;
	bool bUppercase = false;

	if (Style)
	{
		const FArnawaTextStyle TextStyle = Style->GetText(Variant);
		FontInfo = TextStyle.Font;
		Color = TextStyle.Color;
		LineHeightPx = TextStyle.LineHeight;
		bUppercase = TextStyle.bUppercase;
	}
	else if (Tokens && !TypeRole.IsNone())
	{
		FArnawaTypeRole Role;
		if (!Tokens->GetTypeRole(TypeRole, Role))
		{
			return;
		}
		FontInfo = Role.Font;
		Color = Role.Color;
		LineHeightPx = Role.LineHeight;
	}
	else
	{
		return;
	}

	if (FontInfo.HasValidFont())
	{
		SetFont(FontInfo);
	}
	SetColorAndOpacity(FSlateColor(Color));
	SetTextTransformPolicy(bUppercase ? ETextTransformPolicy::ToUpper : ETextTransformPolicy::None);

	// CSS line-height is a multiple of the FONT SIZE; Slate's LineHeightPercentage multiplies the
	// font's own max character height instead (TextLayout.cpp: LineSize.Y = UnscaleLineHeight *
	// LineHeightPercentage), which for a text font is already ~1.2-1.4x the size. Converting
	// through the measured natural height is what makes the two mean the same thing.
	if (LineHeightPx > 0.f && FontInfo.HasValidFont() && FSlateApplication::IsInitialized())
	{
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const float NaturalHeight = static_cast<float>(FontMeasure->GetMaxCharacterHeight(FontInfo, 1.0f));
		if (NaturalHeight > 0.f)
		{
			SetLineHeightPercentage(LineHeightPx / NaturalHeight);
		}
	}
}

#if WITH_EDITOR
const FText UArnawaStyleText::GetPaletteCategory()
{
	return NSLOCTEXT("Arnawa", "DesignSystemPalette", "Arnawa|Design System");
}
#endif
