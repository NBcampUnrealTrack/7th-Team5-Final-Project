#include "UKOGASDebugLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbilityTypes.h"

#if WITH_EDITOR

// ─── GameplayEvent ──────────────────────────────────────────────────

void UUKOGASDebugLibrary::SendDebugEvent(AActor* TargetActor, FGameplayTag EventTag, float Magnitude)
{
    if (!TargetActor) return;
     
    FGameplayEventData EventData;
    EventData.EventMagnitude = Magnitude;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, EventData);
}


// ─── GameplayEffect ─────────────────────────────────────────────────

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
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    
    if (ASC) ASC->RemoveActiveGameplayEffect(Handle);
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


// ─── GameplayAbility ────────────────────────────────────────────────

FGameplayAbilitySpecHandle UUKOGASDebugLibrary::GrantDebugAbility(
    AActor* TargetActor,
    TSubclassOf<UGameplayAbility> GAClass, 
    int32 Level)
{
    if (!TargetActor || !GAClass) return FGameplayAbilitySpecHandle();

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!ASC) return FGameplayAbilitySpecHandle();

    FGameplayAbilitySpec Spec(GAClass, Level);
    return ASC->GiveAbility(Spec);
}

void UUKOGASDebugLibrary::RemoveDebugAbility(AActor* TargetActor, FGameplayAbilitySpecHandle Handle)
{
    if (!TargetActor || !Handle.IsValid()) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->ClearAbility(Handle);
}

TArray<FString> UUKOGASDebugLibrary::GetActiveAbilityNames(AActor* TargetActor)
{
    TArray<FString> Names;
    if (!TargetActor) return Names;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!ASC) return Names;

    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
    {
        if (Spec.Ability)
            Names.Add(Spec.Ability->GetName());
    }
    return Names;
}


// ─── GameplayTag ────────────────────────────────────────────────────

FGameplayTagContainer UUKOGASDebugLibrary::GetCurrentTags(AActor* TargetActor)
{
    FGameplayTagContainer Tags;
    if (!TargetActor) return Tags;
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->GetOwnedGameplayTags(Tags);

    return Tags;
}

void UUKOGASDebugLibrary::AddLooseTag(AActor* TargetActor, FGameplayTag Tag)
{
    if (!TargetActor || !Tag.IsValid()) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->AddLooseGameplayTag(Tag);
}

void UUKOGASDebugLibrary::RemoveLooseTag(AActor* TargetActor, FGameplayTag Tag)
{
    if (!TargetActor || !Tag.IsValid()) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (ASC) ASC->RemoveLooseGameplayTag(Tag);
}

	
// ─── Attributes  ────────────────────────────────────────────────────

TArray<FKOAttributeInfo> UUKOGASDebugLibrary::GetAllAttributes(AActor* TargetActor)
{
    TArray<FKOAttributeInfo> Result;
    if (!TargetActor) return Result;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!ASC) return Result;

    for (UAttributeSet* AttrSet : ASC->GetSpawnedAttributes())
    {
        if (!AttrSet) continue;

        for (TFieldIterator<FProperty> It(AttrSet->GetClass()); It; ++It)
        {
            FStructProperty* StructProp = CastField<FStructProperty>(*It);
            if (!StructProp || StructProp->Struct != FGameplayAttributeData::StaticStruct())
                continue;

            FGameplayAttributeData* Data =
                StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(AttrSet);

            FKOAttributeInfo Info;
            Info.AttributeSetName = AttrSet->GetClass()->GetName();
            Info.AttributeName    = It->GetName();
            Info.BaseValue        = Data->GetBaseValue();
            Info.CurrentValue     = Data->GetCurrentValue();
            Result.Add(Info);
        }
    }
    return Result;
}

void UUKOGASDebugLibrary::SetAttributeBaseValue(AActor* TargetActor, const FString& AttributeName, float NewValue)
{
    if (!TargetActor) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!ASC) return;

    for (UAttributeSet* AttrSet : ASC->GetSpawnedAttributes())
    {
        if (!AttrSet) continue;

        for (TFieldIterator<FProperty> It(AttrSet->GetClass()); It; ++It)
        {
            FStructProperty* StructProp = CastField<FStructProperty>(*It);
            if (!StructProp || StructProp->Struct != FGameplayAttributeData::StaticStruct())
                continue;
            if (!It->GetName().Equals(AttributeName, ESearchCase::IgnoreCase))
                continue;
            
            FGameplayAttribute Attr(StructProp);
            ASC->SetNumericAttributeBase(Attr, NewValue);
            return;
        }
    }
}

#endif
