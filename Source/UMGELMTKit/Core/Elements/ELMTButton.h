// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "CommonButtonBase.h"
#include "Animation/CurveSequence.h"
#include "ELMTButton.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTButtonStyle : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TSubclassOf<UCommonButtonStyle> CommonButtonStyle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnELMTButtonHoverEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnELMTButtonHoverAlphaChanged, float, Alpha);

UCLASS(meta=(DisplayName="ELMT Button", Category="ELMT|Interactive"))
class UMGELMTKIT_API UELMTButton : public UButton
{
	GENERATED_BODY()

public:
	/** Stable identifier for automated QA/test tooling to find this widget in the live tree. Not shown to players. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Testing, meta=(ExposeOnSpawn=true))
	FName TestId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta=(ExposeOnSpawn=true))
	TSubclassOf<UELMTButtonStyle> ButtonStyle;

	/** Length of the smooth hover-in/hover-out tween, in seconds. Default matches the reference
	 *  mockup's CSS `transition: opacity .12s` on .gcard/.tip. Separate from the Normal/Hovered
	 *  brush swap on the base UButton, which still snaps instantly - this drives OnHoverAlphaChanged
	 *  for anything that wants a smooth blend instead (a tooltip fade, a card glow). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Animation", meta=(ExposeOnSpawn=true))
	float HoverAnimationDuration = 0.12f;

	UPROPERTY(BlueprintAssignable, Category = "Button|Events")
	FOnELMTButtonHoverEvent OnSimulatedHoverBegin;

	UPROPERTY(BlueprintAssignable, Category = "Button|Events")
	FOnELMTButtonHoverEvent OnSimulatedHoverEnd;

	/** Fires every frame while the hover tween is in flight, with the current blend (0 = unhovered, 1 = hovered). */
	UPROPERTY(BlueprintAssignable, Category = "Button|Animation")
	FOnELMTButtonHoverAlphaChanged OnHoverAlphaChanged;

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SimulateHover(bool bHover);

	UFUNCTION(BlueprintCallable, Category = "Button")
	void ClearSimulatedHover();

	UFUNCTION(BlueprintPure, Category = "Button")
	bool IsHoverSimulated() const { return bIsHoverSimulated; }

	UFUNCTION(BlueprintPure, Category = "Button|Animation")
	float GetHoverAlpha() const { return CurrentHoverAlpha; }

private:
	// AddDynamic (dynamic multicast binding, used in SynchronizeProperties) requires the bound
	// method to carry reflection metadata - a plain private method compiles fine but fails at
	// runtime with "Unable to bind delegate... function might not be marked as a UFUNCTION"
	// (a handled ensure, not a crash - found by actually running this in a live editor, not by
	// inspection). TickHoverCurve doesn't need this: FTickerDelegate::CreateUObject binds via a
	// raw C++ pointer-to-member, no reflection involved.
	UFUNCTION()
	void HandleNativeHoverBegin();

	UFUNCTION()
	void HandleNativeHoverEnd();

	bool TickHoverCurve(float DeltaTime);

	bool bIsHoverSimulated = false;
	FButtonStyle CachedStyleBeforeSimulation;

	bool bTargetHovered = false;
	float CurrentHoverAlpha = 0.0f;

	// Held by pointer so the old sequence is DESTROYED before a new one plays. FCurveSequence::Play
	// assigns TickerHandle unconditionally (CurveSequence.cpp:97) and only ~FCurveSequence removes
	// it (CurveSequence.cpp:43-46) - Pause() does not - so replacing the value, by assignment or by
	// a second Play, orphans the previous ticker. That ticker holds a raw pointer to this very
	// member (AddTicker binds CreateRaw(this, &FCurveSequence::TickPlay)), so an orphan outliving
	// the widget ticks freed memory. Hover on and off inside the 120 ms tween and then tear the
	// widget down - which every screen change on a click does - and that is the crash.
	TUniquePtr<FCurveSequence> HoverCurve;

	// Destroys any sequence in flight, then starts a fresh one. The one safe way to replay.
	void RestartHoverCurve();
};
