// Copyright Karon Team 5. All Rights Reserved.

#include "KOLoadingUiSubsystem.h"
#include "UI/Loading/KOLoadingUIWidget.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Containers/Ticker.h"

void UKOLoadingUiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		CVar->Set(1, ECVF_SetByCode);
		UE_LOG(LogTemp, Log, TEXT("Loading UI Subsystem : 에디터 내 심리스 트래블을 자동으로 활성화 했습니다."));
	}
#endif
}

void UKOLoadingUiSubsystem::TransitionToLevel(const FName& TargetLevelName,
                                              TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass)
{
	ShowLoadingScreen(LoadingWidgetClass);

	//아래 경로를 확인해 레벨오픈
	FString PackagePath = FString::Printf(TEXT("/Game/Karon/Map/%s"), *TargetLevelName.ToString());

	if (FPackageName::DoesPackageExist(PackagePath))
	{
		CachedLevelPath = FName(*PackagePath);
		GetWorld()->ServerTravel(TargetLevelName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadUISubsystem 오류: '%s' 레벨을 찾을 수 없습니다! 아름과 경로를 확인하세요."),
		       *PackagePath);
	}
}

void UKOLoadingUiSubsystem::ShowLoadingScreen(TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass)
{
	if (bIsLoadingActive || LoadingWidgetClass == nullptr)
	{
		return;
	}
	
	bIsLoadingActive = true;
	CurrentState = ELoadingState::WaitingForSignal;
	FinalizeTimer = 0.f;

	CurrentLoadingWidget = Cast<UKOLoadingUIWidget>(CreateWidget<UCommonActivatableWidget>(
		GetGameInstance(), LoadingWidgetClass));
	if (CurrentLoadingWidget)
	{
		GEngine->GameViewport->AddViewportWidgetContent(CurrentLoadingWidget->TakeWidget(), 100);
		
		TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UKOLoadingUiSubsystem::TickLoading)
			);
	}
}

void UKOLoadingUiSubsystem::HideLoadingScreen()
{
	if (CurrentState == ELoadingState::WaitingForSignal)
	{
		CurrentState = ELoadingState::Finalizing;
		FinalizeTimer = 0.f;
		
		if (CurrentLoadingWidget)
		{
			CurrentLoadingWidget->ReceiveFinalizeSignal();
		}
	}
}

bool UKOLoadingUiSubsystem::TickLoading(float DeltaTime)
{
	if (bIsLoadingActive ==false || CurrentLoadingWidget == nullptr) return false;

	switch (CurrentState)
	{
	case ELoadingState::WaitingForSignal:
		break;

	case ELoadingState::Finalizing:
		{
			FinalizeTimer += DeltaTime;
            
			// 신호를 받은 뒤 설정한 임의의 지연 시간(예: 1초) 동안 틱을 더 돌리며 완충
			if (FinalizeTimer >= TargetFinalizeTime)
			{
				CurrentState = ELoadingState::Finished;

				// 티커 해제
				if (TickDelegateHandle.IsValid())
				{
					FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
					TickDelegateHandle.Reset();
				}
				// 메모리에서 위젯 제거 및 정리 완료
				GEngine->GameViewport->RemoveViewportWidgetContent(CurrentLoadingWidget->TakeWidget());
				CurrentLoadingWidget = nullptr;
				bIsLoadingActive = false;
                
				return false; 
			}
		}
		break;

	case ELoadingState::Finished:
		return false;
	}

	return true;
}
