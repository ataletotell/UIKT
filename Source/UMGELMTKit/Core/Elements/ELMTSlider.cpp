// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTSlider.h"

void UELMTSlider::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (SliderStyle)
	{
		const UELMTSliderStyle* StyleObj = SliderStyle.GetDefaultObject();
		if (StyleObj)
		{
			SetWidgetStyle(StyleObj->Style);
		}
	}
}
