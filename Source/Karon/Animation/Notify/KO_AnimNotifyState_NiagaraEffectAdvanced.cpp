#include "KO_AnimNotifyState_NiagaraEffectAdvanced.h"

#include "NiagaraComponent.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Items/Equipment/KOWeaponBase.h"

void UKO_AnimNotifyState_NiagaraEffectAdvanced::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(GetSpawnedEffect(MeshComp));
	if (!NiagaraComp) return;

	AKOCharacterBase* Character = Cast<AKOCharacterBase>(MeshComp->GetOwner());
	if (!Character) return;

	UMeshComponent* TraceMesh = nullptr;
	UKOEquipmentComponent* EquipComp = Character->GetEquipmentComponent();

	if (EquipComp && EquipComp->HasWeapon())
	{
		AKOWeaponBase* WeaponActor = EquipComp->CurrentWeaponActor;
		if (UStaticMeshComponent* StaticMesh = WeaponActor->GetMesh())
		{
			TraceMesh = StaticMesh;
		}
	}

	if (!TraceMesh)
	{
		TraceMesh = MeshComp;
	}

	if (TraceMesh)
	{
		FVector StartLocation = TraceMesh->GetSocketLocation(StartSocketName);
		FVector EndLocation = TraceMesh->GetSocketLocation(EndSocketName);

		NiagaraComp->SetVariableVec3(FName("StartPoint"), StartLocation);
		NiagaraComp->SetVariableVec3(FName("EndPoint"), EndLocation);
	}
}
