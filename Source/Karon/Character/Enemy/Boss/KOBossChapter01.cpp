#include "KOBossChapter01.h"

#include "AIController.h"
#include "KOAIC_BossChapter01.h"
#include "TimerManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
 
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
				BB->SetValueAsBool(AKOAIC_BossChapter01::bIsPhase2Key, true);
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
	
	// ─── 추가 : BB bIsGroggyKey 설정 ─────────────────────────
	// GA에서 BB 직접 접근 대신 이 함수 호출
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGroggyKey, true);
		}
	}
 
	OpenCore();
}
 
// 그로기 종료
void AKOBossChapter01::OnGroggyEnd()
{
	if (bIsDead)
	{
		return;
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
			BB->SetValueAsBool(AKOAIC_BossChapter01::bIsDeadKey, true);
		}
	}
}

// ─── 추가 : 기믹 돌진 종료 시 BB 키 해제 ────────────────────
// GA에서 BB 직접 접근 대신 이 함수 호출
void AKOBossChapter01::NotifyGimmickDashEnd()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGimmickReadyKey, false);
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
