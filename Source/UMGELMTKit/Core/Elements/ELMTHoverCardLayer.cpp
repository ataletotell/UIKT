// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTHoverCardLayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanelSlot.h"

namespace
{
	/** The best panel on a screen to hang a full-screen layer from: one that can take another
	 *  child, preferring an Overlay or a Canvas because those stack rather than reflow. */
	UPanelWidget* FindHostPanel(UWidget* Widget, int32 Depth = 0)
	{
		if (!Widget || Depth > 8)
		{
			return nullptr;
		}
		UPanelWidget* Best = nullptr;
		if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			if (Panel->CanAddMoreChildren())
			{
				// An Overlay or Canvas is taken at once: a layer added to a VerticalBox would be
				// laid out as another row instead of covering the screen.
				if (Panel->IsA<UOverlay>() || Panel->IsA<UCanvasPanel>())
				{
					return Panel;
				}
				Best = Panel;
			}
			const int32 Count = Panel->GetChildrenCount();
			for (int32 Index = 0; Index < Count; ++Index)
			{
				if (UPanelWidget* Found = FindHostPanel(Panel->GetChildAt(Index), Depth + 1))
				{
					if (Found->IsA<UOverlay>() || Found->IsA<UCanvasPanel>())
					{
						return Found;
					}
					Best = Best ? Best : Found;
				}
			}
		}
		return Best;
	}

}

void UELMTHoverCardLayer::NativeConstruct()
{
	Super::NativeConstruct();

	// The layer covers the screen so it can place a card anywhere, but it must never take the
	// mouse: it sits above everything, so a hit-testable layer would swallow every click on the
	// screen underneath it.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	EnsureCard();
}

void UELMTHoverCardLayer::NativeDestruct()
{
	Attachments.Empty();
	CurrentTarget.Reset();
	Super::NativeDestruct();
}

void UELMTHoverCardLayer::EnsureLayout()
{
	EnsureCard();
}

void UELMTHoverCardLayer::EnsureCard()
{
	if (Card || !WidgetTree)
	{
		return;
	}

	// Built in code when the layer itself was, so the whole feature works with no authored assets.
	// The screens are generated and rebuilt from their mockups, so anything authored into one is
	// gone on the next build; a layer that had to be placed by hand would not last a day.
	if (!WidgetTree->RootWidget)
	{
		CardCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CardCanvas"));
		WidgetTree->RootWidget = CardCanvas;
	}

	// No class set means the plain C++ card, which builds its own `.tip` layout.
	Card = CreateWidget<UELMTHoverCard>(this,
		HoverCardClass ? HoverCardClass.Get() : UELMTHoverCard::StaticClass());
	if (!Card)
	{
		return;
	}

	// Before AddChild: the card builds its own tree, and a tree set after Slate has constructed the
	// widget is never drawn. That failure is silent - the card reports visible, its opacity tweens,
	// every log looks right, and it measures zero by zero.
	Card->EnsureLayout();

	UPanelWidget* Host = CardCanvas ? Cast<UPanelWidget>(CardCanvas.Get()) : Cast<UPanelWidget>(GetRootWidget());
	if (!Host)
	{
		return;
	}
	if (UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(Host->AddChild(Card)))
	{
		// Auto-size to the content: the card is as wide as its longest row, like the mockup's
		// `width:max(14cqw,160px)` shrink-to-fit box.
		CardSlot->SetAutoSize(true);
	}
	Card->SetRenderOpacity(0.f);
	Card->SetVisibility(ESlateVisibility::Collapsed);
}

