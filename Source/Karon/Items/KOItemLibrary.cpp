// Copyright Karon Team 5. All Rights Reserved.

#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

const UKOLoadSubsystem* UKOItemLibrary::GetLoadSubsystem(const UObject* WorldContext)
{
    return UKOLoadSubsystem::Get(WorldContext);
}

bool UKOItemLibrary::GetItemRow(const UObject* WorldContext, FName ItemId, FKOItemRow& OutRow)
{
    const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
    if (!LS)
    {
        return false;
    }

    const FKOItemRow* Row = LS->FindItemRow(ItemId);
    if (!Row)
    {
        return false;
    }

    OutRow = *Row;
    return true;
}

FText UKOItemLibrary::GetItemDisplayName(const UObject* WorldContext, FName ItemId)
{
    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        if (const FKOItemRow* Row = LS->FindItemRow(ItemId))
        {
            return Row->DisplayName;
        }
    }
    return FText::GetEmpty();
}

int32 UKOItemLibrary::GetItemMaxStack(const UObject* WorldContext, FName ItemId)
{
    constexpr int32 DefaultMaxStack = 100;

    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        if (const FKOItemRow* Row = LS->FindItemRow(ItemId))
        {
            return Row->MaxStack;
        }
    }
    return DefaultMaxStack;
}

FGameplayTagContainer UKOItemLibrary::GetItemCategories(const UObject* WorldContext, FName ItemId)
{
    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        if (const FKOItemRow* Row = LS->FindItemRow(ItemId))
        {
            return Row->Categories;
        }
    }
    return FGameplayTagContainer();
}

UTexture2D* UKOItemLibrary::GetItemIcon(const UObject* WorldContext, FName ItemId)
{
    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        return LS->ResolveItemIcon(ItemId);
    }
    return nullptr;
}

UStaticMesh* UKOItemLibrary::GetItemMesh(const UObject* WorldContext, FName ItemId)
{
    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        return LS->ResolveItemMesh(ItemId);
    }
    return nullptr;
}
