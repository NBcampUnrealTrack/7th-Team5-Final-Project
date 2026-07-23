#include "KO_AnimNotify_CameraShake.h"

void UKO_AnimNotify_CameraShake::Notify(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	APawn* Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!Pawn) return;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;
	
	PC->ClientStartCameraShake(ShakeClass, Scale); 
}
