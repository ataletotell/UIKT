// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTButton.h"

void UELMTButton::SynchronizeProperties()
{
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		ClearFlags(RF_Transactional);
	}

	Super::SynchronizeProperties();

	if (!ButtonStyle) return;

	const UELMTButtonStyle* StyleObj = ButtonStyle.GetDefaultObject();
	if (!StyleObj || !StyleObj->CommonButtonStyle) return;

	const UCommonButtonStyle* CommonStyle = StyleObj->CommonButtonStyle.GetDefaultObject();
	if (!CommonStyle) return;

	FButtonStyle NewStyle;
	NewStyle.SetNormal(CommonStyle->NormalBase);
	NewStyle.SetHovered(CommonStyle->NormalHovered);
	NewStyle.SetPressed(CommonStyle->NormalPressed);
	NewStyle.SetDisabled(CommonStyle->Disabled);
	SetStyle(NewStyle);
	CachedStyleBeforeSimulation = NewStyle;
}

void UELMTButton::SimulateHover(bool bHover)
{
	if (bHover == bIsHoverSimulated) return;
	bIsHoverSimulated = bHover;

	if (bHover)
	{
		CachedStyleBeforeSimulation = GetStyle();
		FButtonStyle ForcedStyle = CachedStyleBeforeSimulation;
		ForcedStyle.Normal = ForcedStyle.Hovered;
		SetStyle(ForcedStyle);
		OnSimulatedHoverBegin.Broadcast();
	}
	else
	{
		SetStyle(CachedStyleBeforeSimulation);
		OnSimulatedHoverEnd.Broadcast();
	}
}

void UELMTButton::ClearSimulatedHover()
{
	SimulateHover(false);
}


