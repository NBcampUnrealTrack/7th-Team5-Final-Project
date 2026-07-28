// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaSource.h"
#include "Subsystems/WorldSubsystem.h"
#include "KOTutorialSubsystem.generated.h"

struct FStreamableHandle;
struct FStreamableManager;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FKOVideoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMediaSource> VideoSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VideoName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VideoDesciption;
};

UCLASS()
class KARON_API UKOTutorialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UKOTutorialSubsystem* Get(UObject* WorldContextO);

	UFUNCTION(BlueprintCallable)
	void Preload(UObject* WorldContext, FName TutorialName);

	UFUNCTION(BlueprintCallable)
	void PreloadAll(UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	void UnloadAll(UObject* WorldContext);

	/**
	 * [의존성 안내]
	* 이 함수를 호출하기 전에  Preload(), PreloadAll()이 먼저 실행되어야 합니다.
	* 
	* @note Preload(), PreloadAll() 실행 없이 호출할 경우 비동기 로드가 되지 않고, 동기 로드로 처리됩니다.
	*/
	UFUNCTION(BlueprintCallable)
	void SetTutorial(UObject* WorldContext, FName TutorialName);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	FStreamableManager* StreamableManager = nullptr;

	UPROPERTY()
	TMap<FName, FKOVideoData> VideoMap;
	
	TMap<FName, TSharedPtr<FStreamableHandle>> StreamableHandles;
};
