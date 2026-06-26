// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTNavigationLibrary.h"
#include "Components/Widget.h"

void UELMTNavigationLibrary::SetNavigationStop(UWidget* Widget, EUINavigation Direction)
{
	if (Widget)
	{
		Widget->SetNavigationRuleBase(Direction, EUINavigationRule::Stop);
	}
}

void UELMTNavigationLibrary::SetNavigationWrap(UWidget* Widget, EUINavigation Direction)
{
	if (Widget)
	{
		Widget->SetNavigationRuleBase(Direction, EUINavigationRule::Wrap);
	}
}

void UELMTNavigationLibrary::SetNavigationEscape(UWidget* Widget, EUINavigation Direction)
{
	if (Widget)
	{
		Widget->SetNavigationRuleBase(Direction, EUINavigationRule::Escape);
	}
}

void UELMTNavigationLibrary::ContainFocusToWidget(UWidget* Panel)
{
	if (!Panel) return;
	Panel->SetNavigationRuleBase(EUINavigation::Up,    EUINavigationRule::Stop);
	Panel->SetNavigationRuleBase(EUINavigation::Down,  EUINavigationRule::Stop);
	Panel->SetNavigationRuleBase(EUINavigation::Left,  EUINavigationRule::Stop);
	Panel->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
}

void UELMTNavigationLibrary::WrapNavigationOnWidget(UWidget* Panel)
{
	if (!Panel) return;
	Panel->SetNavigationRuleBase(EUINavigation::Up,    EUINavigationRule::Wrap);
	Panel->SetNavigationRuleBase(EUINavigation::Down,  EUINavigationRule::Wrap);
	Panel->SetNavigationRuleBase(EUINavigation::Left,  EUINavigationRule::Wrap);
	Panel->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Wrap);
}
