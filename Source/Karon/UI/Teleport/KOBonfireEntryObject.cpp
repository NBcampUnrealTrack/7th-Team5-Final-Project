// Copyright Karon Team 5. All Rights Reserved.

#include "KOBonfireEntryObject.h"

void UKOBonfireEntryObject::Initialize(const FBonfireUIData& InData)
{
	BonfireData = InData;
}

const FName& UKOBonfireEntryObject::GetBonfireID() const
{
	return BonfireData.BonfireID;
}

const FText& UKOBonfireEntryObject::GetDisplayName() const
{
	return BonfireData.DisplayName;
}
