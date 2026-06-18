// Fill out your copyright notice in the Description page of Project Settings.


#include "KOBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "Components/WidgetComponent.h"
#include "Data/Character/Enemy/KOEnemyDataAsset.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"
#include "SubSystem/KOEnemyDataSubsystem.h"
#include "UI/Enemy/KOEnemyHPBar.h"


// Sets default values
AKOBaseEnemy::AKOBaseEnemy(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	//ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	//어트리뷰트셋 생성
	HealthSet=CreateDefaultSubobject<UKOHealthSet>(TEXT("HealthSet"));
	MovementSet=CreateDefaultSubobject<UKOMovementSet>(TEXT("MovementSet"));
	CombatSet=CreateDefaultSubobject<UKOCombatSet>(TEXT("CombatSet"));

	
	//WeaponSkeletalMeshComponent 생성 및 부착
	WeaponMeshComponent=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMeshComponent->SetupAttachment(GetMesh(), HandSocketName);
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	//Enemy HPBar 부착
	EnemyHPBarWidgetComponent=CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComponent"));
	EnemyHPBarWidgetComponent->SetupAttachment(GetMesh());
	EnemyHPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
}

void AKOBaseEnemy::SetupEnemy(UKOEnemyDataSubsystem* DataSubsystem,int32 Level)
{
	if (DataSubsystem==nullptr)
	{
		return;
	}
	
	FEnemyNameLevelInfo NameLevelInfo;
	NameLevelInfo.EnemyNameTag=EnemyNameTag;
	NameLevelInfo.Level=Level;
	
	if (FEnemyInfo* EnemyInfo=DataSubsystem->GetEnemyData(NameLevelInfo))
	{
		HealthSet->InitMaxHealth(EnemyInfo->Health);
		HealthSet->InitHealth(EnemyInfo->Health);
		CombatSet->InitAttackPower(EnemyInfo->AttackPower);
		CombatSet->InitDefense(EnemyInfo->Defense);
		CombatSet->InitAttackSpeed(EnemyInfo->AttackSpeed);
	}
}

// Called when the game starts or when spawned
void AKOBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->GiveGrantSet();
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
		
		//ASC Duration Callback
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute())
		.AddUObject(this, &AKOBaseEnemy::OnHitCallback);
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
	
}


void AKOBaseEnemy::OnHitCallback(const FOnAttributeChangeData& Data)
{
	//체력이 0이라면 사망 콟백을 HPBar, AIController로 전달
	if (Data.NewValue==0.f)
	{
		OnHPChangedEvent.ExecuteIfBound(0.f,Data.OldValue-Data.NewValue);
		OnEnemyDead.Broadcast();
	}
	
	//체력이 감소했다면 피격 콜백을 HPBar, AIController로 전달
	else if (Data.NewValue<Data.OldValue)
	{
		OnHPChangedEvent.ExecuteIfBound(Data.NewValue/HealthSet->GetMaxHealth(),Data.OldValue-Data.NewValue);
		OnCharacterHit.ExecuteIfBound();
	}
}

void AKOBaseEnemy::OnBattleChanged(bool bIsBattle)
{
	OnBattleEvent.ExecuteIfBound(bIsBattle);
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
	if (CombatSet)
	{
		return CombatSet->GetAttackPower();
	}
	return 0.f;
}



