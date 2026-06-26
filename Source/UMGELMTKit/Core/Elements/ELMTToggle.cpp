// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTToggle.h"

void UELMTToggle::SynchronizeProperties()
{
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		ClearFlags(RF_Transactional);
	}

	if (ToggleStyle)
	{
		const UELMTToggleStyle* StyleObj = ToggleStyle.GetDefaultObject();
		if (StyleObj)
		{
			SetWidgetStyle(StyleObj->CheckBoxStyle);
		}
	}

	Super::SynchronizeProperties();
}
