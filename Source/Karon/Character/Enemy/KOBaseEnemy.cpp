// Fill out your copyright notice in the Description page of Project Settings.


#include "KOBaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "Component/KOAnimNotifyComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/Character/Enemy/KOEnemyDataAsset.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"
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
	//TODO: 공격력 DDD로 전환. 현재는 테스트용 공격력 10
	CombatSet->InitAttackPower(10.f);
	
	//AnimNotifyComponent 생성
	AnimNotifyComponent=CreateDefaultSubobject<UKOAnimNotifyComponent>(TEXT("KOAnimNotifyComponent"));
	
	//WeaponSkeletalMeshComponent 생성 및 부착
	WeaponMeshComponent=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMeshComponent->SetupAttachment(GetMesh(), HandSocketName);
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	//Enemy HPBar 부착
	EnemyHPBarWidgetComponent=CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComponent"));
	EnemyHPBarWidgetComponent->SetupAttachment(GetMesh());
	EnemyHPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
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
	
	if (IsValid(AbilitySystemComponent))
	{
		GiveDefaultAbilities();
		
		//ASC Duration Callback

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute())
		.AddUObject(this, &AKOBaseEnemy::OnHitCallback);
	}
	//HPBar Binding
	if (EnemyHPBarWidgetComponent)
	{
		if (UKOEnemyHPBar* HPBar = Cast<UKOEnemyHPBar>(EnemyHPBarWidgetComponent->GetWidget()))
		{
			OnHPChanged.BindUObject(HPBar, &UKOEnemyHPBar::OnHPChanged);
		}
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

void AKOBaseEnemy::OnHitCallback(const FOnAttributeChangeData& Data)
{
	//체력이 0이라면 사망 콟백을 HPBar, AIController로 전달
	if (Data.NewValue==0.f)
	{
		OnHPChanged.ExecuteIfBound(0.f);
		OnCharacterDead.ExecuteIfBound();
	}
	
	//체력이 감소했다면 피격 콜백을 HPBar, AIController로 전달
	else if (Data.NewValue<Data.OldValue)
	{
		OnHPChanged.ExecuteIfBound(Data.NewValue/HealthSet->GetMaxHealth());
		OnCharacterHit.ExecuteIfBound();
	}
}

FVector AKOBaseEnemy::GetSocketLocation()
{
	if (WeaponMeshComponent->GetSkeletalMeshAsset()!=nullptr)
	{
		return WeaponMeshComponent->GetSocketTransform(WeaponSocketName,RTS_World).GetLocation();
	}
	
	
	
	//TODO: 무기없을때 소켓 정보 받아오기
	
	return FVector::ZeroVector;
}



