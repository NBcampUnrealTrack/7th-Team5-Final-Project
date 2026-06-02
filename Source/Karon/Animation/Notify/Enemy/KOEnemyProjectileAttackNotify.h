// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOEnemyProjectileAttackNotify.generated.h"

class AKOEnemyProjectileActor;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyProjectileAttackNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							const FAnimNotifyEventReference& EventReference)override;

protected:
	FName SocketName=FName("ProjectileSocket");
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AKOEnemyProjectileActor> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	float DamageMultiplier=1.f;
};
