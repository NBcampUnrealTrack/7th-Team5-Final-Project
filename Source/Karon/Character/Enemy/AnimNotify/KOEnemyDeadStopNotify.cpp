// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyDeadStopNotify.h"

#include "Character/Enemy/KOBaseEnemy.h"

void UKOEnemyDeadStopNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                    const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp||!MeshComp->GetOwner())
	{
		return;
	}
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	//애니메이션 일시정지
	MeshComp->bPauseAnims = true;
	MeshComp->GlobalAnimRateScale=0.f;
	
}
