#include "KOEquipmentComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/KOCharacterBase.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Items/Equipment/KOWeaponBase.h"
#include "Utility/Log/KOLogManager.h"
#include "Data/KODataTableTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"


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

	if (!CanEquip())
	{
		if (OnEquipmentChangeBlocked.IsBound()) OnEquipmentChangeBlocked.Broadcast();
		return;
	}
	
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character) return;

	bool bWeaponDrawn = IsWeaponDrawn();
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
		bWeaponDrawn ? Def->AttachSockets.EquipSocket.SocketName : Def->AttachSockets.UnequipSocket.SocketName
	);

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC && Def->GrantedSet)
	{
		Def->GrantedSet->GiveToAsc(ASC, ActiveHandles);
	}
	
	CurrentWeaponActor = NewWeapon;
	CurrentWeaponConfig = Def;
	
	AttachWeaponToSocket(bWeaponDrawn);

	SetWeaponSlot(bWeaponDrawn ? EWeaponSlot::Hand : EWeaponSlot::Holster);

	KO_LOG(GAS, Warning, TEXT("Equiped Weapon : %s"), *Def->WeaponName.ToString());
}

bool UKOEquipmentComponent::UnequipWeapon()
{
	if (!CanEquip())
	{
		if (OnEquipmentChangeBlocked.IsBound())
			OnEquipmentChangeBlocked.Broadcast();
		
		return false;
	}

	// 무기 정보가 살아 있을 때 먼저 애니메이션을 Holster 상태로 돌린다.
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

	// GAS 무기 능력 제거
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (Character)
	{
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (ASC)
		{
			ActiveHandles.RemoveFromASC(ASC);
		}
	}

	// 무기 액터 제거
	if (CurrentWeaponActor)
	{
		CurrentWeaponActor->Destroy();
		CurrentWeaponActor = nullptr;
	}

	// 무기 상태를 비운다.
	CurrentWeaponConfig = nullptr;
	CurrentWeaponItemId = NAME_None;
	CurrentWeaponSlot = EWeaponSlot::Holster;

	SyncWeaponDrawnTagToASC();

	return true;
}

void UKOEquipmentComponent::DrawWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Hand) return;
	
	AttachWeaponToSocket(true);
	SetWeaponSlot(EWeaponSlot::Hand);
}

void UKOEquipmentComponent::SheatheWeapon()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig || CurrentWeaponSlot == EWeaponSlot::Holster) return;
	
	AttachWeaponToSocket(false);
	SetWeaponSlot(EWeaponSlot::Holster);
}

void UKOEquipmentComponent::ToggleWeaponDrawState()
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig) return;
	
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
	if (InWeaponItemId.IsNone() || !Def) return false;

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

