#include "KOWeaponBase.h"
#include "Data/Equipment/KOWeaponDefinition.h"

AKOWeaponBase::AKOWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AKOWeaponBase::InitializeWeapon(const UKOWeaponDefinition* Def)
{
	if (!Def) return;

	WeaponDef = Def;

	UStaticMesh* LoadedMesh = Def->WeaponMesh.LoadSynchronous();
	Mesh->SetStaticMesh(LoadedMesh);
}
