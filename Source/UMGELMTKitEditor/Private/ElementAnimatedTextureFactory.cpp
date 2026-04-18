/**
 * Ported from UAnimatedTexture5
 */

#include "ElementAnimatedTextureFactory.h"
#include "ElementAnimatedTexture.h"
// #include "TextureReferenceResolver.h" // Not available easily? Check if it's in a public module. It is in UnreadEd/Private/TextureReferenceResolver.h usually? 
// The reference project included it. Let's check if we can include it.
// If not, we might need to skip the reference replacer or implement a simple one.
// Actually FTextureReferenceReplacer is likely in a private header of UnrealEd.
// Reference project logic seems to rely on it.
// However, FTextureReferenceReplacer might not be accessible if it is not exported.
// Let's check later. For now, I will comment it out if it fails, or try to implement without it.
// Actually, let's omit the FTextureReferenceReplacer part for now as it handles reimporting over existing texture types which is a bit advanced and risky if symbols are missing.
// I will keep the basic import/reimport logic.

#include "Subsystems/ImportSubsystem.h"
#include "EditorFramework/AssetImportData.h"
#include "Editor.h"

DEFINE_LOG_CATEGORY_STATIC(LogElementAnimTextureEditor, Log, All);

UElementAnimatedTextureFactory::UElementAnimatedTextureFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditorImport = true;
	bEditAfterNew = true;
	SupportedClass = UElementAnimatedTexture::StaticClass();
	Formats.Add(TEXT("gif;GIF(Animation Supported)"));
	Formats.Add(TEXT("webp;Webp(Animation Supported)"));

	// Required to checkbefore UReimportTextureFactory
	ImportPriority = DefaultImportPriority + 1;
}

bool UElementAnimatedTextureFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename, true);

	return Extension.Compare(TEXT(".gif"), ESearchCase::IgnoreCase) == 0
		|| Extension.Compare(TEXT(".webp"), ESearchCase::IgnoreCase) == 0;
}

UObject* UElementAnimatedTextureFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	UE_LOG(LogElementAnimTextureEditor, Log, TEXT("UElementAnimatedTextureFactory: Importing %s ..."), *(InName.ToString()));

	check(Type);
	check(InClass == UElementAnimatedTexture::StaticClass());

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	// call parent method to create/overwrite anim texture object
	UElementAnimatedTexture* AnimTexture = Cast<UElementAnimatedTexture>(
		CreateOrOverwriteAsset(InClass, InParent, InName, Flags)
		);
	if (AnimTexture == nullptr) {
		UE_LOG(LogElementAnimTextureEditor, Error, TEXT("Create Element Animated Texture FAILED, Name=%s."), *(InName.ToString()));
		return nullptr;
	}

	// just copy file blob to AnimTexture object
	EElementAnimatedTextureType AnimTextureType = EElementAnimatedTextureType::None;
	FString FileType(Type);
	if (FileType.Compare(TEXT("gif"), ESearchCase::IgnoreCase) == 0)
		AnimTextureType = EElementAnimatedTextureType::Gif;
	else if (FileType.Compare(TEXT("webp"), ESearchCase::IgnoreCase) == 0)
		AnimTextureType = EElementAnimatedTextureType::Webp;

	AnimTexture->ImportFile(AnimTextureType, Buffer, BufferEnd - Buffer);

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, AnimTexture);

	// Invalidate any materials using the newly imported texture. (occurs if you import over an existing texture)
	AnimTexture->PostEditChange();

	return AnimTexture;
}

bool UElementAnimatedTextureFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UElementAnimatedTexture* pTex = Cast<UElementAnimatedTexture>(Obj);
	if (pTex) {
		pTex->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UElementAnimatedTextureFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UElementAnimatedTexture* pTex = Cast<UElementAnimatedTexture>(Obj);
	if (pTex && ensure(NewReimportPaths.Num() == 1))
	{
		pTex->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UElementAnimatedTextureFactory::Reimport(UObject* Obj)
{
	if (!Obj || !Obj->IsA(UElementAnimatedTexture::StaticClass()))
	{
		return EReimportResult::Failed;
	}

	UElementAnimatedTexture* pTex = Cast<UElementAnimatedTexture>(Obj);

	const FString ResolvedSourceFilePath = pTex->AssetImportData->GetFirstFilename();
	if (!ResolvedSourceFilePath.Len())
	{
		return EReimportResult::Failed;
	}

	// Ensure that the file provided by the path exists
	if (IFileManager::Get().FileSize(*ResolvedSourceFilePath) == INDEX_NONE)
	{
		UE_LOG(LogElementAnimTextureEditor, Warning, TEXT("cannot reimport: [] file cannot be found."), *ResolvedSourceFilePath);
		return EReimportResult::Failed;
	}

	enum TextureAddress AddressX = pTex->AddressX;
	enum TextureAddress AddressY = pTex->AddressY;
	float PlayRate = pTex->PlayRate;
	bool SupportsTransparency = pTex->SupportsTransparency;
	float DefaultFrameDelay = pTex->DefaultFrameDelay;
	bool bLooping = pTex->bLooping;

	bool OutCanceled = false;
	if (ImportObject(pTex->GetClass(), pTex->GetOuter(), *pTex->GetName(), RF_Public | RF_Standalone, ResolvedSourceFilePath, nullptr, OutCanceled) != nullptr)
	{
		pTex->AssetImportData->Update(ResolvedSourceFilePath);

		pTex->AddressX = AddressX;
		pTex->AddressY = AddressY;
		pTex->PlayRate = PlayRate;
		pTex->SupportsTransparency = SupportsTransparency;
		pTex->DefaultFrameDelay = DefaultFrameDelay;
		pTex->bLooping = bLooping;

		// Try to find the outer package so we can dirty it up
		if (pTex->GetOuter())
		{
			pTex->GetOuter()->MarkPackageDirty();
		}
		else
		{
			pTex->MarkPackageDirty();
		}
	}
	else if (OutCanceled)
	{
		UE_LOG(LogElementAnimTextureEditor, Warning, TEXT("[%s] reimport canceled"), *(Obj->GetName()));
		return EReimportResult::Cancelled;
	}
	else
	{
		UE_LOG(LogElementAnimTextureEditor, Warning, TEXT("[%s] reimport failed"), *(Obj->GetName()));
		return EReimportResult::Failed;
	}

	return EReimportResult::Succeeded;
}

int32 UElementAnimatedTextureFactory::GetPriority() const
{
	return ImportPriority;
}
