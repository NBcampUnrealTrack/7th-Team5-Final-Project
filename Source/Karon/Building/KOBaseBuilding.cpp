#include "KOBaseBuilding.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Components/WidgetComponent.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Data/KODataTableTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GMRouterSubsystem.h"
#include "Items/KOItemLibrary.h"

#include "StructUtils/InstancedStruct.h"
#include "UI/KOUISubsystem.h"
#include "Utility/Messaging/KOMessageTypes.h"

#include "Camera/PlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AKOBaseBuilding::AKOBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.2f;
	
	OperatingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OperatingAudioComponent"));
	OperatingAudioComponent->SetupAttachment(RootComponent);
	// 설비가 스폰되자마자 자동으로 소리가 나지 않도록 한다.
	OperatingAudioComponent->bAutoActivate = false;
	// 플레이어가 일시정지해도 소리가 계속 날 필요가 없다면 false.
	OperatingAudioComponent->bIsUISound = false;
}

void AKOBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
	CachedProcessor = FindComponentByClass<UKOFactoryProcessorComponent>();

	TArray<UWidgetComponent*> WidgetComponents;
	GetComponents<UWidgetComponent>(WidgetComponents);

	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (WidgetComponent && WidgetComponent->GetName() == TEXT("PressureWidget"))
		{
			PressureWarningWidget = WidgetComponent;
			break;
		}
	}

	if (PressureWarningWidget)
	{
		PressureWarningWidget->SetHiddenInGame(true);
	}
	
	if (OperatingAudioComponent)
	{
		OperatingAudioComponent->SetSound(OperatingSound);
		OperatingAudioComponent->SetVolumeMultiplier(OperatingSoundVolume);
		OperatingAudioComponent->SetPitchMultiplier(OperatingSoundPitch);
	}
}

void AKOBaseBuilding::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	bWarningWidgetRangeActive = IsPlayerWithinWarningWidgetDistance();
	
	if (!IsPlayerWithinWarningWidgetDistance())
	{
		if (PressureWarningWidget)
		{
			PressureWarningWidget->SetHiddenInGame(true);
		}

		return;
	}
	
	RefreshPressureWarning();
	UpdatePressureWarningFacingCamera();
}

bool AKOBaseBuilding::IsPlayerWithinWarningWidgetDistance() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const APawn* PlayerPawn = World->GetFirstPlayerController()
		? World->GetFirstPlayerController()->GetPawn()
		: nullptr;

	if (!PlayerPawn)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(
		GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	return DistanceSquared <= FMath::Square(WarningWidgetActivationDistance);
}

void AKOBaseBuilding::RefreshPressureWarning()
{	
	if (!PressureWarningWidget)
	{
		return;
	}

	if (!bShowPressureWarning)
	{
		PressureWarningWidget->SetHiddenInGame(true);
		return;
	}
	
	if (bPressureWarningSuppressed)
	{
		PressureWarningWidget->SetHiddenInGame(true);
		return;
	}

	if (!CachedProcessor)
	{
		PressureWarningWidget->SetHiddenInGame(true);
		return;
	}

	const bool bPressureAvailable = IsPressureAvailable();
	PressureWarningWidget->SetHiddenInGame(bPressureAvailable);
}

void AKOBaseBuilding::UpdatePressureWarningFacingCamera()
{
	if (!PressureWarningWidget)
	{
		return;
	}
	
	if (PressureWarningWidget->bHiddenInGame)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector WidgetLocation = PressureWarningWidget->GetComponentLocation();
	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);

	PressureWarningWidget->SetWorldRotation(LookAtRotation);
}

void AKOBaseBuilding::ReleaseOperatingSoundBlock()
{
	bOperatingSoundBlocked = false;

	// 설치음이 끝난 시점에도 실제 가동 상태라면 가동음을 시작한다.
	StartOperatingSoundIfAllowed();
}

void AKOBaseBuilding::StartOperatingSoundIfAllowed()
{
	if (!bOperatingSoundRequested)
	{
		return;
	}

	// 설치 사운드가 끝나지 않았으면 가동음을 시작하지 않는다.
	if (bOperatingSoundBlocked)
	{
		return;
	}

	if (!OperatingAudioComponent || !OperatingSound)
	{
		return;
	}

	if (OperatingAudioComponent->IsPlaying())
	{
		OperatingAudioComponent->Stop();
	}

	OperatingAudioComponent->SetSound(OperatingSound);
	OperatingAudioComponent->SetPitchMultiplier(OperatingSoundPitch);

	if (OperatingSoundFadeInTime > 0.0f)
	{
		OperatingAudioComponent->FadeIn(
			OperatingSoundFadeInTime,
			OperatingSoundVolume
		);
	}
	else
	{
		OperatingAudioComponent->SetVolumeMultiplier(OperatingSoundVolume);
		OperatingAudioComponent->Play();
	}
}

