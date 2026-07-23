#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOQuestGateActor.generated.h"

class UBoxComponent;
class USceneComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class KARON_API AKOQuestGateActor : public AActor
{
	GENERATED_BODY()

public:
	AKOQuestGateActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "KO|QuestGate")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "KO|QuestGate")
	TObjectPtr<UBoxComponent> BlockCollision;

	// 안개 이펙트 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "KO|QuestGate")
	TObjectPtr<UNiagaraComponent> FogEffect;

	// 안개 Niagara System
	UPROPERTY(EditAnywhere, Category = "KO|QuestGate")
	TObjectPtr<UNiagaraSystem> FogSystem;

	// 퀘스트 ID
	UPROPERTY(EditAnywhere, Category = "KO|QuestGate")
	FName RequiredCompletedQuestId = NAME_None;

	UFUNCTION()
	void HandleQuestChanged(FName NewQuestId);

	void RefreshGateState();
	bool ShouldGateOpen() const;

	void OpenGate();
	void CloseGate();

	bool bIsOpen = false;
};