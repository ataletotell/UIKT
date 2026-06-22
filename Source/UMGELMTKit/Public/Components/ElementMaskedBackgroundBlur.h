/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "ElementMaskedBackgroundBlur.generated.h"

class UTexture2D;
class UMaterialInterface;
class SBackgroundBlur;
class SRetainerWidget;

/**
 * A premium UIKT widget that captures and blurs the gameplay screen or UI behind it (exactly like native UBackgroundBlur),
 * but automatically applies a custom mask texture for localized screen blurring (e.g. glasses lenses, frosted shapes).
 */
UCLASS(meta = (DisplayName = "Element Masked Background Blur"))
class UMGELMTKIT_API UElementMaskedBackgroundBlur : public UContentWidget
{
	GENERATED_BODY()

public:
	UElementMaskedBackgroundBlur();

	// ── MASKED BLUR PROPERTIES ───────────────────────────────────────────────

	/** The material to apply as the masked blur effect (should use our UIBlur HLSL shader) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur")
	UMaterialInterface* EffectMaterial;

	/** The grayscale mask texture (White = blurred/opaque, Black = sharp/transparent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur")
	UTexture2D* MaskTexture;

	/** Maximum mask blur radius in pixels (for the custom material shader) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0"))
	float MaxRadius;

	/** Custom shader Quality: 0 = Low (8 taps), 1 = Med (16 taps), 2 = High (32 taps), 3 = Epic (64 taps) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float Quality;

	/** Dither noise jitter strength (0.0 = off, 1.0 = full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masked Blur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float JitterStrength;

	// ── NATIVE BACKGROUND BLUR PROPERTIES ────────────────────────────────────

	/** How strong the native background capture-blur is (0.0 = no blur, 100.0 = max blur) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background Blur", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BlurStrength;

	/** Whether to apply alpha to the native background blur */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background Blur")
	bool bApplyAlphaToBlur;

	/** The padding around the child widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background Blur")
	FMargin Padding;

public:
	// ── RUNTIME SETTERS ──────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetEffectMaterial(UMaterialInterface* InEffectMaterial);

	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetMaskTexture(UTexture2D* InMaskTexture);

	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetMaxRadius(float InMaxRadius);

	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetQuality(float InQuality);

	UFUNCTION(BlueprintCallable, Category = "Masked Blur")
	void SetJitterStrength(float InJitterStrength);

	UFUNCTION(BlueprintCallable, Category = "Background Blur")
	void SetBlurStrength(float InBlurStrength);

	UFUNCTION(BlueprintCallable, Category = "Background Blur")
	void SetApplyAlphaToBlur(bool InbApplyAlphaToBlur);

	UFUNCTION(BlueprintCallable, Category = "Background Blur")
	void SetPadding(FMargin InPadding);

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

private:
	/** Internal helper to push masking parameters to the Dynamic Material Instance */
	void UpdateMaterialParameters();

	/** Reference to the native Slate retainer widget that compiles and draws our material */
	TSharedPtr<SRetainerWidget> MyRetainerWidget;

	/** Reference to the native Slate background blur widget */
	TSharedPtr<SBackgroundBlur> MyBackgroundBlur;
};
