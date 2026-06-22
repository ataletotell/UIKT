/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/RetainerBox.h"
#include "ElementMaskedBlur.generated.h"

class UTexture2D;

/**
 * A custom UMG Retainer Box widget that automates background/children masking and blurring
 * using the custom /LanternRed/UI/UIBlur.ush HLSL shader.
 */
UCLASS(meta = (DisplayName = "Element Masked Blur"))
class UMGELMTKIT_API UElementMaskedBlur : public URetainerBox
{
	GENERATED_BODY()

public:
	UElementMaskedBlur();

	/** The grayscale mask texture (White = blurred/opaque, Black = sharp/transparent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur")
	UTexture2D* MaskTexture;

	/** Maximum blur radius in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0"))
	float MaxRadius;

	/** Blur Quality: 0 = Low (8 taps), 1 = Med (16 taps), 2 = High (32 taps), 3 = Epic (64 taps) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float Quality;

	/** Jitter dither noise intensity to break up banding (0.0 = off, 1.0 = full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float JitterStrength;

public:
	/** Dynamically sets the mask texture at runtime */
	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetMaskTexture(UTexture2D* InMaskTexture);

	/** Dynamically sets the maximum blur radius at runtime */
	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetMaxRadius(float InMaxRadius);

	/** Dynamically sets the blur quality tier at runtime */
	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetQuality(float InQuality);

	/** Dynamically sets the jitter dither strength at runtime */
	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetJitterStrength(float InJitterStrength);

protected:
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

private:
	/** Internal helper to push current properties into the dynamic material instance */
	void UpdateMaterialParameters();
};
