#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01GimmickPillar.h"

#include "Karon/AbilitySystem/Tag/KOGameplayTags.h"

AKOBossCH01GimmickPillar::AKOBossCH01GimmickPillar()
{
	PrimaryActorTick.bCanEverTick = false;
 
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

void AKOBossCH01GimmickPillar::BreakPillar()
{
	if (bBroken)
	{
		return;
	}

	bBroken = true;
	ApplyBrokenState();
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
 
