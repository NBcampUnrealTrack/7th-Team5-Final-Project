#include "Character/Enemy/Boss/Gimmick/KOBossCH01GimmickPillar.h"

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
}
 
