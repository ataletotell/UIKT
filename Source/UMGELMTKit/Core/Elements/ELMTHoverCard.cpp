// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTHoverCard.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

UELMTHoverCard::UELMTHoverCard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The same two font assets the kitchen sink's own extractor already imported (DA_ArnawaTokens'
	// h4 and small/body roles point at these). Referencing them here rather than a fresh import
	// keeps every hover card on the one Fraunces/Work Sans the rest of the design system uses,
	// without this plugin taking a hard reference on UArnawaDesignTokens itself.
	static ConstructorHelpers::FObjectFinder<UFont> TitleFontFinder(
		TEXT("/Game/UI/Test/Fonts/Fraunces_600_o25_8_Font.Fraunces_600_o25_8_Font"));
	static ConstructorHelpers::FObjectFinder<UFont> BodyFontFinder(
		TEXT("/Game/UI/Test/Fonts/WorkSans_400_Font.WorkSans_400_Font"));

	// Slate size is a point size at 96 DPI (CSS px * 0.75); the mockup's `.tip` is font-size:.85em
	// against roughly a 14-15px body context, so title and body both land well under the page's
	// own heading sizes - a tooltip reads at a glance, it does not compete with the page.
	TitleFont.FontObject = TitleFontFinder.Object;
	TitleFont.Size = 11.f;

	BodyFont.FontObject = BodyFontFinder.Object;
	BodyFont.Size = 9.5f;

	RowFont.FontObject = BodyFontFinder.Object;
	RowFont.Size = 9.5f;
}

void UELMTHoverCard::NativeConstruct()
{
	Super::NativeConstruct();

	// A card never takes the mouse. The mockup says `pointer-events:none` on `.tip`, and it matters
	// for more than fidelity: the card is drawn over the very widget whose hover is keeping it
	// open, so if it were hit-testable it would steal the mouse from that widget, the hover would
	// end, the card would hide, the mouse would return to the widget, and it would flicker forever.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	EnsureLayout();
	RebuildRows();
}

void UELMTHoverCard::EnsureLayout()
{
	// A card authored as a Blueprint brings its own tree and its own look, and this does nothing.
	// A card created straight from C++ has an empty tree, so it builds itself one here.
	//
	// That matters more than it looks: the screens this sits on are GENERATED, and everything
	// inside a generated Widget Blueprint is destroyed on the next build. A hover card that had to
	// be authored into a screen by hand would survive exactly one rebuild. Built in code, it
	// cannot be lost.
	UE_LOG(LogTemp, Log, TEXT("[HoverCard] EnsureLayout: tree=%s root=%s"),
		WidgetTree ? TEXT("yes") : TEXT("NULL"),
		(WidgetTree && WidgetTree->RootWidget) ? *WidgetTree->RootWidget->GetName() : TEXT("none"));
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	// A fixed width, and the reason is not only fidelity to `width:max(14cqw,160px)`. Auto-wrapping
	// text inside an auto-sizing parent has no width to wrap against: it asks how wide it may be,
	// is told nothing, wraps to zero, and the whole card measures zero by zero while still
	// reporting itself visible. Giving the card a known width is what makes it measurable at all.
	USizeBox* Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
	if (WrapWidth > 0.f)
	{
		Frame->SetWidthOverride(WrapWidth);
	}
	Frame->SetVisibility(ESlateVisibility::HitTestInvisible);
	WidgetTree->RootWidget = Frame;

	UBorder* Ground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Ground"));
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FSlateColor(BackgroundColor);
	Brush.OutlineSettings.Color = FSlateColor(BorderColor);
	Brush.OutlineSettings.Width = 1.f;
	// FixedRadius, not the default HalfHeightRadius, which ignores CornerRadii entirely.
	Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	Brush.OutlineSettings.CornerRadii = FVector4(CornerRadius, CornerRadius, CornerRadius, CornerRadius);
	Ground->SetBrush(Brush);
	Ground->SetPadding(ContentPadding);
	Ground->SetVisibility(ESlateVisibility::HitTestInvisible);
	Frame->AddChild(Ground);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	Ground->AddChild(Column);

	Txt_Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_Title"));
	Txt_Title->SetColorAndOpacity(FSlateColor(TitleColor));
	Txt_Title->SetFont(TitleFont);
	Column->AddChild(Txt_Title);

	Txt_Body = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Txt_Body"));
	Txt_Body->SetColorAndOpacity(FSlateColor(BodyColor));
	Txt_Body->SetFont(BodyFont);
	Column->AddChild(Txt_Body);

	RowBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RowBox"));
	Column->AddChild(RowBox);
	UE_LOG(LogTemp, Log, TEXT("[HoverCard] EnsureLayout built root=%s"), *Ground->GetName());
}

