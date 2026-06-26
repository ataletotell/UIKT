// Fill out your copyright notice in the Description page of Project Settings.


#include "ELMTCombo.h"
#include "CommonTextBlock.h"

void UELMTCombo::SynchronizeProperties() {
	// Strip RF_Transactional in game worlds to prevent undo TransBuffer from
	// retaining PIE widget references and causing assert on PIE end.
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		ClearFlags(RF_Transactional);
	}

	Super::SynchronizeProperties();

	if (!ComboBoxStyle)
		return;

	TObjectPtr<UELMTComboStyle> StyleInstance = ComboBoxStyle.GetDefaultObject();
	FComboBoxStyle EditedStyle = StyleInstance->Style;

	if (StyleInstance->CommonButtonStyle) {
		TObjectPtr<UCommonButtonStyle> ButtonInstance = StyleInstance->CommonButtonStyle.GetDefaultObject();

		EditedStyle.ComboButtonStyle.ButtonStyle.SetNormal(ButtonInstance->NormalBase);
		EditedStyle.ComboButtonStyle.ButtonStyle.SetPressed(ButtonInstance->NormalPressed);
		EditedStyle.ComboButtonStyle.ButtonStyle.SetHovered(ButtonInstance->NormalHovered);
		EditedStyle.ComboButtonStyle.ButtonStyle.SetDisabled(ButtonInstance->Disabled);


	}

	SetWidgetStyle(EditedStyle);
	SetItemStyle(StyleInstance->ItemStyle);

	CachedComboStyle = EditedStyle;
}

void UELMTCombo::SimulateHover(bool bHover)
{
	if (bHover == bIsHoverSimulated)
	{
		return;
	}

	bIsHoverSimulated = bHover;

	if (bHover)
	{
		FComboBoxStyle ForcedStyle = CachedComboStyle;
		ForcedStyle.ComboButtonStyle.ButtonStyle.Normal = ForcedStyle.ComboButtonStyle.ButtonStyle.Hovered;
		SetWidgetStyle(ForcedStyle);
		OnSimulatedHoverBegin.Broadcast();
	}
	else
	{
		SetWidgetStyle(CachedComboStyle);
		OnSimulatedHoverEnd.Broadcast();
	}
}

void UELMTCombo::ToggleSimulatedHover()
{
	SimulateHover(!bIsHoverSimulated);
}

void UELMTCombo::ClearSimulatedHover()
{
	SimulateHover(false);
}



