#include "KOEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Items/Equipment/KOWeaponBase.h"
#include "Utility/Log/KOLogManager.h"
#include "GameplayTagContainer.h"


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
	
	FTransform WeaponSocketLocal = NewWeapon->GetMesh()->GetSocketTransform(
		Def->GripSocket,
		ERelativeTransformSpace::RTS_Component
	);
	NewWeapon->SetActorRelativeTransform(WeaponSocketLocal.Inverse());

	CurrentWeaponActor = NewWeapon;
	CurrentWeaponConfig = Def;
	
	SetWeaponSlot(WeaponDrawn ? EWeaponSlot::Hand : EWeaponSlot::Holster);
	
	KO_LOG(GAS,Warning, TEXT("Equiped Weapon : %s"), *Def->WeaponName.ToString());
}


void UKOEquipmentComponent::UnequipWeapon()
{	
	// 1. 무기 정보가 살아 있을 때 먼저 애니메이션을 Holster 상태로 돌린다.
	if (CurrentWeaponConfig)
	{
		SetWeaponSlot(EWeaponSlot::Holster);
	}
	else
	{
		CurrentWeaponSlot = EWeaponSlot::Holster;

		if (SkeletalMesh && DefaultAnimLayerClass)
		{
			SkeletalMesh->LinkAnimClassLayers(DefaultAnimLayerClass);
		}
	}
	
	// 2. GAS 무기 능력 제거
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (Character)
	{
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (ASC)
		{
			ActiveHandles.RemoveFromASC(ASC);
		}
	}

	// 3. 무기 액터 제거
	if (CurrentWeaponActor)
	{
		CurrentWeaponActor->Destroy();
		CurrentWeaponActor = nullptr;
	}
	
	// 4. 마지막에 무기 상태를 비운다.
	CurrentWeaponConfig = nullptr;
	CurrentWeaponItemId = NAME_None;
	CurrentWeaponSlot = EWeaponSlot::Holster;
	
	SyncWeaponDrawnTagToASC();
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

void UKOEquipmentComponent::ToggleWeaponDrawState()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig)
	{
		return;
	}

	if (CurrentWeaponSlot == EWeaponSlot::Hand)
	{
		SheatheWeapon();
	}
	else
	{
		DrawWeapon();
	}
}

bool UKOEquipmentComponent::EquipWeaponFromItem(FName InWeaponItemId, UKOWeaponDefinition* Def)
{
	if (InWeaponItemId.IsNone() || !Def)
	{
		return false;
	}

	EquipWeapon(Def);

	if (!CurrentWeaponActor || CurrentWeaponConfig != Def)
	{
		SyncWeaponDrawnTagToASC();
		return false;
	}

	CurrentWeaponItemId = InWeaponItemId;

	SyncWeaponDrawnTagToASC();

	return true;
}

bool UKOEquipmentComponent::RestoreWeaponFromSave(FName InWeaponItemId, UKOWeaponDefinition* Def, EWeaponSlot SavedSlot)
{
	if (InWeaponItemId.IsNone() || !Def)
	{
		UnequipWeapon();
		SyncWeaponDrawnTagToASC();
		return false;
	}

	EquipWeapon(Def);

	CurrentWeaponItemId = InWeaponItemId;
	
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[EquipmentLoad] Owner=%s Component=%s Ptr=%p ItemId=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(this),
		this,
		*CurrentWeaponItemId.ToString()
	);

	if (SavedSlot == EWeaponSlot::Hand)
	{
		DrawWeapon();
	}
	else
	{
		SheatheWeapon();
	}

	SyncWeaponDrawnTagToASC();
	return true;
}

void UKOEquipmentComponent::SetWeaponSlot(EWeaponSlot NewSlot)
{
	CurrentWeaponSlot = NewSlot;

	if (SkeletalMesh)
	{
		TSubclassOf<UAnimInstance> NewAnimLayer;

		if (CurrentWeaponConfig)
		{
			if (NewSlot == EWeaponSlot::Hand)
			{
				NewAnimLayer = CurrentWeaponConfig->WeaponABP_Carrying
					? CurrentWeaponConfig->WeaponABP_Carrying
					: nullptr;
			}
			else
			{
				NewAnimLayer = CurrentWeaponConfig->WeaponABP_Sheathed
					? CurrentWeaponConfig->WeaponABP_Sheathed
					: nullptr;
			}
		}

		NewAnimLayer = NewAnimLayer ? NewAnimLayer : DefaultAnimLayerClass;
		SkeletalMesh->LinkAnimClassLayers(NewAnimLayer);
	}

	SyncWeaponDrawnTagToASC();
}

void UKOEquipmentComponent::SyncWeaponDrawnTagToASC()
{
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const FGameplayTag WeaponDrawnTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Character.WeaponDrawn"));

	const bool bWeaponDrawn =
		CurrentWeaponActor != nullptr &&
		CurrentWeaponConfig != nullptr &&
		CurrentWeaponSlot == EWeaponSlot::Hand;

	ASC->SetLooseGameplayTagCount(
		WeaponDrawnTag,
		bWeaponDrawn ? 1 : 0
	);
}