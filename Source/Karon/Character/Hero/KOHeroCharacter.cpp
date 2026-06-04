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
#include "Animation/KOAnimInstance.h"
#include "Components/CapsuleComponent.h"


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
	
	PreCMCTick = CreateDefaultSubobject<UKOPreCMCTickComponent>(TEXT("PreCMCTick"));
	Trajectory  = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("Trajectory"));
	Trajectory->PrimaryComponentTick.AddPrerequisite(
		PreCMCTick, PreCMCTick->PrimaryComponentTick
	);
	
	StaminaSet = CreateDefaultSubobject<UKOStaminaSet>(FName("StaminaSet"));
	CombatSet = CreateDefaultSubobject<UKOCombatSet>(FName("CombatSet"));
	
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
	
	MovementSet = PS->GetMovementSet();
	HealthSet = PS->GetHealthSet();
	
	BindMovementSet(); 
}

void AKOHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaTime, false);
	}
}


void AKOHeroCharacter::UpdateGait(EGait DesiredGait)
{
	PreviousGait = CurrentGait;
	
	CurrentGait = DesiredGait;
	
	UKOAnimInstance* AnimInstance =  Cast<UKOAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInstance) return;
	
	AnimInstance->ReceiveGait(DesiredGait);
}
