// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "ELMTSpinner.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTSpinnerStyle : public UObject
{
	GENERATED_BODY()

public:
	/** Texture to spin. Should be square for best results. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	UTexture2D* SpinnerTexture = nullptr;

	/** Rotation speed in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	float SpinSpeed = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FVector2D Size = FVector2D(32.f, 32.f);
};

UCLASS(meta=(DisplayName="ELMT Spinner", Category="ELMT|Display"))
class UMGELMTKIT_API UELMTSpinner : public UImage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTSpinnerStyle> SpinnerStyle;

	/** Start spinning on construct. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spinner)
	bool bAutoPlay = true;

	virtual void SynchronizeProperties() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Spinner")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "Spinner")
	void Stop();

	UFUNCTION(BlueprintPure, Category = "Spinner")
	bool IsSpinning() const { return bIsSpinning; }

private:
	void TickSpin();

	FTimerHandle SpinTimerHandle;
	bool bIsSpinning = false;
	float CurrentAngle = 0.f;
};
