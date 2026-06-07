#include "KOEnemyAttackNotifyState.h"

#include "AbilitySystemComponent.h"
#include "Karon.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Ability/Enemy/KOEnemyAttackGameplayAbility.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"

#include "Character/Hero/KOHeroCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UKOEnemyAttackNotifyState::UKOEnemyAttackNotifyState()
{
	bIsNativeBranchingPoint = true;
}

void UKOEnemyAttackNotifyState::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp ||
		!MeshComp->GetOwner() ||
		!MeshComp->GetAnimInstance())
	{
		return;
	}
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!Enemy)
	{
		return;
	}
	UAbilitySystemComponent* AbilitySystemComponent = Enemy->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return;
	}
	UKOEnemyAttackGameplayAbility* EnemyGA = Cast<UKOEnemyAttackGameplayAbility>(
		AbilitySystemComponent->GetAnimatingAbility());
	CachedAbilities.Add(MeshComp, EnemyGA);

	//소켓 위치로 변수 초기화
	//각 타입에 따라서 트레이스하는 소켓 위치가 달라진다.

	EnemyGA->CurrentAttackSocketLocation = Enemy->GetSocketLocation();
	EnemyGA->PresentAttackSocketLocation = Enemy->GetSocketLocation();

}
void UKOEnemyAttackNotifyState::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	CachedAbilities[MeshComp]->bIsAttacked=false;
	//캐싱 맵에서 제거
	CachedAbilities.Remove(MeshComp);
}

void UKOEnemyAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
	if (CachedAbilities[MeshComp]->bIsAttacked)
	{
		return;
	}
	
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (!IsValid(Enemy) || Enemy->GetAbilitySystemComponent() == nullptr)
	{
		return;
	}

	CachedAbilities[MeshComp]->CurrentAttackSocketLocation = Enemy->GetSocketLocation();

	//Sphere Trace
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Enemy);
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	//ECC_Player 채널로 Single Trace, 플레이어만 콜리전 가능
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		Enemy->GetWorld(),
		CachedAbilities[MeshComp]->PresentAttackSocketLocation,
		CachedAbilities[MeshComp]->CurrentAttackSocketLocation,
		TraceRadius,
		//Pawn만 Trace 처리
		UEngineTypes::ConvertToTraceType(ECC_Player),
		false, // bTraceComplex
		ActorsToIgnore,
		DebugType,
		HitResult,
		true, // bIgnoreSelf
		FLinearColor::Red, // TraceColor
		FLinearColor::Green, // TraceHitColor
		2.0f // DrawTime
	);
	//이전 소켓의 위치를 갱신
	CachedAbilities[MeshComp]->PresentAttackSocketLocation = CachedAbilities[MeshComp]->CurrentAttackSocketLocation;
	//피격되지 않았으면 Early Return
	if (!bHit)
	{
		return;
	}

	AActor* HittedActor = HitResult.GetActor();
	if (!HittedActor)
	{
		return;
	}
	//플레이어 클래스만 데미지 가능
	AKOHeroCharacter* Player = Cast<AKOHeroCharacter>(HittedActor);
	if (!Player)
	{
		return;
	}
	
	//Event_SkillHit 태그로 전달
	FGameplayEventData HitGameplayEventData;
	HitGameplayEventData.Target = HittedActor;
	Enemy->GetAbilitySystemComponent()->HandleGameplayEvent(KOGameplayTags::Event_SkillHit, &HitGameplayEventData);
	//재타격 방지
	CachedAbilities[MeshComp]->bIsAttacked=true;
}

