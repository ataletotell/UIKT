/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "ElementAnimatedImage.generated.h"

class UElementAnimatedTexture;

/**
 * UMG Widget that extends UImage to provide easy access to Animated Texture controls.
 */
UCLASS(meta = (DisplayName = "Element Animated Image"))
class UMGELMTKIT_API UElementAnimatedImage : public UImage
{
	GENERATED_BODY()

public:
	/** Optionally set an animated texture to play automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Image")
	bool bAutoPlay = true;

public:
	/** Starts playing the currently assigned animated texture. */
	UFUNCTION(BlueprintCallable, Category = "Element Animated Image")
	void Play();

	/** Starts playing from the specified frame index. */
	UFUNCTION(BlueprintCallable, Category = "Element Animated Image")
	void PlayFromFrame(int32 FrameIndex);

	/** Stops (pauses) the currently assigned animated texture. */
	UFUNCTION(BlueprintCallable, Category = "Element Animated Image")
	void Stop();

	/** Pauses the currently assigned animated texture (Alias for Stop). */
	UFUNCTION(BlueprintCallable, Category = "Element Animated Image")
	void Pause();

	/** Sets the brush to the given animated texture and optionally plays it. */
	UFUNCTION(BlueprintCallable, Category = "Element Animated Image")
	void SetBrushFromAnimatedTexture(UElementAnimatedTexture* Texture, bool bMatchSize = true);

protected:
	virtual void SynchronizeProperties() override;

private:
	UElementAnimatedTexture* GetAnimatedTexture() const;

	/** Independent instance of the texture used for runtime playback */
	UPROPERTY(Transient)
	UElementAnimatedTexture* IndependentTexture;
};
