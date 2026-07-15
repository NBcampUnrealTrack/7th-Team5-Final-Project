#include "KOBossBase.h"
 
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "KOAIC_BossController.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h" 
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "KOBossDataAsset.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Subsystem/KOSaveSubsystem.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Components/SkeletalMeshComponent.h"

AKOBossBase::AKOBossBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	HealthSet   = CreateDefaultSubobject<UKOHealthSet>("HealthSet");
	MovementSet = CreateDefaultSubobject<UKOMovementSet>("MovementSet");
	CombatSet = CreateDefaultSubobject<UKOCombatSet>("CombatSet");
	GroggySet = CreateDefaultSubobject<UKOGroggySet>("GroggySet");
}

void AKOBossBase::NotifyPlayerDetected()
{
	if (bPlayerDetected)
	{
		return;
	}

	bPlayerDetected = true;
	
	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		SaveSubsystem->NotifyActorTargetingPlayer(this);
	}
	
	OnBossDetectedPlayer.Broadcast(this);
}

void AKOBossBase::NotifyPlayerLost()
{
	bPlayerDetected = false;
	CurrentTarget = nullptr;

	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		SaveSubsystem->NotifyActorStoppedTargetingPlayer(this);
	}
}

void AKOBossBase::NotifyDeathAnimEnd()
{
	OnBossDeathAnimEnd.Broadcast();
}

void AKOBossBase::OnCharacterDead(AActor* DeathInstigator)
{
	Super::OnCharacterDead(DeathInstigator);
	
	bIsDead = true;
	NotifyPlayerLost();
	
	OnBossDeath();
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement(); 
	
		if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->BrainComponent))
		{
			BTComp->StopTree();
		}
	}
	
	OnBossDied.Broadcast();
	
	// 퀘스트
	FName BossId = BossSaveId;
	if (BossId.IsNone())
	{
		BossId = GetClass()->GetFName();
	}

	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifyBossDefeated(BossId);
	}
}


void AKOBossBase::RestoreBossFromSave(const FTransform& SavedTransform, bool bWasAlive)
{
	SetActorTransform(SavedTransform, false, nullptr, ETeleportType::TeleportPhysics);

	if (bWasAlive)
	{
		// 저장 당시 살아 있었던 상태로 복구
		bIsDead = false;
		bPlayerDetected = false;
		CurrentTarget = nullptr;

		SetActorEnableCollision(true);
		SetCanBeDamaged(true);
		
		// ASC 복구 -> Dead 태그 제거
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(this, this);
			AbilitySystemComponent->CancelAllAbilities();

			const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(
				FName(TEXT("State.Character.Dead")),
				false
			);

			if (DeadTag.IsValid())
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(DeadTag, 0);

				FGameplayTagContainer DeadTags;
				DeadTags.AddTag(DeadTag);
				
				AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(DeadTags);
				AbilitySystemComponent->RemoveActiveEffectsWithTags(DeadTags);
			}

			if (DataAsset)
			{
				AbilitySystemComponent->ApplyModToAttributeUnsafe(
					UKOHealthSet::GetHealthAttribute(),
					EGameplayModOp::Override,
					DataAsset->MaxHealth
				);
			}
		}

		// 메쉬 / 애니메이션 복구
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetHiddenInGame(false);
			MeshComp->SetVisibility(true, true);
			MeshComp->SetComponentTickEnabled(true);
			MeshComp->bPauseAnims = false;

			MeshComp->SetSimulatePhysics(false);
			MeshComp->SetAllBodiesSimulatePhysics(false);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.0f);
			}

			if (DataAsset)
			{
				if (UClass* AnimClass = DataAsset->AnimInstance.Get())
				{
					MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					MeshComp->SetAnimInstanceClass(AnimClass);

					// AnimBP 상태머신 강제 초기화
					MeshComp->InitAnim(true);
				}
			}
		}

		// 이동 복구
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetComponentTickEnabled(true);
			MoveComp->SetMovementMode(MOVE_Walking);
			MoveComp->StopMovementImmediately();
		}

		// AI 복구
		if (!GetController())
		{
			SpawnDefaultController();
		}

		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->StopMovement();
			
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				// Blackboard 초기화
				BB->SetValueAsBool(AKOAIC_BossController::bIsDeadKey, false);
				BB->SetValueAsBool(AKOAIC_BossController::bIsGroggyKey, false);
				BB->SetValueAsBool(AKOAIC_BossController::bIsGimmickReadyKey, false);
				BB->SetValueAsBool(AKOAIC_BossController::bIsPhase2Key, false);
			}
			
			// 그로기 상태 초기화
			OnGroggyEnd();

			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->BrainComponent))
			{
				BTComp->RestartTree();
			}
			else if (AIC->BrainComponent)
			{
				AIC->BrainComponent->RestartLogic();
			}
		}

		// 기믹/페이즈 상태 초기화
		bPhase2Triggered = false;
		FiredGimmickRatios.Empty();
	}
	else
	{
		// 저장 당시 죽어 있었던 상태로 복구
		bIsDead = true;
		bPlayerDetected = false;

		SetCanBeDamaged(false);
		SetActorEnableCollision(false);

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->ApplyModToAttributeUnsafe(
				UKOHealthSet::GetHealthAttribute(),
				EGameplayModOp::Override,
				0.f
			);
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
		}

		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->StopMovement();

			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->BrainComponent))
			{
				BTComp->StopTree();
			}
			else if (AIC->BrainComponent)
			{
				AIC->BrainComponent->StopLogic(TEXT("Boss loaded dead"));
			}
		}
	}
}

