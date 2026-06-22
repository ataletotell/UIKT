/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#include "Components/ElementMaskedBlur.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

UElementMaskedBlur::UElementMaskedBlur()
{
	// Default settings matching standard high-quality presets
	MaxRadius = 20.0f;
	Quality = 1.0f;
	JitterStrength = 0.5f;
	MaskTexture = nullptr;

	// Automatically configure the URetainerBox texture parameter to match our custom HLSL input "Tex"
	SetTextureParameter(FName(TEXT("Tex")));
}

void UElementMaskedBlur::SetMaskTexture(UTexture2D* InMaskTexture)
{
	if (MaskTexture != InMaskTexture)
	{
		MaskTexture = InMaskTexture;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBlur::SetMaxRadius(float InMaxRadius)
{
	if (MaxRadius != InMaxRadius)
	{
		MaxRadius = InMaxRadius;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBlur::SetQuality(float InQuality)
{
	if (Quality != InQuality)
	{
		Quality = InQuality;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBlur::SetJitterStrength(float InJitterStrength)
{
	if (JitterStrength != InJitterStrength)
	{
		JitterStrength = InJitterStrength;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBlur::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// Push current property values to the dynamic material when UMG synchronizes properties (edit-time & begin play)
	UpdateMaterialParameters();
}

void UElementMaskedBlur::UpdateMaterialParameters()
{
	UMaterialInstanceDynamic* DynamicMaterial = GetEffectMaterial();
	if (DynamicMaterial)
	{
		DynamicMaterial->SetTextureParameterValue(TEXT("Mask"), MaskTexture);
		DynamicMaterial->SetScalarParameterValue(TEXT("MaxRadius"), MaxRadius);
		DynamicMaterial->SetScalarParameterValue(TEXT("Quality"), Quality);
		DynamicMaterial->SetScalarParameterValue(TEXT("JitterStrength"), JitterStrength);
	}
}

#if WITH_EDITOR
const FText UElementMaskedBlur::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "UIKT", "UIKT");
}
#endif
