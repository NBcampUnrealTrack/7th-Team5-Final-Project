// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOAudioSubsystem.generated.h"

class UAudioComponent;
/**
 * 레벨 전환에 유지되어야 하는 사운드 주입용 서브 시스템
 */
UCLASS()
class KARON_API UKOAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 레벨 전환에도 파괴되지 않는 사운드 재생*/
	UFUNCTION(BlueprintCallable, Category = "Audio | Transitions")
	void PlayPersistentTransitionSound(USoundBase* SoundInside);
	
private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> TransitionAudioComponent;
};
