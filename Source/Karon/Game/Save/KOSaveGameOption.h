#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "KOSaveGameOption.generated.h"

USTRUCT(BlueprintType)
struct FKOSoundOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float Master = 0.8f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float BGM = 0.8f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float SE = 0.8f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float UI = 0.8f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float Environment = 0.8f;
};

USTRUCT(BlueprintType)
struct FKOGraphicsOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FIntPoint Resolution = FIntPoint(1920, 1080);

	/** 화면 해상도 스케일 (50 ~ 100) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float ResolutionScale = 100.0f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bFullscreen = true;

	/** FPS 상한. 0 = 제한 없음 */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 FrameLimit = 60;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bVSync = false;

	/** 0=낮음 1=보통 2=높음 3=최고 (기본값: 높음) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 ShadowQuality = 2;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 ViewDistanceQuality = 2;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 AntiAliasingQuality = 2;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 TextureQuality = 2;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 EffectsQuality = 2;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 ShadingQuality = 2;
};

UCLASS(BlueprintType)
class KARON_API UKOSaveGameOption : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;
	static constexpr int32 UserIndex = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FKOSoundOptions Sound;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FKOGraphicsOptions Graphics;

	/** SaveGame에서 로드하거나, 없으면 기본값으로 생성해 반환 */
	UFUNCTION(BlueprintCallable, Category = "KO|Option", meta = (WorldContext = "WorldContextObject"))
	static UKOSaveGameOption* LoadOrCreate(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "KO|Option", meta = (WorldContext = "WorldContextObject"))
	static bool Save(UObject* WorldContextObject, UKOSaveGameOption* Option);
};