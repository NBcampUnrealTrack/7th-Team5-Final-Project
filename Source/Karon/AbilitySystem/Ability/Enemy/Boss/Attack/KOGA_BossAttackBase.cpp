#include "KOGA_BossAttackBase.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOBossDataAsset.h"

UKOGA_BossAttackBase::UKOGA_BossAttackBase()
{
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Attacking);
 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
}
 
void UKOGA_BossAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	if (!IsTargetInRange())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	// 몽타주 재생 태스크 생성
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
	this,
	NAME_None,
	AttackMontage,
	MontageSpeed,
	NAME_None,
	false,
	1.0f
	);
 
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
 
	MontageTask->ReadyForActivation();
}
 
void UKOGA_BossAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UKOGA_BossAttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossAttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UKOGA_BossAttackBase::IsTargetInRange() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return false;
 
	APawn* Pawn = Cast<APawn>(Avatar);
	if (!Pawn)
	{
		return false;
	}
 
	AAIController* AIC = Cast<AAIController>(Pawn->GetController());
	if (!AIC)
	{
		return false;
	}
 
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}
 
	AActor* Target = Cast<AActor>(
		BB->GetValueAsObject(AKOAIC_BossChapter01::TargetActorKey));
	if (!Target)
	{
		return false;
	}
 
	// DA에서 GA 클래스 기준으로 AttackRange 조회
	AKOBossBase* Boss = Cast<AKOBossBase>(Avatar);
	UKOBossDataAsset* DA = Boss ? Boss->GetDataAsset() : nullptr;
	const float AttackRange = DA ? DA->GetAttackRange(GetClass()) : 300.f;
 
	return FVector::Dist(Avatar->GetActorLocation(),Target->GetActorLocation()) <= AttackRange;
}

void UKOGA_BossAttackBase::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}
	
	if (!DamageEffectClass)
	{
		return;
	}
 
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI)
	{
		return;
	}
 
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}
 
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}
	
	const UKOCombatSet* CombatSet = SourceASC->GetSet<UKOCombatSet>();
	if (!CombatSet)
	{
		return;
	}
	
	const float AttackPower = CombatSet->GetAttackPower();
 
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());
 
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
 
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Damage,AttackPower);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
