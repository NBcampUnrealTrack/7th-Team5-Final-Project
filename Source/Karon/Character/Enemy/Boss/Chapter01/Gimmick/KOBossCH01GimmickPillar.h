#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossCH01GimmickPillar.generated.h"

class UNiagaraSystem;
class AStaticMeshActor;

UCLASS()
class KARON_API AKOBossCH01GimmickPillar : public AActor
{
	GENERATED_BODY()
	
public:
	AKOBossCH01GimmickPillar();
	
	/** 기둥을 파괴 상태로 변경한다. 실제 Actor는 Destroy하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|Gimmick|Pillar")
	void BreakPillar();

	// 세이브 로드
	void RestoreFromSave(bool bSavedBroken);

	FName GetPillarSaveId() const { return PillarSaveId; }
	bool IsBrokenForSave() const { return bBroken; }
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	void ApplyBrokenState();
	void ExplodeDebris();
	void UpdateDebrisFade(float DeltaTime);
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Pillar | Component")
	TObjectPtr<UStaticMeshComponent> PillarMesh;
	
	// 세이브 로드
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pillar|Save")
	FName PillarSaveId = NAME_None;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pillar|Save")
	bool bBroken = false;
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Effect")
	TObjectPtr<UNiagaraSystem> DustEffect;
	
	// 파편화
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	TObjectPtr<UStaticMesh> DebrisMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	int32 DebrisMinCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	int32 DebrisMaxCount = 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	FVector2D DebrisScaleRange = FVector2D(0.12f, 0.28f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	float DebrisImpulseStrength = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	float DebrisLifeSpan = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pillar | Break")
	float DebrisFadeOutDuration = 0.6f;

private:
	struct FDebrisFadeInfo
	{
		TWeakObjectPtr<AStaticMeshActor> DebrisActor;
		FVector InitialScale = FVector::OneVector;
		float ElapsedTime = 0.f;
		bool bFadeStarted = false;
	};

	TArray<FDebrisFadeInfo> FadingDebris;
};
