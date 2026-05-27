#include "KOBuildAssignMenuWidget.h"

#include "KOBuildAssignEntryWidget.h"
#include "Subsystem/KOLoadSubsystem.h"

#include "Components/PanelWidget.h"
#include "Components/VerticalBoxSlot.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOBuildMenu, Log, All);

void UKOBuildAssignMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RebuildEntries();
}

void UKOBuildAssignMenuWidget::RebuildEntries()
{
	if (!EntryContainer)
	{
		UE_LOG(LogKOBuildMenu, Warning, TEXT("[BuildAssignMenu] EntryContainer가 없습니다."));
		return;
	}

	if (!EntryWidgetClass)
	{
		UE_LOG(LogKOBuildMenu, Warning, TEXT("[BuildAssignMenu] EntryWidgetClass가 설정되지 않았습니다."));
		return;
	}

	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		UE_LOG(LogKOBuildMenu, Warning, TEXT("[BuildAssignMenu] KOLoadSubsystem을 찾을 수 없습니다."));
		return;
	}

	EntryContainer->ClearChildren();

	TArray<FName> FactoryIds;
	LoadSub->GetBuildableFactoryIds(Query, FactoryIds);

	for (const FName FactoryId : FactoryIds)
	{
		const FKOFactoryRow* Row = LoadSub->FindFactoryRow(FactoryId);
		if (!Row)
		{
			continue;
		}

		UKOBuildAssignEntryWidget* EntryWidget =
			CreateWidget<UKOBuildAssignEntryWidget>(
				GetOwningPlayer(),
				EntryWidgetClass
			);

		if (!EntryWidget)
		{
			continue;
		}

		UTexture2D* Icon = LoadSub->ResolveFactoryIcon(FactoryId);

		EntryWidget->SetupEntry(
			FactoryId,
			Row->DisplayName,
			Icon
		);

		UPanelSlot* AddedSlot = EntryContainer->AddChild(EntryWidget);

		if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(AddedSlot))
		{
			VerticalSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}
}