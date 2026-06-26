// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "ELMTFocusPanel.generated.h"

/**
 * Content widget that traps gamepad/keyboard focus within its boundary.
 * Use as root of modals and popups to prevent focus escaping to widgets behind.
 */
UCLASS(meta=(DisplayName="ELMT Focus Panel", Category="ELMT|Navigation"))
class UMGELMTKIT_API UELMTFocusPanel : public UContentWidget
{
	GENERATED_BODY()

public:
	/** Stop focus from leaving this panel on all 4 navigation directions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bContainFocus = true;

	/** On NativeConstruct, move focus to first focusable child automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bAutoFocusFirstChild = true;

	virtual void SynchronizeProperties() override;
	virtual UWidget* GetDesiredFocusTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void FocusFirstChild();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void ApplyNavigationRules();
};
