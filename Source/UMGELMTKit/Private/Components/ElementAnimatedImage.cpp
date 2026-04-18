/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#include "Components/ElementAnimatedImage.h"
#include "ElementAnimatedTexture.h"
#include "Engine/World.h"

void UElementAnimatedImage::Play()
{
	if (UElementAnimatedTexture* AnimTexture = GetAnimatedTexture())
	{
		AnimTexture->Play();
	}
}

void UElementAnimatedImage::PlayFromFrame(int32 FrameIndex)
{
	if (UElementAnimatedTexture* AnimTexture = GetAnimatedTexture())
	{
		AnimTexture->PlayFromFrame(FrameIndex);
	}
}

void UElementAnimatedImage::Stop()
{
	if (UElementAnimatedTexture* AnimTexture = GetAnimatedTexture())
	{
		AnimTexture->Stop();
	}
}

void UElementAnimatedImage::Pause()
{
	Stop();
}

void UElementAnimatedImage::SetBrushFromAnimatedTexture(UElementAnimatedTexture* Texture, bool bMatchSize)
{
	if (Texture)
	{
		// In game world, use an independent copy to allow separate playback control
		UWorld* World = GetWorld();
		if (World && (World->IsGameWorld() || World->IsPreviewWorld()))
		{
			// Create a duplicate if we don't have one or if the source changed
			if (IndependentTexture == nullptr || IndependentTexture->GetClass() != Texture->GetClass() || IndependentTexture->GetName() != Texture->GetName())
			{
				IndependentTexture = DuplicateObject<UElementAnimatedTexture>(Texture, this);
			}
			Texture = IndependentTexture;
		}

		FSlateBrush NewBrush = GetBrush();
		NewBrush.SetResourceObject(Texture);

		if (bMatchSize)
		{
			NewBrush.ImageSize.X = Texture->GetSurfaceWidth();
			NewBrush.ImageSize.Y = Texture->GetSurfaceHeight();
		}

		SetBrush(NewBrush);

		if (bAutoPlay)
		{
			Texture->Play();
		}
	}
	else
	{
		IndependentTexture = nullptr;
		FSlateBrush NewBrush = GetBrush();
		NewBrush.SetResourceObject(nullptr);
		SetBrush(NewBrush);
	}
}

void UElementAnimatedImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// Check if a texture was assigned via properties (e.g. in Editor)
	// and ensure we switch to an independent copy if we are in game
	UElementAnimatedTexture* AssignedTexture = Cast<UElementAnimatedTexture>(GetBrush().GetResourceObject());
	if (AssignedTexture && AssignedTexture != IndependentTexture)
	{
		SetBrushFromAnimatedTexture(AssignedTexture, false);
	}
	else if (bAutoPlay)
	{
		Play();
	}
}

UElementAnimatedTexture* UElementAnimatedImage::GetAnimatedTexture() const
{
	if (IndependentTexture)
	{
		return IndependentTexture;
	}

	if (UObject* ResourceObject = GetBrush().GetResourceObject())
	{
		return Cast<UElementAnimatedTexture>(ResourceObject);
	}
	return nullptr;
}
