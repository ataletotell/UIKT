// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTPill.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Styling/SlateBrush.h"

namespace
{
	// RoundedBox brushes need RoundingType explicitly set to FixedRadius - the default
	// (HalfHeightRadius) ignores CornerRadii entirely and always renders a full pill/stadium
	// shape sized off the box's own height, ignoring the requested radius; confirmed by actually
	// rendering it, not assumed. A zero outline width also renders nothing at all for a
	// RoundedBox brush (no fill, no stroke), also confirmed by rendering it.
	FSlateBrush MakePillBrush(const FLinearColor& FillColor, const FLinearColor& OutlineColor, float CornerRadius, float OutlineWidth)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(FillColor);

		FSlateBrushOutlineSettings Outline;
		Outline.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Outline.CornerRadii = FVector4(CornerRadius, CornerRadius, CornerRadius, CornerRadius);
		Outline.Color = FSlateColor(OutlineColor);
		Outline.Width = FMath::Max(OutlineWidth, 0.1f);
		Brush.OutlineSettings = Outline;

		return Brush;
	}
}

void UELMTPill::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	EnsureInternalContent();
	RefreshVisuals();
}

void UELMTPill::EnsureInternalContent()
{
	if (InternalContent)
	{
		return;
	}

	InternalContent = NewObject<UHorizontalBox>(this, NAME_None, RF_Transient);
	IconTextBlock = NewObject<UTextBlock>(this, NAME_None, RF_Transient);
	LabelTextBlock = NewObject<UTextBlock>(this, NAME_None, RF_Transient);

	if (UHorizontalBoxSlot* IconSlot = InternalContent->AddChildToHorizontalBox(IconTextBlock))
	{
		IconSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));
	}
	InternalContent->AddChildToHorizontalBox(LabelTextBlock);

	SetContent(InternalContent);
}

void UELMTPill::RefreshVisuals()
{
	const UELMTPillStyle* Style = PillStyle ? PillStyle.GetDefaultObject() : nullptr;
	const float CornerRadius = Style ? Style->CornerRadius : 14.f;
	const float OutlineWidth = Style ? Style->OutlineWidth : 1.5f;
	const FLinearColor OutlineBg = Style ? Style->OutlineModeBackgroundColor : FLinearColor(0.06f, 0.04f, 0.03f, 1.f);
	const FLinearColor FilledText = Style ? Style->FilledTextColor : FLinearColor::White;
	const FLinearColor OutlineText = Style ? Style->OutlineTextColor : FLinearColor(0.9f, 0.9f, 0.9f, 1.f);
	const FMargin ContentPaddingValue = Style ? Style->ContentPadding : FMargin(10.f, 3.f, 10.f, 3.f);

	const FLinearColor FillColor = bFilled ? AccentColor : OutlineBg;
	SetBrush(MakePillBrush(FillColor, AccentColor, CornerRadius, OutlineWidth));
	SetPadding(ContentPaddingValue);

	const FLinearColor TextColor = bFilled ? FilledText : OutlineText;
	const bool bHasFontOverride = LabelFontOverride.FontObject != nullptr;
	const FSlateFontInfo EffectiveFont = bHasFontOverride ? LabelFontOverride : (Style ? Style->LabelFont : FSlateFontInfo());

	if (IconTextBlock)
	{
		IconTextBlock->SetText(IconGlyph);
		IconTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
		IconTextBlock->SetVisibility(IconGlyph.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
		IconTextBlock->SetFont(EffectiveFont);
	}
	if (LabelTextBlock)
	{
		LabelTextBlock->SetText(Label);
		LabelTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
		LabelTextBlock->SetFont(EffectiveFont);
	}
}

void UELMTPill::SetLabel(const FText& NewLabel)
{
	Label = NewLabel;
	if (LabelTextBlock)
	{
		LabelTextBlock->SetText(Label);
	}
}

void UELMTPill::SetFilled(bool bNewFilled)
{
	bFilled = bNewFilled;
	RefreshVisuals();
}

void UELMTPill::SetAccentColor(FLinearColor NewColor)
{
	AccentColor = NewColor;
	RefreshVisuals();
}
