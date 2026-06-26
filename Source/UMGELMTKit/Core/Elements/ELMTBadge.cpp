// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTBadge.h"

void UELMTBadge::SynchronizeProperties()
{
	if (BadgeStyle)
	{
		const UELMTBadgeStyle* StyleObj = BadgeStyle.GetDefaultObject();
		if (StyleObj)
		{
			FLinearColor VariantColor = GetVariantColor();
			Background = StyleObj->BackgroundBrush;
			Background.TintColor = FSlateColor(FLinearColor::White);
			BrushColor = VariantColor;
		}
	}
	Super::SynchronizeProperties();
}

void UELMTBadge::SetVariant(EELMTBadgeVariant NewVariant)
{
	Variant = NewVariant;
	SynchronizeProperties();
}

FLinearColor UELMTBadge::GetVariantColor() const
{
	if (!BadgeStyle) return FLinearColor::White;
	const UELMTBadgeStyle* StyleObj = BadgeStyle.GetDefaultObject();
	if (!StyleObj) return FLinearColor::White;

	switch (Variant)
	{
	case EELMTBadgeVariant::Primary: return StyleObj->PrimaryColor;
	case EELMTBadgeVariant::Warning: return StyleObj->WarningColor;
	case EELMTBadgeVariant::Danger:  return StyleObj->DangerColor;
	case EELMTBadgeVariant::Success: return StyleObj->SuccessColor;
	case EELMTBadgeVariant::Neutral: return StyleObj->NeutralColor;
	default:                         return FLinearColor::White;
	}
}
