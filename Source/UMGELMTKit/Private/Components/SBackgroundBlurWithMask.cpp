// Copyright Qibo Pang 2022. All Rights Reserved.

#include "Components/SBackgroundBlurWithMask.h"
#include "Render/BackgroundBlurDrawer.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"

static int32 bAllowBackgroundBlur      = 1;
static int32 MaxKernelSize             = 255;
static int32 bDownsampleForBlur        = 1;
static int32 bForceLowQualityBrushFallback = 0;

SBackgroundBlurWithMask::SBackgroundBlurWithMask()
	: BackgroundBlurDrawer(new FBackgroundBlurDrawer())
{
}

SBackgroundBlurWithMask::~SBackgroundBlurWithMask()
{
	ENQUEUE_RENDER_COMMAND(SafeDeleteBackgroundBlurDrawer)(
		[BackgroundBlurDrawer = BackgroundBlurDrawer](FRHICommandListImmediate& RHICmdList) mutable
		{
			BackgroundBlurDrawer.Reset();
		}
	);
}

void SBackgroundBlurWithMask::Construct(const FArguments& InArgs)
{
	bApplyAlphaToBlur        = InArgs._bApplyAlphaToBlur;
	LowQualityFallbackBrush  = InArgs._LowQualityFallbackBrush;
	BlurStrength             = InArgs._BlurStrength;
	BlurRadius               = InArgs._BlurRadius;

	ChildSlot
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.Padding(InArgs._Padding)
		[
			InArgs._Content.Widget
		];

	SetCanTick(false);
}

void SBackgroundBlurWithMask::SetContent(const TSharedRef<SWidget>& InContent)
{
	ChildSlot.AttachWidget(InContent);
}