bool AKOBaseBuilding::IsPressureAvailable() const
{
	if (!CachedProcessor)
	{
		return true;
	}

	if (const UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
	{
		return Energy->GetConsumerNetworkProductionRate(CachedProcessor) > KINDA_SMALL_NUMBER;
	}

	return false;
}

void AKOBaseBuilding::InitializeBuildingData(FName InFactoryId)
{
	if (InFactoryId.IsNone())
	{
		return;
	}

	FactoryId = InFactoryId;

	if (UKOEnergyProducerComponent* Producer = FindComponentByClass<UKOEnergyProducerComponent>())
	{
		Producer->InitializeFromRecipe();
	}
}

const FKOFactoryRow* AKOBaseBuilding::GetFactoryRow() const
{
	if (FactoryId.IsNone())
	{
		return nullptr;
	}

	return UKOItemLibrary::GetFactoryRow(this, FactoryId);
}

void AKOBaseBuilding::SetPressureWarningSuppressed(bool bSuppressed)
{
	bPressureWarningSuppressed = bSuppressed;

	if (bPressureWarningSuppressed && PressureWarningWidget)
	{
		PressureWarningWidget->SetHiddenInGame(true);
	}
}

bool AKOBaseBuilding::CanInteract(AActor* /*Interactor*/) const
{
	return !FactoryId.IsNone();
}

void AKOBaseBuilding::OnInteract(AActor* Interactor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FKOBuildingInteractedMessage Msg;
	Msg.FactoryId  = FactoryId;
	Msg.Building   = this;
	Msg.Instigator = Interactor;

	UGMRouterSubsystem::BroadcastMessage(
		World,
		KOGameplayTags::Data_Message_Building_Interacted,
		FInstancedStruct::Make(Msg));

	// 컴포넌트 유무로 표시할 UI 결정 (Processor / Producer는 상호 배타).
	FGameplayTag WidgetTag;
	if (FindComponentByClass<UKOFactoryProcessorComponent>())
	{
		WidgetTag = KOGameplayTags::UI_Widget_Factory_Processor;
	}
	else if (FindComponentByClass<UKOEnergyProducerComponent>())
	{
		WidgetTag = KOGameplayTags::UI_Widget_Factory_Producer;
	}

	if (WidgetTag.IsValid())
	{
		// 위젯 열기. 닫기는 Back(팩토리 위젯의 bIsBackHandler).
		UKOUISubsystem::OpenWidget(this, WidgetTag);
	}
}

FText AKOBaseBuilding::GetInteractionPrompt() const
{
	if (const FKOFactoryRow* Row = GetFactoryRow())
	{
		return Row->DisplayName;
	}
	return FText::GetEmpty();
}

void AKOBaseBuilding::SetOperatingSoundActive(bool bActive)
{	
	if (!OperatingAudioComponent || !OperatingSound)
	{
		return;
	}
	
	// 요청 상태가 변하지 않은 경우
	if (bOperatingSoundRequested == bActive)
	{
		// 재생 요청 상태인데 실제 소리가 멈췄다면 다시 시작
		if (bActive &&
			!bOperatingSoundBlocked &&
			!OperatingAudioComponent->IsPlaying())
		{
			StartOperatingSoundIfAllowed();
		}

		return;
	}

	bOperatingSoundRequested = bActive;

	if (!bActive)
	{
		if (!OperatingAudioComponent->IsPlaying())
		{
			return;
		}

		if (OperatingSoundFadeOutTime > 0.0f)
		{
			OperatingAudioComponent->FadeOut(
				OperatingSoundFadeOutTime,
				0.0f
			);
		}
		else
		{
			OperatingAudioComponent->Stop();
		}

		return;
	}

	StartOperatingSoundIfAllowed();
}

bool AKOBaseBuilding::IsOperatingSoundActive() const
{
	return OperatingAudioComponent && OperatingAudioComponent->IsPlaying();
}

void AKOBaseBuilding::PlayOperationBlockedSound()
{
	if (OperatingAudioComponent)
	{
		OperatingAudioComponent->Stop();
	}
	
	if (!PressureShortageSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		PressureShortageSound,
		GetActorLocation(),
		FRotator::ZeroRotator,
		PressureShortageSoundVolume,
		PressureShortageSoundPitch,
		PressureShortageSoundStartTime
	);
}

void AKOBaseBuilding::BlockOperatingSound(float Duration)
{
	GetWorldTimerManager().ClearTimer(OperatingSoundBlockTimer);

	if (Duration <= KINDA_SMALL_NUMBER)
	{
		bOperatingSoundBlocked = false;
		StartOperatingSoundIfAllowed();
		return;
	}

	bOperatingSoundBlocked = true;

	// 혹시 이미 가동음이 시작됐다면 설치음 전에 즉시 끈다.
	if (OperatingAudioComponent &&
		OperatingAudioComponent->IsPlaying())
	{
		OperatingAudioComponent->Stop();
	}

	GetWorldTimerManager().SetTimer(
		OperatingSoundBlockTimer,
		this,
		&AKOBaseBuilding::ReleaseOperatingSoundBlock,
		Duration,
		false
	);
}

void AKOBaseBuilding::StopOperatingSoundImmediately()
{
	bOperatingSoundRequested = false;
	bOperatingSoundBlocked = false;

	GetWorldTimerManager().ClearTimer(
		OperatingSoundBlockTimer
	);

	if (OperatingAudioComponent)
	{
		OperatingAudioComponent->Stop();
	}
}
