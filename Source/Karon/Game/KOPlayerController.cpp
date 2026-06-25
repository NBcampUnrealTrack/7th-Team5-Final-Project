#include "KOPlayerController.h"

#include "AbilitySystemInterface.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "Component/Input/KOInputComponent.h"
#include "Component/Interaction/KOInteractionComponent.h"
#include "Component/Build/KOGridBuildComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Component/Build/KOBuildUIComponent.h"
#include "UI/Map/KOMapUIComponent.h"
#include "UI/KOUISubsystem.h"
#include "Utility/Log/KOLogManager.h"

#include "StructUtils/InstancedStruct.h"
#include "Engine/GameInstance.h"
#include "Items/KOItemSlot.h"
#include "Utility/Messaging/KOMessageTypes.h"

class AKOMapUIComponent;
struct FKOBuildModeChangedMessage;

AKOPlayerController::AKOPlayerController()
{
	InteractionComponent = CreateDefaultSubobject<UKOInteractionComponent>(TEXT("InteractionComponent"));
	GridBuildComponent   = CreateDefaultSubobject<UKOGridBuildComponent>(TEXT("GridBuildComponent"));
	BuildUIComponent     = CreateDefaultSubobject<UKOBuildUIComponent>(TEXT("BuildUIComponent"));
	InventoryComponent   = CreateDefaultSubobject<UKOInventoryComponent>(TEXT("InventoryComponent"));
	MapUIComponent		 = CreateDefaultSubobject<UKOMapUIComponent>(TEXT("MapUIComponent"));
}

void AKOPlayerController::OnItemReceived(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKODropItemMessage* ItemMessage = Payload.GetPtr<FKODropItemMessage>())
	{
		//아이템 추가
		TryAddItemWithUI(ItemMessage->ItemId,ItemMessage->Count);
	}
}

void AKOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultIMC, 0);
	}

	// 루트 레이아웃은 UISubsystem이 UKOUISettings::RootLayoutMap을 참조해 생성·소유한다.
	// 컨트롤러는 위젯 클래스/인스턴스를 직접 들지 않고 컨텍스트 태그만 넘긴다.
	if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
	{
		UISubsystem->SetRootLayout(KOGameplayTags::UI_Layout_InGame);
		
		if (GetPawn())
		{
			UISubsystem->OpenWidget(GetWorld(), KOGameplayTags::UI_Widget_InGameHUD);
		}
		else
		{
			GetWorldTimerManager().SetTimerForNextTick([this, UISubsystem]()
			{
				if (UISubsystem && GetPawn())
				{
					UISubsystem->OpenWidget(GetWorld(), KOGameplayTags::UI_Widget_InGameHUD);
				}
			});
		}
	}
	
	// 건설 모드 진입/종료에 따른 BuildIMC 전환을 토글키가 아닌 모드 변경 메시지로 구동.
	BuildModeChangedCallback.BindDynamic(this, &AKOPlayerController::OnBuildModeChanged);
	BuildModeChangedHandle = UGMRouterSubsystem::Subscribe(
		GetWorld(),
		KOGameplayTags::Data_Message_Build_ModeChanged,
		BuildModeChangedCallback);

	if (UKOInventoryComponent* FoundInventoryComponent  = FindComponentByClass<UKOInventoryComponent>())
	{
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("ModuleDismantler"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("Boiler"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("AlloyMaker"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("GearPress"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("PipeWorkshop"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("UndergroundMiningModule"),
			1
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("CornerBelt"),
			20
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Factory,
			TEXT("StraightBelt"),
			20
		);
		
		// 아이템 -----------------------------------------

		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("BasicModule"),
			50
		);

		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Coal"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("CoalDust"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Copper"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Tin"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Bronze"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("BronzePlate"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("DamagedMiningPipe"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("MiningPipe"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("Gear"),
			50
		);
		
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			TEXT("BronzeSword"),
			1
		);
	}
	
	FGameplayTag Channel = KOGameplayTags::Event_DropItem;		
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &AKOPlayerController::OnItemReceived);	
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
}

void AKOPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BuildModeChangedHandle.IsValid())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>())
			{
				GMS->Unsubscribe(BuildModeChangedHandle);
			}
		}
		BuildModeChangedHandle = FGameplayMessageHandle();
	}
	BuildModeChangedCallback.Clear();

	Super::EndPlay(EndPlayReason);
}

