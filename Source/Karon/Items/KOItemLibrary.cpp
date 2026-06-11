// Copyright Karon Team 5. All Rights Reserved.

#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Building/KOBaseBuilding.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

const UKOLoadSubsystem* UKOItemLibrary::GetLoadSubsystem(const UObject* WorldContext)
{
    return UKOLoadSubsystem::Get(WorldContext);
}

const FKOItemRow* UKOItemLibrary::GetItemRow(const UObject* WorldContext, FName ItemId)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return nullptr;
    }
    return LS->FindItemRow(ItemId);
}

const FKOFactoryRow* UKOItemLibrary::GetFactoryRow(const UObject* WorldContext, FName FactoryId)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return nullptr;
    }
    return LS->FindFactoryRow(FactoryId);
}

FText UKOItemLibrary::GetDisplayName(const UObject* WorldContext, EKOSlotKind Kind, FName Id)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return FText::GetEmpty();
    }

    switch (Kind)
    {
    case EKOSlotKind::Item:
        if (const FKOItemRow* Row = LS->FindItemRow(Id))
        {
            return Row->DisplayName;
        }
        break;
    case EKOSlotKind::Factory:
        if (const FKOFactoryRow* Row = LS->FindFactoryRow(Id))
        {
            return Row->DisplayName;
        }
        break;
    }
    return FText::GetEmpty();
}

int32 UKOItemLibrary::GetMaxStack(const UObject* WorldContext, EKOSlotKind Kind, FName Id)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);

    switch (Kind)
    {
    case EKOSlotKind::Item:
        if (LS)
        {
            if (const FKOItemRow* Row = LS->FindItemRow(Id))
            {
                return Row->MaxStack;
            }
        }
        return DefaultItemMaxStack;

    case EKOSlotKind::Factory:
        if (LS)
        {
            if (const FKOFactoryRow* Row = LS->FindFactoryRow(Id))
            {
                return Row->MaxStack;
            }
        }
        return DefaultFactoryMaxStack;
    }
    return DefaultItemMaxStack;
}

UTexture2D* UKOItemLibrary::GetIcon(const UObject* WorldContext, EKOSlotKind Kind, FName Id)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return nullptr;
    }

    switch (Kind)
    {
    case EKOSlotKind::Item:    return LS->ResolveItemIcon(Id);
    case EKOSlotKind::Factory: return LS->ResolveFactoryIcon(Id);
    }
    return nullptr;
}

bool UKOItemLibrary::HasRow(const UObject* WorldContext, EKOSlotKind Kind, FName Id)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return false;
    }

    switch (Kind)
    {
    case EKOSlotKind::Item:    return LS->FindItemRow(Id)    != nullptr;
    case EKOSlotKind::Factory: return LS->FindFactoryRow(Id) != nullptr;
    }
    return false;
}

void UKOItemLibrary::GatherFactoryPortSlots(const UObject* WorldContext, const AKOBaseBuilding* Building, TArray<FKOFactoryPortSlot>& OutSlots)
{
    OutSlots.Reset();

    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS || !Building)
    {
        return;
    }

    TArray<FName> InputItemIds;
    TArray<FName> OutputItemIds;

    // 레시피 한 건의 입력/출력 ItemId 를 누적하는 로컬 헬퍼(중복 제거).
    auto AppendRecipe = [LS, &InputItemIds, &OutputItemIds](const FKORecipeRow* Recipe)
    {
        if (!Recipe)
        {
            return;
        }
        for (const TPair<FGameplayTag, int32>& In : Recipe->Inputs)
        {
            const FName ItemId = LS->FindItemIdByTag(In.Key);
            if (!ItemId.IsNone())
            {
                InputItemIds.AddUnique(ItemId);
            }
        }
        for (const TPair<FGameplayTag, int32>& Out : Recipe->Outputs)
        {
            const FName ItemId = LS->FindItemIdByTag(Out.Key);
            if (!ItemId.IsNone())
            {
                OutputItemIds.AddUnique(ItemId);
            }
        }
    };

    if (const UKOFactoryProcessorComponent* Processor = Building->FindComponentByClass<UKOFactoryProcessorComponent>())
    {
        // Processor 머신: "현재 선택된 레시피"의 포트만 노출. 미선택이면 아무것도 안 보임(선택 전 노출 방지).
        const FName SelectedRecipeId = Processor->GetSelectedRecipe();
        if (SelectedRecipeId.IsNone())
        {
            return;
        }
        AppendRecipe(LS->FindRecipeRow(SelectedRecipeId));
    }
    else
    {
        // Processor 없는 머신(Producer 등): 레시피 선택 개념이 없으므로 카테고리 매칭 레시피 합집합으로 폴백.
        const FKOFactoryRow* FactoryRow = Building->GetFactoryRow();
        if (!FactoryRow || !FactoryRow->FactoryCategoryTag.IsValid())
        {
            return;
        }

        TArray<FName> AllRecipes;
        LS->GetAllRecipeIds(AllRecipes);
        for (const FName& RecipeId : AllRecipes)
        {
            const FKORecipeRow* Recipe = LS->FindRecipeRow(RecipeId);
            if (!Recipe || !Recipe->AllowedFactoryTag.IsValid())
            {
                continue;
            }
            if (!FactoryRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag))
            {
                continue;
            }
            AppendRecipe(Recipe);
        }
    }

    // 일반 포트 모델: 수집한 아이템을 그룹 내 PortIndex(위치)에 매핑. ItemId 는 표시 힌트.
    OutSlots.Reserve(InputItemIds.Num() + OutputItemIds.Num());
    for (int32 Index = 0; Index < InputItemIds.Num(); ++Index)
    {
        OutSlots.Emplace(EKOPortKind::Input, Index, InputItemIds[Index]);
    }
    for (int32 Index = 0; Index < OutputItemIds.Num(); ++Index)
    {
        OutSlots.Emplace(EKOPortKind::Output, Index, OutputItemIds[Index]);
    }
}