#include "KOHeroCharacter.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "Component/KOLockOnComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/KOPreCMCTickComponent.h"
#include "Game/KOPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "CharacterTrajectoryComponent.h"


AKOHeroCharacter::AKOHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	SprintArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SprintArm"));
	SprintArm->SetupAttachment(RootComponent);
	SprintArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SprintArm);
	
	PreCMCTick = CreateDefaultSubobject<UKOPreCMCTickComponent>(TEXT("PreCMCTick"));
	Trajectory  = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("Trajectory"));
	Trajectory->PrimaryComponentTick.AddPrerequisite(
		PreCMCTick,
		PreCMCTick->PrimaryComponentTick
	);
	
	LockOnComponent = CreateDefaultSubobject<UKOLockOnComponent>(FName("LockOnComponent"));
	StaminaSet = CreateDefaultSubobject<UKOStaminaSet>(FName("StaminaSet"));
	CombatSet = CreateDefaultSubobject<UKOCombatSet>(FName("CombatSet"));
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
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GiveDefaultAbilities(); 
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void AKOHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaTime, false);
	}
}

void AKOHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

