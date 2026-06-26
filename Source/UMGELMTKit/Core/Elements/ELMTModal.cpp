// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTModal.h"
#include "ELMTFocusPanel.h"

void UELMTModal::NativeConstruct()
{
	Super::NativeConstruct();

	if (UELMTFocusPanel* FocusPanel = GetModalFocusPanel())
	{
		FocusPanel->FocusFirstChild();
	}
}

void UELMTModal::Confirm()
{
	OnModalConfirmed();
	OnConfirmed.Broadcast();
	if (GetParent() != nullptr)
	{
		RemoveFromParent();
	}
}

void UELMTModal::Cancel()
{
	OnModalCancelled();
	OnCancelled.Broadcast();
	if (GetParent() != nullptr)
	{
		RemoveFromParent();
	}
}