bool UELMTHoverCardLayer::BindSlateHover(UWidget* Target)
{
	const TSharedPtr<SWidget> Slate = Target->GetCachedWidget();
	if (!Slate.IsValid())
	{
		return false;
	}

	// A widget only receives mouse-enter if it is hit-testable, and the html-to-umg builder marks
	// every painted widget SelfHitTestInvisible so its layers do not swallow clicks meant for the
	// buttons beneath. That is right for painting and fatal for hovering, so a target that has
	// been made click-through is given its hit test back here. Visible rather than
	// SelfHitTestInvisible: the target itself has to be hit, not just its children.
	if (Target->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
	{
		Target->SetVisibility(ESlateVisibility::Visible);
	}

	TWeakObjectPtr<UELMTHoverCardLayer> WeakThis(this);
	TWeakObjectPtr<UWidget> WeakTarget(Target);

	Slate->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
		[WeakThis, WeakTarget](const FGeometry&, const FPointerEvent&)
		{
			UELMTHoverCardLayer* Layer = WeakThis.Get();
			UWidget* Hovered = WeakTarget.Get();
			if (!Layer || !Hovered)
			{
				return;
			}
			if (const FAttachment* Found = Layer->Attachments.Find(WeakTarget))
			{
				// Resolve() asks the provider, if there is one, so a live card is built now rather
				// than having been frozen when it was attached.
				Layer->ShowFor(Hovered, Found->Resolve(Hovered));
			}
		}));

	Slate->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
		[WeakThis, WeakTarget](const FPointerEvent&)
		{
			UELMTHoverCardLayer* Layer = WeakThis.Get();
			// Only hide if this target is still the one showing. Moving between two hoverable
			// widgets fires the new one's enter before the old one's leave, and without this
			// check the leave would immediately close the card the enter just opened.
			if (Layer && Layer->CurrentTarget == WeakTarget)
			{
				Layer->Hide();
			}
		}));
	return true;
}

bool UELMTHoverCardLayer::Attach(UWidget* Target, const FELMTHoverCardData& Data)
{
	if (!Target)
	{
		return false;
	}

	FAttachment& Entry = Attachments.FindOrAdd(Target);
	Entry.Data = Data;
	if (!Entry.bBound)
	{
		// A widget has no Slate widget to bind until it has been built, so a caller that attaches
		// while constructing its own UI would otherwise silently get nothing. The attachment is
		// kept either way and NativeTick keeps trying, which makes this safe to call at any point
		// in a screen's life rather than only after the first paint.
		Entry.bBound = BindSlateHover(Target);
	}
	return Entry.bBound;
}

void UELMTHoverCardLayer::AttachByRect(UUserWidget* Screen, FVector2D DesignPosition,
	FVector2D DesignSize, const FELMTHoverCardData& Data)
{
	AttachBuilderByRect(Screen, DesignPosition, DesignSize, NAME_None, FString(), Data);
}

void UELMTHoverCardLayer::AttachBuilderByRect(UUserWidget* Screen, FVector2D DesignPosition,
	FVector2D DesignSize, FName Builder, const FString& Key, const FELMTHoverCardData& Fallback)
{
	// A row with a builder is worth queueing even with nothing written in it: the builder is where
	// its content comes from. Only a row that is empty AND has no builder has nothing to show.
	if (!Screen || (Builder.IsNone() && Fallback.IsEmpty()))
	{
		return;
	}
	FPendingRect Pending;
	Pending.Screen = Screen;
	Pending.Position = DesignPosition;
	Pending.Size = DesignSize;
	Pending.Data = Fallback;
	Pending.Builder = Builder;
	Pending.Key = Key;
	PendingRects.Add(MoveTemp(Pending));
}

int32 UELMTHoverCardLayer::AttachAllForScreen(UUserWidget* Screen, UDataTable* Cards)
{
	if (!Screen || !Cards)
	{
		return 0;
	}

	const FString ScreenName = Screen->GetClass()->GetName();
	TArray<FELMTHoverCardTableRow*> Rows;
	Cards->GetAllRows(TEXT("ELMTHoverCardLayer"), Rows);

	// A generated screen's class is the Blueprint's name with _C on the end, so compare by prefix
	// rather than requiring the table to know about Unreal's naming.
	TArray<const FELMTHoverCardTableRow*> Mine;
	for (const FELMTHoverCardTableRow* Row : Rows)
	{
		if (Row && ScreenName.StartsWith(Row->Screen.ToString()))
		{
			Mine.Add(Row);
		}
	}

	// Reading order: down the screen, then across. A builder that serves several of one component
	// is told WHICH one by its position in this order, so the order has to be a property of the
	// screen rather than of the table. GetAllRows walks a TMap, and the order a TMap gives back is
	// not something to hang a card's contents on.
	Mine.Sort([](const FELMTHoverCardTableRow& A, const FELMTHoverCardTableRow& B)
	{
		return FMath::IsNearlyEqual(A.Rect.Min.Y, B.Rect.Min.Y)
			? A.Rect.Min.X < B.Rect.Min.X
			: A.Rect.Min.Y < B.Rect.Min.Y;
	});

	int32 Queued = 0;
	TMap<FName, int32> SeenPerComponent;
	for (const FELMTHoverCardTableRow* Row : Mine)
	{
		// The first CSS class is the component type, and a builder is registered under that name.
		FString Component = Row->Owner;
		Component.Split(TEXT(" "), &Component, nullptr);
		const FName Builder(*Component);
		const int32 Ordinal = SeenPerComponent.FindOrAdd(Builder)++;

		AttachBuilderByRect(Screen, FVector2D(Row->Rect.Min), FVector2D(Row->Rect.Max),
			Builder, FString::FromInt(Ordinal), Row->Card);
		++Queued;
	}
	return Queued;
}

