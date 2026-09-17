// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArnawaComponentStyle.h"

FSlateBrush FArnawaBoxStyle::MakeBrush() const
{
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FSlateColor(BackgroundColor);

	FSlateBrushOutlineSettings Outline;
	// FixedRadius is mandatory: the default (HalfHeightRadius) ignores CornerRadii entirely and
	// always renders a full stadium sized off the box's own height.
	Outline.RoundingType = ESlateBrushRoundingType::FixedRadius;
	Outline.CornerRadii = CornerRadii;
	if (BorderWidth > 0.f && BorderColor.A > 0.f)
	{
		Outline.Color = FSlateColor(BorderColor);
		Outline.Width = BorderWidth;
	}
	else
	{
		// A RoundedBox brush with a zero outline width draws nothing at all - not just no stroke,
		// no fill either - so an unbordered box still needs a hairline outline of its own colour.
		Outline.Color = FSlateColor(BackgroundColor);
		Outline.Width = 0.1f;
	}
	Brush.OutlineSettings = Outline;
	return Brush;
}

FArnawaBoxStyle UArnawaComponentStyle::GetBox(FName Variant) const
{
	if (!Variant.IsNone())
	{
		if (const FArnawaBoxStyle* Found = BoxVariants.Find(Variant))
		{
			return *Found;
		}
	}
	return Box;
}

FArnawaTextStyle UArnawaComponentStyle::GetText(FName Variant) const
{
	if (!Variant.IsNone())
	{
		if (const FArnawaTextStyle* Found = TextVariants.Find(Variant))
		{
			return *Found;
		}
	}
	return Text;
}
