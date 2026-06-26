// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTIconButton.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "Input/Events.h"

UELMTIconButton::UELMTIconButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnPressed.AddDynamic(this, &UELMTIconButton::HandleOnPressed);
	OnReleased.AddDynamic(this, &UELMTIconButton::HandleOnReleased);
}

void UELMTIconButton::SynchronizeProperties()
{
	// UButton is RF_Transactional. In game worlds (including PIE) that causes
	// Modify() calls inside SetStyle/AddChild to write PIE widget refs into the
	// undo TransBuffer, blocking GC after PIE ends (PlayLevel.cpp:544 assert).
	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		ClearFlags(RF_Transactional);
	}

	Super::SynchronizeProperties();

	if (!IconImage)
	{
		SetupIconImage();
	}

	UpdateIcon();

	if (IconButtonStyle)
	{
		const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject();
		if (StyleObj)
		{
			if (IconImage)
			{
				IconImage->SetDesiredSizeOverride(StyleObj->IconSize);
				if (!FeedbackTimerHandle.IsValid())
				{
					IconImage->SetColorAndOpacity(IconColor);
					IconImage->SetRenderScale(FVector2D::UnitVector);
				}
			}

			SetStyle(StyleObj->ButtonStyle);
		}
	}

	if (!ButtonTooltip.IsEmpty())
	{
		SetToolTipText(ButtonTooltip);
	}
}

void UELMTIconButton::SetIcon(UTexture2D* NewIcon)
{
	Icon = NewIcon;
	UpdateIcon();
}

void UELMTIconButton::SetIconColor(FLinearColor NewColor)
{
	IconColor = NewColor;
	if (IconImage && !FeedbackTimerHandle.IsValid())
	{
		IconImage->SetColorAndOpacity(IconColor);
	}
}

void UELMTIconButton::SetIconSize(FVector2D NewSize)
{
	if (IconImage)
	{
		IconImage->SetDesiredSizeOverride(NewSize);
	}
}

FVector2D UELMTIconButton::GetIconSize() const
{
	if (IconImage)
	{
		return IconImage->GetDesiredSize();
	}
	return FVector2D::ZeroVector;
}

void UELMTIconButton::SetIconWidth(float Width)
{
	if (IconImage)
	{
		FVector2D CurrentSize = IconImage->GetDesiredSize();
		IconImage->SetDesiredSizeOverride(FVector2D(Width, CurrentSize.Y));
	}
}

void UELMTIconButton::SetIconHeight(float Height)
{
	if (IconImage)
	{
		FVector2D CurrentSize = IconImage->GetDesiredSize();
		IconImage->SetDesiredSizeOverride(FVector2D(CurrentSize.X, Height));
	}
}

void UELMTIconButton::SimulateHover(bool bHover)
{
	if (bHover == bIsHoverSimulated)
	{
		return;
	}

	bIsHoverSimulated = bHover;

	if (bHover)
	{
		CachedStyleBeforeSimulation = GetStyle();
		FButtonStyle ForcedStyle = CachedStyleBeforeSimulation;
		ForcedStyle.Normal = ForcedStyle.Hovered;
		SetStyle(ForcedStyle);
		OnSimulatedHoverBegin.Broadcast();
	}
	else
	{
		SetStyle(CachedStyleBeforeSimulation);
		OnSimulatedHoverEnd.Broadcast();
	}
}

void UELMTIconButton::ToggleSimulatedHover()
{
	SimulateHover(!bIsHoverSimulated);
}

void UELMTIconButton::ClearSimulatedHover()
{
	SimulateHover(false);
}

void UELMTIconButton::UpdateIcon()
{
	if (!IconImage)
	{
		SetupIconImage();
	}

	if (IconImage && Icon)
	{
		IconImage->SetBrushFromTexture(Icon, true);
		if (!FeedbackTimerHandle.IsValid())
		{
			IconImage->SetColorAndOpacity(IconColor);
		}
	}
}

void UELMTIconButton::SetupIconImage()
{
	if (!IconImage)
	{
		IconImage = NewObject<UImage>(this, UImage::StaticClass());
		if (IconImage)
		{
			IconImage->SetVisibility(ESlateVisibility::Visible);
			IconImage->SetRenderOpacity(1.0f);

			if (IconButtonStyle)
			{
				const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject();
				if (StyleObj)
				{
					IconImage->SetDesiredSizeOverride(StyleObj->IconSize);
				}
			}

			AddChild(IconImage);
		}
	}
}

void UELMTIconButton::HandleOnPressed()
{
	if (GetActiveFeedbackType() == EELMTButtonFeedbackType::None)
	{
		return;
	}

	bIsPressedState = true;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(FeedbackTimerHandle, this, &UELMTIconButton::TickFeedbackAnimation, 0.01f, true);
	}
}

void UELMTIconButton::HandleOnReleased()
{
	bIsPressedState = false;
}