UWidget* UELMTHoverCardLayer::FindWidgetByRect(UUserWidget* Screen, FVector2D DesignPosition,
	FVector2D DesignSize, FELMTHoverCardMiss* OutMiss)
{
	if (!Screen || !Screen->WidgetTree)
	{
		return nullptr;
	}

	const FGeometry ScreenGeo = Screen->GetCachedGeometry();
	const FVector2D ScreenSize = ScreenGeo.GetLocalSize();
	if (ScreenSize.X <= KINDA_SMALL_NUMBER || ScreenSize.Y <= KINDA_SMALL_NUMBER)
	{
		return nullptr;   // not laid out yet; the caller retries next tick
	}

	// The capture is 1920x1080, so a widget's measured rect has to be put back into design space
	// before it can be compared. ONE factor for both axes, not one each: a screen drawn at 2560x1440
	// is the design at 1.333, and dividing each axis by its own ratio would also "work" at any other
	// aspect, by quietly turning a square tile into a tall thin one and matching nothing.
	const float Scale = 1920.f / ScreenSize.X;

	// The aspect is recorded, not enforced. These screens do not scale to fit, they RE-LAY OUT: at
	// 1080x1225 the ledger's tiles measured 39x191 where the design says 142x142, so no factor maps
	// one onto the other and every card misses. But refusing outright on any departure from 16:9
	// would be worse than the problem, because a windowed 1920x1080 loses its title bar and arrives
	// as 1920x1057, which is the design for every practical purpose. So a few percent is fine, and
	// anything past that only changes what gets REPORTED when nothing matches - the match itself is
	// still attempted, and still has to pass the same error bar as any other.
	const float ExpectedHeight = ScreenSize.X * (1080.f / 1920.f);
	const bool bWrongAspect = FMath::Abs(ScreenSize.Y - ExpectedHeight) > ExpectedHeight * 0.02f;

	const FVector2D ToDesign(Scale, Scale);
	const FVector2D ScreenOrigin = ScreenGeo.GetAbsolutePosition();

	UWidget* Best = nullptr;
	float BestError = TNumericLimits<float>::Max();
	FVector2D BestPos = FVector2D::ZeroVector;
	FVector2D BestSize = FVector2D::ZeroVector;
	FVector2D BestRawPos = FVector2D::ZeroVector;
	FVector2D BestRawSize = FVector2D::ZeroVector;
	Screen->WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (!Widget || !Widget->GetCachedWidget().IsValid())
		{
			return;
		}
		const FGeometry& Geo = Widget->GetCachedGeometry();
		const FVector2D Size = Geo.GetLocalSize();
		if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER)
		{
			return;
		}
		const FVector2D Pos = (Geo.GetAbsolutePosition() - ScreenOrigin) / FMath::Max(ScreenGeo.Scale, KINDA_SMALL_NUMBER);
		const FVector2D DesignPos = Pos * ToDesign;
		const FVector2D DesignWH = Size * ToDesign;

		// Position matters more than size: several widgets share a rect's corner (a border, its
		// content, its overlay), and the one whose size also matches is the one meant.
		const float Error = (DesignPos - DesignPosition).Size() + (DesignWH - DesignSize).Size();
		if (Error < BestError)
		{
			BestError = Error;
			Best = Widget;
			BestPos = DesignPos;
			BestSize = DesignWH;
			BestRawPos = Pos;
			BestRawSize = Size;
		}
	});

	if (OutMiss)
	{
		OutMiss->ScreenSize = ScreenSize;
		OutMiss->bWrongAspect = bWrongAspect;
		OutMiss->Nearest = Best;
		OutMiss->Error = BestError;
		OutMiss->NearestDesignRect = FBox2D(BestPos, BestSize);
		OutMiss->NearestRawRect = FBox2D(BestRawPos, BestRawSize);
	}

	// A few pixels of slack for rounding; beyond that the rect belongs to no widget on this screen
	// and attaching the nearest one would put a card on something unrelated.
	return (BestError <= 12.f) ? Best : nullptr;
}

