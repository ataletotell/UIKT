/**
 * Ported from UAnimatedTexture5
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Tickable.h"
#include "ElementAnimatedTexture.generated.h"

class FElementAnimatedTextureDecoder;

UENUM()
enum class EElementAnimatedTextureType : uint8
{
	None,
	Gif,
	Webp
};

/**
 * Element Animated Texture
 */
UCLASS(BlueprintType, Category = "Element Animated Texture")
class UMGELMTKIT_API UElementAnimatedTexture : public UTexture, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture", meta = (DisplayName = "X-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture", meta = (DisplayName = "Y-axis Tiling Method"), AssetRegistrySearchable, AdvancedDisplay)
		TEnumAsByte<enum TextureAddress> AddressY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture")
		bool SupportsTransparency = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture")
		float DefaultFrameDelay = 1.0f / 10;	// used while Frame.Delay==0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture")
		float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element Animated Texture")
		bool bLooping = true;

public:	// Playback APIs
	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void Play();

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void PlayFromStart();

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void Stop();

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		bool IsPlaying() const { return bPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void PlayFromFrame(int32 FrameIndex);

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void SetCurrentFrame(int32 FrameIndex);

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void SetLooping(bool bNewLooping) { bLooping = bNewLooping; }

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		bool IsLooping() const { return bLooping; }

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		void SetPlayRate(float NewRate) { PlayRate = NewRate; }

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		float GetPlayRate() const { return PlayRate; }

	UFUNCTION(BlueprintCallable, Category = "Element Animated Texture")
		float GetAnimationLength() const;

public:	// UTexture Interface
	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;
	virtual float GetSurfaceDepth() const override { return 0; }
	virtual uint32 GetSurfaceArraySize() const override { return 0; }
	virtual ETextureClass GetTextureClass() const override { return ETextureClass::TwoDDynamic; }

	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }

public:	// FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UElementAnimatedTexture, STATGROUP_Tickables);
	}
	virtual bool IsTickableInEditor() const
	{
		return true;
	}

	virtual UWorld* GetTickableGameObjectWorld() const
	{
		return GetWorld();
	}
public:	// UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public: // Internal APIs
	void ImportFile(EElementAnimatedTextureType InFileType, const uint8* InBuffer, uint32 InBufferSize);

	float RenderFrameToTexture();

private:
	UPROPERTY()
		EElementAnimatedTextureType FileType = EElementAnimatedTextureType::None;

	UPROPERTY()
		TArray<uint8> FileBlob;

private:
	TSharedPtr<FElementAnimatedTextureDecoder, ESPMode::ThreadSafe> Decoder;

	float AnimationLength = 0.0f;
	float FrameDelay = 0.0f;
	float FrameTime = 0.0f;
	bool bPlaying = true;
};
