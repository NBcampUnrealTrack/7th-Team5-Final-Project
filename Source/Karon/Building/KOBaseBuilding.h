#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBaseBuilding.generated.h"

struct FKOFactoryRow;

UCLASS()
class KARON_API AKOBaseBuilding : public AActor
{
	GENERATED_BODY()

public:
	AKOBaseBuilding();

public:
	// 건설 직후 BuildComponent가 FactoryId를 넘겨주는 함수
	UFUNCTION(BlueprintCallable, Category = "Building")
	void InitializeBuildingData(FName InFactoryId);

	UFUNCTION(BlueprintPure, Category = "Building")
	FName GetFactoryId() const { return FactoryId; }

	/** LoadSubsystem을 통해 자신이 참조하는 Factory Row를 조회 */
	const FKOFactoryRow* GetFactoryRow() const;

protected:
	// 실제 월드에 설치된 건물이 참조할 Factory DataTable의 RowName
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Building")
	FName FactoryId = NAME_None;
};
