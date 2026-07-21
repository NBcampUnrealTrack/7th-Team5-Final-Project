#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossCH01Platform.generated.h"

class UNiagaraSystem;
class UGameplayEffect;
class UBoxComponent;
class AStaticMeshActor;

UCLASS()
class KARON_API AKOBossCH01Platform : public AActor
{
	GENERATED_BODY()
	
public:
	AKOBossCH01Platform();
 
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Platform | Component")
	TObjectPtr<USceneComponent> PlatformRoot;

	UPROPERTY(VisibleAnywhere, Category = "Platform | Component")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;
 
	// 낙하 중 피격 판정용
	UPROPERTY(VisibleAnywhere, Category = "Platform | Component")
	TObjectPtr<UBoxComponent> DamageCollision;
 
	UPROPERTY(VisibleAnywhere, Category = "Platform | Indicator")
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Fall")
	float FallSpeed = 500.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Fall")
	float HoverDuration = 1.5f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Option")
	float PlatformLifeSpan = 5.f;
 
	// 발판 위 판정 오차 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Option")
	float PlatformTopTolerance = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Debug")
	bool bShowDebug = true;
	
	// 착지 시 SFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | SFX")
	TObjectPtr<USoundBase> LandSFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | VFX")
	TObjectPtr<UNiagaraSystem> LandVFX;

	// 파괴 시 SFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | SFX")
	TObjectPtr<USoundBase> BreakSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | VFX")
	TObjectPtr<UNiagaraSystem> BreakVFX;
	
	// 파괴 연출용 파편 메시 (비워두면 발판 자체 메시를 축소해서 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	TObjectPtr<UStaticMesh> DebrisMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	int32 DebrisMinCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	int32 DebrisMaxCount = 9;

	// 파편 크기 (발판 크기 대비 배율, 무작위 범위)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	FVector2D DebrisScaleRange = FVector2D(0.12f, 0.28f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	float DebrisImpulseStrength = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	float DebrisLifeSpan = 2.f;

	// 파편이 사라지기 전 작아지며 페이드 아웃하는 데 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Break")
	float DebrisFadeOutDuration = 0.6f;

private:
	bool bLanded = false;
	bool bFalling = false;
	bool bBroken = false;

	// ─── 추가 : Ease In 낙하용 경과 시간 ─────────────────────
	float FallElapsedTime = 0.f;
	
	UPROPERTY()
	TObjectPtr<AActor> PlayerOnPlatform;
 
	FTimerHandle LifeSpanTimerHandle;
	FTimerHandle HoverTimerHandle;
	
	struct FDebrisFadeInfo
	{
		TWeakObjectPtr<AStaticMeshActor> DebrisActor;
		FVector InitialScale = FVector::OneVector;
		float ElapsedTime = 0.f;
		bool bFadeStarted = false;
	};

	TArray<FDebrisFadeInfo> FadingDebris;
	
	void CheckPlayerOnPlatform();
	
	void SpawnIndicatorOnGround();
 
	UFUNCTION()
	void OnDamageCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
 
	void SetOnPlatformTag(AActor* TargetActor, bool bAdd);
	void StartFall();
	void OnLanded();
	void LifeTimeEnd();
	void BreakApart();
	void UpdateDebrisFade(float DeltaTime);
};
