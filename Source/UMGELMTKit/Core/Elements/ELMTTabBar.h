// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HorizontalBox.h"
#include "ELMTTabBar.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnELMTTabChanged, int32, NewTabIndex);

/**
 * Manages tab selection across child tab buttons.
 * Add child ELMTButton widgets in Blueprint; call SelectTab/SelectNext/SelectPrevious
 * from input actions or shoulder button bindings for gamepad support.
 */
UCLASS(meta=(DisplayName="ELMT Tab Bar", Category="ELMT|Navigation"))
class UMGELMTKIT_API UELMTTabBar : public UHorizontalBox
{
	GENERATED_BODY()

public:
	/** Fires when active tab changes. NewTabIndex is 0-based. */
	UPROPERTY(BlueprintAssignable, Category = "TabBar|Events")
	FOnELMTTabChanged OnTabChanged;

	/** Currently active tab index (0-based). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TabBar")
	int32 ActiveTabIndex = 0;

	/** If true, SelectNext wraps from last to first tab and vice-versa. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TabBar")
	bool bWrapNavigation = true;

	virtual void SynchronizeProperties() override;

	/** Select a tab by 0-based index. Broadcasts OnTabChanged. */
	UFUNCTION(BlueprintCallable, Category = "TabBar")
	void SelectTab(int32 Index);

	/** Select next tab. Bind to gamepad right shoulder for standard tab navigation. */
	UFUNCTION(BlueprintCallable, Category = "TabBar")
	void SelectNext();

	/** Select previous tab. Bind to gamepad left shoulder for standard tab navigation. */
	UFUNCTION(BlueprintCallable, Category = "TabBar")
	void SelectPrevious();

	UFUNCTION(BlueprintPure, Category = "TabBar")
	int32 GetActiveTabIndex() const { return ActiveTabIndex; }

	UFUNCTION(BlueprintPure, Category = "TabBar")
	int32 GetTabCount() const;
};
