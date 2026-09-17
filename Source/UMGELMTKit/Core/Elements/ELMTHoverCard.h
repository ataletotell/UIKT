// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "ELMTHoverCard.generated.h"

class UTextBlock;
class UVerticalBox;

/** One label/value line in a hover card, the mockup's `.tip .r`: label on the left in a muted
 *  colour, value hard right in the readable one. */
USTRUCT(BlueprintType)
struct FELMTHoverCardRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FText Value;
};

/** What a hover card shows: the mockup's `.tip b` title plus any number of `.tip .r` rows. */
USTRUCT(BlueprintType)
struct FELMTHoverCardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FText Title;

	/** Free prose under the title, before the rows. In the mockups this is the loose text sitting
	 *  directly inside `.tip`, with no element of its own: "Fires the moment you step in." */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	TArray<FELMTHoverCardRow> Rows;

	bool IsEmpty() const { return Title.IsEmpty() && Body.IsEmpty() && Rows.Num() == 0; }
};

/**
 * One authored hover card, as a DataTable row.
 *
 * The mockups already carry these: a `.tip` element sitting inside the thing it describes. Rather
 * than retyping that content in C++, the pipeline extracts it (scripts/extract_hover_cards.py)
 * into a table a screen can read at run time and hand straight to UELMTHoverCardLayer::Attach.
 *
 * WidgetName is the widget the card belongs to, using the same names interaction_manifest.py gives
 * the pipeline's bindable widgets, so a card points at a real widget rather than a CSS class that
 * no longer exists once the screen is built.
 */
USTRUCT(BlueprintType)
struct FELMTHoverCardTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The screen asset this card belongs to, e.g. WBP_Screen19_ShipLedgerHoldSatchel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FName Screen;

	/**
	 * The rect the hovered widget occupies in the 1920x1080 design space, as (Min = position,
	 * Max = size). This is how a card finds its widget: a built screen keeps no CSS classes, but
	 * every card owner in the mockups has a unique rectangle, so the rect names it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FBox2D Rect = FBox2D(ForceInit);

	/**
	 * The CSS classes the hovered element carries, first one first, e.g. "sg" or "lot tall".
	 *
	 * This also decides which live BUILDER makes the card, if any: the first class is the component
	 * type, and a game registers a builder under that name
	 * (UELMTHoverCardLayer::RegisterBuilder). So `.sg` rows are built by the "sg" builder and show
	 * a faction's real standing, while a class nobody registered falls back to Card below.
	 *
	 * Doing it by class rather than by a column of its own is not a shortcut: the class IS the
	 * component type in the mockups, so a second column would only ever restate it, and could
	 * disagree with it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FString Owner;

	/**
	 * What that widget's card shows.
	 *
	 * Where a builder is registered for this row's component this is the fallback, used when the
	 * screen is opened outside a running game and there is no service to ask. Otherwise it is the
	 * whole card.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card")
	FELMTHoverCardData Card;
};

/**
 * The floating card a hover reveals: a title and a list of label/value rows.
 *
 * This is only the CONTENT. It does not decide when it is visible or where it sits - that is
 * UELMTHoverCardLayer's job, so one card instance can serve every hoverable widget on a screen
 * instead of every widget carrying a hidden copy of one.
 *
 * The look comes from the design system (the mockup's `.tip`: dark ground, thin warm border,
 * gold title, muted label against a lighter value). Subclass this in Blueprint, or let the
 * html-to-umg pipeline generate a subclass from the captured `.tip` node.
 */
UCLASS(Blueprintable, meta = (DisplayName = "ELMT Hover Card", Category = "ELMT|Composites"))
class UMGELMTKIT_API UELMTHoverCard : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Stable identifier for automated QA tooling to find this widget in the live tree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Testing, meta = (ExposeOnSpawn = true))
	FName TestId;

	/**
	 * Builds the default `.tip` tree if this card has none, for a card created straight from C++.
	 *
	 * Must be called BEFORE the card is added to a parent. Setting WidgetTree->RootWidget after
	 * Slate has already built the widget changes nothing that is drawn: the card then measures
	 * zero by zero and is invisible while looking, in every log, exactly like a working card.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void EnsureLayout();

	/** Fills the card in. Rows are rebuilt each time, so a caller can show a value that changes. */
	UFUNCTION(BlueprintCallable, Category = "Hover Card")
	void SetData(const FELMTHoverCardData& InData);

	/** What the card is currently showing. */
	UFUNCTION(BlueprintPure, Category = "Hover Card")
	const FELMTHoverCardData& GetData() const { return Data; }

	/** Override in Blueprint to build a row your own way. Return null to use the default
	 *  label-left/value-right line. */
	UFUNCTION(BlueprintNativeEvent, Category = "Hover Card")
	UWidget* MakeRowWidget(const FELMTHoverCardRow& Row);
	virtual UWidget* MakeRowWidget_Implementation(const FELMTHoverCardRow& Row) { return nullptr; }