void AKOPlayerController::OnBuildModeChanged(FGameplayTag /*Channel*/, const FInstancedStruct& Payload)
{
	const FKOBuildModeChangedMessage* Message = Payload.GetPtr<FKOBuildModeChangedMessage>();
	if (!Message)
	{
		return;
	}

	const bool bWasActive = Message->PreviousMode != EKOGridBuildMode::None;
	const bool bIsActive  = Message->NewMode != EKOGridBuildMode::None;

	if (bIsActive && !bWasActive)
	{
		EnterBuildIMC();
	}
	else if (!bIsActive && bWasActive)
	{
		ExitBuildIMC();
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
			&ThisClass::Input_OpenBuildMode,
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
			KOGameplayTags::Input_Native_OpenPlayerMenu,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_OpenPlayerMenu,
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
		
		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_Build_Rotate,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::Input_BuildRotate,
			true
		);
		
		KOIC->BindNativeAction(
			InputConfig,
			KOGameplayTags::Input_Native_ToggleMap,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_ToggleMap,
			true
		);
		
		KOIC->BindAction(
		IAWeapon,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_Weapon
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
			KO_LOG(Input, Log, TEXT("[PlayerController] : %s Pressed."), *InputTag.GetTagName().ToString());
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

	KO_LOG(Input, Log, TEXT("[PlayerController] : %s Released."), *InputTag.GetTagName().ToString());
	KOASC->AbilityInputTagReleased(InputTag);
}

void AKOPlayerController::Input_Interact(const FInputActionValue& /*Value*/)
{
	if (BuildUIComponent && BuildUIComponent->IsBuildMenuOpen())
	{
		BuildUIComponent->CloseBuildMenu();
	}
	
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void AKOPlayerController::Input_OpenBuildMode(const FInputActionValue& /*Value*/)
{
	if (!BuildUIComponent)
	{
		return;
	}

	// 열기 전용. 이미 열려 있으면 무시(닫기는 Back). IMC 전환은 OnBuildModeChanged가 담당.
	if (BuildUIComponent->IsBuildMenuOpen())
	{
		BuildUIComponent->CloseBuildMenu();
		return;
	}

	BuildUIComponent->OpenBuildMenu();
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

void AKOPlayerController::Input_BuildRotate(const FInputActionValue& Value)
{
	if (!BuildUIComponent)
	{
		return;
	}

	const float AxisValue = Value.Get<float>();

	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const int32 Direction = AxisValue > 0.0f ? -1 : 1;

	BuildUIComponent->RotateBuildPreview(Direction);
}

void AKOPlayerController::Input_OpenPlayerMenu(const FInputActionValue& /*Value*/)
{	
	// 건설 중에는 메뉴창을 열지 않는다.
	if (BuildUIComponent && BuildUIComponent->IsBuildMenuOpen())
	{
		return;
	}
	
	if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
	{
		if (UISubsystem->FindActiveWidget(KOGameplayTags::UI_Widget_PlayerMenu))
		{
			UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
			return;
		}
	}

	UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_PlayerMenu);
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

void AKOPlayerController::TryAddItemWithUI(FName ItemId, int32 Count)
{
	if (UKOInventoryComponent* FoundInventoryComponent  = FindComponentByClass<UKOInventoryComponent>())
	{
		FoundInventoryComponent ->TryAddItem(
			EKOSlotKind::Item,
			ItemId,
			Count
		);
	}
	
}

void AKOPlayerController::Input_ToggleMap(const FInputActionValue& Value)
{
	if (MapUIComponent)
	{
		MapUIComponent->ToggleMainMap();
	}
}

void AKOPlayerController::Input_Weapon(const FInputActionValue& Value)
{
	OnWeaponCreate.Broadcast();
}
