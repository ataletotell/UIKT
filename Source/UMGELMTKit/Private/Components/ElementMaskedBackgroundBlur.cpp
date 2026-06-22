/**
 * Copyright (c) 2024 UIKT. All Rights Reserved.
 */

#include "Components/ElementMaskedBackgroundBlur.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Slate/SRetainerWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

UElementMaskedBackgroundBlur::UElementMaskedBackgroundBlur()
{
	// Default dynamic masking settings
	EffectMaterial = nullptr;
	MaxRadius = 20.0f;
	Quality = 1.0f;
	JitterStrength = 0.5f;
	MaskTexture = nullptr;

	// Default native background capture blur settings
	BlurStrength = 5.0f;
	bApplyAlphaToBlur = true;
	Padding = FMargin(0.0f);
}

TSharedRef<SWidget> UElementMaskedBackgroundBlur::RebuildWidget()
{
	// 1. Construct the Slate Retainer Widget directly
	MyRetainerWidget = SNew(SRetainerWidget);

	// Automatically configure the URetainerBox texture parameter to match our custom HLSL input "Tex"
	MyRetainerWidget->SetTextureParameter(FName(TEXT("Tex")));

	// 2. Construct our custom SBackgroundBlur widget
	MyBackgroundBlur = SNew(SBackgroundBlur)
		.BlurStrength(BlurStrength)
		.bApplyAlphaToBlur(bApplyAlphaToBlur)
		.Padding(Padding);

	// 3. Chain the content: Child Widget -> BackgroundBlur -> Retainer Box
	if (GetChildrenCount() > 0)
	{
		MyBackgroundBlur->SetContent(GetChildAt(0)->TakeWidget());
	}

	// 4. Inject the background blur into the retainer slot
	MyRetainerWidget->SetContent(MyBackgroundBlur.ToSharedRef());

	// 5. Apply the initial effect material
	if (EffectMaterial)
	{
		MyRetainerWidget->SetEffectMaterial(EffectMaterial);
	}

	return MyRetainerWidget.ToSharedRef();
}

void UElementMaskedBackgroundBlur::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// Push the effect material to the retainer widget
	if (MyRetainerWidget.IsValid())
	{
		MyRetainerWidget->SetEffectMaterial(EffectMaterial);
	}

	// Push background blur properties to the Slate widget if it exists
	if (MyBackgroundBlur.IsValid())
	{
		MyBackgroundBlur->SetBlurStrength(BlurStrength);
		MyBackgroundBlur->SetApplyAlphaToBlur(bApplyAlphaToBlur);
		MyBackgroundBlur->SetPadding(Padding);
	}

	// Update the custom mask and radius settings on the Dynamic Material Instance
	UpdateMaterialParameters();
}

void UElementMaskedBackgroundBlur::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	// Safe memory cleanup
	MyRetainerWidget.Reset();
	MyBackgroundBlur.Reset();
}

void UElementMaskedBackgroundBlur::SetEffectMaterial(UMaterialInterface* InEffectMaterial)
{
	if (EffectMaterial != InEffectMaterial)
	{
		EffectMaterial = InEffectMaterial;
		if (MyRetainerWidget.IsValid())
		{
			MyRetainerWidget->SetEffectMaterial(EffectMaterial);
		}
		UpdateMaterialParameters();
	}
}

void UElementMaskedBackgroundBlur::SetMaskTexture(UTexture2D* InMaskTexture)
{
	if (MaskTexture != InMaskTexture)
	{
		MaskTexture = InMaskTexture;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBackgroundBlur::SetMaxRadius(float InMaxRadius)
{
	if (MaxRadius != InMaxRadius)
	{
		MaxRadius = InMaxRadius;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBackgroundBlur::SetQuality(float InQuality)
{
	if (Quality != InQuality)
	{
		Quality = InQuality;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBackgroundBlur::SetJitterStrength(float InJitterStrength)
{
	if (JitterStrength != InJitterStrength)
	{
		JitterStrength = InJitterStrength;
		UpdateMaterialParameters();
	}
}

void UElementMaskedBackgroundBlur::SetBlurStrength(float InBlurStrength)
{
	if (BlurStrength != InBlurStrength)
	{
		BlurStrength = InBlurStrength;
		if (MyBackgroundBlur.IsValid())
		{
			MyBackgroundBlur->SetBlurStrength(BlurStrength);
		}
	}
}

void UElementMaskedBackgroundBlur::SetApplyAlphaToBlur(bool InbApplyAlphaToBlur)
{
	if (bApplyAlphaToBlur != InbApplyAlphaToBlur)
	{
		bApplyAlphaToBlur = InbApplyAlphaToBlur;
		if (MyBackgroundBlur.IsValid())
		{
			MyBackgroundBlur->SetApplyAlphaToBlur(bApplyAlphaToBlur);
		}
	}
}

void UElementMaskedBackgroundBlur::SetPadding(FMargin InPadding)
{
	Padding = InPadding;
	if (MyBackgroundBlur.IsValid())
	{
		MyBackgroundBlur->SetPadding(Padding);
	}
}

void UElementMaskedBackgroundBlur::UpdateMaterialParameters()
{
	if (MyRetainerWidget.IsValid())
	{
		UMaterialInstanceDynamic* DynamicMaterial = MyRetainerWidget->GetEffectMaterial();
		if (DynamicMaterial)
		{
			DynamicMaterial->SetTextureParameterValue(TEXT("Mask"), MaskTexture);
			DynamicMaterial->SetScalarParameterValue(TEXT("MaxRadius"), MaxRadius);
			DynamicMaterial->SetScalarParameterValue(TEXT("Quality"), Quality);
			DynamicMaterial->SetScalarParameterValue(TEXT("JitterStrength"), JitterStrength);
		}
	}
}

#if WITH_EDITOR
const FText UElementMaskedBackgroundBlur::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "UIKT", "UIKT");
}
#endif