protected:
	virtual void NativeConstruct() override;

	/** The `.tip b` line. Optional so a Blueprint subclass can lay the card out its own way. */
	UPROPERTY(BlueprintReadOnly, Category = "Hover Card", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Title;

	/** The prose paragraph. Optional, like the rest. */
	UPROPERTY(BlueprintReadOnly, Category = "Hover Card", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Body;

	/** Holds one widget per row. Cleared and refilled by SetData. */
	UPROPERTY(BlueprintReadOnly, Category = "Hover Card", meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RowBox;

	/**
	 * Width the card wraps its text at, in pixels. The mockup fixes `.tip` at
	 * `width:max(14cqw,160px)`, which is about 269px at 1920 wide.
	 *
	 * Without a cap a card is as wide as its longest line, and a value like "a festival, a fire, a
	 * press-gang, a windfall" would stretch it most of the way across the screen. Set 0 to let the
	 * card size to its content instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	float WrapWidth = 269.f;

	/** The `.tip` ground: #1c1712 in the mockup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor BackgroundColor = FLinearColor(0.110f, 0.090f, 0.071f, 1.f);   // #1c1712

	/** The `1px solid #4a3d2b` edge. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor BorderColor = FLinearColor(0.290f, 0.239f, 0.169f, 1.f);       // #4a3d2b

	/** The gold `.tip b` title, var(--gold-hover). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor TitleColor = FLinearColor(0.922f, 0.831f, 0.580f, 1.f);        // #ebd494

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	float CornerRadius = 8.f;

	/** The mockup's `padding:.7cqw .9cqw`, about 13px by 17px at 1920. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FMargin ContentPadding = FMargin(17.f, 13.f, 17.f, 13.f);

	/** Colour of the prose paragraph. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor BodyColor = FLinearColor(0.925f, 0.898f, 0.847f, 1.f);   // #ece5d8

	/** How the width of a row is split between its label column and its value column. The mockup's
	 *  labels are short ("could be", "costs") and its values carry the sentence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	float LabelFillWeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	float ValueFillWeight = 2.2f;

	/** Gap between the label and value columns, the mockup's `gap:1em`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	float RowGap = 12.f;

	/** Colour of a row's label, matching the mockup's muted `.tip .r`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor LabelColor = FLinearColor(0.659f, 0.620f, 0.553f, 1.f);   // #a89e8d

	/** Colour of a row's value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FLinearColor ValueColor = FLinearColor(0.925f, 0.898f, 0.847f, 1.f);   // #ece5d8

	/** The `.tip b` title face: Fraunces 600, matching the mockup's gold serif title. Defaulted
	 *  from the kitchen sink's own imported font asset (the same one DA_ArnawaTokens' h4 role
	 *  uses) so a card never falls back to Slate's default font, which is what an unset
	 *  FSlateFontInfo silently does - large, sans, and nothing like the design system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FSlateFontInfo TitleFont;

	/** The prose paragraph's face: Work Sans 400, small - a tooltip's body text, not a page's. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FSlateFontInfo BodyFont;

	/** A row's label/value face. Same family as BodyFont; kept separate so a subclass can set the
	 *  rows a step smaller than the body paragraph without touching both. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Card|Style")
	FSlateFontInfo RowFont;

public:
	UELMTHoverCard(const FObjectInitializer& ObjectInitializer);

private:
	void RebuildRows();
	void Wrap(UTextBlock* Text) const;
	void WrapInColumn(UTextBlock* Text) const;

	UPROPERTY()
	FELMTHoverCardData Data;
};