void UKOEquipmentComponent::AttachWeaponToSocket(bool bDrawn)
{
	if (!CurrentWeaponActor || !CurrentWeaponConfig) return;
	
	const FKOAttachSocket& Socket = bDrawn ? 
		CurrentWeaponConfig->AttachSockets.EquipSocket : CurrentWeaponConfig->AttachSockets.UnequipSocket;
	
	CurrentWeaponActor->AttachToComponent(
		BodyMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		Socket.SocketName
	);
	
	CurrentWeaponActor->SetActorRelativeTransform(Socket.Transform);
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

void UKOEquipmentComponent::LoadArmorFromSave(const TMap<EKOEquipmentSlotType, FName>& SavedArmorItemIds)
{
	EquippedArmorItemIds = SavedArmorItemIds;

	// 혹시 잘못 저장된 Weapon 슬롯 데이터가 있으면 제거
	EquippedArmorItemIds.Remove(EKOEquipmentSlotType::Weapon);

	RecalculateArmorDefense();
}

bool UKOEquipmentComponent::EquipArmorFromItem(EKOEquipmentSlotType SlotType, FName ItemId)
{
	if (SlotType == EKOEquipmentSlotType::Weapon)
	{
		return false;
	}

	if (ItemId.IsNone())
	{
		return false;
	}
	//방어구도 제한하려면 if (IsEquipmentChangeBlockedBySkill()) 추가

	EquippedArmorItemIds.FindOrAdd(SlotType) = ItemId;

	RecalculateArmorDefense();

	return true;
}

bool UKOEquipmentComponent::UnequipArmor(EKOEquipmentSlotType SlotType)
{
	if (SlotType == EKOEquipmentSlotType::Weapon)
	{
		return false;
	}
	//방어구도 제한하려면 if (IsEquipmentChangeBlockedBySkill()) 추가

	EquippedArmorItemIds.Remove(SlotType);

	RecalculateArmorDefense();

	return true;
}

FName UKOEquipmentComponent::GetEquippedArmorItemId(EKOEquipmentSlotType SlotType) const
{
	if (const FName* Found = EquippedArmorItemIds.Find(SlotType))
	{
		return *Found;
	}

	return NAME_None;
}

void UKOEquipmentComponent::RecalculateArmorDefense()
{
	TotalArmorDefense = 0;

	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub) return; 

	for (const TPair<EKOEquipmentSlotType, FName>& Pair : EquippedArmorItemIds)
	{
		const EKOEquipmentSlotType SlotType = Pair.Key;
		const FName ItemId = Pair.Value;

		if (SlotType == EKOEquipmentSlotType::Weapon || ItemId.IsNone()) continue; 

		const FKOItemRow* ItemRow = LoadSub->FindItemRow(ItemId);
		if (!ItemRow) continue; 
		
		const FKOEquipmentRow* EquipmentRow =
			LoadSub->FindEquipmentRowByItemTag(ItemRow->ItemTag);

		if (!EquipmentRow) continue; 

		TotalArmorDefense += EquipmentRow->Defense;
	}

	ApplyArmorDefenseEffect();
}

void UKOEquipmentComponent::ApplyArmorDefenseEffect()
{
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character) return;

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC) return; 

	// 기존에 적용된 방어구 방어력 GE는 제거하고, 새 총합으로 다시 적용한다.
	if (ArmorDefenseEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ArmorDefenseEffectHandle);
		ArmorDefenseEffectHandle = FActiveGameplayEffectHandle();
	}

	if (ArmorDefenseEffectClass == nullptr || TotalArmorDefense <= 0)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ArmorDefenseEffectClass, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	Spec.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_Defense, TotalArmorDefense);

	ArmorDefenseEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
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
				NewAnimLayer = CurrentWeaponConfig->WeaponAnimationSet.WeaponABP_Carrying
					               ? CurrentWeaponConfig->WeaponAnimationSet.WeaponABP_Carrying
					               : nullptr;
			}
			else
			{
				NewAnimLayer = CurrentWeaponConfig->WeaponAnimationSet.WeaponABP_Sheathed
					               ? CurrentWeaponConfig->WeaponAnimationSet.WeaponABP_Sheathed
					               : nullptr;
			}
		}

		NewAnimLayer = NewAnimLayer ? NewAnimLayer : DefaultAnimLayerClass;
		SkeletalMesh->LinkAnimClassLayers(NewAnimLayer);
	}

	SyncWeaponDrawnTagToASC();
}

bool UKOEquipmentComponent::CanEquip() const
{
	AKOCharacterBase* Character = GetOwner<AKOCharacterBase>();
	if (!Character || !BodyMesh) return false; 
	
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC) return false; 
	
	return !ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Attacking);
}

void UKOEquipmentComponent::SyncWeaponDrawnTagToASC()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;
	
	const bool bWeaponDrawn =
		CurrentWeaponActor != nullptr &&
		CurrentWeaponConfig != nullptr &&
		CurrentWeaponSlot == EWeaponSlot::Hand;

	ASC->SetLooseGameplayTagCount(
		KOGameplayTags::State_Character_WeaponDrawn,
		bWeaponDrawn ? 1 : 0
	);
}
