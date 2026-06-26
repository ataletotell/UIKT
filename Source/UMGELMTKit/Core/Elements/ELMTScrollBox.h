// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "ELMTScrollBox.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTScrollBoxStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FScrollBoxStyle ScrollBoxStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FScrollBarStyle ScrollBarStyle;
};

UCLASS(meta=(DisplayName="ELMT Scroll Box", Category="ELMT|Containers"))
class UMGELMTKIT_API UELMTScrollBox : public UScrollBox
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTScrollBoxStyle> ScrollStyle;

	virtual void SynchronizeProperties() override;
};
