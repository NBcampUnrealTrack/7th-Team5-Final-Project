// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "KOLoadingUiSubsystem.generated.h"

class UCommonActivatableWidget;
class UKOLoadingUIWidget;
/**
 * 로딩 위젯 관리용 별도 서브시스템
 */

UENUM()
enum class ELoadingState : uint8
{
	WaitingForSignal,	// 1. 호출 대기 중
	Finalizing,			// 2. 신호 처리 중
	Finished			// 3. 작업 완료 후처리 중
};

UCLASS()
class KARON_API UKOLoadingUiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** 레벨 전환 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Loading Subsystem")
	void TransitionToLevel(const FName& TargetLevelName, TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass);
	
	/** 직접 로딩화면만 띄우면 호출*/
	void ShowLoadingScreen(TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass);
	
	/** 전환이 완료 될 위치에서 호출*/
	void HideLoadingScreen();

	UFUNCTION(BlueprintPure,     Category = "Loading UI")
	bool IsLoadingScreenActive() const { return CurrentLoadingWidget != nullptr; }

private:
	bool TickLoading(float DeltaTime);
	
	UPROPERTY()
	TObjectPtr<UKOLoadingUIWidget> CurrentLoadingWidget;
	
	bool bIsLoadingActive = false;
	FName CachedLevelPath;
	FTSTicker::FDelegateHandle TickDelegateHandle;
	
	ELoadingState CurrentState;
	float FinalizeTimer = 0.f;
	const float TargetFinalizeTime = 1.f;
};
