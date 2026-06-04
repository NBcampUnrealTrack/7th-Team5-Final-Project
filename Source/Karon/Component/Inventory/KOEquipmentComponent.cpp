#include "KOEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Items/Equipment/KOWeaponBase.h"

UKOEquipmentComponent::UKOEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOEquipmentComponent::EquipWeapon(UKOWeaponDefinition* Def)
{
	if (!Def) return;

	// 기존 무기가 있으면 선제 해제
	if (CurrentWeaponActor)
	{
		UnequipWeapon();
	}

	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character) return;

	// 무기 액터 스폰
	FActorSpawnParameters Params;
	Params.Owner = Character;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AKOWeaponBase* NewWeapon = GetWorld()->SpawnActor<AKOWeaponBase>(AKOWeaponBase::StaticClass(), Params);
	if (!NewWeapon) return;

	NewWeapon->InitializeWeapon(Def);

	// 초기엔 칼집 소켓에 어태치
	NewWeapon->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		Def->UnEquipSocket
	);

	// GAS 부여
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC && Def->GrantedSet)
	{
		Def->GrantedSet->GiveToAsc(ASC, ActiveHandles);
	}

	CurrentWeaponActor = NewWeapon;
	CurrentWeaponConfig = Def;
	CurrentWeaponSlot = EWeaponSlot::Holster;
}

void UKOEquipmentComponent::UnequipWeapon()
{
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (Character)
	{
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (ASC)
		{
			ActiveHandles.RemoveFromASC(ASC);
		}
	}

	if (CurrentWeaponActor)
	{
		CurrentWeaponActor->Destroy();
		CurrentWeaponActor = nullptr;
	}

	CurrentWeaponConfig = nullptr;
	CurrentWeaponSlot = EWeaponSlot::Holster;
}

void UKOEquipmentComponent::DrawWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Hand) return;

	CurrentWeaponActor->AttachToComponent(
		GetOwner<AKOCharacterBase>()->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentWeaponConfig->EquipSocket
	);

	CurrentWeaponSlot = EWeaponSlot::Hand;
}

void UKOEquipmentComponent::SheatheWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Holster) return;

	CurrentWeaponActor->AttachToComponent(
		GetOwner<AKOCharacterBase>()->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentWeaponConfig->UnEquipSocket
	);

	CurrentWeaponSlot = EWeaponSlot::Holster;
}
