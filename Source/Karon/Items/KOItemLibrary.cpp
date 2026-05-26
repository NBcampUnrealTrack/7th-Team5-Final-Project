// Copyright Karon Team 5. All Rights Reserved.

#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

namespace
{
    constexpr int32 GDefaultItemMaxStack    = 100;
    constexpr int32 GDefaultFactoryMaxStack = 1;
}

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

UStaticMesh* UKOItemLibrary::GetItemMesh(const UObject* WorldContext, FName ItemId)
{
    if (const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext))
    {
        return LS->ResolveItemMesh(ItemId);
    }
    return nullptr;
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
        return GDefaultItemMaxStack;

    case EKOSlotKind::Factory:
        if (LS)
        {
            if (const FKOFactoryRow* Row = LS->FindFactoryRow(Id))
            {
                return Row->MaxStack;
            }
        }
        return GDefaultFactoryMaxStack;
    }
    return GDefaultItemMaxStack;
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
