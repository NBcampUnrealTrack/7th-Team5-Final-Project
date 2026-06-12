#include "KOGA_AnimNotify_ConsumeBuffer.h"

#include "Component/Combat/KOComboComponent.h"

void UKOGA_AnimNotify_ConsumeBuffer::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] 노티파이 도달"));
	
	AActor* OwnerActor = MeshComp->GetOwner();
	
	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			if (UKOComboComponent* ComboComp = Controller->FindComponentByClass<UKOComboComponent>())
			{
				UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] 컨트롤러 컴포넌트 발견 ConsumeBuffer 호출"));
				ComboComp->ConsumeBuffer();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[AnimNotify Error] 컨트롤러에서 ComboComponent를 찾을 수 없음."));
			}
		}
	}
}
