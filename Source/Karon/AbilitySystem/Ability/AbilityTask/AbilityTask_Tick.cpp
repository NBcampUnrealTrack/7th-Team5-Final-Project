#include "AbilityTask_Tick.h"

UAbilityTask_Tick* UAbilityTask_Tick::CreateTickTask(UGameplayAbility* OwningAbility)
{
	UAbilityTask_Tick* Task = NewAbilityTask<UAbilityTask_Tick>(OwningAbility);
	Task->bTickingTask = true; 
	return Task;
}

void UAbilityTask_Tick::Activate()
{
	Super::Activate();
}

void UAbilityTask_Tick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}
