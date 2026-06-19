#include "KOEnemyGameplayAbility.h"
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
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		//Commit Failed
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) 
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// TODO: Tag 변경 
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, KOGameplayTags::Event_SkillHit, nullptr, false, false);

	WaitEventTask->EventReceived.AddDynamic(this, &UKOEnemyGameplayAbility::OnNotifyHitEvent);
	WaitEventTask->ReadyForActivation();
	
	const UKOCombatSet* CombatSet = GetCombatSet();
	if (MontageDatas.IsEmpty() || !CombatSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	float PlayRate = MontageDatas[0].PlayRate * CombatSet->GetAttackSpeed(); 
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageDatas[0].Montage, PlayRate);
	
	PlayMontageTask->OnCompleted.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UKOEnemyGameplayAbility::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();
}

void UKOEnemyGameplayAbility::OnMontageCompleted()
{
	//Ability가 종료되었다는 것을 BT에도 전달
	if (AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(GetAvatarCharacter()))
	{
		Enemy->OnGameplayAbilityEnd.ExecuteIfBound();
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOEnemyGameplayAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOEnemyGameplayAbility::OnNotifyHitEvent(FGameplayEventData HitGameplayEventData)
{
	ApplyHitEffects(&HitGameplayEventData);
	
	// AActor* HittedActor = const_cast<AActor*>(HitGameplayEventData.Target.Get());
	// if (!IsValid(HittedActor))
	// {
	// 	return;
	// }
	//
	// AKOCharacterBase* HittedPlayer= Cast<AKOCharacterBase>(HittedActor);
	// AKOCharacterBase* AttackedPlayer= Cast<AKOCharacterBase>(GetAvatarActorFromActorInfo());
	//
	// IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HittedActor);
	// if (TargetASI == nullptr)
	// {
	// 	return;
	// }
	// UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	// if (TargetASC == nullptr)
	// {
	// 	return;
	// }
	//
	// UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	// if (!SourceASC)
	// {
	// 	return;
	// }
	//
	// FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	// Context.AddSourceObject(GetAvatarActorFromActorInfo()); // 소스 오브젝트는 현재 캐릭터(Avatar)
	//
	// FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
	//
	// const UKOCombatSet* CombatSet = GetCombatSet(); 
	//
	// if (SpecHandle.IsValid() && IsValid(CombatSet))
	// {
	// 	// TODO: ExecCalc 로 이전 해야함. 
	// 	
	// 	//AssetTag 로 검색
	// 	const FGameplayTagContainer& AssetTags=GetAssetTags();
	// 	FGameplayTag AssetTag=AssetTags.GetByIndex(0);
	// 	
	// 	//공격자의 총합 데미지
	// 	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	// 	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(AvatarActor);
	// 	float SkillMultiplier = 1.f;
	// 	UKOEnemyDataSubsystem* SkillSubsystem = UKOEnemyDataSubsystem::Get(this);
	// 	if (SkillSubsystem && AssetTag != FGameplayTag::EmptyTag && Enemy)
	// 	{
	// 		FEnemySkillInfo SkillInfo;
	// 		SkillInfo.SkillTag=AssetTag;
	// 		SkillInfo.EnemyNameTag=Enemy->EnemyNameTag;
	// 		SkillMultiplier=SkillSubsystem->GetSkillData(SkillInfo);
	// 	}
	// 	
	// 	UE_LOG(LogTemp,Warning,TEXT("%f"),SkillMultiplier);
	// 	
	// 	float SkillFinalDamage = CombatSet->GetAttackPower() * SkillMultiplier;
	//
	// 	SpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Damage, SkillFinalDamage);
	// 	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	// }
	//
	// //디버프 GE
	// for (auto DebuffEffectClass: DebuffEffectClassMap)
	// {
	// 	FGameplayEffectSpecHandle DebuffSpecHandle = SourceASC->MakeOutgoingSpec(DebuffEffectClass.Key, 1.0f, Context);
	// 	if (DebuffSpecHandle.IsValid())
	// 	{
	// 		DebuffSpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_DebuffTime, DebuffEffectClass.Value);
	// 		SourceASC->ApplyGameplayEffectSpecToTarget(*DebuffSpecHandle.Data.Get(), TargetASC);
	// 	}
	// 	
	// }
	
	
}

