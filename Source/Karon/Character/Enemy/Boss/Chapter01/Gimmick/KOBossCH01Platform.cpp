#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01Platform.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AKOBossCH01Platform::AKOBossCH01Platform()
{
	PrimaryActorTick.bCanEverTick = true;
 
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = PlatformMesh;
 
	DamageCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageCollision"));
	DamageCollision->SetupAttachment(RootComponent);
	DamageCollision->SetCollisionProfileName(TEXT("BlockAll"));
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 
	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	IndicatorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	IndicatorMesh->SetVisibility(false);
}
 
void AKOBossCH01Platform::BeginPlay()
{
	Super::BeginPlay();
 
	// 충돌 바인딩
	DamageCollision->OnComponentHit.AddDynamic(
		this, &AKOBossCH01Platform::OnDamageCollisionHit
	);
 
	// 인디케이터 바닥 위치
	GetWorldTimerManager().SetTimerForNextTick(
		this, &AKOBossCH01Platform::SpawnIndicatorOnGround
	);
	
	GetWorldTimerManager().SetTimer(
		HoverTimerHandle,
		this,
		&AKOBossCH01Platform::StartFall,
		HoverDuration,
		false
	);
}
 
void AKOBossCH01Platform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	// 착지 후 플레이어 위치 체크
	if (bLanded)
	{
		CheckPlayerOnPlatform();
		return;
	}
 
	if (!bFalling)
	{
		return;
	}
 
	FVector NewLocation = GetActorLocation();
	NewLocation.Z -= FallSpeed * DeltaTime;

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
 
	const float MeshHalfHeight = PlatformMesh->Bounds.BoxExtent.Z;
 
	if (GetWorld()->LineTraceSingleByChannel(
		GroundHit,
		NewLocation,
		NewLocation - FVector(0.f, 0.f, MeshHalfHeight + 1.f),
		ECC_WorldStatic,
		QueryParams))
	{
		// 바닥 위에 정확히 위치 고정
		NewLocation.Z = GroundHit.ImpactPoint.Z + MeshHalfHeight;
		SetActorLocation(NewLocation);
		if (!bLanded) { OnLanded(); }
		return;
	}
 
	SetActorLocation(NewLocation);
}
 
// 플레이어 위치 기반 태그 체크
void AKOBossCH01Platform::CheckPlayerOnPlatform()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!PlayerCharacter)
	{
		return;
	}
 
	FVector PlatformLoc = GetActorLocation();
	FVector MeshExtent = PlatformMesh->Bounds.BoxExtent;
 
	// 플레이어 발 위치
	const float HalfHeight = PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float PlayerFeetZ = PlayerCharacter->GetActorLocation().Z - HalfHeight;
	
	const float PlatformTopZ = PlatformLoc.Z + MeshExtent.Z;
	const float DeltaX = FMath::Abs(PlayerCharacter->GetActorLocation().X - PlatformLoc.X);
	const float DeltaY = FMath::Abs(PlayerCharacter->GetActorLocation().Y - PlatformLoc.Y);
 
	const bool bInRange = DeltaX <= MeshExtent.X && DeltaY <= MeshExtent.Y;
	const bool bOnTop = PlayerFeetZ >= PlatformTopZ - PlatformTopTolerance;
 
	// 태그 부여 범위 디버그
	// 초록 : 범위 안 / 빨강 : 범위 밖
	if (bShowDebug)
	{
		const FColor BoxColor = (bInRange && bOnTop) ? FColor::Green : FColor::Red;
		DrawDebugBox(
			GetWorld(),
			FVector(PlatformLoc.X, PlatformLoc.Y,
				PlatformTopZ - PlatformTopTolerance),
			FVector(MeshExtent.X, MeshExtent.Y, PlatformTopTolerance),
			BoxColor,
			false,
			-1.f,
			0,
			2.f
		);
	}
	
	if (bInRange && bOnTop)
	{
		// 아직 발판 위에 없으면 태그 부여
		if (PlayerOnPlatform != PlayerCharacter)
		{
			PlayerOnPlatform = PlayerCharacter;
			SetOnPlatformTag(PlayerCharacter, true);
		}
	}
	else
	{
		// 발판에서 벗어났으면 태그 제거
		if (PlayerOnPlatform == PlayerCharacter)
		{
			PlayerOnPlatform = nullptr;
			SetOnPlatformTag(PlayerCharacter, false);
		}
	}
}
 
void AKOBossCH01Platform::SpawnIndicatorOnGround()
{
	FHitResult GroundHit;
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 5000.f);
 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
 
	if (GetWorld()->LineTraceSingleByChannel(
		GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		IndicatorMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		IndicatorMesh->SetWorldLocation(GroundHit.ImpactPoint + FVector(0.f, 0.f, 1.f));
 
		// 발판 로컬 크기에 맞게 스케일 설정
		FBoxSphereBounds LocalBounds = PlatformMesh->CalcLocalBounds();
		FVector LocalExtent = LocalBounds.BoxExtent * PlatformMesh->GetComponentScale();
		
		IndicatorMesh->SetWorldScale3D(FVector(
			LocalExtent.X / 50.f,
			LocalExtent.Y / 50.f,
			1.f
		));
 
		// 발판 Yaw 회전 반영
		IndicatorMesh->SetWorldRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
		IndicatorMesh->SetVisibility(true);
	}
}
 
// 대기 후 낙하 시작
void AKOBossCH01Platform::StartFall()
{
	bFalling = true;
	IndicatorMesh->SetVisibility(false);
}
 
// 낙석 충돌 데미지처리
void AKOBossCH01Platform::OnDamageCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor)
	{
		return;
	}
	
	if (OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
	{
		return;
	}
	
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASI)
	{
		return;
	}
	
	ApplyDamageToTarget(OtherActor);
	Destroy();
}

void AKOBossCH01Platform::OnLanded()
{
	bLanded = true;
 
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageCollision->OnComponentHit.RemoveAll(this);
 
	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));
 
	GetWorldTimerManager().SetTimer(
		LifeSpanTimerHandle,
		this,
		&AKOBossCH01Platform::LifeTimeEnd,
		PlatformLifeSpan,
		false
	);
}
 
void AKOBossCH01Platform::LifeTimeEnd()
{
	if (PlayerOnPlatform)
	{
		SetOnPlatformTag(PlayerOnPlatform, false);
		PlayerOnPlatform = nullptr;
	}
 
	Destroy();
}
 
void AKOBossCH01Platform::SetOnPlatformTag(AActor* TargetActor, bool bAdd)
{
	if (!TargetActor)
	{
		return;
	}
 
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!ASI)
	{
		return;
	}
 
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
 
	if (bAdd)
	{
		ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_OnPlatform);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_OnPlatform);
	}
}
 
void AKOBossCH01Platform::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!FallDamageEffectClass || !TargetActor)
	{
		return;
	}
	
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI)
	{
		return;
	}
 
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}
 
	IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!OwnerASI)
	{
		return;
	}
 
	UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}
 
	const UKOCombatSet* CombatSet = OwnerASC->GetSet<UKOCombatSet>();
	if (!CombatSet)
	{
		return;
	}
 
	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(GetOwner());
	
	FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(FallDamageEffectClass, 1.f, Context);
 
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Damage,CombatSet->GetAttackPower());
		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
