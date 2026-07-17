#include "KOQuestGuideWidget.h"

#include "Components/TextBlock.h"
#include "Subsystem/KOQuestGuideSubsystem.h"

void UKOQuestGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		CachedQuestGuide = GameInstance->GetSubsystem<UKOQuestGuideSubsystem>();
	}

	if (CachedQuestGuide)
	{
		CachedQuestGuide->InitializeQuestGuide(QuestGuideTable, StartQuestId);

		CachedQuestGuide->OnQuestChanged.AddDynamic(this, &UKOQuestGuideWidget::HandleQuestChanged);
	}

	RefreshQuestText();
}

void UKOQuestGuideWidget::NativeDestruct()
{
	if (CachedQuestGuide)
	{
		CachedQuestGuide->OnQuestChanged.RemoveDynamic(this, &UKOQuestGuideWidget::HandleQuestChanged);
	}

	Super::NativeDestruct();
}

void UKOQuestGuideWidget::HandleQuestChanged(FName NewQuestId)
{
	RefreshQuestText();
}

void UKOQuestGuideWidget::RefreshQuestText()
{
	if (!CachedQuestGuide)
	{
		return;
	}

	if (TitleText)
	{
		TitleText->SetText(CachedQuestGuide->GetTitleText());
	}

	if (ObjectiveText)
	{
		ObjectiveText->SetText(CachedQuestGuide->GetObjectiveText());
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(CachedQuestGuide->GetDescriptionText());
	}
}