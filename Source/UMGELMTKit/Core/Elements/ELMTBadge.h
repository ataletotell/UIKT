// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "ELMTBadge.generated.h"

UENUM(BlueprintType)
enum class EELMTBadgeVariant : uint8
{
	Primary = 0,
	Warning = 1,
	Danger  = 2,
	Success = 3,
	Neutral = 4,
};

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTBadgeStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	FSlateBrush BackgroundBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Colors)
	FLinearColor PrimaryColor = FLinearColor(0.1f, 0.4f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Colors)
	FLinearColor WarningColor = FLinearColor(1.f, 0.65f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Colors)
	FLinearColor DangerColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Colors)
	FLinearColor SuccessColor = FLinearColor(0.1f, 0.75f, 0.3f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Colors)
	FLinearColor NeutralColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.f);
};

UCLASS(meta=(DisplayName="ELMT Badge", Category="ELMT|Display"))
class UMGELMTKIT_API UELMTBadge : public UBorder
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTBadgeStyle> BadgeStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	EELMTBadgeVariant Variant = EELMTBadgeVariant::Primary;

	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Badge")
	void SetVariant(EELMTBadgeVariant NewVariant);

private:
	FLinearColor GetVariantColor() const;
};
