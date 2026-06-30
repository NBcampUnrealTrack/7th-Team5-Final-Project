#include "KOEnemyGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "SubSystem/KOEnemyDataSubsystem.h"

UKOEnemyGameplayAbility::UKOEnemyGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOEnemyGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, KOGameplayTags::Event_Hit, nullptr, false, false);

	WaitEventTask->EventReceived.AddDynamic(this, &UKOEnemyGameplayAbility::OnNotifyHitEvent);
	WaitEventTask->ReadyForActivation();
	
	

	const UKOCombatSet* CombatSet = GetCombatSet();
	if (MontageData.IsEmpty() || !CombatSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float PlayRate = MontageData[0].PlayRate * CombatSet->GetAttackSpeed();

	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageData[0].Montage, PlayRate);

	PlayMontageTask->OnCompleted.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();
}

void UKOEnemyGameplayAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOEnemyGameplayAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOEnemyGameplayAbility::OnNotifyHitEvent(FGameplayEventData HitGameplayEventData)
{
	if (!HitGameplayEventData.Target) return;
	
	ApplyHitEffects(&HitGameplayEventData);
	SendAttackEventsToTarget(&HitGameplayEventData);
	
	// const UObject* RawTarget = HitGameplayEventData.Target;
	// AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget));
	// if (!TargetActor) return;
	//
	// UAbilitySystemComponent* SourceASC = GetASC();
	// UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	// if (!SourceASC || !TargetASC) return;
	//
	// FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	// Context.AddSourceObject(GetAvatarCharacter());
	//
	// FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
	//
	// const UKOCombatSet* CombatSet = GetCombatSet();
	//
	// if (SpecHandle.IsValid() && IsValid(CombatSet))
	// {
	// 	//AssetTag 로 검색
	// 	const FGameplayTagContainer& AssetTags = GetAssetTags();
	// 	FGameplayTag AssetTag = AssetTags.GetByIndex(0);
	//
	// 	//공격자의 총합 데미지
	// 	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(GetAvatarCharacter());
	// 	float SkillMultiplier = 1.f;
	// 	UKOEnemyDataSubsystem* SkillSubsystem = UKOEnemyDataSubsystem::Get(this);
	// 	
	// 	if (SkillSubsystem && AssetTag != FGameplayTag::EmptyTag && Enemy)
	// 	{
	// 		FEnemySkillInfo SkillInfo;
	// 		SkillInfo.SkillTag = AssetTag;
	// 		SkillInfo.EnemyNameTag = Enemy->EnemyNameTag;
	// 		SkillMultiplier = SkillSubsystem->GetSkillData(SkillInfo);
	// 	}
	// 	
	// 	float SkillFinalDamage = CombatSet->GetAttackPower() * SkillMultiplier;
	//
	// 	SpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Damage, SkillFinalDamage);
	// 	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	// }
	
}
