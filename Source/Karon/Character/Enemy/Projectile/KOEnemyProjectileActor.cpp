// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyProjectileActor.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/Effect/KOGameplayEffectContext.h"
#include "Data/KO_HitData.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "Components/SphereComponent.h"
#include "Game/KOProjectilePoolSubsystem.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AKOEnemyProjectileActor::AKOEnemyProjectileActor()
{
	//루트 스피어 컴포넌트로 충돌 판정을 진행
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAll"));
	SphereComponent->OnComponentHit.AddDynamic(this, &AKOEnemyProjectileActor::OnProjectileHit);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECR_Ignore);
	//스태틱 메시 설정 및 콜리전 해제
	ProjectileStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ProjectileStaticMesh->SetupAttachment(SphereComponent);
	ProjectileStaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileStaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECR_Ignore);
	//PMC 컴포넌트 생성
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	//PMC 설정. 디폴트 초기속도 : 1000.f
	ProjectileMovementComponent->InterpLocationTime = 0.05f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->InitialSpeed = 1000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	
	//나이아가라 컴포넌트 생성
	TrailEffectComponent=CreateDefaultSubobject<UNiagaraComponent>("TrailEffectComponent");
	TrailEffectComponent->SetupAttachment(SphereComponent);
	TrailEffectComponent->bAutoActivate = false;
	
	//풀링 대기로 멈춰있는다.
	ProjectileMovementComponent->bAutoActivate= false;	

}

// Called when the game starts or when spawned
void AKOEnemyProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	//스폰시에는 Active하지 않음
	SetActiveAndCollision(false);
}

void AKOEnemyProjectileActor::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetWorld()==nullptr||GetWorld()->GetSubsystem<UKOProjectilePoolSubsystem>()==nullptr)
	{
		return;
	}
	AKOHeroCharacter* HittedCharacter=Cast<AKOHeroCharacter>(OtherActor);
	//충돌했는데 플레이어 캐릭터가 아니라면 풀로 되돌린다.
	if (!HittedCharacter)
	{
		UE_LOG(LogTemp,Warning,TEXT("%s"),*OtherActor->GetName());
		ReturnToPool();
		return;
	}
	
	//공격자 다운캐스팅
	AKOBaseEnemy* AttackedCharacter=Cast<AKOBaseEnemy>(GetOwner());
	
	//맞은 플레이어의 ASC를 가져온다.
	UAbilitySystemComponent* TargetASC = HittedCharacter->GetAbilitySystemComponent();
	if (TargetASC == nullptr)
	{
		ReturnToPool();
		return;
	}
	
	//공격자의 ASC를 가져온다.
	UAbilitySystemComponent* CharacterASC = AttackedCharacter->GetAbilitySystemComponent();
	if (!CharacterASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = CharacterASC->MakeEffectContext();
	Context.AddSourceObject(AttackedCharacter); // 소스 오브젝트는 현재 캐릭터(Avatar)
	
	if (FKOGameplayEffectContext* KOContext = static_cast<FKOGameplayEffectContext*>(Context.Get()))
	{
		if (CachedHitData)
		{
			KOContext->SetHitData(CachedHitData);
		}
	}
	
	FGameplayEffectSpecHandle SpecHandle = CharacterASC->MakeOutgoingSpec(Enemy->ProjectileDamageEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid() )
	{
		SpecHandle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_AttackCoefficient, ProjectileDamage);
		CharacterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
	ReturnToPool();
}

void AKOEnemyProjectileActor::SetProjectile(AKOBaseEnemy* InEnemy,float AttackPoint,float DamageMultiplier)
{
	SetOwner(InEnemy);
	Enemy=InEnemy;
	SetActorScale3D(InEnemy->ProjectileScale);
	ProjectileStaticMesh->SetStaticMesh(InEnemy->ProjectileMesh);
	TrailEffectComponent->SetAsset(InEnemy->ImpactEffect);
	TrailEffectComponent->Activate(true);
	SphereComponent->IgnoreActorWhenMoving(InEnemy,true);
	ProjectileDamage=DamageMultiplier;
}

void AKOEnemyProjectileActor::SetActiveAndCollision(bool InActive)
{
	if (InActive)
	{
		if (!SphereComponent||!ProjectileMovementComponent)
		{
			return;
		}
		SetActorHiddenInGame(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		ProjectileMovementComponent->Activate(true);
		
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle,this,&AKOEnemyProjectileActor::LifeTimeEnd,5.0f,false);
		
	}
	else
	{
		if (!SphereComponent||!ProjectileMovementComponent)
		{
			return;
		}
		SetActorHiddenInGame(true);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void AKOEnemyProjectileActor::LifeTimeEnd()
{
	ReturnToPool();
}

void AKOEnemyProjectileActor::ReturnToPool()
{
	if (GetWorld()&&GetWorld()->GetSubsystem<UKOProjectilePoolSubsystem>())
	{
		UE_LOG(LogTemp,Warning,TEXT("returntopool"))
		//Owner를 비워준다.
		SetOwner(nullptr);
		ProjectileStaticMesh->SetStaticMesh(nullptr);
		TrailEffectComponent->SetAsset(nullptr);
		TrailEffectComponent->Activate(false);
		SphereComponent->IgnoreActorWhenMoving(Enemy,false);
		CachedHitData = nullptr;
		GetWorld()->GetSubsystem<UKOProjectilePoolSubsystem>()->ReturnToPool(this);
	}
}


