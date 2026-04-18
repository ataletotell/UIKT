#include "ELMTRichText.h"

#include "CommonTextBlock.h"

void UELMTRichText::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (OverwriteTextStyle)
	{
		if (const UCommonTextStyle* StyleCDO = OverwriteTextStyle.GetDefaultObject())
		{
			FTextBlockStyle RefinedStyle;
			StyleCDO->ToTextBlockStyle(RefinedStyle);
			
			if (bOverrideTextColor)
			{
				RefinedStyle.ColorAndOpacity = TextColorOverride;
			}
			
			SetDefaultTextStyle(RefinedStyle);
		}
		return;
	}

	if (!RichTextStyle)
	{
		return;
	}

	const TObjectPtr<UELMTRichTextStyle> StyleCDO = RichTextStyle.GetDefaultObject();

	// If a CommonButtonStyle is provided, pull its normal text style
	if (StyleCDO->CommonButtonStyle)
	{
		const TObjectPtr<UCommonButtonStyle> ButtonCDO = StyleCDO->CommonButtonStyle.GetDefaultObject();
		if (const TObjectPtr<UCommonTextStyle> ButtonTextStyle = ButtonCDO->GetNormalTextStyle())
		{
			// Convert CommonTextStyle to a TextBlockStyle
			FTextBlockStyle DerivedStyle;
			ButtonTextStyle->ToTextBlockStyle(DerivedStyle);
			StyleCDO->TextStyle = DerivedStyle;
		}
	}

	// Apply the text style to this rich text block
	FTextBlockStyle FinalStyle = StyleCDO->TextStyle;
	if (bOverrideTextColor)
	{
		FinalStyle.ColorAndOpacity = TextColorOverride;
	}
	SetDefaultTextStyle(FinalStyle);
}

