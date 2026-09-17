// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/EditableTextBox.h"
#include "CommonButtonBase.h"
#include "ELMTTextBox.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTTextBoxStyle : public UObject {
	GENERATED_BODY()

public:
	/**
	 * Set the combo button style to match the CommonButtonStyle
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TSubclassOf<UCommonButtonStyle> CommonButtonStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FEditableTextBoxStyle Style;
};

UCLASS()
class UMGELMTKIT_API UELMTTextBox : public UEditableTextBox {
	GENERATED_BODY()

public:
	/** Stable identifier for automated QA/test tooling to find this widget in the live tree. Not shown to players. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Testing, meta = (ExposeOnSpawn = true))
	FName TestId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTTextBoxStyle> TextBoxStyle;

	virtual void SynchronizeProperties() override;

};
