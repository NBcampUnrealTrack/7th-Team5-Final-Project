// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyAttackNotifyState.h"

#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Component/KOAnimNotifyComponent.h"

void UKOEnemyAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp||!MeshComp->GetOwner())
	{
		return;
	}
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!IsValid(Enemy))
	{
		return;
	}
	
	UKOAnimNotifyComponent* AnimNotifyComponent= Enemy->GetAnimNotifyComponent();
	if (AnimNotifyComponent!=nullptr)
	{
		return;
	}
	
	//소켓 위치로 변수 초기화
	//타입에 따라서 컴포넌트 x3. 각 타입에 따라서 트레이스하는 소켓 위치가 달라진다.
	/*
	AnimNotifyComponent->CurrentAttackSocketLocation  = Player->GetWeaponMeshComp()->GetSocketTransform(SocketName, RTS_World).
												  GetLocation();
	AnimNotifyComponent->PresentAttackSocketLocation = Player->GetWeaponMeshComp()->GetSocketTransform(SocketName, RTS_World).
												  GetLocation();
	*/
}

void UKOEnemyAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UKOEnemyAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
