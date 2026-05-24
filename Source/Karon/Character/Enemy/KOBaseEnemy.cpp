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
	WeaponMeshComponent->SetupAttachment(GetMesh(), HandSocketName);
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

FVector AKOBaseEnemy::GetSocketLocation()
{
	if (WeaponMeshComponent->GetSkeletalMeshAsset()!=nullptr)
	{
		return WeaponMeshComponent->GetSocketTransform(WeaponSocketName,RTS_World).GetLocation();
	}
	
	// 1. 컴포넌트 자체가 유효한지 확인
	if (!WeaponMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GetSocketLocation] WeaponMeshComponent가 nullptr입니다!"));
		return FVector::ZeroVector;
	}

	// 2. 메쉬 에셋이 들어있는지 확인
	if (WeaponMeshComponent->GetSkeletalMeshAsset() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GetSocketLocation] SkeletalMeshAsset이 지정되지 않았습니다!"));
		return FVector::ZeroVector;
	}
    
	// 3. 소켓이 실제로 존재하는지 안전검사
	if (!WeaponMeshComponent->DoesSocketExist(WeaponSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GetSocketLocation] 소켓 이름(%s)을 찾을 수 없습니다! 부모 위치를 반환합니다."), *WeaponSocketName.ToString());
		return WeaponMeshComponent->GetComponentLocation(); // 0,0,0 대신 컴포넌트 위치라도 반환
	}
	
	UE_LOG(LogTemp, Warning, TEXT("이외의 이유"));
	//TODO: 무기없을때 소켓 정보 받아오기
	
	return FVector::ZeroVector;
}