void UELMTHoverCard::Wrap(UTextBlock* Text) const
{
	if (!Text)
	{
		return;
	}
	// Full-width text: the title and the prose. Wrapping at an explicit width is also what keeps
	// the card narrow, because a TextBlock's desired width stops growing at the wrap point, so the
	// card ends up about WrapWidth across instead of as wide as its longest unbroken line.
	Text->SetAutoWrapText(true);
	Text->SetWrapTextAt(0.f);
}

void UELMTHoverCard::WrapInColumn(UTextBlock* Text) const
{
	if (!Text)
	{
		return;
	}
	// Text inside a row wraps at the width of ITS column, not the card's. Giving these the card's
	// WrapTextAt would tell a label roughly 85px wide to wrap at 269, so "could be" would never
	// break and the columns would be pushed out of shape. AutoWrapText alone uses the allotted
	// geometry, which is exactly the column.
	Text->SetAutoWrapText(true);
	Text->SetWrapTextAt(0.f);
}

void UELMTHoverCard::SetData(const FELMTHoverCardData& InData)
{
	Data = InData;
	RebuildRows();
}

void UELMTHoverCard::RebuildRows()
{
	if (Txt_Title)
	{
		Txt_Title->SetText(Data.Title);
		// A card with rows but no title should not leave an empty gold line above them.
		Txt_Title->SetVisibility(Data.Title.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		Wrap(Txt_Title);
	}

	if (Txt_Body)
	{
		Txt_Body->SetText(Data.Body);
		Txt_Body->SetColorAndOpacity(FSlateColor(BodyColor));
		Txt_Body->SetVisibility(Data.Body.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		Wrap(Txt_Body);
	}

	if (!RowBox)
	{
		return;
	}

	RowBox->ClearChildren();
	for (const FELMTHoverCardRow& Row : Data.Rows)
	{
		UWidget* RowWidget = MakeRowWidget(Row);
		if (!RowWidget)
		{
			// The default line: label left, value hard right, which is what
			// `display:flex; justify-content:space-between` gives in the mockup.
			UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

			UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Label->SetText(Row.Label);
			Label->SetColorAndOpacity(FSlateColor(LabelColor));
			Label->SetFont(RowFont);
			if (UHorizontalBoxSlot* LabelSlot = Cast<UHorizontalBoxSlot>(Line->AddChild(Label)))
			{
				// The label keeps a narrow column of its own and wraps inside it, which is how a
				// two word label like "could be" ends up stacked over two lines in the mockup.
				// FSlateChildSize has no (rule, value) constructor; the weight is a separate field.
				FSlateChildSize LabelSize(ESlateSizeRule::Fill);
				LabelSize.Value = LabelFillWeight;
				LabelSlot->SetSize(LabelSize);
				LabelSlot->SetPadding(FMargin(0.f, 0.f, RowGap, 0.f));
			}
			WrapInColumn(Label);

			UTextBlock* Value = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Value->SetText(Row.Value);
			Value->SetColorAndOpacity(FSlateColor(ValueColor));
			Value->SetFont(RowFont);
			// A short value sits hard right, a long one wraps and reads from the left. That is what
			// `justify-content:space-between` does in the mockup: with one line of slack the value
			// is pushed to the far edge ("0 to 2 h"), and with none it simply fills its column
			// ("a festival, a fire, a press-gang, a windfall").
			Value->SetJustification(ETextJustify::Right);
			if (UHorizontalBoxSlot* ValueSlot = Cast<UHorizontalBoxSlot>(Line->AddChild(Value)))
			{
				FSlateChildSize ValueSize(ESlateSizeRule::Fill);
				ValueSize.Value = ValueFillWeight;
				ValueSlot->SetSize(ValueSize);
				ValueSlot->SetHorizontalAlignment(HAlign_Fill);
			}
			WrapInColumn(Value);
			RowWidget = Line;
		}

		if (UVerticalBoxSlot* RowSlot = Cast<UVerticalBoxSlot>(RowBox->AddChild(RowWidget)))
		{
			RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}
	}
}
