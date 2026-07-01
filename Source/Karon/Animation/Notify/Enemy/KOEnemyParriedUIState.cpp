// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyParriedUIState.h"

#include "Character/Enemy/KOBaseEnemy.h"

UKOEnemyParriedUIState::UKOEnemyParriedUIState()
{
	bIsNativeBranchingPoint = true;
}

void UKOEnemyParriedUIState::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	
	if (!MeshComp || !MeshComp->GetOwner() || !MeshComp->GetAnimInstance())
	{
		return;
	}
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!Enemy)
	{
		return;
	}
	// 에너미 그로기 UI 활성화
	Enemy->OnParriedEvent.ExecuteIfBound(true);
	Enemy->ChangeLockOnGroggy(true);
}

void UKOEnemyParriedUIState::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	
	if (!MeshComp || !MeshComp->GetOwner() || !MeshComp->GetAnimInstance())
	{
		return;
	}
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!Enemy)
	{
		return;
	}
	// 에너미 그로기 UI 비활성화
	Enemy->OnParriedEvent.ExecuteIfBound(false);
	Enemy->ChangeLockOnGroggy(false);
}
