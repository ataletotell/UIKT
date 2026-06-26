// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ELMTModal.generated.h"

class UELMTFocusPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnELMTModalConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnELMTModalCancelled);

/**
 * Abstract base for modal dialogs (pause confirms, prompts, etc).
 * Subclass in Blueprint: add your visual layout, call Confirm()/Cancel() from buttons.
 * Pairs with ELMTFocusPanel to trap focus within the modal.
 */
UCLASS(Abstract, Blueprintable, meta=(DisplayName="ELMT Modal", Category="ELMT|Composites"))
class UMGELMTKIT_API UELMTModal : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Modal|Events")
	FOnELMTModalConfirmed OnConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Modal|Events")
	FOnELMTModalCancelled OnCancelled;

	/** Call from Confirm button OnClicked. Broadcasts OnConfirmed then removes from parent. */
	UFUNCTION(BlueprintCallable, Category = "Modal")
	void Confirm();

	/** Call from Cancel button OnClicked or back action. Broadcasts OnCancelled then removes from parent. */
	UFUNCTION(BlueprintCallable, Category = "Modal")
	void Cancel();

	/**
	 * Override in Blueprint to return the ELMTFocusPanel that wraps the modal content.
	 * When provided, focus is moved into it on construct.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Modal")
	UELMTFocusPanel* GetModalFocusPanel() const;
	virtual UELMTFocusPanel* GetModalFocusPanel_Implementation() const { return nullptr; }

	/** Called after Confirm() before RemoveFromParent — override in BP for close animation. */
	UFUNCTION(BlueprintNativeEvent, Category = "Modal")
	void OnModalConfirmed();
	virtual void OnModalConfirmed_Implementation() {}

	/** Called after Cancel() before RemoveFromParent — override in BP for close animation. */
	UFUNCTION(BlueprintNativeEvent, Category = "Modal")
	void OnModalCancelled();
	virtual void OnModalCancelled_Implementation() {}

protected:
	virtual void NativeConstruct() override;
};
