// Fill out your copyright notice in the Description page of Project Settings.


#include "KOBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "GMRouterSubsystem.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "AbilitySystem/Attribute/KOGuardSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "Components/WidgetComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"
#include "SubSystem/KOEnemyDataSubsystem.h"
#include "Subsystem/KOSaveSubsystem.h"
#include "UI/Enemy/KOEnemyBaseUI.h"
#include "UI/Enemy/KOEnemyHPBar.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Subsystem/KOQuestGuideSubsystem.h"


// Sets default values
AKOBaseEnemy::AKOBaseEnemy(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	//ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	//어트리뷰트셋 생성
	HealthSet = CreateDefaultSubobject<UKOHealthSet>(TEXT("HealthSet"));
	MovementSet = CreateDefaultSubobject<UKOMovementSet>(TEXT("MovementSet"));
	CombatSet = CreateDefaultSubobject<UKOCombatSet>(TEXT("CombatSet"));
	GuardSet = CreateDefaultSubobject<UKOGuardSet>(TEXT("GuardSet"));
	GroggySet = CreateDefaultSubobject<UKOGroggySet>("GroggySet");

	
	//WeaponSkeletalMeshComponent 생성 및 부착
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMeshComponent->SetupAttachment(GetMesh(), HandSocketName);
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	//Enemy HPBar 부착
	EnemyHPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComponent"));
	EnemyHPBarWidgetComponent->SetupAttachment(GetMesh());
	EnemyHPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	
	//Enemy LockOn 부착
	EnemyLockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidgetComponent"));
	EnemyLockOnWidgetComponent->SetupAttachment(GetMesh(),LockOnSocketName);
	EnemyLockOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	
	//Enemy Parried 부착
	EnemyParriedWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ParriedWidgetComponent"));
	EnemyParriedWidgetComponent->SetupAttachment(GetMesh(),LockOnSocketName);
	EnemyParriedWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
}

void AKOBaseEnemy::SetupEnemy(UKOEnemyDataSubsystem* DataSubsystem,int32 Level)
{
	if (DataSubsystem==nullptr)
	{
		return;
	}
	EnemyLevel=Level;
	
	FEnemyNameLevelInfo NameLevelInfo;
	NameLevelInfo.EnemyNameTag=EnemyNameTag;
	NameLevelInfo.Level=Level;
	
	if (FEnemyInfo* EnemyInfo=DataSubsystem->GetEnemyData(NameLevelInfo))
	{
		HealthSet->InitMaxHealth(EnemyInfo->Health);
		HealthSet->SetHealth(EnemyInfo->Health);
		CombatSet->SetAttackPower(EnemyInfo->AttackPower);
		CombatSet->SetDefense(EnemyInfo->Defense);
		CombatSet->SetAttackSpeed(EnemyInfo->AttackSpeed);
		
		//그로기는 현재는 생성자에서 설정
		GroggySet->InitMaxGroggyHealth(MaxGroggyHealth);
		GroggySet->SetGroggyHealth(MaxGroggyHealth);
		UE_LOG(LogTemp,Warning,TEXT("%f"),EnemyInfo->AttackPower);
		
		AbilitySystemComponent->ForceReplication();
	}
}

void AKOBaseEnemy::SetMonsterSaveInfoForLoad(FName InClusterSaveId, FName InMonsterSaveId)
{
	ClusterSaveId = InClusterSaveId;
	MonsterSaveId = InMonsterSaveId;
}

void AKOBaseEnemy::RestoreMonsterFromSave(const FTransform& SavedTransform)
{
	bDeadForSave = false;

	SetActorTransform(SavedTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetCanBeDamaged(true);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (HealthSet)
		{
			AbilitySystemComponent->ApplyModToAttributeUnsafe(
				UKOHealthSet::GetHealthAttribute(),
				EGameplayModOp::Override,
				HealthSet->GetMaxHealth()
			);
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetComponentTickEnabled(true);
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->StopMovementImmediately();
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetHiddenInGame(false);
		MeshComp->SetVisibility(true, true);
		MeshComp->SetComponentTickEnabled(true);
		MeshComp->bPauseAnims = false;
		MeshComp->SetSimulatePhysics(false);

		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.0f);
		}
	}

	OnCharacterReset.ExecuteIfBound();
}

void AKOBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->GiveGrantSet();
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
		
		// Bind Attributes Changed Functions 
		InitializeAttributes(); 
	}
	
	if (GroggySet)
	{
		GroggySet->OnGroggyTriggered.AddUObject(this, &ThisClass::OnGroggyBegin);
	}
	
	//HPBar Binding
	if (EnemyHPBarWidgetComponent)
	{
		if (UKOEnemyHPBar* HPBar = Cast<UKOEnemyHPBar>(EnemyHPBarWidgetComponent->GetWidget()))
		{
			OnHPChangedEvent.BindUObject(HPBar, &UKOEnemyHPBar::OnHPChanged);
			OnBattleEvent.BindUObject(HPBar,&UKOEnemyHPBar::OnBattleChanged);
		}
	}
	
	//LockOn InitLocation & Binding
	if (EnemyLockOnWidgetComponent)
	{
		LocalLockOnInitialLocation=EnemyLockOnWidgetComponent->GetRelativeLocation();
		if (UKOEnemyBaseUI* LockOn = Cast<UKOEnemyBaseUI>(EnemyLockOnWidgetComponent->GetWidget()))
		{
			OnLockOnEvent.BindUObject(LockOn, &UKOEnemyBaseUI::OnVisibilityChanged);
		}
	}
	
	//LockOn Binding
	if (EnemyParriedWidgetComponent)
	{
		if (UKOEnemyBaseUI* Parried = Cast<UKOEnemyBaseUI>(EnemyParriedWidgetComponent->GetWidget()))
		{
			OnParriedEvent.BindUObject(Parried, &UKOEnemyBaseUI::OnVisibilityChanged);
		}
	}
	
}

void AKOBaseEnemy::InitializeAttributes()
{
	Super::InitializeAttributes();
	
	if (HealthSet)
	{
		HealthSet->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);
	}
}

void AKOBaseEnemy::OnCharacterDead(AActor* DeathInstigator)
{
	Super::OnCharacterDead(DeathInstigator);
	
	bDeadForSave = true;

	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		SaveSubsystem->NotifyActorStoppedTargetingPlayer(this);
		SaveSubsystem->MarkMonsterDead(MonsterSaveId);
	}
	
	OnEnemyDead.Broadcast();
	
	DropItem();
	
	// 퀘스트
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifyMonsterKilled(MonsterSaveId);
	}
}


void AKOBaseEnemy::OnHealthChanged(float OldValue, float NewValue)
{
	OnHPChangedEvent.ExecuteIfBound( NewValue/ HealthSet->GetMaxHealth(), OldValue - NewValue);
}

void AKOBaseEnemy::DropItem()
{
	if (UKOEnemyDataSubsystem* DataSubsystem=UKOEnemyDataSubsystem::Get(this))
	{
		
		FEnemyNameLevelInfo EnemyNameLevelInfo;
		EnemyNameLevelInfo.EnemyNameTag=EnemyNameTag;
		EnemyNameLevelInfo.Level=EnemyLevel;
		
		TArray<FEnemyDropItemInfo>* DropItemArray=DataSubsystem->GetEnemyDropItemArray(EnemyNameLevelInfo);
		if (DropItemArray==nullptr)
		{
			return;
		}
		
		float RandValue=FMath::RandRange(0.f,100.f);
		float Value=0.f;
		TMap<FName,int32> ItemMessageMap;
		
		for (FEnemyDropItemInfo& DropItem : *DropItemArray)
		{
			Value=DropItem.DropPercent;
			if (RandValue<=Value)
			{
				ItemMessageMap.FindOrAdd(DropItem.DropItemName)+=DropItem.Count;
			}
		}
		
		for (auto MessagePair : ItemMessageMap)
		{
			FKODropItemMessage ItemMessage;
			ItemMessage.ItemId=MessagePair.Key;
			ItemMessage.Count=MessagePair.Value;
			
			UGMRouterSubsystem::BroadcastMessage(GetWorld(),
				KOGameplayTags::Event_DropItem,
				FInstancedStruct::Make(ItemMessage));
		}
	}
}

void AKOBaseEnemy::OnGroggyBegin()
{
	FGameplayEventData EventData;
	AbilitySystemComponent->HandleGameplayEvent(KOGameplayTags::Event_CounterAttack, &EventData);
}


void AKOBaseEnemy::OnBattleChanged(bool bIsBattle)
{
	OnBattleEvent.ExecuteIfBound(bIsBattle);
}

void AKOBaseEnemy::ChangeLockOnGroggy(bool bIsGroggied)
{
	if (bIsGroggied)
	{
		EnemyLockOnWidgetComponent->AddRelativeLocation(LocalLockOnOffset);
	}
	else
	{
		EnemyLockOnWidgetComponent->SetRelativeLocation(LocalLockOnInitialLocation);
	}
}

FVector AKOBaseEnemy::GetSocketLocation()
{
	if (WeaponMeshComponent->GetSkeletalMeshAsset()!=nullptr)
	{
		return WeaponMeshComponent->GetSocketTransform(WeaponSocketName,RTS_World).GetLocation();
	}
	
	return GetMesh()->GetSocketTransform(SkeletonSocketName,RTS_World).GetLocation();
}

float AKOBaseEnemy::GetAttackPoint()
{
	if (CombatSet) return CombatSet->GetAttackPower();
	
	return 0.f;
}



