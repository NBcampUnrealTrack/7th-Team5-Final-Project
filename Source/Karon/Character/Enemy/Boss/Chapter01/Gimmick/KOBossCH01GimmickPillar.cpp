#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01GimmickPillar.h"

#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Karon/AbilitySystem/Tag/KOGameplayTags.h"
#include "Engine/StaticMeshActor.h"

AKOBossCH01GimmickPillar::AKOBossCH01GimmickPillar()
{
	PrimaryActorTick.bCanEverTick = true;
 
	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMesh->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = PillarMesh;
 
	// 돌진 GA에서 태그로 기둥 구분
	Tags.Add(KOGameplayTags::Object_BossCH01_Gimmick_Pillar.GetTag().GetTagName());
}

void AKOBossCH01GimmickPillar::BeginPlay()
{
	Super::BeginPlay();
	
	ApplyBrokenState();
}

void AKOBossCH01GimmickPillar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateDebrisFade(DeltaTime);
}

void AKOBossCH01GimmickPillar::BreakPillar()
{
	if (bBroken)
	{
		return;
	}

	bBroken = true;
	ApplyBrokenState();
	
	// 기둥 파괴 시 파편 생성 연출 실행
	ExplodeDebris();
}

void AKOBossCH01GimmickPillar::RestoreFromSave(bool bSavedBroken)
{
	bBroken = bSavedBroken;
	ApplyBrokenState();
}

void AKOBossCH01GimmickPillar::ApplyBrokenState()
{
	if (!PillarMesh)
	{
		return;
	}

	if (bBroken)
	{
		PillarMesh->SetVisibility(false, true);
		PillarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PillarMesh->SetGenerateOverlapEvents(false);

		SetActorEnableCollision(false);
		SetCanBeDamaged(false);
	}
	else
	{
		PillarMesh->SetVisibility(true, true);
		PillarMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PillarMesh->SetCollisionProfileName(TEXT("BlockAll"));
		PillarMesh->SetGenerateOverlapEvents(true);

		SetActorEnableCollision(true);
		SetCanBeDamaged(true);
	}
}

void AKOBossCH01GimmickPillar::ExplodeDebris()
{
	const FVector Origin = GetActorLocation();

	if (DustEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DustEffect,
			Origin,
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}
	
	UStaticMesh* FragmentMesh = DebrisMesh.Get();
	if (!FragmentMesh && PillarMesh)
	{
		FragmentMesh = PillarMesh->GetStaticMesh();
	}

	if (FragmentMesh && PillarMesh)
	{
		const FVector MeshExtent = PillarMesh->Bounds.BoxExtent;
		const int32 SpawnCount = FMath::RandRange(DebrisMinCount, DebrisMaxCount);

		for (int32 i = 0; i < SpawnCount; ++i)
		{
			// 기둥 범위 안 무작위 위치에서 파편 스폰
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

			// 기둥의 원래 재질을 그대로 적용
			for (int32 MatIndex = 0; MatIndex < PillarMesh->GetNumMaterials(); ++MatIndex)
			{
				DebrisComp->SetMaterial(MatIndex, PillarMesh->GetMaterial(MatIndex));
			}

			const float RandomScale = FMath::FRandRange(DebrisScaleRange.X, DebrisScaleRange.Y);
			Debris->SetActorScale3D(FVector(RandomScale));

			DebrisComp->SetCollisionProfileName(TEXT("PhysicsActor"));
			DebrisComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			DebrisComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			DebrisComp->SetSimulatePhysics(true);

			// 무작위 방향으로 튕겨나가게 처리
			DebrisComp->AddImpulse(FMath::VRand() * DebrisImpulseStrength, NAME_None, true);
			DebrisComp->AddAngularImpulseInDegrees(FMath::VRand() * DebrisImpulseStrength, NAME_None, true);
			
			FDebrisFadeInfo FadeInfo;
			FadeInfo.DebrisActor = Debris;
			FadeInfo.InitialScale = Debris->GetActorScale3D();
			FadingDebris.Add(FadeInfo);
		}
	}
}

void AKOBossCH01GimmickPillar::UpdateDebrisFade(float DeltaTime)
{
	if (FadingDebris.Num() == 0)
	{
		if (bBroken)
		{
			PrimaryActorTick.SetTickFunctionEnable(false);
		}
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
 
