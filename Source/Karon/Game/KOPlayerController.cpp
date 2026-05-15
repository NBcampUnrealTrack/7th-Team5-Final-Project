#include "KOPlayerController.h"

#include "AbilitySystemInterface.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "Component/KOInputComponent.h"

AKOPlayerController::AKOPlayerController()
{
}

void AKOPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultIMC, 0);
	}
}

void AKOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UKOInputComponent* KOIC = Cast<UKOInputComponent>(InputComponent);
	if (!IsValid(KOIC)) return;
	
	if (InputConfig)
	{
		// Bind Native Input Actions
		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Move,
			ETriggerEvent::Triggered, 
			this, 
			&ThisClass::Input_Move,
			true
		);
		
		KOIC->BindNativeAction(
		InputConfig,
		KOGameplayTags::Input_Native_Look,
		ETriggerEvent::Triggered, 
		this, 
		&ThisClass::Input_Look,
		true
		);
		
		// Bind Abilities Input Actions
		TArray<uint32> BindHandles;
		KOIC->BindAbilityActions(
			InputConfig,
			this,
			&ThisClass::Input_AbilityPressed,
			&ThisClass::Input_AbilityReleased, 
			BindHandles
		);
	}
}

void AKOPlayerController::Input_Move(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return; 
	
	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotator MoveRotation(0.f, GetControlRotation().Yaw, 0.f);
	
	const FVector ForwardDirection = MoveRotation.RotateVector(FVector::ForwardVector);
	const FVector RightDirection = MoveRotation.RotateVector(FVector::RightVector);
	
	ControlledPawn->AddMovementInput(ForwardDirection, MoveValue.Y); 
	ControlledPawn->AddMovementInput(RightDirection, MoveValue.X); 
}

void AKOPlayerController::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();
	
	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void AKOPlayerController::Input_AbilityPressed(FGameplayTag InputTag)
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPawn()))
	{
		if (UKOAbilitySystemComponent* KOASC = Cast<UKOAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			KOASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void AKOPlayerController::Input_AbilityReleased(FGameplayTag InputTag)
{
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetPawn()); 
	if (!ASI) return;
	
	UKOAbilitySystemComponent* KOASC = Cast<UKOAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	if (!KOASC) return;
	
	KOASC->AbilityInputTagReleased(InputTag);
	
	
}

