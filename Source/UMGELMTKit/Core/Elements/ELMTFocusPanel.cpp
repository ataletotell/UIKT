// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTFocusPanel.h"
#include "Widgets/Layout/SBox.h"

TSharedRef<SWidget> UELMTFocusPanel::RebuildWidget()
{
	return Super::RebuildWidget();
}

void UELMTFocusPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyNavigationRules();
}

UWidget* UELMTFocusPanel::GetDesiredFocusTarget() const
{
	if (UWidget* Content = GetContent())
	{
		return Content;
	}
	return const_cast<UELMTFocusPanel*>(this);
}

void UELMTFocusPanel::FocusFirstChild()
{
	if (UWidget* Target = GetDesiredFocusTarget())
	{
		Target->SetFocus();
	}
}

void UELMTFocusPanel::ApplyNavigationRules()
{
	if (!bContainFocus) return;
	SetNavigationRuleBase(EUINavigation::Up,    EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Down,  EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Left,  EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
}
