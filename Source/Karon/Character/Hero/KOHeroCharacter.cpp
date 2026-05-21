#include "KOHeroCharacter.h"

#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/KOLockOnComponent.h"
#include "Component/KOInputComponent.h"          
#include "EnhancedInputSubsystems.h"             
#include "Data/KOInputConfig.h"            
#include "GameFramework/CharacterMovementComponent.h" 
#include "Game/KOPlayerState.h"


AKOHeroCharacter::AKOHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	StaminaSet = CreateDefaultSubobject<UKOStaminaSet>(FName("StaminaSet"));
	CombatSet = CreateDefaultSubobject<UKOCombatSet>(FName("CombatSet"));
	// 카메라 방향으로 캐릭터 회전 OFF
	bUseControllerRotationYaw   = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;

	// 이동 방향으로 캐릭터가 자동 회전
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	
	LockOnComponent = CreateDefaultSubobject<UKOLockOnComponent>(FName("LockOnComponent"));
}


void AKOHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

