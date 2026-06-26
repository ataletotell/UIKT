// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTSpinner.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UELMTSpinner::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!SpinnerStyle) return;
	const UELMTSpinnerStyle* StyleObj = SpinnerStyle.GetDefaultObject();
	if (!StyleObj) return;

	if (StyleObj->SpinnerTexture)
	{
		SetBrushFromTexture(StyleObj->SpinnerTexture, true);
	}
	SetColorAndOpacity(StyleObj->Color);
	SetDesiredSizeOverride(StyleObj->Size);

	if (bAutoPlay && !bIsSpinning && GetWorld() && GetWorld()->IsGameWorld())
	{
		Play();
	}
}

void UELMTSpinner::BeginDestroy()
{
	bIsSpinning = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpinTimerHandle);
	}
	Super::BeginDestroy();
}

void UELMTSpinner::Play()
{
	bIsSpinning = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SpinTimerHandle, this, &UELMTSpinner::TickSpin, 0.016f, true);
	}
}

void UELMTSpinner::Stop()
{
	bIsSpinning = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpinTimerHandle);
	}
	CurrentAngle = 0.f;
	SetRenderTransformAngle(0.f);
}

void UELMTSpinner::TickSpin()
{
	if (!bIsSpinning) return;

	float Speed = 180.f;
	if (SpinnerStyle)
	{
		if (const UELMTSpinnerStyle* StyleObj = SpinnerStyle.GetDefaultObject())
		{
			Speed = StyleObj->SpinSpeed;
		}
	}

	float DeltaTime = 0.016f;
	if (UWorld* World = GetWorld())
	{
		DeltaTime = FMath::Max(0.001f, World->GetDeltaSeconds());
	}

	CurrentAngle = FMath::Fmod(CurrentAngle + Speed * DeltaTime, 360.f);
	SetRenderTransformAngle(CurrentAngle);
}
