#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
#include "Data/KODataTableTypes.h"
#include "KOBuildAssignMenuWidget.generated.h"

class UPanelWidget;
class UKOBuildAssignEntryWidget;

UCLASS()
class KARON_API UKOBuildAssignMenuWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Build|AssignMenu")
	void RebuildEntries();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> EntryContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build|AssignMenu")
	TSubclassOf<UKOBuildAssignEntryWidget> EntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|AssignMenu")
	FKOBuildMenuQuery Query;
};