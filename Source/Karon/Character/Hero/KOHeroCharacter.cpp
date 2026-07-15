#include "KOHeroCharacter.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "Component/Movement/KOPreCMCTickComponent.h"
#include "Game/KOPlayerState.h"
#include "CharacterTrajectoryComponent.h"
#include "Karon.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/Ability/Status/KOGA_OverClock.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Animation/KOAnimInstance.h"
#include "Component/Camera/KOCameraComponent.h"
#include "Component/Camera/KOSpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Game/KOGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Map/FOW/KOVisionComponent.h"
#include "TimerManager.h"


AKOHeroCharacter::AKOHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	
	SpringArm = CreateDefaultSubobject<UKOSpringArmComponent>(TEXT("SprintArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	// SpringArm->TargetArmLength = 300.f; 
	
	// SpringArm->bEnableCameraLag = true;
	//SpringArm->bEnableCameraRotationLag = true;
	
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	
	SpringArm->CameraLagSpeed = 20.f; 
	SpringArm->CameraRotationLagSpeed = 50.f; 
	
	Camera = CreateDefaultSubobject<UKOCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->BaseFOV = 90.f; 
	
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	VisionComponent = CreateDefaultSubobject<UKOVisionComponent>(TEXT("VisionComponent"));
	PreCMCTick = CreateDefaultSubobject<UKOPreCMCTickComponent>(TEXT("PreCMCTick"));
	Trajectory  = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("Trajectory"));
	Trajectory->PrimaryComponentTick.AddPrerequisite(
		PreCMCTick, PreCMCTick->PrimaryComponentTick
	);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Player, ECollisionResponse::ECR_Block);
}


void AKOHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	MainAnimInstance = GetMesh()->GetAnimInstance();
}

void AKOHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AKOPlayerState* PS = GetPlayerState<AKOPlayerState>();
	if (!PS) return;
	
	AbilitySystemComponent = Cast<UKOAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;
	
	AbilitySystemComponent->InitAbilityActorInfo(PS,this);
	InitializeAttributes();
	
	AbilitySystemComponent->GiveGrantSet(); 
}

void AKOHeroCharacter::InitializeAttributes()
{
	AKOPlayerState* PS = GetPlayerState<AKOPlayerState>();
	if (!PS) return;
	
	StaminaSet = PS->GetStaminaSet();
	CombatSet = PS->GetCombatSet();
	MovementSet = PS->GetMovementSet();
	HealthSet = PS->GetHealthSet();
	GuardSet = PS->GetGuardSet();
	
	Super::InitializeAttributes();
}

void AKOHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaTime, false);
	}
}

void AKOHeroCharacter::OnCharacterDead(AActor* DeathInstigator)
{
	Super::OnCharacterDead(DeathInstigator);
	
	AKOGameMode* GM = GetWorld()->GetAuthGameMode<AKOGameMode>();
	if (!GM) return;
	
	GM->HandlePlayerDeath(DeathInstigator);
}

void AKOHeroCharacter::RespawnWithoutSave(const FTransform& RespawnTransform)
{
	if (!AbilitySystemComponent || !HealthSet)
	{
		return;
	}

	RestoreAliveStateFromLoad();
	
	AbilitySystemComponent->SetNumericAttributeBase(
		UKOHealthSet::GetHealthAttribute(),
		HealthSet->GetMaxHealth()
	);

	SetActorTransform(
		RespawnTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);
	
	// 취소된 오버클락 어빌리티 재실행
	ReactivateOverClockAfterRespawn();
}

void AKOHeroCharacter::ReactivateOverClockAfterRespawn()
{
	// 사망 몽타주와 Death Ability 종료가 반영된 다음 실행
	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (!AbilitySystemComponent)
			{
				return;
			}

			for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
			{
				if (!Spec.Ability)
				{
					continue;
				}

				if (!Spec.Ability->IsA(UKOGA_OverClock::StaticClass()))
				{
					continue;
				}

				if (!Spec.IsActive())
				{
					AbilitySystemComponent->TryActivateAbility(Spec.Handle);
				}

				return;
			}
		})
	);
}

bool AKOHeroCharacter::IsLockOn() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(KOGameplayTags::State_Character_LockOn);
}

void AKOHeroCharacter::SetMotionWarpTarget(const FName& WarpTargetName)
{
	if (!MotionWarpingComponent) return;

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		WarpTargetName,
		GetActorLocation(),
		GetControlRotation()
	);
}

void AKOHeroCharacter::SetMotionWarpTargetWithLocation(const FName& WarpTargetName, const FVector& Location)
{
	if (!MotionWarpingComponent) return;
	
	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		WarpTargetName,
		Location,
		GetControlRotation()
	);
}

void AKOHeroCharacter::ClearMotionWarpTarget(const FName& WarpTargetName)
{
	if (!MotionWarpingComponent) return;
	
	MotionWarpingComponent->RemoveWarpTarget(WarpTargetName);
}


void AKOHeroCharacter::UpdateGait(EGait DesiredGait)
{
	PreviousGait = CurrentGait;
	
	CurrentGait = DesiredGait;
}
