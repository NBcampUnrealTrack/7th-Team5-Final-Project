#include "KOQuestGuideWidget.h"

#include "Components/TextBlock.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Items/KOItemLibrary.h"

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
	
	const FKOQuestGuideRow* Row = CachedQuestGuide->GetCurrentQuestRow();

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
	
	if (ProgressText)
	{
		if (Row && Row->CompleteType == EKOQuestCompleteType::KillMonster)
		{
			const int32 CurrentProgress = CachedQuestGuide->GetCurrentProgress();

			ProgressText->SetText(
				FText::Format(
					NSLOCTEXT(
						"KOQuestGuide",
						"QuestProgress",
						"처치 수 : {0} / {1}"
					),
					FText::AsNumber(CurrentProgress),
					FText::AsNumber(Row->RequiredCount)
				)
			);

			ProgressText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ProgressText->SetText(FText::GetEmpty());
			ProgressText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (RewardText)
	{
		if (!Row || Row->Rewards.IsEmpty())
		{
			RewardText->SetText(FText::GetEmpty());
			RewardText->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}

		FString RewardString = TEXT("보상 : ");

		for (const FKOQuestReward& Reward : Row->Rewards)
		{
			if (Reward.ItemId.IsNone() || Reward.Count <= 0)
			{
				continue;
			}

			FText DisplayName = UKOItemLibrary::GetDisplayName(
				this,
				Reward.SlotKind,
				Reward.ItemId
			);

			if (DisplayName.IsEmpty())
			{
				DisplayName = FText::FromName(Reward.ItemId);
			}

			RewardString += FString::Printf(
				TEXT("%s x%d\n"),
				*DisplayName.ToString(),
				Reward.Count
			);
		}

		RewardString.RemoveFromEnd(TEXT("\n"));

		RewardText->SetText(FText::FromString(RewardString));
		RewardText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}