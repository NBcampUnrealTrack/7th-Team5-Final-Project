#include "UKOGASDebugLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbilityTypes.h"

void UUKOGASDebugLibrary::SendDebugEvent(AActor* TargetActor, FGameplayTag EventTag, float Magnitude)
{
#if WITH_EDITOR
    if (!TargetActor) return;
     
    FGameplayEventData EventData;
    EventData.EventMagnitude = Magnitude;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, EventData);
#endif
}

FActiveGameplayEffectHandle UUKOGASDebugLibrary::ApplyDebugGE(AActor* TargetActor, TSubclassOf<UGameplayEffect> GEClass)
{
#if WITH_EDITOR
    if (!TargetActor || !GEClass) return FActiveGameplayEffectHandle();
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!ASC) return FActiveGameplayEffectHandle();
    
    FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec   = ASC->MakeOutgoingSpec(GEClass, 1.f, Ctx);
    return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
#else
    return FActiveGameplayEffectHandle();
#endif
}

void UUKOGASDebugLibrary::RemoveDebugGE(AActor* TargetActor, FActiveGameplayEffectHandle Handle)
{
#if WITH_EDITOR
    if (!TargetActor || !Handle.IsValid()) return;
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->RemoveActiveGameplayEffect(Handle);
#endif
}

FGameplayTagContainer UUKOGASDebugLibrary::GetCurrentTags(AActor* TargetActor)
{
#if WITH_EDITOR
    FGameplayTagContainer Tags;
    if (!TargetActor) return Tags;
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->GetOwnedGameplayTags(Tags);

    return Tags;
#else
    return FGameplayTagContainer();
#endif
}

TArray<FString> UUKOGASDebugLibrary::GetActiveGENames(AActor* TargetActor)
{
#if WITH_EDITOR
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
#else
    return TArray<FString>();
#endif
}