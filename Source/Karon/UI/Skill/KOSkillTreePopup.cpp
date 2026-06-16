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
}

void UKOSkillTreePopup::NativeDestruct()
{
	SkillComponent = nullptr;
	
	Super::NativeDestruct();
}

void UKOSkillTreePopup::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	SetupAndBindSkillNodes();
}

void UKOSkillTreePopup::NativeOnDeactivated()
{
	for(UKOSkillNodeWidget* Node : CachedSkillNodes)
	{
		if (IsValid(Node))
		{
			Node->OnSkillNodeClicked.RemoveAll(this);
		}
	}
	CachedSkillNodes.Empty();
	
	Super::NativeOnDeactivated();
}

void UKOSkillTreePopup::RefreshAllSkillNodes() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	const UObject* WorldContext = GetWorld();
	if (OwningPawn == nullptr || WorldContext == nullptr)
	{
		return;
	}
	
	if (SkillComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillComponent가 없습니다."));
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
			Node->InitializeNode(Node->SkillName, SkillRow->SkillTag, SkillRow->UnlockCosts, CurrentState);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillName [%s] 에 해당하는 Row를 찾을 수 없습니다."),
				*Node->SkillName.ToString());
		}
	}
}

void UKOSkillTreePopup::SetupAndBindSkillNodes()
{
	for(UKOSkillNodeWidget* Node : CachedSkillNodes)
	{
		if (IsValid(Node))
		{
			Node->OnSkillNodeClicked.RemoveAll(this);
		}
	}
	CachedSkillNodes.Empty();
	
	TArray<UKOSkillNodeWidget*> RetrievedNodes = BP_GetAllSkillNodes();
	for(UKOSkillNodeWidget* Node : RetrievedNodes)
	{
		if (IsValid(Node))
		{
			CachedSkillNodes.Add(Node);
			
			Node->OnSkillNodeClicked.AddUObject(this, &UKOSkillTreePopup::HandleSkillNodeClicked);
		}
	}
	RefreshAllSkillNodes();
}

void UKOSkillTreePopup::HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode)
{
	if (ClickedNode == nullptr)
	{
		return;
	}
	RefreshAllSkillNodes();
}
