// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/KODataTableTypes.h"
#include "KOSkillLibrary.generated.h"

class UKOLoadSubsystem;
/**
 * 
 */
UCLASS()
class KARON_API UKOSkillLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static const FKOSkillRow* GetSkillRow(const UObject* WorldContext, FName SkillId);
	
	static FText       GetDisplayName(const UObject* WorldContext, FName SkillId);
	static UTexture2D* GetIcon(const UObject*        WorldContext, FName SkillId);
	static bool        HasRow(const UObject*         WorldContext, FName SkillId);
	
private:
	static const UKOLoadSubsystem* GetLoadSubsystem(const UObject* WorldContext);
};
