// Element UI Rich Text - styled rich text block using CommonUI button/text styles
#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Components/RichTextBlock.h"
#include "ELMTRichText.generated.h"

/**
 * Style container for the Element UI Rich Text.
 * Allows reusing a CommonButtonStyle's text attributes for consistency.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class UMGELMTKIT_API UELMTRichTextStyle : public UObject
{
	GENERATED_BODY()

public:
	/** Optional Common Button Style to pull text styling (font, color, shadow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Style)
	TSubclassOf<UCommonButtonStyle> CommonButtonStyle;

	/** Base text style applied to the rich text block */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Style)
	FTextBlockStyle TextStyle;
};

/**
 * Element UI Rich Text widget.
 * Text-only alternative to the Element UI Button base; supports rich text markup.
 */
UCLASS(meta = (DisplayName = "Element UI Rich Text"))
class UMGELMTKIT_API UELMTRichText : public URichTextBlock
{
	GENERATED_BODY()

public:
	/** Style definition (optional). If set, pulled in during SynchronizeProperties. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UELMTRichTextStyle> RichTextStyle;

	/** Optional Overwrite Text Style to use a Common Text Style directly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element UI")
	TSubclassOf<class UCommonTextStyle> OverwriteTextStyle;

	/** If true, the text color will be overridden by TextColorOverride */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Row, meta = (InlineEditConditionToggle))
	bool bOverrideTextColor = false;

	/** The color to use if bOverrideTextColor is true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Row, meta = (EditCondition = "bOverrideTextColor"))
	FSlateColor TextColorOverride;

	virtual void SynchronizeProperties() override;
};




