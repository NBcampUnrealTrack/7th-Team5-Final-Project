#include "KOBossBase.h"
 
#include "AbilitySystemComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h" 
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "KOBossDataAsset.h"

AKOBossBase::AKOBossBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	HealthSet   = CreateDefaultSubobject<UKOHealthSet>("HealthSet");
	MovementSet = CreateDefaultSubobject<UKOMovementSet>("MovementSet");
	
	CombatSet = CreateDefaultSubobject<UKOCombatSet>("CombatSet");
}

void AKOBossBase::NotifyPlayerDetected()
{
	if (bPlayerDetected)
	{
		return;
	}

	bPlayerDetected = true;
	OnBossDetectedPlayer.Broadcast();
}

void AKOBossBase::NotifyDeathAnimEnd()
{
	OnBossDeathAnimEnd.Broadcast();
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
		HealthSet->OnHealthChanged.AddUniqueDynamic(
			this, &AKOBossBase::OnHealthChangedCallback
		);
	}
}

// 델리게이트 콜백
void AKOBossBase::OnHealthChangedCallback(float OldVal, float NewVal)
{
	if (!DataAsset)
	{
		return;
	}
 
	const float MaxHP = HealthSet->GetMaxHealth();
	if (MaxHP <= 0.f)
	{
		return;
	}
 
	const float Ratio = NewVal / MaxHP;
 
	if (NewVal <= 0.f)
	{
		OnBossDeath();
		return;
	}
 
	if (Ratio <= PhaseRatio && !bPhase2Triggered)
	{
		bPhase2Triggered = true;
		OnPhaseChanged(2);
	}
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
	// TODO: 이거 GrantSet에 GE 적용해서 하면 한번에 처리 가능해요
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
 
	// AbilitySystemComponent->ApplyModToAttributeUnsafe(
	// 	UKOMovementSet::GetMaxMoveSpeedAttribute(),
	// 	EGameplayModOp::Override,
	// 	DataAsset->MoveSpeed
	// );
	
	// AbilitySystemComponent->ApplyModToAttributeUnsafe(
	// 	UKOMovementSet::GetMoveSpeedAttribute(),
	// 	EGameplayModOp::Override,
	// 	DataAsset->MoveSpeed
	// );
 
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
