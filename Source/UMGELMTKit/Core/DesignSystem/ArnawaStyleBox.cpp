// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArnawaStyleBox.h"

#include "ArnawaComponentStyle.h"

void UArnawaStyleBox::SetStyleAsset(UArnawaComponentStyle* NewStyle, FName NewVariant)
{
	Style = NewStyle;
	Variant = NewVariant;
	ApplyStyle();
}

void UArnawaStyleBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyStyle();
}

void UArnawaStyleBox::ApplyStyle()
{
	if (!Style)
	{
		return;
	}

	const FArnawaBoxStyle BoxStyle = Style->GetBox(Variant);
	SetBrush(BoxStyle.MakeBrush());
	SetPadding(BoxStyle.Padding);

	// UBorder tints the whole brush by BrushColor on top of the brush's own tint, which would
	// double up the background colour the brush already carries.
	SetBrushColor(FLinearColor::White);
}

#if WITH_EDITOR
const FText UArnawaStyleBox::GetPaletteCategory()
{
	return NSLOCTEXT("Arnawa", "DesignSystemPalette", "Arnawa|Design System");
}
#endif
