// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "ELMTIconButton.generated.h"

UENUM(BlueprintType)
enum class EIconAlignment : uint8
{
	Center = 0,
	Left = 1,
	Right = 2,
	Top = 3,
	Bottom = 4,
	TopLeft = 5,
	TopRight = 6,
	BottomLeft = 7,
	BottomRight = 8
};

UENUM(BlueprintType)
enum class EELMTButtonFeedbackType : uint8
{
	None = 0,
	Bounce = 1,
	Flash = 2,
	Offset = 3,
	RotateShake = 4
};

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTIconButtonStyle : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FButtonStyle ButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Icon)
	FVector2D IconSize = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Icon)
	EIconAlignment IconAlignment = EIconAlignment::Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	EELMTButtonFeedbackType FeedbackType = EELMTButtonFeedbackType::Bounce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	float FeedbackDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	float FeedbackScaleTarget = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FVector2D FeedbackOffsetTarget = FVector2D(0.0f, 4.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FLinearColor FeedbackColorTarget = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	float FeedbackRotationTarget = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	bool bAnimateWholeButton = false;
};

UCLASS()
class UMGELMTKIT_API UELMTIconButton : public UButton {
	GENERATED_BODY()

public:
	UELMTIconButton(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Icon, meta = (ExposeOnSpawn = true))
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTIconButtonStyle> IconButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Icon)
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Icon)
	FText ButtonTooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	bool bOverrideFeedbackSettings = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	EELMTButtonFeedbackType FeedbackType = EELMTButtonFeedbackType::Bounce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	float FeedbackDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	float FeedbackScaleTarget = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	FVector2D FeedbackOffsetTarget = FVector2D(0.0f, 4.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	FLinearColor FeedbackColorTarget = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	float FeedbackRotationTarget = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback", meta = (EditCondition = "bOverrideFeedbackSettings"))
	bool bAnimateWholeButton = false;

	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = Icon)
	void SetIcon(UTexture2D* NewIcon);

	UFUNCTION(BlueprintCallable, Category = Icon)
	void SetIconColor(FLinearColor NewColor);

	UFUNCTION(BlueprintCallable, Category = Icon)
	void SetIconSize(FVector2D NewSize);

	UFUNCTION(BlueprintCallable, Category = Icon)
	FVector2D GetIconSize() const;

	UFUNCTION(BlueprintCallable, Category = Icon)
	void SetIconWidth(float Width);

	UFUNCTION(BlueprintCallable, Category = Icon)
	void SetIconHeight(float Height);

private:
	UPROPERTY()
	UImage* IconImage;

	void UpdateIcon();
	void SetupIconImage();

	UFUNCTION()
	void HandleOnPressed();

	UFUNCTION()
	void HandleOnReleased();

	void TickFeedbackAnimation();

	FTimerHandle FeedbackTimerHandle;

	bool bIsPressedState = false;

	float CurrentFeedbackScale = 1.0f;
	FVector2D CurrentFeedbackOffset = FVector2D::ZeroVector;
	float CurrentFeedbackRotation = 0.0f;
	FLinearColor CurrentFeedbackColor = FLinearColor::White;

	// Helper getters
	EELMTButtonFeedbackType GetActiveFeedbackType() const;
	float GetActiveFeedbackDuration() const;
	float GetActiveFeedbackScaleTarget() const;
	FVector2D GetActiveFeedbackOffsetTarget() const;
	FLinearColor GetActiveFeedbackColorTarget() const;
	float GetActiveFeedbackRotationTarget() const;
	bool GetActiveAnimateWholeButton() const;
};
