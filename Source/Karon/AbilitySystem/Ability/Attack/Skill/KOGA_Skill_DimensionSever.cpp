#include "KOGA_Skill_DimensionSever.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UKOGA_Skill_DimensionSever::UKOGA_Skill_DimensionSever()
{
}

void UKOGA_Skill_DimensionSever::ExecuteAttack(float ChargePercentage)
{
	Super::ExecuteAttack(ChargePercentage);
	
	float DamageMultiplier = 0.3f + ChargePercentage;
	
	if (AttackMontage)
	{
		AttackMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AttackMontage,
			1.0f,
			NAME_None,
			false
		);
		
		if (AttackMontageTask)
		{
			AttackMontageTask->OnCompleted.AddDynamic(this, &ThisClass::UKOGA_Skill_DimensionSever::OnAttackMontageEnded);
			AttackMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::UKOGA_Skill_DimensionSever::OnAttackMontageEnded);
			AttackMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::UKOGA_Skill_DimensionSever::OnAttackMontageEnded);
			AttackMontageTask->OnCancelled.AddDynamic(this, &ThisClass::UKOGA_Skill_DimensionSever::OnAttackMontageEnded);
			
			AttackMontageTask->ReadyForActivation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] AttackMontage가 비어 있습니다! 에디터에서 설정해주세요."), *GetName());
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	
}

void UKOGA_Skill_DimensionSever::OnAttackMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
