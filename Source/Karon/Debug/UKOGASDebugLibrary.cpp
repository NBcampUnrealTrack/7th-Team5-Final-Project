#include "UKOGASDebugLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbilityTypes.h"

#if WITH_EDITOR

void UUKOGASDebugLibrary::SendDebugEvent(AActor* TargetActor, FGameplayTag EventTag, float Magnitude)
{
	if (!TargetActor) return;
	 
	FGameplayEventData EventData;
	EventData.EventMagnitude = Magnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, EventData);
}

FActiveGameplayEffectHandle UUKOGASDebugLibrary::ApplyDebugGE(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass)
{
	if (!TargetActor || !GEClass) return FActiveGameplayEffectHandle();
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!ASC) return FActiveGameplayEffectHandle();
	
	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec   = ASC->MakeOutgoingSpec(GEClass, 1.f, Ctx);
	return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UUKOGASDebugLibrary::RemoveDebugGE(AActor* TargetActor, FActiveGameplayEffectHandle Handle)
{
	if (!TargetActor || !Handle.IsValid()) return;
	
	UAbilitySystemComponent* ASC =UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC) ASC->RemoveActiveGameplayEffect(Handle);
}

FGameplayTagContainer UUKOGASDebugLibrary::GetCurrentTags(AActor* TargetActor)
{
	FGameplayTagContainer Tags;
	if (!TargetActor) return Tags;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC) ASC->GetOwnedGameplayTags(Tags);

	return Tags;
}

TArray<FString> UUKOGASDebugLibrary::GetActiveGENames(AActor* TargetActor)
{
	TArray<FString> Names;
	if (!TargetActor) return Names;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!ASC) return Names;
	
	FGameplayEffectQuery Query = 
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer());
	
	TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffects(Query);
	for (const FActiveGameplayEffectHandle& Handle : Handles)
	{
		if (const FActiveGameplayEffect* GE = ASC->GetActiveGameplayEffect(Handle))
			Names.Add(GE->Spec.Def->GetName());
	}
	
	return Names;
}

#endif
