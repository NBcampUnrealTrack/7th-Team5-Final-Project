// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyAttackNotifyState.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Component/KOAnimNotifyComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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
	if (AnimNotifyComponent==nullptr)
	{
		return;
	}
	
	//소켓 위치로 변수 초기화
	//각 타입에 따라서 트레이스하는 소켓 위치가 달라진다.

	AnimNotifyComponent->CurrentAttackSocketLocation  = Enemy->GetSocketLocation();
	AnimNotifyComponent->PresentAttackSocketLocation = Enemy->GetSocketLocation();
	
}

void UKOEnemyAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp||!MeshComp->GetOwner())
	{
		return;
	}
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!IsValid(Enemy)||Enemy->GetAbilitySystemComponent()==nullptr)
	{
		return;
	}
	UKOAnimNotifyComponent* AnimNotifyComponent= Enemy->GetAnimNotifyComponent();
	if (AnimNotifyComponent==nullptr)
	{
		return;
	}
	
	AnimNotifyComponent->CurrentAttackSocketLocation = Enemy->GetSocketLocation();

	//Sphere Trace
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Enemy);
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	
	
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Enemy->GetWorld(),
		AnimNotifyComponent->PresentAttackSocketLocation,
		AnimNotifyComponent->CurrentAttackSocketLocation,
		TraceRadius,
		//Pawn만 Trace 처리
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, // bTraceComplex
		ActorsToIgnore,
		DebugType,
		HitResults,
		true, // bIgnoreSelf
		FLinearColor::Red, // TraceColor
		FLinearColor::Green, // TraceHitColor
		2.0f // DrawTime
	);
	//이전 소켓의 위치를 갱신
	AnimNotifyComponent->PresentAttackSocketLocation = AnimNotifyComponent->CurrentAttackSocketLocation;
	//피격되지 않았으면 Early Return
	if (!bHit)
	{
		return;
	}
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HittedActor=HitResult.GetActor();
		//1번 공격시 이미 피격당한 목록에 있다면 중복 타격 방지
		if (!HittedActor||AnimNotifyComponent->HittedCharacterArray.Contains(HittedActor))
		{
			continue;
		}
		
		//피격당한 목록에 추가
		AnimNotifyComponent->HittedCharacterArray.Add(HittedActor);
		FGameplayEventData HitGameplayEventData;
		
		HitGameplayEventData.Target=HittedActor;
		Enemy->GetAbilitySystemComponent()->HandleGameplayEvent(KOGameplayTags::Event_SkillHit,&HitGameplayEventData);
		
		
	}
}

void UKOEnemyAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!IsValid(Enemy)||Enemy->GetAbilitySystemComponent()==nullptr)
	{
		return;
	}
	UKOAnimNotifyComponent* AnimNotifyComponent= Enemy->GetAnimNotifyComponent();
	if (AnimNotifyComponent==nullptr)
	{
		return;
	}
	//피격당한 목록을 비워준다.
	AnimNotifyComponent->HittedCharacterArray.Empty();
}
