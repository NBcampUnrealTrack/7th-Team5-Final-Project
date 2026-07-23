#include "KOBossChapter01.h"

#include "AIController.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"
#include "Materials/MaterialInstanceDynamic.h"

AKOBossChapter01::AKOBossChapter01(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FaceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FaceLight"));
}

void AKOBossChapter01::BeginPlay()
{
	Super::BeginPlay();
}

void AKOBossChapter01::OnBossInitialized()
{
	if (GetMesh())
	{
		CoreMID = GetMesh()->CreateDynamicMaterialInstance(CoreMaterialIndex);

		const bool bSocketExists = GetMesh()->DoesSocketExist(FaceLightSocket);

		if (FaceLight && bSocketExists)
		{
			FaceLight->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FaceLightSocket
			);
			FaceLight->SetLightColor(FaceLightColorNormal);
			FaceLight->SetIntensity(FaceLightIntensity);
			FaceLight->SetAttenuationRadius(FaceLightRadius);
		}
	}

	CloseCore();

	if (SmokeData)
	{
		InitVFXComponents();
		StartSmokePattern(SmokeData->IdlePattern);
	}
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

		if (FaceLight)
		{
			FaceLight->SetLightColor(FaceLightColorPhase2);
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

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsGroggyKey, true);
		}
	}

	if (FaceLight)
	{
		FaceLight->SetVisibility(false);
	}

	if (SmokeData)
	{
		StopSmokePattern();
		SetAllVFXActive(false);
		StartSmokePattern(SmokeData->GroggyPattern);
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

	if (FaceLight)
	{
		FaceLight->SetVisibility(true);
	}

	if (SmokeData)
	{
		StopSmokePattern();
		SetAllVFXActive(false);
		StartSmokePattern(SmokeData->IdlePattern);
	}

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

	if (FaceLight)
	{
		FaceLight->SetVisibility(false);
	}

	StopSmokePattern();
	SetAllVFXActive(false);
}

void AKOBossChapter01::OnCharacterDead(AActor* DeathInstigator)
{
	Super::OnCharacterDead(DeathInstigator);

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(AKOAIC_BossController::bIsDeadKey, true);
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

	if (SmokeData)
	{
		SetAllVFXActive(false);
		StartSmokePattern(SmokeData->IdlePattern);
	}
}

void AKOBossChapter01::OnGimmickReady()
{
	if (SmokeData)
	{
		StopSmokePattern();
		StartSmokePattern(SmokeData->GimmickPattern);
	}
}

void AKOBossChapter01::OnDashSmokeBegin()
{
	if (SmokeData)
	{
		StopSmokePattern();
		StartSmokePattern(SmokeData->DashPattern);
	}
}

void AKOBossChapter01::OnDashSmokeEnd()
{
	if (SmokeData)
	{
		StopSmokePattern();
		SetAllVFXActive(false);
		StartSmokePattern(SmokeData->IdlePattern);
	}
}

// 코어 개방
void AKOBossChapter01::OpenCore()
{
	bCoreOpen = true;

	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue("EmissiveIntensity", CoreEmissiveIntensity);
	}
}

// 코어 닫기
void AKOBossChapter01::CloseCore()
{
	bCoreOpen = false;

	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue("EmissiveIntensity", 0.f);
	}
}

void AKOBossChapter01::InitVFXComponents()
{
	VFXComponents.Empty();
	if (!SmokeData || !GetMesh())
	{
		return;
	}

	for (const FKOBossVFXChannel& Channel : SmokeData->VFXChannels)
	{
		TArray<UNiagaraComponent*> ChannelComps;

		if (!Channel.Effect)
		{
			VFXComponents.Add(ChannelComps);
			continue;
		}

		for (const FName& SocketName : Channel.Sockets)
		{
			if (!GetMesh()->DoesSocketExist(SocketName))
			{
				ChannelComps.Add(nullptr);
				continue;
			}

			UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				Channel.Effect, GetMesh(), SocketName,
				FVector::ZeroVector, FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget, false);

			if (Comp) Comp->Deactivate();
			ChannelComps.Add(Comp);
		}

		VFXComponents.Add(ChannelComps);
	}
}

void AKOBossChapter01::StartSmokePattern(const FKOBossSmokePattern& Pattern)
{
	StopSmokePattern();
	PuffStep = 0;

	if (Pattern.StepInterval <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		//WeakPtr를 통해 크래시 방지
		TWeakObjectPtr<AKOBossChapter01> WeakThis(this); 
		
		World->GetTimerManager().SetTimer(
			PuffTimerHandle,
			[WeakThis, Pattern]()
			{
				if (AKOBossChapter01* StrongThis = WeakThis.Get())
				{
					StrongThis->OnPuffStep(Pattern);
				}
			},
			Pattern.StepInterval, true, 0.f);
	}
}

void AKOBossChapter01::StopSmokePattern()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PuffTimerHandle);
		World->GetTimerManager().ClearTimer(PuffOffTimerHandle);
	}
}

void AKOBossChapter01::OnPuffStep(FKOBossSmokePattern Pattern)
{
	if (PuffStep < Pattern.Steps.Num())
	{
		const FKOBossSmokePuffStep& Step = Pattern.Steps[PuffStep];
		SetVFXActive(Step.ChannelIndex, Step.SocketIndex, true, Step.SpawnRate);

		const float Duration = Step.Duration;
		const int32 Ch = Step.ChannelIndex;
		const int32 Sock = Step.SocketIndex;
		if (UWorld* World = GetWorld())
		{
			TWeakObjectPtr<AKOBossChapter01> WeakThis(this);
			
			World->GetTimerManager().SetTimer(
				PuffOffTimerHandle,
				[WeakThis, Ch, Sock]()
				{
					if (AKOBossChapter01* StrongThis = WeakThis.Get())
					{
						StrongThis->SetVFXActive(Ch, Sock, false);
					}
				},
				Duration, false);
		}

		PuffStep++;
	}
	else
	{
		StopSmokePattern();
		PuffStep = 0;

		if (!Pattern.bLoop)
		{
			return;
		}

		if (Pattern.PauseInterval > 0.f)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
				PuffTimerHandle,
				[this, Pattern]() { StartSmokePattern(Pattern); },
				Pattern.PauseInterval, false);
			}
		}
		else
		{
			StartSmokePattern(Pattern);
		}
	}
}

void AKOBossChapter01::SetVFXActive(int32 ChannelIndex, int32 SocketIndex, bool bActive, float SpawnRate)
{
	if (!VFXComponents.IsValidIndex(ChannelIndex))
	{
		return;
	}
	TArray<UNiagaraComponent*>& Channel = VFXComponents[ChannelIndex];
	if (!Channel.IsValidIndex(SocketIndex))
	{
		return;
	}
	UNiagaraComponent* Comp = Channel[SocketIndex];
	if (!Comp)
	{
		return;
	}

	if (bActive)
	{
		if (IsValid(Comp))
		{
			Comp->Activate(true);
			Comp->SetFloatParameter(FName("SpawnRate"), SpawnRate);
		}
	}
	else
	{
		if (Comp)
		{
			Comp->Deactivate();
		}
	}
}

void AKOBossChapter01::SetAllVFXActive(bool bActive)
{
	for (int32 ChIdx = 0; ChIdx < VFXComponents.Num(); ++ChIdx)
	{
		for (int32 SockIdx = 0; SockIdx < VFXComponents[ChIdx].Num(); ++SockIdx)
		{
			SetVFXActive(ChIdx, SockIdx, bActive);
		}
	}
}
