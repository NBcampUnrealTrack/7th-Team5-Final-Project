#include "KOEnemyAttackGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "SubSystem/KOEnemyDataSubsystem.h"

UKOEnemyAttackGameplayAbility::UKOEnemyAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UKOEnemyAttackGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
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
	//Ability가 종료되었다는 것을 BT에도 전달
	if (AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(GetAvatarActorFromActorInfo()))
	{
		Enemy->OnGameplayAbilityEnd.ExecuteIfBound();
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOEnemyAttackGameplayAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOEnemyAttackGameplayAbility::OnNotifyHitEvent(FGameplayEventData HitGameplayEventData)
{
	//맞은 플레이어의 ASI, ASC를 가져온다.
	
	AActor* HittedActor = const_cast<AActor*>(HitGameplayEventData.Target.Get());
	if (!IsValid(HittedActor))
	{
		return;
	}
	
	AKOCharacterBase* HittedPlayer=Cast<AKOCharacterBase>(HittedActor);
	AKOCharacterBase* AttackedPlayer=Cast<AKOCharacterBase>(GetAvatarActorFromActorInfo());
	
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

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo()); // 소스 오브젝트는 현재 캐릭터(Avatar)

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);

	const UKOCombatSet* CombatSet = Cast<UKOCombatSet>(SourceASC->GetAttributeSet(UKOCombatSet::StaticClass()));
	if (SpecHandle.IsValid() || IsValid(CombatSet))
	{
		//AssetTag 로 검색
		const FGameplayTagContainer& AssetTags=GetAssetTags();
		FGameplayTag AssetTag=AssetTags.GetByIndex(0);
		
		
		//공격자의 총합 데미지
		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(AvatarActor);
		float SkillMultiplier=1.f;
		UKOEnemyDataSubsystem* SkillSubsystem=UKOEnemyDataSubsystem::Get(this);
		if (SkillSubsystem!=nullptr&&AssetTag!=FGameplayTag::EmptyTag&&Enemy!=nullptr)
		{
			FEnemySkillInfo SkillInfo;
			SkillInfo.SkillTag=AssetTag;
			SkillInfo.EnemyNameTag=Enemy->EnemyNameTag;
			SkillMultiplier=SkillSubsystem->GetSkillData(SkillInfo);
		}
		UE_LOG(LogTemp,Warning,TEXT("%f"),SkillMultiplier);
		float SkillFinalDamage = CombatSet->GetAttackPower()*SkillMultiplier;

		SpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Damage, SkillFinalDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
	
	//디버프 GE
	for (auto DebuffEffectClass: DebuffEffectClassMap)
	{
		FGameplayEffectSpecHandle DebuffSpecHandle = SourceASC->MakeOutgoingSpec(DebuffEffectClass.Key, 1.0f, Context);
		if (DebuffSpecHandle.IsValid())
		{
			DebuffSpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_DebuffTime, DebuffEffectClass.Value);
			SourceASC->ApplyGameplayEffectSpecToTarget(*DebuffSpecHandle.Data.Get(), TargetASC);
		}
		
	}
	
	
}

