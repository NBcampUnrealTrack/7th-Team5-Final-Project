#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01Platform.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AKOBossCH01Platform::AKOBossCH01Platform()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlatformRoot"));
	RootComponent = PlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootComponent);
	PlatformMesh->SetCollisionProfileName(TEXT("NoCollision"));
 
	DamageCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageCollision"));
	DamageCollision->SetupAttachment(RootComponent);
	
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageCollision->SetGenerateOverlapEvents(true);
 
	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	IndicatorMesh->SetupAttachment(RootComponent);
	IndicatorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	IndicatorMesh->SetVisibility(false);
}
 
void AKOBossCH01Platform::BeginPlay()
{
	Super::BeginPlay();
 
	// 충돌 바인딩
	DamageCollision->OnComponentBeginOverlap.AddDynamic(
		this, &AKOBossCH01Platform::OnDamageCollisionBeginOverlap
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
	UpdateDebrisFade(DeltaTime);

	if (bBroken)
	{
		return;
	}

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

void AKOBossCH01Platform::OnDamageCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASI)
	{
		return;
	}

	BreakApart();
}

void AKOBossCH01Platform::OnLanded()
{
	bLanded = true;
 
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageCollision->OnComponentBeginOverlap.RemoveAll(this);
 
	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));
	
	if (LandSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandSFX, GetActorLocation());
	}
 
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
	BreakApart();
}

// 파편을 무작위로 흩뿌리고 잠시 뒤 액터를 제거
void AKOBossCH01Platform::BreakApart()
{
	bBroken = true;
	
	if (PlayerOnPlatform)
	{
		SetOnPlatformTag(PlayerOnPlatform, false);
		PlayerOnPlatform = nullptr;
	}
	
	if (BreakSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, GetActorLocation());
	}
	
	PlatformMesh->SetVisibility(false);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UStaticMesh* FragmentMesh = DebrisMesh.Get();
	if (!FragmentMesh)
	{
		FragmentMesh = PlatformMesh->GetStaticMesh();
	}

	if (FragmentMesh)
	{
		const FVector MeshExtent = PlatformMesh->Bounds.BoxExtent;
		const FVector Origin = GetActorLocation();
		const int32 SpawnCount = FMath::RandRange(DebrisMinCount, DebrisMaxCount);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			// 발판 범위 안 무작위 위치에서 파편 스폰
			const FVector RandomOffset(
				FMath::FRandRange(-MeshExtent.X, MeshExtent.X),
				FMath::FRandRange(-MeshExtent.Y, MeshExtent.Y),
				FMath::FRandRange(-MeshExtent.Z * 0.5f, MeshExtent.Z * 0.5f)
			);
			const FRotator RandomRotation(
				FMath::FRandRange(0.f, 360.f),
				FMath::FRandRange(0.f, 360.f),
				FMath::FRandRange(0.f, 360.f)
			);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AStaticMeshActor* Debris = GetWorld()->SpawnActor<AStaticMeshActor>(
				Origin + RandomOffset, RandomRotation, SpawnParams);

			if (!Debris)
			{
				continue;
			}

			UStaticMeshComponent* DebrisComp = Debris->GetStaticMeshComponent();
			DebrisComp->SetMobility(EComponentMobility::Movable);
			DebrisComp->SetStaticMesh(FragmentMesh);

			// 발판 재질을 그대로 사용해 시각적으로 이질감이 없게
			for (int32 MatIndex = 0; MatIndex < PlatformMesh->GetNumMaterials(); ++MatIndex)
			{
				DebrisComp->SetMaterial(MatIndex, PlatformMesh->GetMaterial(MatIndex));
			}

			const float RandomScale = FMath::FRandRange(DebrisScaleRange.X, DebrisScaleRange.Y);
			Debris->SetActorScale3D(FVector(RandomScale));

			DebrisComp->SetCollisionProfileName(TEXT("PhysicsActor"));
			DebrisComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			DebrisComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			DebrisComp->SetSimulatePhysics(true);

			// 무작위 방향으로 튕겨나가게
			DebrisComp->AddImpulse(FMath::VRand() * DebrisImpulseStrength, NAME_None, true);
			DebrisComp->AddAngularImpulseInDegrees(FMath::VRand() * DebrisImpulseStrength, NAME_None, true);
			
			FDebrisFadeInfo FadeInfo;
			FadeInfo.DebrisActor = Debris;
			FadeInfo.InitialScale = Debris->GetActorScale3D();
			FadingDebris.Add(FadeInfo);
		}
	}
	
	SetLifeSpan(DebrisLifeSpan + 0.1f);
}

void AKOBossCH01Platform::UpdateDebrisFade(float DeltaTime)
{
	if (FadingDebris.Num() == 0)
	{
		return;
	}
 
	const float FadeStartTime = FMath::Max(DebrisLifeSpan - DebrisFadeOutDuration, 0.f);
 
	for (int32 i = FadingDebris.Num() - 1; i >= 0; --i)
	{
		FDebrisFadeInfo& Info = FadingDebris[i];
 
		AStaticMeshActor* DebrisActor = Info.DebrisActor.Get();
		if (!DebrisActor)
		{
			FadingDebris.RemoveAtSwap(i);
			continue;
		}
 
		Info.ElapsedTime += DeltaTime;
 
		if (Info.ElapsedTime < FadeStartTime)
		{
			continue;
		}
		
		if (!Info.bFadeStarted)
		{
			Info.bFadeStarted = true;
			if (UStaticMeshComponent* DebrisComp = DebrisActor->GetStaticMeshComponent())
			{
				DebrisComp->SetSimulatePhysics(false);
				DebrisComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
 
		const float FadeAlpha = FMath::Clamp(
			(Info.ElapsedTime - FadeStartTime) / FMath::Max(DebrisFadeOutDuration, KINDA_SMALL_NUMBER),
			0.f, 1.f
		);
 
		DebrisActor->SetActorScale3D(Info.InitialScale * (1.f - FadeAlpha));
 
		if (FadeAlpha >= 1.f)
		{
			DebrisActor->Destroy();
			FadingDebris.RemoveAtSwap(i);
		}
	}
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
