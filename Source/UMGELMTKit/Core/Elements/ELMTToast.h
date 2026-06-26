// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ELMTBadge.h"
#include "ELMTToast.generated.h"

USTRUCT(BlueprintType)
struct FELMTToastMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EELMTBadgeVariant Variant = EELMTBadgeVariant::Neutral;
};

/**
 * Non-blocking notification widget with an internal queue.
 * Subclass in Blueprint to define the visual layout.
 * Call ShowMessage() or ShowToast() to enqueue notifications.
 * Override OnShowToast / OnHideToast in Blueprint to drive your show/hide animations.
 */
UCLASS(Abstract, Blueprintable, meta=(DisplayName="ELMT Toast", Category="ELMT|Composites"))
class UMGELMTKIT_API UELMTToast : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Enqueue a plain text message with default duration and neutral variant. */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowMessage(const FText& Message, float Duration = 3.f);

	/** Enqueue a fully configured toast. */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowToast(const FELMTToastMessage& Toast);

	/** Clear all queued messages and hide current toast immediately. */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ClearQueue();

	/** Override in Blueprint — play your show animation, update text/color from Toast data. */
	UFUNCTION(BlueprintNativeEvent, Category = "Toast")
	void OnShowToast(const FELMTToastMessage& Toast);
	virtual void OnShowToast_Implementation(const FELMTToastMessage& Toast) {}

	/** Override in Blueprint — play your hide animation. Call NotifyHideComplete() when done. */
	UFUNCTION(BlueprintNativeEvent, Category = "Toast")
	void OnHideToast();
	virtual void OnHideToast_Implementation();

	/** Call this from Blueprint after your hide animation finishes to advance the queue. */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void NotifyHideComplete();

protected:
	virtual void NativeDestruct() override;

private:
	TArray<FELMTToastMessage> MessageQueue;
	bool bIsShowing = false;
	FTimerHandle HideTimerHandle;

	void ShowNext();
	void OnTimerExpired();
};
