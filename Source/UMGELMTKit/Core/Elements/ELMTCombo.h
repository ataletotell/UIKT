// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "CommonButtonBase.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "Input/Events.h"
#include "ELMTCombo.generated.h"


UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTComboStyle : public UObject {
	GENERATED_BODY()

public:
	/**
	 * Set the combo button style to match the CommonButtonStyle
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TSubclassOf<UCommonButtonStyle> CommonButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FComboBoxStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FTableRowStyle ItemStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style)
	FScrollBarStyle ScrollBarStyle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnELMTComboSimulatedHoverEvent);

UCLASS()
class UMGELMTKIT_API UELMTCombo : public UComboBoxString {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ELMT_Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTComboStyle> ComboBoxStyle;

	// --- Simulated Hover Events ---

	UPROPERTY(BlueprintAssignable, Category = "ComboBox|Events")
	FOnELMTComboSimulatedHoverEvent OnSimulatedHoverBegin;

	UPROPERTY(BlueprintAssignable, Category = "ComboBox|Events")
	FOnELMTComboSimulatedHoverEvent OnSimulatedHoverEnd;

	// --- Overrides ---

	virtual void SynchronizeProperties() override;

	// --- Simulated Hover ---

	/** Force the combo button into or out of hovered visual state regardless of cursor position. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SimulateHover(bool bHover);

	/** Toggle simulated hover on/off. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void ToggleSimulatedHover();

	/** True while simulated hover is active. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	bool IsHoverSimulated() const { return bIsHoverSimulated; }

	/** Remove simulated hover (same as SimulateHover(false)). */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void ClearSimulatedHover();

private:
	bool bIsHoverSimulated = false;
	FComboBoxStyle CachedComboStyle;
};
