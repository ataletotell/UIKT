// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ELMTHoverCard.h"
#include "Engine/DataTable.h"
#include "ELMTHoverCardLayer.generated.h"

class UCanvasPanel;

/**
 * What a rect lookup measured when it failed to find its widget.
 *
 * A miss on its own says nothing about the cause, and the causes look identical in a log: a screen
 * that has not been laid out, a screen laid out at a different aspect, and a rect that genuinely
 * belongs to no widget all just print "no widget at rect". This carries the numbers that tell them
 * apart, including the nearest widget read BOTH ways, scaled into design space and taken as-is,
 * because which of those two readings is correct depends on whether the screen stretches.
 */
struct FELMTHoverCardMiss
{
	FVector2D ScreenSize = FVector2D::ZeroVector;
	UWidget* Nearest = nullptr;
	float Error = 0.f;
	FBox2D NearestDesignRect = FBox2D(ForceInit);
	FBox2D NearestRawRect = FBox2D(ForceInit);
	/** The screen is not 16:9, so design rects do not correspond to anything on it. */
	bool bWrongAspect = false;
};

/**
 * Builds a card at the moment it is hovered.
 *
 * The design system's hovers describe LIVE values - "today's delta", "hours left", "paid and the
 * reaction" - so a card baked at attach time would be stale by the time anyone saw it. Refreshing
 * instead would mean wiring every card to whichever services could change it. A hover is rare and a
 * card is small, so it is cheaper and far simpler to build one on demand, and it is then always
 * current by construction.
 */
DECLARE_DELEGATE_RetVal_TwoParams(FELMTHoverCardData, FELMTHoverCardProvider,
	UWidget* /*Target*/, const FString& /*Key*/);

/**
 * The one place hover cards appear on a screen, and the thing that makes ANY widget hoverable.
 *
 * Attach(Target, Data) works on any UWidget without subclassing it or changing the widget tree:
 * it binds Slate's own mouse-enter/leave on the target's underlying SWidget. So a Border produced
 * by the html-to-umg pipeline, an ELMTPill, a plain Image or a Blueprint-authored panel all become
 * hoverable the same way, with one call and no common base class.
 *
 * One card serves the whole screen. The alternative - a hidden card inside every hoverable widget -
 * is what the mockup does with CSS, and it does not port: it would multiply widgets, and a card
 * nested inside its target is clipped by that target's bounds the moment it is bigger.
 *
 * Placement follows the mockup's `.tip`: sit above the target with a small gap, flip below when
 * there is no room above, and stay inside the layer horizontally. Opacity and a small slide are
 * tweened over HoverFadeDuration, matching `transition: opacity .12s, transform .12s`.
 *
 * Put one of these at the top of a screen, above everything else, and leave it empty.
 */
