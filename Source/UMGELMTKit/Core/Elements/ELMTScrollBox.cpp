// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTScrollBox.h"

void UELMTScrollBox::SynchronizeProperties()
{
	if (ScrollStyle)
	{
		const UELMTScrollBoxStyle* StyleObj = ScrollStyle.GetDefaultObject();
		if (StyleObj)
		{
			SetWidgetStyle(StyleObj->ScrollBoxStyle);
		}
	}
	Super::SynchronizeProperties();
}
