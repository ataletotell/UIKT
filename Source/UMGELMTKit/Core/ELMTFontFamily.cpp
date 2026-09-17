// Fill out your copyright notice in the Description page of Project Settings.

#include "ELMTFontFamily.h"

#include "Engine/Font.h"

void UELMTFontFamily::RegisterWeight(EELMTFontWeight Weight, bool bItalic, UFont* Font)
{
	if (!Font)
	{
		return;
	}
	(bItalic ? ItalicFonts : UprightFonts).Add(Weight, Font);
}

UFont* UELMTFontFamily::GetFont(EELMTFontWeight Weight, bool bItalic) const
{
	const TMap<EELMTFontWeight, TObjectPtr<UFont>>& SameStyle = bItalic ? ItalicFonts : UprightFonts;

	if (const TObjectPtr<UFont>* Exact = SameStyle.Find(Weight))
	{
		return *Exact;
	}

	// nearest lighter-or-equal weight in the same style
	for (int32 Candidate = static_cast<int32>(Weight) - 1; Candidate >= 0; --Candidate)
	{
		if (const TObjectPtr<UFont>* Found = SameStyle.Find(static_cast<EELMTFontWeight>(Candidate)))
		{
			return *Found;
		}
	}

	// Regular in the same style
	if (const TObjectPtr<UFont>* RegularSameStyle = SameStyle.Find(EELMTFontWeight::Regular))
	{
		return *RegularSameStyle;
	}

	// opposite style at the same weight, then its Regular
	const TMap<EELMTFontWeight, TObjectPtr<UFont>>& OtherStyle = bItalic ? UprightFonts : ItalicFonts;
	if (const TObjectPtr<UFont>* OtherExact = OtherStyle.Find(Weight))
	{
		return *OtherExact;
	}
	if (const TObjectPtr<UFont>* OtherRegular = OtherStyle.Find(EELMTFontWeight::Regular))
	{
		return *OtherRegular;
	}

	return nullptr;
}

FSlateFontInfo UELMTFontFamily::MakeFontInfo(EELMTFontWeight Weight, bool bItalic, float Size) const
{
	FSlateFontInfo Info;
	Info.FontObject = GetFont(Weight, bItalic);
	Info.Size = Size;
	return Info;
}