UCLASS(Blueprintable, meta = (DisplayName = "ELMT Hover Card Layer", Category = "ELMT|Composites"))
class UMGELMTKIT_API UELMTHoverCardLayer : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The card class this layer spawns. Set it to the design-system `.tip` subclass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card", meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTHoverCard> HoverCardClass;

	/** Seconds for the fade in or out. Matches the mockup's `transition: opacity .12s`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	float HoverFadeDuration = 0.12f;

	/** Gap in pixels between the target and the card, the mockup's `bottom:104%`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	float AnchorGap = 8.f;

	/** Roughly how tall a card is, used only to decide whether one would fall off the top of the
	 *  screen and should flip below its target. An estimate is enough: being wrong only picks the
	 *  less good side, never a wrong position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	float EstimatedCardHeight = 120.f;

	/** How far the card slides while fading, the mockup's `translateY`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	float SlideDistance = 6.f;

	/**
	 * Builds this layer's own canvas and card, for a layer created straight from C++.
	 *
	 * Must be called BEFORE the layer is added to a parent, for the same reason
	 * UELMTHoverCard::EnsureLayout must be: a WidgetTree root assigned after Slate has built the
	 * widget is never drawn, and the failure is silent - the card reports visible, its opacity
	 * tweens, and it is arranged at zero by zero forever.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void EnsureLayout();

	/**
	 * Makes Target hoverable: hovering it shows this layer's card filled with Data.
	 *
	 * Safe to call again on the same target to replace the data. Pass an empty Data to show
	 * nothing. The target keeps working exactly as before; only its hover is observed.
	 *
	 * @return false if the target has no Slate widget yet. Call this after the target is
	 *   constructed (NativeConstruct or later), not from a constructor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	bool Attach(UWidget* Target, const FELMTHoverCardData& Data);

	/**
	 * Makes hoverable whichever widget of Screen occupies a given rectangle of the 1920x1080 design
	 * space, without needing that widget to have a name.
	 *
	 * This is what lets the mockups' own hover cards reach the game. A built screen keeps no CSS
	 * classes, so "the third .om" cannot be looked up at run time, and only targets that
	 * interaction_manifest.py happened to name could be attached - nine of thirty-three. Every card
	 * owner does have a unique rect though, and the screens are built 1:1, so the rectangle names
	 * the widget by itself.
	 *
	 * Resolved on tick rather than immediately: a widget has no geometry until it has been laid out,
	 * so this is safe to call while a screen is still building.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void AttachByRect(UUserWidget* Screen, FVector2D DesignPosition, FVector2D DesignSize,
		const FELMTHoverCardData& Data);

	/** AttachByRect for a card a registered builder makes. Data is the fallback if no builder of
	 *  that name is registered by the time the widget is found. */
	void AttachBuilderByRect(UUserWidget* Screen, FVector2D DesignPosition, FVector2D DesignSize,
		FName Builder, const FString& Key, const FELMTHoverCardData& Fallback);

	/**
	 * Attaches every card a table holds for one screen. Rows are matched on their Screen column
	 * against the screen's class name, so one table serves the whole game.
	 * @return how many cards were queued.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	int32 AttachAllForScreen(UUserWidget* Screen, UDataTable* Cards);

	/** Ensures a screen has a hover layer, making one on its topmost overlay if not, and attaches
	 *  every card the table holds for it. One call gives a screen all of its hovers. */
	UFUNCTION(BlueprintCallable, Category = "Hover Card", meta = (DisplayName = "Setup Hover Cards For Screen"))
	static UELMTHoverCardLayer* SetupForScreen(UUserWidget* Screen, UDataTable* Cards);

	/**
	 * Makes Target hoverable with a card built fresh each time it is hovered.
	 *
	 * Use this wherever the card shows something that changes during a run. Attach() with fixed
	 * data stays right for text that cannot change, such as what a glyph means.
	 */
	void AttachProvider(UWidget* Target, FELMTHoverCardProvider Provider,
		const FString& Key = FString(), const FELMTHoverCardData& Fallback = FELMTHoverCardData());

	/**
	 * Registers a named builder, so a table row saying Builder="sigil" becomes a live card.
	 *
	 * Registered on the LAYER rather than globally because a builder reads from a running game, and
	 * a layer belongs to a screen inside one. A screen opened on its own registers nothing and
	 * quietly falls back to each row's authored Card.
	 */
	void RegisterBuilder(FName Name, FELMTHoverCardProvider Builder);

	/** Names every builder registered here. Lets a caller check its wiring arrived. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	TArray<FName> GetRegisteredBuilders() const;

	/** Stops a target being hoverable, and hides the card if that target is the one showing. */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void Detach(UWidget* Target);

	/** Shows the card against a target right now, without waiting for a hover. */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void ShowFor(UWidget* Target, const FELMTHoverCardData& Data);

	/** Begins the fade out. */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void Hide();

	/** Every widget this layer has made hoverable. Useful to a test, and to anyone asking why a
	 *  particular thing does or does not show a card. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	TArray<UWidget*> GetAttachedTargets() const;

	/** How many cards are still waiting for their owner to be laid out. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	int32 GetPendingCount() const { return PendingRects.Num(); }

	/** The widget whose card is showing, or null. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	UWidget* GetHoveredTarget() const { return CurrentTarget.Get(); }

	/**
	 * The widget of Screen whose laid-out rect best matches one in design space, or null.
	 *
	 * Public because it IS the addressing scheme, and a test that goes through the tick instead can
	 * only ever check it in whatever aspect the play window happens to be, which is not the one the
	 * screens were authored for. OutMiss carries what was measured when nothing matched.
	 */
	static UWidget* FindWidgetByRect(UUserWidget* Screen, FVector2D DesignPosition,
		FVector2D DesignSize, FELMTHoverCardMiss* OutMiss = nullptr);

	/** The one card this layer shows. Exposed so a test can read back what a hover produced, which
	 *  is the only way to check that a live card says what its service says. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	UELMTHoverCard* GetCard() const { return Card; }

	/** Finds the nearest hover card layer above a widget, so a target does not have to be told
	 *  which layer to use. Returns null when the widget is not inside a screen that has one. */
	UFUNCTION(BlueprintPure, Category = "Hover Card", meta = (WorldContext = "Context"))
	static UELMTHoverCardLayer* FindLayerFor(UWidget* Context);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Holds the card. Optional: without one, the layer's own root is used. */
	UPROPERTY(BlueprintReadOnly, Category = "Hover Card", meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> CardCanvas;

private:
	/** What a target should show, and whether we already bound its Slate hover. */
	struct FAttachment
	{
		FELMTHoverCardData Data;
		/** When bound, this is asked for the card instead of using Data. */
		FELMTHoverCardProvider Provider;
		/** Which instance of a repeated component this target is, passed to the provider. */
		FString Key;
		bool bBound = false;

		FELMTHoverCardData Resolve(UWidget* Target) const
		{
			if (!Provider.IsBound())
			{
				return Data;
			}
			// A builder returns an empty card when it has nothing to read: no run started, an empty
			// hold, a sigil whose letter it does not know. Falling back to the mockup's own words
			// keeps a card that used to work from silently going blank.
			const FELMTHoverCardData Built = Provider.Execute(Target, Key);
			return Built.IsEmpty() ? Data : Built;
		}
	};

	/** A card waiting for its owner to have geometry. */
	struct FPendingRect
	{
		TWeakObjectPtr<UUserWidget> Screen;
		FVector2D Position = FVector2D::ZeroVector;
		FVector2D Size = FVector2D::ZeroVector;
		FELMTHoverCardData Data;
		/** The builder that should make this card, or None to just show Data. */
		FName Builder;
		/** Which instance of a repeated component this is, handed to the builder. */
		FString Key;
		/** Ticks spent looking, so a card that never finds its widget says so once. */
		int32 Attempts = 0;
	};

	void ResolvePendingRects();

	void EnsureCard();
	bool BindSlateHover(UWidget* Target);
	void PositionCard(const FGeometry& MyGeometry);

	UPROPERTY()
	TObjectPtr<UELMTHoverCard> Card;

	TMap<TWeakObjectPtr<UWidget>, FAttachment> Attachments;
	TMap<FName, FELMTHoverCardProvider> Builders;
	TArray<FPendingRect> PendingRects;
	TWeakObjectPtr<UWidget> CurrentTarget;

	/** 0 hidden, 1 fully shown. Driven by NativeTick toward TargetAlpha. */
	float Alpha = 0.f;
	float TargetAlpha = 0.f;
	/** True when the card sits below its target because there was no room above. */
	bool bFlipped = false;
};