void UELMTHoverCardLayer::ResolvePendingRects()
{
	bool bReportedAspect = false;
	for (int32 Index = PendingRects.Num() - 1; Index >= 0; --Index)
	{
		FPendingRect& Pending = PendingRects[Index];
		UUserWidget* Screen = Pending.Screen.Get();
		if (!Screen)
		{
			PendingRects.RemoveAtSwap(Index);
			continue;
		}
		FELMTHoverCardMiss Miss;
		if (UWidget* Target = FindWidgetByRect(Screen, Pending.Position, Pending.Size, &Miss))
		{
			// A builder registered after this row was queued still wins: the lookup happens now,
			// when the widget is finally found, not when the table was read.
			if (const FELMTHoverCardProvider* Builder = Builders.Find(Pending.Builder))
			{
				// Data comes along as the fallback: a builder with no run to read from returns an
				// empty card, and the mockup's own words are better than nothing.
				AttachProvider(Target, *Builder, Pending.Key, Pending.Data);
			}
			else
			{
				// A row queued only because its component might have had a builder, with nothing
				// written in it, would otherwise hover an empty card. Attach() ignores empty data.
				Attach(Target, Pending.Data);
			}
			PendingRects.RemoveAtSwap(Index);
		}
		else
		{
			// Given up on, not merely reported. Every attempt walks the screen's whole widget tree,
			// so a rect that matches nothing would keep doing that every tick for as long as the
			// screen is open, and it will not start matching later: by 30 ticks the screen has
			// long been laid out.
			if (++Pending.Attempts >= 30)
			{
				if (Miss.bWrongAspect)
				{
					// Said once for the screen, not once per card: thirty-one copies of the same
					// sentence buries the one line that explains all of them.
					if (!bReportedAspect)
					{
						bReportedAspect = true;
						UE_LOG(LogTemp, Warning, TEXT("[HoverCard] %s is laid out %.0fx%.0f, which is not the 16:9 it was designed for - these screens re-lay-out rather than scale, so hover cards cannot find their widgets here"),
							*Screen->GetClass()->GetName(), Miss.ScreenSize.X, Miss.ScreenSize.Y);
					}
					PendingRects.RemoveAtSwap(Index);
					continue;
				}
				UE_LOG(LogTemp, Warning, TEXT("[HoverCard] no widget at rect (%.0f,%.0f %.0fx%.0f) on %s")
					TEXT(" - screen laid out %.0fx%.0f, nearest %s scaled (%.0f,%.0f %.0fx%.0f)")
					TEXT(" raw (%.0f,%.0f %.0fx%.0f) error %.0f - card '%s' will not appear"),
					Pending.Position.X, Pending.Position.Y, Pending.Size.X, Pending.Size.Y,
					*Screen->GetClass()->GetName(),
					Miss.ScreenSize.X, Miss.ScreenSize.Y,
					Miss.Nearest ? *Miss.Nearest->GetName() : TEXT("none"),
					Miss.NearestDesignRect.Min.X, Miss.NearestDesignRect.Min.Y,
					Miss.NearestDesignRect.Max.X, Miss.NearestDesignRect.Max.Y,
					Miss.NearestRawRect.Min.X, Miss.NearestRawRect.Min.Y,
					Miss.NearestRawRect.Max.X, Miss.NearestRawRect.Max.Y,
					Miss.Error, *Pending.Data.Title.ToString());
				PendingRects.RemoveAtSwap(Index);
			}
		}
	}
}