void AKOBossBase::BeginPlay()
{
	Super::BeginPlay();
 
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 테스트용 데이터에셋 로드
	if (DefaultDataAsset)
	{
		StartAsyncLoad(DefaultDataAsset);
	}
	
	if (HealthSet)
	{
		HealthSet->OnHealthChanged.AddUniqueDynamic(this, &AKOBossBase::OnHealthChangedCallback);
	}
	
	if (MovementSet)
	{
		MovementSet->OnMaxWalkSpeedBaseChanged.AddUniqueDynamic(this, &AKOBossBase::OnMoveSpeedChangedCallback);
	}
	
	if (GroggySet)
	{
		GroggySet->OnGroggyTriggered.AddUObject(this, &AKOBossBase::OnGroggyBegin);
	}
}

void AKOBossBase::OnHealthChangedCallback(float OldVal, float NewVal)
{
	if (!DataAsset)
	{
		return;
	}
 
	const float MaxHP = HealthSet->GetMaxHealth();
	if (MaxHP <= 0.f) return;
	
	const float Ratio = NewVal / MaxHP;
 
	if (Ratio <= PhaseRatio && !bPhase2Triggered)
	{
		bPhase2Triggered = true;
		OnPhaseChanged(2);
	}
	
	for (float GimmickRatio : GimmickReadyRatios)
	{
		if (Ratio <= GimmickRatio && !FiredGimmickRatios.Contains(GimmickRatio))
		{
			AAIController* AIC = Cast<AAIController>(GetController());
			if (!AIC)
			{
				break;
			}

			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				if (!BB->GetValueAsBool(AKOAIC_BossController::bIsGimmickReadyKey))
				{
					FiredGimmickRatios.Add(GimmickRatio);
					BB->SetValueAsBool(AKOAIC_BossController::bIsGimmickReadyKey, true);

					break;
				}
			}
		}
	}
}

void AKOBossBase::OnMoveSpeedChangedCallback(float OldVal, float NewVal)
{
	GetCharacterMovement()->MaxWalkSpeed = NewVal;
}

void AKOBossBase::RestoreToFull()
{
	NotifyPlayerLost();
	
	if (!AbilitySystemComponent || !DataAsset) return;

	// HP 최대치 복구
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOHealthSet::GetHealthAttribute(),
		EGameplayModOp::Override,
		DataAsset->MaxHealth
	);

	// Groggy 최대치 복구
	if (GroggySet)
	{
		GroggySet->SetGroggyHealth(GroggySet->GetMaxGroggyHealth());
	}

	// 그로기 상태였다면 종료
	OnGroggyEnd();
}

// 비동기 로드 시작 
void AKOBossBase::StartAsyncLoad(UKOBossDataAsset* InDataAsset)
{
	if (!InDataAsset)
	{
		return;
	}
 
	DataAsset = InDataAsset;
 
	TArray<FSoftObjectPath> AssetsToLoad;
 
	if (!DataAsset->SkeletalMesh.IsNull())
	{
		AssetsToLoad.Add(DataAsset->SkeletalMesh.ToSoftObjectPath());
	}
	
	if (!DataAsset->AnimInstance.IsNull())
	{
		AssetsToLoad.Add(DataAsset->AnimInstance.ToSoftObjectPath());
	}
	
	if (!DataAsset->BossBGM.IsNull())
	{
		AssetsToLoad.Add(DataAsset->BossBGM.ToSoftObjectPath());
	}
	
	if (AssetsToLoad.IsEmpty())
	{
		OnAssetsLoaded();
		return;
	}
 
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
 
	StreamableHandle = StreamableManager.RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateUObject(
			this, &AKOBossBase::OnAssetsLoaded
		)
	);
}
 
// 로드 완료 콜백
void AKOBossBase::OnAssetsLoaded()
{
	if (!DataAsset)
	{
		return;
	}
 
	if (!IsValid(this)) return;
 
	ApplyMeshAndAnim();
	ApplyStats();
	ApplyAbilities();
	
	OnBossInitialized();
 
	OnBossReady.Broadcast();
}

void AKOBossBase::ApplyMeshAndAnim()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}
	
	if (USkeletalMesh* SkeletalMesh = DataAsset->SkeletalMesh.Get())
	{
		MeshComp->SetSkeletalMesh(SkeletalMesh);
	}
	
	if (UClass* AnimClass = DataAsset->AnimInstance.Get())
	{
		MeshComp->SetAnimInstanceClass(AnimClass);
	}
}

void AKOBossBase::ApplyStats()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	
	if (!DataAsset)
	{
		return;
	}
	
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOHealthSet::GetMaxHealthAttribute(),
		EGameplayModOp::Override,
		DataAsset->MaxHealth
	);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOHealthSet::GetHealthAttribute(),
		EGameplayModOp::Override,
		DataAsset->MaxHealth
	);
 
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOMovementSet::GetMaxWalkSpeedAttribute(),
		EGameplayModOp::Override,
		DataAsset->MoveSpeed
	);
 
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOCombatSet::GetAttackPowerAttribute(),
		EGameplayModOp::Override,
		DataAsset->AttackPower
	);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOCombatSet::GetDefenseAttribute(),
		EGameplayModOp::Override,
		DataAsset->Defense
	);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UKOCombatSet::GetAttackSpeedAttribute(),
		EGameplayModOp::Override,
		DataAsset->AttackSpeed
	);
}

void AKOBossBase::ApplyAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DataAsset->BossAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}
		
		FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
		AbilitySystemComponent->GiveAbility(Spec);
	}
}
