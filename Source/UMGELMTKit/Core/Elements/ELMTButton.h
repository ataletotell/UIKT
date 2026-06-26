// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "CommonButtonBase.h"
#include "ELMTButton.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTButtonStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TSubclassOf<UCommonButtonStyle> CommonButtonStyle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnELMTButtonHoverEvent);

UCLASS(meta=(DisplayName="ELMT Button", Category="ELMT|Interactive"))
class UMGELMTKIT_API UELMTButton : public UButton
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTButtonStyle> ButtonStyle;

	UPROPERTY(BlueprintAssignable, Category = "Button|Events")
	FOnELMTButtonHoverEvent OnSimulatedHoverBegin;

	UPROPERTY(BlueprintAssignable, Category = "Button|Events")
	FOnELMTButtonHoverEvent OnSimulatedHoverEnd;

	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SimulateHover(bool bHover);

	UFUNCTION(BlueprintCallable, Category = "Button")
	void ClearSimulatedHover();

	UFUNCTION(BlueprintPure, Category = "Button")
	bool IsHoverSimulated() const { return bIsHoverSimulated; }

private:
	bool bIsHoverSimulated = false;
	FButtonStyle CachedStyleBeforeSimulation;
};
