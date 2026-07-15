#include "KOCharacterBase.h"

#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Component/Movement/KOCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/KOAbilitySystemComponent.h"
#include "Utility/Log/KOLogManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"


AKOCharacterBase::AKOCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UKOCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본값은 기존과 동일한 C++ 클래스. 자식 Blueprint에서 EquipmentComponentClass를
	// BP_EquipmentComponent 등으로 교체하면 그 클래스로 생성된다 (PostInitializeComponents 참고).
	EquipmentComponentClass = UKOEquipmentComponent::StaticClass();
}

UAbilitySystemComponent* AKOCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKOCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!EquipmentComponent)
	{
		const TSubclassOf<UKOEquipmentComponent> ClassToUse =
			EquipmentComponentClass
				? EquipmentComponentClass
				: TSubclassOf<UKOEquipmentComponent>(UKOEquipmentComponent::StaticClass());

		EquipmentComponent = NewObject<UKOEquipmentComponent>(this, ClassToUse, TEXT("EquipmentComponent"));
		if (EquipmentComponent)
		{
			EquipmentComponent->RegisterComponent();
		}
	}
}

void AKOCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AKOCharacterBase::InitializeAttributes()
{
	if (MovementSet)
	{
		MovementSet->OnMaxWalkSpeedBaseChanged.AddDynamic(this, &ThisClass::OnMaxWalkSpeedChanged);
		MovementSet->OnMaxAccelerationChanged.AddDynamic(this, &ThisClass::OnMaxAccelerationChanged);
		MovementSet->OnBrakingDecelerationWalkingChanged.AddDynamic(this, &ThisClass::OnBrakingDecelerationChanged);
		MovementSet->OnGroundFrictionChanged.AddDynamic(this, &ThisClass::OnGroundFrictionChanged);
		MovementSet->OnMaxWalkSpeedCrouchedChanged.AddDynamic(this, &ThisClass::OnMaxWalkSpeedCrouchedChanged);
		MovementSet->OnJumpStrengthChanged.AddDynamic(this, &ThisClass::OnJumpStrengthChanged);
		MovementSet->OnGravityScaleChanged.AddDynamic(this, &ThisClass::OnGravityScaleChanged);
	}
	else
	{
		KO_LOGS(GAS, Attribute, Error, TEXT("%s has not MovementSet."), *GetName());
	}
}

void AKOCharacterBase::OnCharacterDead(AActor* DeathInstigator)
{
	KO_LOG(Combat, Warning, TEXT("[%s] is Dead.      Instigator : [%s]"),
	       *GetName(),
	       DeathInstigator ? *DeathInstigator->GetName() : TEXT("Unknown")
	);

	bIsDead = true;

	// TODO: 임시 코드 
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(KOGameplayTags::State_Character_Dead);
	}

	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->DisableMovement();
		CMC->StopMovementImmediately();
	}
}

void AKOCharacterBase::OnMaxWalkSpeedChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxWalkSpeed = NewValue;
}

void AKOCharacterBase::OnMaxAccelerationChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxAcceleration = NewValue;
}

void AKOCharacterBase::OnBrakingDecelerationChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->BrakingDecelerationWalking = NewValue;
}

void AKOCharacterBase::OnGroundFrictionChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->GroundFriction = NewValue;
}

void AKOCharacterBase::OnMaxWalkSpeedCrouchedChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->MaxWalkSpeedCrouched = NewValue;
}

void AKOCharacterBase::OnJumpStrengthChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->JumpZVelocity = NewValue;
}

void AKOCharacterBase::OnGravityScaleChanged(float OldValue, float NewValue)
{
	GetCharacterMovement()->GravityScale = NewValue;
}

void AKOCharacterBase::RestoreAliveStateFromLoad()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (ASC)
	{
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_Dead, 0);

		FGameplayTagContainer DeadTags;
		DeadTags.AddTag(KOGameplayTags::State_Character_Dead);

		ASC->RemoveActiveEffectsWithGrantedTags(DeadTags);
	}
	
	bIsDead = false;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.0f);
		}

		MeshComp->bPauseAnims = false;
	}

	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->SetMovementMode(MOVE_Walking);
		CMC->StopMovementImmediately();
	}
}
