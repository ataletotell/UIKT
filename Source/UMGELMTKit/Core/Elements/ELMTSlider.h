// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "ELMTSlider.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTSliderStyle : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSliderStyle Style;

	/** Handle tint applied when slider has gamepad/keyboard focus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Focus)
	FLinearColor FocusTintColor = FLinearColor(1.f, 0.85f, 0.f, 1.f);
};

UCLASS(meta=(DisplayName="ELMT Slider", Category="ELMT|Interactive"))
class UMGELMTKIT_API UELMTSlider : public USlider {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTSliderStyle> SliderStyle;

	virtual void SynchronizeProperties() override;

};
