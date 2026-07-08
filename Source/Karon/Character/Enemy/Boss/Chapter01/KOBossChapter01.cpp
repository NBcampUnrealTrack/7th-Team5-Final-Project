#include "KOBossChapter01.h"

#include "AIController.h"
#include "TimerManager.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Utility/Log/KOLogManager.h"

AKOBossChapter01::AKOBossChapter01(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
 
void AKOBossChapter01::BeginPlay()
{
	Super::BeginPlay();
}

void AKOBossChapter01::OnBossInitialized()
{
	if (GetMesh())
	{
		CoreMID = GetMesh()->CreateDynamicMaterialInstance(
			CoreMaterialIndex
		);
	}
	
	CloseCore();
}
 
// 페이즈 전환
void AKOBossChapter01::OnPhaseChanged(int32 NewPhase)
{
	if (NewPhase == 2)
	{
		// TODO: 페이즈2 처리
		// 이동속도 증가 GE 와 같은 부가효과 추가
		
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AKOAIC_BossController::bIsPhase2Key, true);
			}
		}
	}
}
 
// 그로기 진입
void AKOBossChapter01::OnGroggyBegin()
{
	if (bIsDead)
	{
		return;
	}
	
	bIsGroggy = true;
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsGroggyKey, true);
		}
	}
 
	OpenCore();
}

void AKOBossChapter01::TriggerGroggy()
{
	if (GroggySet)
	{
		GroggySet->SetGroggyHealth(0.f);
	}
	else
	{
		OnGroggyBegin();
	}
}
 
// 그로기 종료
void AKOBossChapter01::OnGroggyEnd()
{
	if (bIsDead)
	{
		return;
	}
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsGroggyKey, false);
		}
	}
	
	if (GroggySet)
	{
		GroggySet->SetGroggyHealth(GroggySet->GetMaxGroggyHealth());
	}
 
	bIsGroggy = false;
 
	CloseCore();
}
 
// 사망
void AKOBossChapter01::OnBossDeath()
{
	if (bIsDead)
	{
		return;
	}
	
	bIsDead = true;
 
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsDeadKey, true);
		}
	}
}

void AKOBossChapter01::OnCharacterDead(AActor* DeathInstigator)
{
	Super::OnCharacterDead(DeathInstigator);
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsDeadKey, true);
			
			KO_LOG(GAS, Warning, TEXT("bISDeadKey is Changed")); 
		}
	}
}

void AKOBossChapter01::NotifyGimmickDashEnd()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsGimmickReadyKey, false);
		}
	}
}

// 코어 개방
void AKOBossChapter01::OpenCore()
{
	bCoreOpen = true;
 
	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue("EmissiveIntensity",CoreEmissiveIntensity);
	}
}
 
// 코어 닫기
void AKOBossChapter01::CloseCore()
{
	bCoreOpen = false;
 
	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue("EmissiveIntensity",0.f);
	}
}