UELMTHoverCardLayer* UELMTHoverCardLayer::SetupForScreen(UUserWidget* Screen, UDataTable* Cards)
{
	if (!Screen)
	{
		return nullptr;
	}

	UELMTHoverCardLayer* Layer = FindLayerFor(Screen);
	if (!Layer)
	{
		// NOT simply the root. A generated screen's root is often a single-child widget (a SizeBox
		// or a ScrollBox), and UPanelWidget::AddChild on one that is already full fails and returns
		// null - silently. That produced a layer that queued all twenty of the ledger's cards, was
		// never in the widget tree, therefore never ticked, therefore never resolved one of them,
		// and could not be found again. Find a panel that can actually take another child.
		UPanelWidget* Host = FindHostPanel(Screen->GetRootWidget());
		if (!Host)
		{
			UWidget* Root = Screen->GetRootWidget();
			UE_LOG(LogTemp, Warning, TEXT("[HoverCard] %s has no panel with room for a layer (root is %s)"),
				*Screen->GetClass()->GetName(),
				Root ? *Root->GetClass()->GetName() : TEXT("null"));
			return nullptr;
		}
		Layer = CreateWidget<UELMTHoverCardLayer>(Screen, UELMTHoverCardLayer::StaticClass());
		if (!Layer)
		{
			return nullptr;
		}
		// Built before it is added: a WidgetTree root assigned after Slate has constructed the
		// widget is never drawn, and nothing says so.
		Layer->EnsureLayout();
		if (!Host->AddChild(Layer))
		{
			UE_LOG(LogTemp, Warning, TEXT("[HoverCard] %s would not take the layer"), *Host->GetName());
			return nullptr;
		}
	}

	Layer->AttachAllForScreen(Screen, Cards);
	return Layer;
}

TArray<UWidget*> UELMTHoverCardLayer::GetAttachedTargets() const
{
	TArray<UWidget*> Out;
	for (const TPair<TWeakObjectPtr<UWidget>, FAttachment>& Pair : Attachments)
	{
		if (UWidget* Widget = Pair.Key.Get())
		{
			Out.Add(Widget);
		}
	}
	return Out;
}

void UELMTHoverCardLayer::AttachProvider(UWidget* Target, FELMTHoverCardProvider Provider,
	const FString& Key, const FELMTHoverCardData& Fallback)
{
	if (!Target || !Provider.IsBound())
	{
		return;
	}

	FAttachment& Entry = Attachments.FindOrAdd(Target);
	Entry.Provider = Provider;
	Entry.Key = Key;
	Entry.Data = Fallback;
	if (!Entry.bBound)
	{
		Entry.bBound = BindSlateHover(Target);
	}
}

void UELMTHoverCardLayer::RegisterBuilder(FName Name, FELMTHoverCardProvider Builder)
{
	if (Name.IsNone() || !Builder.IsBound())
	{
		return;
	}
	Builders.Add(Name, Builder);
}

TArray<FName> UELMTHoverCardLayer::GetRegisteredBuilders() const
{
	TArray<FName> Names;
	Builders.GetKeys(Names);
	return Names;
}

void UELMTHoverCardLayer::Detach(UWidget* Target)
{
	if (!Target)
	{
		return;
	}
	Attachments.Remove(Target);
	if (CurrentTarget == Target)
	{
		Hide();
	}
}

void UELMTHoverCardLayer::ShowFor(UWidget* Target, const FELMTHoverCardData& Data)
{
	EnsureCard();
	if (!Card || !Target || Data.IsEmpty())
	{
		return;
	}
	CurrentTarget = Target;
	Card->SetData(Data);
	Card->SetVisibility(ESlateVisibility::HitTestInvisible);
	TargetAlpha = 1.f;
}

void UELMTHoverCardLayer::Hide()
{
	TargetAlpha = 0.f;
	CurrentTarget.Reset();
}

void UELMTHoverCardLayer::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ResolvePendingRects();

	// Anything that could not bind yet, because its Slate widget did not exist when Attach was
	// called. Cheap: the map only holds this screen's hoverable widgets, and each entry stops
	// being looked at the moment it binds.
	for (TPair<TWeakObjectPtr<UWidget>, FAttachment>& Pair : Attachments)
	{
		if (!Pair.Value.bBound)
		{
			if (UWidget* Target = Pair.Key.Get())
			{
				Pair.Value.bBound = BindSlateHover(Target);
			}
		}
	}

	if (!Card)
	{
		return;
	}

	if (!FMath::IsNearlyEqual(Alpha, TargetAlpha))
	{
		const float Step = (HoverFadeDuration > KINDA_SMALL_NUMBER)
			? InDeltaTime / HoverFadeDuration : 1.f;
		Alpha = FMath::FInterpConstantTo(Alpha, TargetAlpha, 1.f, Step);
		Card->SetRenderOpacity(Alpha);

		// Fully faded out: stop laying it out at all, so an invisible card cannot affect anything.
		if (Alpha <= KINDA_SMALL_NUMBER && TargetAlpha <= 0.f)
		{
			Card->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}

	if (Alpha > 0.f)
	{
		PositionCard(MyGeometry);
	}
}

