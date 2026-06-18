#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_Tick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickDelegate, float, DeltaTime);

UCLASS()
class KARON_API UAbilityTask_Tick : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
    FOnTickDelegate OnTick;
    
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static UAbilityTask_Tick* CreateTickTask(UGameplayAbility* OwningAbility);
    
    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
};
