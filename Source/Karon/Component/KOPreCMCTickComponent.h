#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOPreCMCTickComponent.generated.h"

/*
 * Pre Character Movement Tick Component 
 * 일반적인 틱 순서:
 * ActorTick -> CMCTick -> [다른 컴포넌트 개입]->AnimationTick
 * 문제: CMC가 이미 위치/속도 계산을 끝낸 후에
 *		다른 컴포넌트가 입력값을 넣으면 한 프레임 딜레이 발생
 *  
 *  
 *  PreCMCTick → [여기서 입력/궤적 계산] → CMC Tick → Animation Tick
 *  이러면 
 */


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOPreCMCTickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOPreCMCTickComponent();

protected:
	virtual void BeginPlay() override;
	
};