void UELMTHoverCardLayer::PositionCard(const FGeometry& MyGeometry)
{
	UWidget* Target = CurrentTarget.Get();
	if (!Target)
	{
		return;
	}

	UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(Card->Slot);
	if (!CardSlot)
	{
		return;
	}

	// Placed every tick rather than once on show, so the card follows a target that moves or
	// resizes, and so it is placed correctly on the first frame after the card has a desired size.
	const FGeometry& TargetGeo = Target->GetCachedGeometry();
	const FVector2D TargetAbs = TargetGeo.GetAbsolutePosition();
	const FVector2D TargetSize = TargetGeo.GetAbsoluteSize();
	const FVector2D LocalTopLeft = MyGeometry.AbsoluteToLocal(TargetAbs);
	const FVector2D LocalSize = TargetSize / FMath::Max(MyGeometry.Scale, KINDA_SMALL_NUMBER);

	// Placed by ALIGNMENT rather than by arithmetic on the card's size, because a UUserWidget's
	// GetDesiredSize stayed zero here however the card was built, and every sum that used it put
	// the card in the wrong place while looking perfectly reasonable in a log.
	//
	// An alignment of (0.5, 1) means "this point is the card's bottom centre", which is exactly
	// what sitting above a target means, and it needs no size at all. Flipped, (0.5, 0) makes the
	// point its top centre.
	const FVector2D TargetCentreTop(LocalTopLeft.X + LocalSize.X * 0.5f, LocalTopLeft.Y);
	const FVector2D TargetCentreBottom(TargetCentreTop.X, LocalTopLeft.Y + LocalSize.Y);

	// Above unless there is no room, which is the mockup's `bottom:104%` with its `tip-below` flip.
	// WrapWidth is the card's own fixed width, so the height it needs is at most a few lines of it.
	bFlipped = TargetCentreTop.Y < EstimatedCardHeight + AnchorGap;

	const float X = TargetCentreTop.X;
	const float Y = bFlipped ? TargetCentreBottom.Y + AnchorGap : TargetCentreTop.Y - AnchorGap;
	CardSlot->SetAlignment(FVector2D(0.5f, bFlipped ? 0.f : 1.f));

	CardSlot->SetPosition(FVector2D(X, Y));

	// The slide half of `transition: opacity .12s, transform .12s`: the card drifts into place as
	// it fades in, and away in the direction it came from. Downwards when it sits above the
	// target, upwards when it has flipped below, so it always appears to come out of the target.
	const float Slide = (1.f - Alpha) * SlideDistance;
	Card->SetRenderTranslation(FVector2D(0.f, bFlipped ? -Slide : Slide));
}

namespace
{
	/** Depth-first through the LIVE widget hierarchy, which is not the same thing as a WidgetTree.
	 *  A WidgetTree holds only what was authored at design time, so a layer added at run time is
	 *  invisible to it - which is how the first version managed to create a second layer on every
	 *  screen it had already set up, and why a test could not find the layer that existed. */
	UELMTHoverCardLayer* FindLayerInLiveTree(UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}
		if (UELMTHoverCardLayer* Layer = Cast<UELMTHoverCardLayer>(Widget))
		{
			return Layer;
		}
		if (UUserWidget* AsUser = Cast<UUserWidget>(Widget))
		{
			if (UELMTHoverCardLayer* Found = FindLayerInLiveTree(AsUser->GetRootWidget()))
			{
				return Found;
			}
		}
		if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			const int32 Count = Panel->GetChildrenCount();
			for (int32 Index = 0; Index < Count; ++Index)
			{
				if (UELMTHoverCardLayer* Found = FindLayerInLiveTree(Panel->GetChildAt(Index)))
				{
					return Found;
				}
			}
		}
		return nullptr;
	}
}

UELMTHoverCardLayer* UELMTHoverCardLayer::FindLayerFor(UWidget* Context)
{
	if (!Context)
	{
		return nullptr;
	}

	// Out to the owning screen, then down through what is actually on it.
	UUserWidget* Owner = Cast<UUserWidget>(Context);
	if (!Owner)
	{
		Owner = Context->GetTypedOuter<UUserWidget>();
	}
	while (Owner)
	{
		if (UELMTHoverCardLayer* Found = FindLayerInLiveTree(Owner))
		{
			return Found;
		}
		Owner = Owner->GetTypedOuter<UUserWidget>();
	}
	return nullptr;
}
