// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTToast.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UELMTToast::ShowMessage(const FText& Message, float Duration)
{
	FELMTToastMessage Toast;
	Toast.Message  = Message;
	Toast.Duration = Duration;
	Toast.Variant  = EELMTBadgeVariant::Neutral;
	ShowToast(Toast);
}

void UELMTToast::ShowToast(const FELMTToastMessage& Toast)
{
	MessageQueue.Add(Toast);
	if (!bIsShowing)
	{
		ShowNext();
	}
}

void UELMTToast::ClearQueue()
{
	MessageQueue.Empty();
	bIsShowing = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}
	OnHideToast();
}

void UELMTToast::NotifyHideComplete()
{
	bIsShowing = false;
	if (MessageQueue.Num() > 0)
	{
		ShowNext();
	}
}

void UELMTToast::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}
	Super::NativeDestruct();
}

void UELMTToast::ShowNext()
{
	if (MessageQueue.Num() == 0) return;

	FELMTToastMessage Next = MessageQueue[0];
	MessageQueue.RemoveAt(0);
	bIsShowing = true;

	OnShowToast(Next);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&UELMTToast::OnTimerExpired,
			FMath::Max(0.1f, Next.Duration),
			false);
	}
}

void UELMTToast::OnHideToast_Implementation()
{
	NotifyHideComplete();
}

void UELMTToast::OnTimerExpired()
{
	OnHideToast();
}
