// Fill out your copyright notice in the Description page of Project Settings.


#include "KOBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "Component/KOAnimNotifyComponent.h"
#include "Data/Character/Enemy/KOEnemyDataAsset.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"


// Sets default values
AKOBaseEnemy::AKOBaseEnemy(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	//ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UKOAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	//어트리뷰트셋 생성
	HealthSet=CreateDefaultSubobject<UKOHealthSet>(TEXT("HealthSet"));
	MovementSet=CreateDefaultSubobject<UKOMovementSet>(TEXT("MovementSet"));
	CombatSet=CreateDefaultSubobject<UKOCombatSet>(TEXT("CombatSet"));
	
	//AnimNotifyComponent 생성
	AnimNotifyComponent=CreateDefaultSubobject<UKOAnimNotifyComponent>(TEXT("KOAnimNotifyComponent"));
	
	//WeaponSkeletalMeshComponent 생성 및 부착
	WeaponMeshComponent=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_r_Socket"));
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
}

void AKOBaseEnemy::SetupEnemy(UKOEnemyDataAsset)
{
	//TODO: 비동기 로드한 데이터로 해당 Enemy에 값을 주입
	//TODO: 무기도 여기서 설정
	//TODO: 오브젝트풀로 돌릴때 무기 Mesh를 nullptr로 변경
}

// Called when the game starts or when spawned
void AKOBaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	//
	if (IsValid(AbilitySystemComponent))
	{
		GiveDefaultAbilities();
	}
}

void AKOBaseEnemy::GiveDefaultAbilities()
{
	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			// Ability Spec 생성
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			// ASC에 Ability 부여
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}



