#include "KOHeroCharacter.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "Camera/CameraComponent.h"
#include "Component/Movement/KOPreCMCTickComponent.h"
#include "Game/KOPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "CharacterTrajectoryComponent.h"
#include "Karon.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Animation/KOAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Game/KOGameMode.h"


AKOHeroCharacter::AKOHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SprintArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = 300.f; 
	
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;
	
	SpringArm->CameraLagSpeed = 20.f; 
	SpringArm->CameraRotationLagSpeed = 50.f; 
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	PreCMCTick = CreateDefaultSubobject<UKOPreCMCTickComponent>(TEXT("PreCMCTick"));
	Trajectory  = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("Trajectory"));
	Trajectory->PrimaryComponentTick.AddPrerequisite(
		PreCMCTick, PreCMCTick->PrimaryComponentTick
	);
	
	//현석 : Enemy에서 SphereTrace를 위해 PlayerChannel 콜리전 Block 설정
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
	
	AbilitySystemComponent->GiveGrantSet(); 
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	
	StaminaSet = PS->GetStaminaSet();
	CombatSet = PS->GetCombatSet();
	MovementSet = PS->GetMovementSet();
	HealthSet = PS->GetHealthSet();
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
