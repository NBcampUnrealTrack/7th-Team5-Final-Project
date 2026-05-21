// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyAttackGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOEnemyAttackGameplayAbility::UKOEnemyAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UKOEnemyAttackGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UKOEnemyAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	//Commit
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		//Commit Failed
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	//Get ASC
	UAbilitySystemComponent* EnemyASC = GetASC();
	if (!EnemyASC)
	{
		return;
	}
	//Play Montage Task(비동기)
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AnimMontage,
			SKillAttackSpeed
		);
	if (!PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//Bind Delegate
	PlayMontageTask->OnCompleted.AddDynamic(this, &UKOEnemyAttackGameplayAbility::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UKOEnemyAttackGameplayAbility::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UKOEnemyAttackGameplayAbility::OnMontageCancelled);
	// Task 활성화
	PlayMontageTask->ReadyForActivation();
	
	//Anim Notify에서 스킬히트 판정이 들어왔을 경우 바인딩
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, KOGameplayTags::Event_SkillHit, nullptr, false, false);

	WaitEventTask->EventReceived.AddDynamic(this, &UKOEnemyAttackGameplayAbility::OnNotifyHitEvent);
	WaitEventTask->ReadyForActivation();

	//GE_CoolDown 적용
	if (CoolTimeEffectClass == nullptr || CoolTime == 0.f)
	{
		return;
	}
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CoolTimeEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(KOGameplayTags::Data_CoolTime, CoolTime);

		SpecHandle.Data.Get()->DynamicGrantedTags.AddTag(CoolTimeTag);

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

void UKOEnemyAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOEnemyAttackGameplayAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOEnemyAttackGameplayAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOEnemyAttackGameplayAbility::OnNotifyHitEvent(FGameplayEventData HitGameplayEventData)
{
	//맞은 플레이어의 ASI, ASC를 가져온다.
	//TODO: 플레이어 디벨롭에 올라오면 구현
	/*
	AActor* HittedActor = const_cast<AActor*>(HitGameplayEventData.Target.Get());
	if (!IsValid(HittedActor))
	{
		return;
	}
	
	AKOHeroCharacter* HittedPlayer=Cast<AEDPlayerCharacter>(HittedActor);
	AEDPlayerCharacter* AttackedPlayer=Cast<AEDPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HittedActor);
	if (TargetASI == nullptr)
	{
		return;
	}
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (TargetASC == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = PlayerASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo()); // 소스 오브젝트는 현재 캐릭터(Avatar)

	FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);

	const UEDPlayerAttributeSet* PlayerAttributeSet = Cast<UEDPlayerAttributeSet>(PlayerASC->GetAttributeSet(UEDPlayerAttributeSet::StaticClass()));
	if (SpecHandle.IsValid() || IsValid(PlayerAttributeSet))
	{
		//AssetTag 로 검색
		const FGameplayTagContainer& AssetTags=GetAssetTags();
		FGameplayTag AssetTag=AssetTags.GetByIndex(0);
		
		const UEDSkillDataSubsystem* EDSkillDataSubsystem=UEDSkillDataSubsystem::Get(GetWorld());
		
		if (AssetTag==FGameplayTag::EmptyTag||!IsValid(EDSkillDataSubsystem))
		{
			return;
		}
		//공격자의 총합 데미지
		float SkillFinalDamage =
			PlayerAttributeSet->GetStrength() * SkillMulStaus->DamageStrengthMultiplier +
			PlayerAttributeSet->GetDexterity() * SkillMulStaus->DamageDexterityMultiplier +
			PlayerAttributeSet->GetIntelligence() * SkillMulStaus->DamageIntelligenceMultiplier
		;

		SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, SkillFinalDamage);
		PlayerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
	
	//디버프 GE
	for (auto DebuffEffectClass: DebuffEffectClassMap)
	{
		FGameplayEffectSpecHandle DebuffSpecHandle = PlayerASC->MakeOutgoingSpec(DebuffEffectClass.Key, 1.0f, Context);
		if (DebuffSpecHandle.IsValid())
		{
			DebuffSpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_DebuffTime, DebuffEffectClass.Value);
			PlayerASC->ApplyGameplayEffectSpecToTarget(*DebuffSpecHandle.Data.Get(), TargetASC);
		}
		
	}
	*/
	
}

