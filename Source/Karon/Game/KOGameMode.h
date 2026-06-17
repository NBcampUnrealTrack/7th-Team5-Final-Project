#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KOGameMode.generated.h"

UCLASS()
class KARON_API AKOGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AKOGameMode();
	
	UFUNCTION(BlueprintCallable)
	void HandlePlayerDeath(AActor* DeathInstigator); 
	
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Player | Death")
	float RespawnDuration = 2.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Player | Death")
	float FadeDuration = 2.f; 
	
};
