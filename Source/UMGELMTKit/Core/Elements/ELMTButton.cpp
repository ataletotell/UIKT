// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTButton.h"

void UELMTButton::SynchronizeProperties()
{
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		ClearFlags(RF_Transactional);
	}

	Super::SynchronizeProperties();

	// OnHovered/OnUnhovered are plain dynamic multicast delegates on UButton, not tied to Slate
	// construction, so binding here is enough - no RebuildWidget override needed. Guard with the
	// delegate's own IsAlreadyBound rather than a tracked bool: a bool can desync from the
	// delegate's real invocation list across a Blueprint recompile (the CDO/instance gets
	// re-synchronized without necessarily clearing a stale flag), which tripped a real
	// "InvocationList[CurFunctionIndex] != InDelegate" ensure when this widget was rendered
	// repeatedly during iteration - found by actually running it, not by inspection.
	if (!OnHovered.IsAlreadyBound(this, &UELMTButton::HandleNativeHoverBegin))
	{
		OnHovered.AddDynamic(this, &UELMTButton::HandleNativeHoverBegin);
	}
	if (!OnUnhovered.IsAlreadyBound(this, &UELMTButton::HandleNativeHoverEnd))
	{
		OnUnhovered.AddDynamic(this, &UELMTButton::HandleNativeHoverEnd);
	}

	if (!ButtonStyle) return;

	const UELMTButtonStyle* StyleObj = ButtonStyle.GetDefaultObject();
	if (!StyleObj || !StyleObj->CommonButtonStyle) return;

	const UCommonButtonStyle* CommonStyle = StyleObj->CommonButtonStyle.GetDefaultObject();
	if (!CommonStyle) return;

	FButtonStyle NewStyle;
	NewStyle.SetNormal(CommonStyle->NormalBase);
	NewStyle.SetHovered(CommonStyle->NormalHovered);
	NewStyle.SetPressed(CommonStyle->NormalPressed);
	NewStyle.SetDisabled(CommonStyle->Disabled);
	SetStyle(NewStyle);
	CachedStyleBeforeSimulation = NewStyle;
}

void UELMTButton::SimulateHover(bool bHover)
{
	if (bHover == bIsHoverSimulated) return;
	bIsHoverSimulated = bHover;

	if (bHover)
	{
		CachedStyleBeforeSimulation = GetStyle();
		FButtonStyle ForcedStyle = CachedStyleBeforeSimulation;
		ForcedStyle.Normal = ForcedStyle.Hovered;
		SetStyle(ForcedStyle);
		OnSimulatedHoverBegin.Broadcast();
	}
	else
	{
		SetStyle(CachedStyleBeforeSimulation);
		OnSimulatedHoverEnd.Broadcast();
	}
}

void UELMTButton::ClearSimulatedHover()
{
	SimulateHover(false);
}

void UELMTButton::ReleaseSlateResources(bool bReleaseChildren)
{
	OnHovered.RemoveDynamic(this, &UELMTButton::HandleNativeHoverBegin);
	OnUnhovered.RemoveDynamic(this, &UELMTButton::HandleNativeHoverEnd);
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UELMTButton::RestartHoverCurve()
{
	// Reset() first: it runs ~FCurveSequence, which is the only thing that removes the ticker the
	// last Play registered. Fresh sequence each time rather than resuming mid-flight on a rapid
	// re-hover - FCurveSequence::PlayReverse only has a widget-owned overload, not a ticker one,
	// so this is the one Play() form that works for a plain UWidget (no owned SWidget to hand it),
	// and a sequence needs at least one AddCurve'd curve before Play does anything. A tiny
	// discontinuity on interrupt, acceptable at a ~120 ms duration.
	HoverCurve.Reset();
	HoverCurve = MakeUnique<FCurveSequence>(0.0f, HoverAnimationDuration, ECurveEaseFunction::QuadOut);
	HoverCurve->Play(FTickerDelegate::CreateUObject(this, &UELMTButton::TickHoverCurve), false, 0.0f);
}

void UELMTButton::HandleNativeHoverBegin()
{
	bTargetHovered = true;
	RestartHoverCurve();
}

void UELMTButton::HandleNativeHoverEnd()
{
	bTargetHovered = false;
	RestartHoverCurve();
}

bool UELMTButton::TickHoverCurve(float DeltaTime)
{
	if (!HoverCurve)
	{
		return false;
	}

	const float Forward = HoverCurve->GetLerp();
	CurrentHoverAlpha = bTargetHovered ? Forward : 1.0f - Forward;
	OnHoverAlphaChanged.Broadcast(CurrentHoverAlpha);
	return HoverCurve->IsPlaying();
}


