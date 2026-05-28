#include "KOPlayerController.h"

#include "AbilitySystemInterface.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "Component/KOInputComponent.h"
#include "Component/KOInteractionComponent.h"
#include "Component/KOGridBuildComponent.h"
#include "Component/KOInventoryComponent.h"
#include "UI/KOActivatableWidget.h"
#include "UI/KOBuildUIComponent.h"
#include "UI/KOUISubsystem.h"
#include "CommonActivatableWidget.h"
#include "Items/KOItemSlot.h"

AKOPlayerController::AKOPlayerController()
{
	InteractionComponent = CreateDefaultSubobject<UKOInteractionComponent>(TEXT("InteractionComponent"));
	GridBuildComponent   = CreateDefaultSubobject<UKOGridBuildComponent>(TEXT("GridBuildComponent"));
	BuildUIComponent     = CreateDefaultSubobject<UKOBuildUIComponent>(TEXT("BuildUIComponent"));
	InventoryComponent   = CreateDefaultSubobject<UKOInventoryComponent>(TEXT("InventoryComponent"));
}

void AKOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultIMC, 0);
	}

	CreateRootLayout();
	
#if !(UE_BUILD_SHIPPING)
	if (UKOInventoryComponent* FoundInventoryComponent  = FindComponentByClass<UKOInventoryComponent>())
	{
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("Boiler"),
			2
		);

		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Coal"),
			50
		);

		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("UndergroundMiningModule"),
			2
		);

		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("MiningPipe"),
			50
		);
	}
#endif
}

void AKOPlayerController::CreateRootLayout()
{
	if (!RootLayoutClass) return;

	RootLayoutInstance = CreateWidget<UKOActivatableWidget>(this, RootLayoutClass);
	if (RootLayoutInstance)
	{
		RootLayoutInstance->AddToViewport();

		// CommonUI ActionRouter가 자식 위젯의 activation을 input config refresh로 전파하려면
		// 루트(RootLayout)가 "receiving input" 상태여야 함. 명시적으로 활성화.
		RootLayoutInstance->ActivateWidget();
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

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Interact,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_Interact,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_ToggleBuildMode,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_ToggleBuildMode,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_Confirm,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_BuildConfirm,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_ToggleDestroy,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_BuildToggleDestroy,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_Cancel,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_BuildCancel,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_QuickSlot1,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_SelectBuildQuickSlot1,
			true
		);
		
		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_QuickSlot2,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_SelectBuildQuickSlot2,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_QuickSlot3,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_SelectBuildQuickSlot3,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_QuickSlot4,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_SelectBuildQuickSlot4,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_QuickSlot5,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_SelectBuildQuickSlot5,
			true
		);

		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_ToggleInventory,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_ToggleInventory,
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

void AKOPlayerController::Input_Interact(const FInputActionValue& /*Value*/)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void AKOPlayerController::Input_ToggleBuildMode(const FInputActionValue& /*Value*/)
{
	if (!BuildUIComponent)
	{
		return;
	}
	
	if (!BuildUIComponent->IsBuildMenuOpen())
	{
		if (UKOUISubsystem* UISub = UKOUISubsystem::Get(this))
		{
			if (UCommonActivatableWidget* InventoryWidget = 
				UISub->FindActiveWidget(KOGameplayTags::UI_Widget_Inventory))
			{
				UISub->PopLayer(InventoryWidget);
			}
		}
	}

	BuildUIComponent->ToggleBuildMenu();
	
	if (BuildUIComponent->IsBuildMenuOpen())
	{
		EnterBuildIMC();
	}
	else
	{
		ExitBuildIMC();
	}
}

void AKOPlayerController::Input_BuildConfirm(const FInputActionValue& /*Value*/)
{
	if (!BuildUIComponent)
	{
		return;
	}

	BuildUIComponent->ConfirmBuildAction();
}

void AKOPlayerController::Input_BuildToggleDestroy(const FInputActionValue& /*Value*/)
{
	if (!BuildUIComponent)
	{
		return;
	}

	BuildUIComponent->ToggleDestroyBuildMode();
}

void AKOPlayerController::Input_BuildCancel(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->CancelBuildAction();
	}
}

void AKOPlayerController::Input_SelectBuildQuickSlot1(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->SelectBuildQuickSlot(0);
	}
}

void AKOPlayerController::Input_SelectBuildQuickSlot2(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->SelectBuildQuickSlot(1);
	}
}

void AKOPlayerController::Input_SelectBuildQuickSlot3(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->SelectBuildQuickSlot(2);
	}
}

void AKOPlayerController::Input_SelectBuildQuickSlot4(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->SelectBuildQuickSlot(3);
	}
}

void AKOPlayerController::Input_SelectBuildQuickSlot5(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent)
	{
		BuildUIComponent->SelectBuildQuickSlot(4);
	}
}

void AKOPlayerController::Input_ToggleInventory(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent && BuildUIComponent->IsBuildMenuOpen())
	{
		return;
	}
	
	UKOUISubsystem* UISub = UKOUISubsystem::Get(this);
	if (!UISub)
	{
		return;
	}
	
	if (UCommonActivatableWidget* InventoryWidget = UISub->FindActiveWidget(KOGameplayTags::UI_Widget_Inventory))
	{
		UISub->PopLayer(InventoryWidget);

		if (BuildUIComponent)
		{
			BuildUIComponent->CloseQuickSlotBar();
		}

		return;
	}
	
	if (BuildUIComponent)
	{
		BuildUIComponent->OpenQuickSlotBar();
	}

	UCommonActivatableWidget* InventoryWidget =	UISub->PushWidget(KOGameplayTags::UI_Widget_Inventory);

	if (!InventoryWidget && BuildUIComponent)
	{
		BuildUIComponent->CloseQuickSlotBar();
	}
}

void AKOPlayerController::EnterBuildIMC()
{
	if (bBuildIMCActive)
	{
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem && BuildIMC)
	{
		Subsystem->AddMappingContext(BuildIMC, 1);
	}

	bBuildIMCActive = true;
}

void AKOPlayerController::ExitBuildIMC()
{
	if (!bBuildIMCActive)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem && BuildIMC)
	{
		Subsystem->RemoveMappingContext(BuildIMC);
	}

	bBuildIMCActive = false;
}
