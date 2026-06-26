// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ELMTNavigationLibrary.generated.h"

/**
 * Blueprint helpers for gamepad/keyboard navigation rules.
 * Call these in BeginPlay or on widget construct to set up nav boundaries.
 */
UCLASS()
class UMGELMTKIT_API UELMTNavigationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Stop navigation from leaving Widget in Direction (Up/Down/Left/Right). */
	UFUNCTION(BlueprintCallable, Category = "ELMT|Navigation", meta=(DefaultToSelf="Widget"))
	static void SetNavigationStop(UWidget* Widget, EUINavigation Direction);

	/** Wrap navigation at Widget boundary in Direction. */
	UFUNCTION(BlueprintCallable, Category = "ELMT|Navigation", meta=(DefaultToSelf="Widget"))
	static void SetNavigationWrap(UWidget* Widget, EUINavigation Direction);

	/** Escape (default UE behaviour) — let focus leave Widget in Direction. */
	UFUNCTION(BlueprintCallable, Category = "ELMT|Navigation", meta=(DefaultToSelf="Widget"))
	static void SetNavigationEscape(UWidget* Widget, EUINavigation Direction);

	/** Stop focus from leaving Panel in all 4 directions — call this on modal root panels. */
	UFUNCTION(BlueprintCallable, Category = "ELMT|Navigation", meta=(DefaultToSelf="Panel"))
	static void ContainFocusToWidget(UWidget* Panel);

	/** Wrap all 4 directions on Panel — useful for horizontal/vertical lists. */
	UFUNCTION(BlueprintCallable, Category = "ELMT|Navigation", meta=(DefaultToSelf="Panel"))
	static void WrapNavigationOnWidget(UWidget* Panel);
};
