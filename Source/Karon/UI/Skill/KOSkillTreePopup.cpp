// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreePopup.h"
#include "UI/Skill/KOSkillNodeWidget.h"
#include "Component/Skill/KOSkillComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Skills/KOSkillLibrary.h"

UKOSkillTreePopup::UKOSkillTreePopup()
{
	// Back(ESC) 입력 시 자동으로 Deactivate되어 닫힌다. (토글 제거 → Back 일원화)
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOSkillTreePopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (AController* OwningController = GetOwningPlayer())
	{
		if (UKOSkillComponent* SkillComp = OwningController->FindComponentByClass<UKOSkillComponent>())
		{
			SkillComponent = SkillComp;
		}
	}
	RefreshAllSkillNodes();
}

void UKOSkillTreePopup::NativeDestruct()
{
	SkillDataTable.Reset();
	
	Super::NativeDestruct();
}

void UKOSkillTreePopup::RefreshAllSkillNodes() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (OwningPawn == nullptr)
	{
		return;
	}
	
	const UObject* WorldContext = GetWorld();
	if (WorldContext == nullptr)
	{
		return;
	}
	
	if (SkillDataTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Data Table이 지정되지 않았습니다."));
		return;
	}
	
	if (SkillComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent가 없습니다."));
		return;
	}
	//BP를 통해 가져오므로 슬롯 추가 시 BP에 등록 필요함
	TArray<UKOSkillNodeWidget*> SkillNodes = BP_GetAllSkillNodes();
	
	for (UKOSkillNodeWidget* Node : SkillNodes)
	{
		if (Node == nullptr || Node->SkillName.IsNone()) continue;
		
		const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(WorldContext, Node->SkillName);
		
		if (SkillRow)
		{
			ESkillState CurrentState = SkillComponent->GetSkillState(Node->SkillName);
			Node->InitializeNode(SkillRow->SkillTag, SkillRow->UnlockCosts, CurrentState);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillId [%s] 에 해당하는 Row를 찾을 수 없습니다."), *Node->SkillName.ToString());
		}
	}
}
