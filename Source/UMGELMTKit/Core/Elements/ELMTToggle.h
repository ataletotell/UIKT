// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CheckBox.h"
#include "ELMTToggle.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTToggleStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FCheckBoxStyle CheckBoxStyle;
};

UCLASS(meta=(DisplayName="ELMT Toggle", Category="ELMT|Interactive"))
class UMGELMTKIT_API UELMTToggle : public UCheckBox
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTToggleStyle> ToggleStyle;

	virtual void SynchronizeProperties() override;

};
