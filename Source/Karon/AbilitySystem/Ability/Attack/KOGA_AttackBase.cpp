#include "KOGA_AttackBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"
#include "GameFramework/Character.h"


UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_AttackBase::SendAttackEventsToTarget(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	const UObject* RawdTarget = InEventData->Target;
	
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawdTarget));
	if (!TargetActor) return;
	
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	for (auto EventTag : AttackEventTags)
	{
		FGameplayEventData EventData;
		EventData.Instigator = Cast<const AActor>(GetAvatarCharacter());
		EventData.Target = TargetActor; 
		
		TargetASC->HandleGameplayEvent(EventTag, &EventData);
	}
}

void UKOGA_AttackBase::ApplyHitEffects(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	
	const UObject* RawTarget = InEventData->Target;
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget));
	if (!TargetActor) return;
	
	UAbilitySystemComponent* SourceASC = GetASC(); 
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC) return;
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarCharacter());
	
	for (const FKOHitEffectData& Effect : HitAppliedEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = 
		   SourceASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!SpecHandle.IsValid()) continue;
		
		for (const auto& Pair : Effect.SetByCallerValues)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}
		
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

UKOCombatSet* UKOGA_AttackBase::GetCombatSet()
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return nullptr;
	
	return Character->GetCombatSet(); 
}

