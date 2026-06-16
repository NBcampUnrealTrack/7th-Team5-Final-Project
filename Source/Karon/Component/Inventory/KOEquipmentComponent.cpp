#include "KOEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Items/Equipment/KOWeaponBase.h"
#include "Utility/Log/KOLogManager.h"


UKOEquipmentComponent::UKOEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character) return;
	
	SkeletalMesh = Character->GetMesh();
	BodyMesh = SkeletalMesh; 
	
	DefaultAnimLayerClass = SkeletalMesh->GetAnimClass(); 
}

void UKOEquipmentComponent::EquipWeapon(UKOWeaponDefinition* Def)
{
	if (!Def) return;
	
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character || !BodyMesh) return;
	
	bool WeaponDrawn = IsWeaponDrawn(); 
	if (CurrentWeaponActor)
	{
		UnequipWeapon();
	}
	
	FActorSpawnParameters Params;
	Params.Owner = Character;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AKOWeaponBase* NewWeapon = GetWorld()->SpawnActor<AKOWeaponBase>(AKOWeaponBase::StaticClass(), Params);
	if (!NewWeapon) return; 
	
	NewWeapon->InitializeWeapon(Def);
	
	NewWeapon->AttachToComponent(
		BodyMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponDrawn ? Def->EquipSocket : Def->UnEquipSocket
	);
	
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC && Def->GrantedSet)
	{
		Def->GrantedSet->GiveToAsc(ASC, ActiveHandles);
	}

	CurrentWeaponActor = NewWeapon;
	CurrentWeaponConfig = Def;
	
	SetWeaponSlot(WeaponDrawn ? EWeaponSlot::Hand : EWeaponSlot::Holster);
	
	KO_LOG(GAS,Warning, TEXT("Equiped Weapon : %s"), *Def->WeaponName.ToString());
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
	SetWeaponSlot(CurrentWeaponSlot = EWeaponSlot::Holster); 
}

void UKOEquipmentComponent::DrawWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Hand) return;

	CurrentWeaponActor->AttachToComponent(
		BodyMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentWeaponConfig->EquipSocket
	);
	SetWeaponSlot(EWeaponSlot::Hand);
}

void UKOEquipmentComponent::SheatheWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Holster) return;

	CurrentWeaponActor->AttachToComponent(
	BodyMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentWeaponConfig->UnEquipSocket
	);
	
	SetWeaponSlot(EWeaponSlot::Holster);
}

void UKOEquipmentComponent::SetWeaponSlot(EWeaponSlot NewSlot)
{
	CurrentWeaponSlot = NewSlot; 
	
	if (!SkeletalMesh) return;
	
	TSubclassOf<UAnimInstance> NewAnimLayer; 
	
	if (CurrentWeaponConfig)
	{
		if (NewSlot == EWeaponSlot::Hand)
		{
			NewAnimLayer = CurrentWeaponConfig->WeaponABP_Carrying ? 
				CurrentWeaponConfig->WeaponABP_Carrying : nullptr; 
		}
		else
		{
			NewAnimLayer = CurrentWeaponConfig->WeaponABP_Sheathed ? 
				CurrentWeaponConfig->WeaponABP_Sheathed : nullptr; 
		}
	}
	NewAnimLayer = NewAnimLayer ? NewAnimLayer : DefaultAnimLayerClass; 
	
	SkeletalMesh->LinkAnimClassLayers(NewAnimLayer);
	UE_LOG(LogTemp, Warning, TEXT("NewAnimLayer : %s"),
	   NewAnimLayer ? 
	   *NewAnimLayer->GetDisplayNameText().ToString() : 
	   TEXT("None")
   );
}
