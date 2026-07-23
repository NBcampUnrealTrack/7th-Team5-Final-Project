#include "KOQuestGateActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Subsystem/KOQuestGuideSubsystem.h"

AKOQuestGateActor::AKOQuestGateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BlockCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockCollision"));
	BlockCollision->SetupAttachment(SceneRoot);
	BlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockCollision->SetCollisionObjectType(ECC_WorldStatic);
	BlockCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BlockCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	FogEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogEffect"));
	FogEffect->SetupAttachment(SceneRoot);
	FogEffect->SetAutoActivate(false);
	FogEffect->SetVisibility(false, true);
}

void AKOQuestGateActor::BeginPlay()
{
	Super::BeginPlay();

	if (FogEffect && FogSystem)
	{
		FogEffect->SetAsset(FogSystem);
	}

	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->OnQuestChanged.AddDynamic(
			this,
			&AKOQuestGateActor::HandleQuestChanged
		);
	}

	RefreshGateState();
}

void AKOQuestGateActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->OnQuestChanged.RemoveDynamic(
			this,
			&AKOQuestGateActor::HandleQuestChanged
		);
	}

	Super::EndPlay(EndPlayReason);
}

void AKOQuestGateActor::HandleQuestChanged(FName NewQuestId)
{
	RefreshGateState();
}

void AKOQuestGateActor::RefreshGateState()
{
	if (ShouldGateOpen())
	{
		OpenGate();
	}
	else
	{
		CloseGate();
	}
}

bool AKOQuestGateActor::ShouldGateOpen() const
{
	if (RequiredCompletedQuestId.IsNone())
	{
		return false;
	}

	const UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this);
	if (!QuestGuide)
	{
		return false;
	}

	return QuestGuide->IsQuestCompleted(RequiredCompletedQuestId);
}

void AKOQuestGateActor::OpenGate()
{
	bIsOpen = true;

	if (BlockCollision)
	{
		BlockCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (FogEffect)
	{
		FogEffect->Deactivate();
		FogEffect->SetVisibility(false, true);
	}
}

void AKOQuestGateActor::CloseGate()
{
	bIsOpen = false;

	if (BlockCollision)
	{
		BlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (FogEffect)
	{
		FogEffect->SetVisibility(true, true);
		FogEffect->Activate(true);
	}
}