void UELMTIconButton::TickFeedbackAnimation()
{
	EELMTButtonFeedbackType ActiveType = GetActiveFeedbackType();
	if (ActiveType == EELMTButtonFeedbackType::None)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FeedbackTimerHandle);
		}
		return;
	}

	float TargetScale = 1.0f;
	FVector2D TargetOffset = FVector2D::ZeroVector;
	float TargetRotation = 0.0f;
	FLinearColor TargetColor = IconColor;

	if (bIsPressedState)
	{
		switch (ActiveType)
		{
		case EELMTButtonFeedbackType::Bounce:
			TargetScale = GetActiveFeedbackScaleTarget();
			break;
		case EELMTButtonFeedbackType::Flash:
			TargetColor = GetActiveFeedbackColorTarget();
			break;
		case EELMTButtonFeedbackType::Offset:
			TargetOffset = GetActiveFeedbackOffsetTarget();
			break;
		case EELMTButtonFeedbackType::RotateShake:
			TargetRotation = GetActiveFeedbackRotationTarget();
			break;
		default:
			break;
		}
	}

	float DeltaTime = 0.01f;
	if (UWorld* World = GetWorld())
	{
		DeltaTime = World->GetDeltaSeconds();
		if (DeltaTime <= 0.0f)
		{
			DeltaTime = 0.01f;
		}
	}

	float InterpSpeed = 15.0f / FMath::Max(0.01f, GetActiveFeedbackDuration());

	CurrentFeedbackScale = FMath::FInterpTo(CurrentFeedbackScale, TargetScale, DeltaTime, InterpSpeed);
	CurrentFeedbackOffset = FMath::Vector2DInterpTo(CurrentFeedbackOffset, TargetOffset, DeltaTime, InterpSpeed);
	CurrentFeedbackRotation = FMath::FInterpTo(CurrentFeedbackRotation, TargetRotation, DeltaTime, InterpSpeed);
	CurrentFeedbackColor = FMath::CInterpTo(CurrentFeedbackColor, TargetColor, DeltaTime, InterpSpeed);

	bool bIsWhole = GetActiveAnimateWholeButton();

	if (bIsWhole)
	{
		SetRenderScale(FVector2D(CurrentFeedbackScale, CurrentFeedbackScale));
		SetRenderTranslation(CurrentFeedbackOffset);
		SetRenderTransformAngle(CurrentFeedbackRotation);
		if (IconImage)
		{
			IconImage->SetColorAndOpacity(CurrentFeedbackColor);
		}
	}
	else
	{
		if (IconImage)
		{
			IconImage->SetRenderScale(FVector2D(CurrentFeedbackScale, CurrentFeedbackScale));
			IconImage->SetRenderTranslation(CurrentFeedbackOffset);
			IconImage->SetRenderTransformAngle(CurrentFeedbackRotation);
			IconImage->SetColorAndOpacity(CurrentFeedbackColor);
		}
	}

	if (!bIsPressedState &&
		FMath::IsNearlyEqual(CurrentFeedbackScale, 1.0f, 0.005f) &&
		CurrentFeedbackOffset.IsNearlyZero(0.1f) &&
		FMath::IsNearlyEqual(CurrentFeedbackRotation, 0.0f, 0.05f) &&
		CurrentFeedbackColor.Equals(IconColor, 0.005f))
	{
		CurrentFeedbackScale = 1.0f;
		CurrentFeedbackOffset = FVector2D::ZeroVector;
		CurrentFeedbackRotation = 0.0f;
		CurrentFeedbackColor = IconColor;

		if (bIsWhole)
		{
			SetRenderScale(FVector2D::UnitVector);
			SetRenderTranslation(FVector2D::ZeroVector);
			SetRenderTransformAngle(0.0f);
		}
		else if (IconImage)
		{
			IconImage->SetRenderScale(FVector2D::UnitVector);
			IconImage->SetRenderTranslation(FVector2D::ZeroVector);
			IconImage->SetRenderTransformAngle(0.0f);
			IconImage->SetColorAndOpacity(IconColor);
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FeedbackTimerHandle);
		}
	}
}

EELMTButtonFeedbackType UELMTIconButton::GetActiveFeedbackType() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackType;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackType;
		}
	}
	return EELMTButtonFeedbackType::Bounce; // Default to Bounce
}

float UELMTIconButton::GetActiveFeedbackDuration() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackDuration;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackDuration;
		}
	}
	return 0.2f;
}

float UELMTIconButton::GetActiveFeedbackScaleTarget() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackScaleTarget;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackScaleTarget;
		}
	}
	return 0.85f;
}

FVector2D UELMTIconButton::GetActiveFeedbackOffsetTarget() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackOffsetTarget;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackOffsetTarget;
		}
	}
	return FVector2D(0.0f, 4.0f);
}

FLinearColor UELMTIconButton::GetActiveFeedbackColorTarget() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackColorTarget;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackColorTarget;
		}
	}
	return FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
}

float UELMTIconButton::GetActiveFeedbackRotationTarget() const
{
	if (bOverrideFeedbackSettings)
	{
		return FeedbackRotationTarget;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->FeedbackRotationTarget;
		}
	}
	return 15.0f;
}

bool UELMTIconButton::GetActiveAnimateWholeButton() const
{
	if (bOverrideFeedbackSettings)
	{
		return bAnimateWholeButton;
	}
	if (IconButtonStyle)
	{
		if (const UELMTIconButtonStyle* StyleObj = IconButtonStyle.GetDefaultObject())
		{
			return StyleObj->bAnimateWholeButton;
		}
	}
	return false;
}