void SBackgroundBlurWithMask::SetApplyAlphaToBlur(bool bInApplyAlphaToBlur)
{
	if (bApplyAlphaToBlur != bInApplyAlphaToBlur)
	{
		bApplyAlphaToBlur = bInApplyAlphaToBlur;
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SBackgroundBlurWithMask::SetBlurRadius(const TAttribute<TOptional<int32>>& InBlurRadius)
{
	if (!BlurRadius.IdenticalTo(InBlurRadius))
	{
		BlurRadius = InBlurRadius;
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SBackgroundBlurWithMask::SetBlurStrength(const TAttribute<float>& InStrength)
{
	if (!BlurStrength.IdenticalTo(InStrength))
	{
		BlurStrength = InStrength;
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SBackgroundBlurWithMask::SetMaskTexture(UTexture* InMaskTexture)
{
	if (MaskTexture != InMaskTexture)
	{
		MaskTexture = InMaskTexture;
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SBackgroundBlurWithMask::SetMaskTextureChannel(EMaskTextureChannel InMaskTextureChannel)
{
	if (MaskTextureChannel != InMaskTextureChannel)
	{
		MaskTextureChannel = InMaskTextureChannel;
		Invalidate(EInvalidateWidget::Layout);
	}
}

void SBackgroundBlurWithMask::SetMaskRevertAlpha(bool bInMaskRevertAlpha)
{
	if (bMaskRevertAlpha != bInMaskRevertAlpha)
	{
		bMaskRevertAlpha = bInMaskRevertAlpha;
		Invalidate(EInvalidateWidget::Layout);
	}
}

void SBackgroundBlurWithMask::SetLowQualityBackgroundBrush(const FSlateBrush* InBrush)
{
	if (LowQualityFallbackBrush != InBrush)
	{
		LowQualityFallbackBrush = InBrush;
		Invalidate(EInvalidateWidget::Paint);
	}
}

void SBackgroundBlurWithMask::SetHAlign(EHorizontalAlignment HAlign)
{
	ChildSlot.HAlign(HAlign);
	Invalidate(EInvalidateWidget::Layout);
}

void SBackgroundBlurWithMask::SetVAlign(EVerticalAlignment VAlign)
{
	ChildSlot.VAlign(VAlign);
	Invalidate(EInvalidateWidget::Layout);
}

void SBackgroundBlurWithMask::SetPadding(const TAttribute<FMargin>& InPadding)
{
	ChildSlot.Padding(InPadding);
	Invalidate(EInvalidateWidget::Layout);
}

bool SBackgroundBlurWithMask::IsUsingLowQualityFallbackBrush() const
{
	return bForceLowQualityBrushFallback == 1 || !FPlatformMisc::SupportsBackbufferSampling();
}

int32 SBackgroundBlurWithMask::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const bool bUsingLowQualityFallbackBrush = IsUsingLowQualityFallbackBrush();

	int32 PostFXLayerId = LayerId;
	const FVector2f LocalSize = AllottedGeometry.GetLocalSize();

	if (bAllowBackgroundBlur && LocalSize.X > 0.0f && LocalSize.Y > 0.0f)
	{
		if (!bUsingLowQualityFallbackBrush)
		{
			const float EffectiveAlpha = InWidgetStyle.GetColorAndOpacityTint().A;
			const float Strength = BlurStrength.Get() * (bApplyAlphaToBlur ? EffectiveAlpha : 1.f);
			if (Strength >= 0.5f && EffectiveAlpha > 0.15f)
			{
				FSlateRect RenderBoundingRect = AllottedGeometry.GetRenderBoundingRect();
				FPaintGeometry PaintGeometry(RenderBoundingRect.GetTopLeft(), RenderBoundingRect.GetSize(), 1.0f);

				int32 RenderTargetWidth  = FMath::RoundToInt(RenderBoundingRect.GetSize().X);
				int32 RenderTargetHeight = FMath::RoundToInt(RenderBoundingRect.GetSize().Y);

				int32 KernelSize = 0;
				int32 DownsampleAmount = 0;
				ComputeEffectiveKernelSize(Strength, KernelSize, DownsampleAmount);

				float ComputedStrength = FMath::Max(.5f, Strength);

				if (DownsampleAmount > 0)
				{
					RenderTargetWidth  = FMath::DivideAndRoundUp(RenderTargetWidth,  DownsampleAmount);
					RenderTargetHeight = FMath::DivideAndRoundUp(RenderTargetHeight, DownsampleAmount);
					ComputedStrength /= DownsampleAmount;
				}

				if (RenderTargetWidth > 0 && RenderTargetHeight > 0)
				{
					OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry));

					if (BackgroundBlurDrawer->InitializeBackgroundBlurParams(OutDrawElements, LayerId, PaintGeometry, FVector4(KernelSize, ComputedStrength, RenderTargetWidth, RenderTargetHeight), DownsampleAmount, MaskTexture, MaskTextureChannel, bMaskRevertAlpha, EffectiveAlpha))
					{
						FSlateDrawElement::MakeCustom(OutDrawElements, LayerId, BackgroundBlurDrawer);
					}

					OutDrawElements.PopClip();
				}

				++PostFXLayerId;
			}
		}
		else if (bAllowBackgroundBlur && bUsingLowQualityFallbackBrush && LowQualityFallbackBrush && LowQualityFallbackBrush->DrawAs != ESlateBrushDrawType::NoDrawType)
		{
			const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
			const FLinearColor FinalColorAndOpacity(InWidgetStyle.GetColorAndOpacityTint() * LowQualityFallbackBrush->GetTint(InWidgetStyle));

			FSlateDrawElement::MakeBox(OutDrawElements, PostFXLayerId, AllottedGeometry.ToPaintGeometry(), LowQualityFallbackBrush, DrawEffects, FinalColorAndOpacity);
			++PostFXLayerId;
		}
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, PostFXLayerId, InWidgetStyle, bParentEnabled);
}

void SBackgroundBlurWithMask::ComputeEffectiveKernelSize(float Strength, int32& OutKernelSize, int32& OutDownsampleAmount) const
{
	OutKernelSize = BlurRadius.Get().Get(FMath::RoundToInt(Strength * 3.f));

	if (bDownsampleForBlur && OutKernelSize > 9)
	{
		OutDownsampleAmount = OutKernelSize >= 64 ? 4 : 2;
		OutKernelSize /= OutDownsampleAmount;
	}

	if (OutKernelSize % 2 == 0)
	{
		++OutKernelSize;
	}

	OutKernelSize = FMath::Clamp(OutKernelSize, 3, MaxKernelSize);
}

FVector2D SBackgroundBlurWithMask::GetViewportSize(UObject* WorldContextObject) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World && World->IsGameWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			if (ViewportClient->GetGameLayerManager().IsValid())
			{
				FVector2D ViewportSize;
				ViewportClient->GetViewportSize(ViewportSize);
				return ViewportSize;
			}
		}
	}
	return FVector2D::ZeroVector;
}
