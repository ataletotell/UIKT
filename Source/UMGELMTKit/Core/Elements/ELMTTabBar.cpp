// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTTabBar.h"
#include "Components/HorizontalBoxSlot.h"

void UELMTTabBar::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	// Ensure active index is clamped to valid range on property sync
	int32 Count = GetTabCount();
	if (Count > 0)
	{
		int32 Clamped = FMath::Clamp(ActiveTabIndex, 0, Count - 1);
		if (Clamped != ActiveTabIndex)
		{
			ActiveTabIndex = Clamped;
			OnTabChanged.Broadcast(ActiveTabIndex);
		}
	}
}

void UELMTTabBar::SelectTab(int32 Index)
{
	int32 Count = GetTabCount();
	if (Count == 0) return;

	int32 ClampedIndex = FMath::Clamp(Index, 0, Count - 1);
	if (ClampedIndex == ActiveTabIndex) return;

	ActiveTabIndex = ClampedIndex;
	OnTabChanged.Broadcast(ActiveTabIndex);
}

void UELMTTabBar::SelectNext()
{
	int32 Count = GetTabCount();
	if (Count == 0) return;

	int32 Next = ActiveTabIndex + 1;
	if (bWrapNavigation)
	{
		Next = Next % Count;
	}
	else
	{
		Next = FMath::Min(Next, Count - 1);
	}
	SelectTab(Next);
}

void UELMTTabBar::SelectPrevious()
{
	int32 Count = GetTabCount();
	if (Count == 0) return;

	int32 Prev = ActiveTabIndex - 1;
	if (bWrapNavigation)
	{
		Prev = (Prev + Count) % Count;
	}
	else
	{
		Prev = FMath::Max(Prev, 0);
	}
	SelectTab(Prev);
}

int32 UELMTTabBar::GetTabCount() const
{
	return GetChildrenCount();
}
