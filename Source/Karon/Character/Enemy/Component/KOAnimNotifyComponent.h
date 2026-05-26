// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOAnimNotifyComponent.generated.h"

/**
 * AnimNotify에서 수정되는 변수를 관리하는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOAnimNotifyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//이전 Notify틱의 소켓의 위치
	UPROPERTY()
	FVector PresentAttackSocketLocation=FVector::ZeroVector;
	//현재 Notify틱의 소켓의 위치
	UPROPERTY()
	FVector CurrentAttackSocketLocation=FVector::ZeroVector;
	//피격된 적을 중복 방지용 저장 액터 컨테이너
	UPROPERTY()
	TArray<AActor*> HittedCharacterArray;
};
