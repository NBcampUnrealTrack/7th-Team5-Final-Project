// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOLoadingUiSubsystem.generated.h"

class UCommonActivatableWidget;
/**
 * 레벨 전환 로딩 위젯 관리용 별도 서브시스템
 */
UCLASS()
class KARON_API UKOLoadingUiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category= "Loading UI")
	void ShowLoadingScreen(TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass);

	UFUNCTION(BlueprintCallable, Category= "Loading UI")
	void HideLoadingScreen();
	
	UFUNCTION(BlueprintPure, Category= "Loading UI")
	bool IsLoadingScreenActive() const { return CurrentLoadingWidget != nullptr; }
private:
	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> CurrentLoadingWidget;
};
