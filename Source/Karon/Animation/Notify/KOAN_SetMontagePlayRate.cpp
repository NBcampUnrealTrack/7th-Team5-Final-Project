#include "Animation/Notify/KOAN_SetMontagePlayRate.h"

#include "GameFramework/Character.h"

void UKOAN_SetMontagePlayRate::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner()) { return; }
 
	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character) { return; }
 
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) { return; }
 
	// 현재 재생 중인 몽타주 속도 변경
	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (!CurrentMontage) { return; }
 
	AnimInstance->Montage_SetPlayRate(CurrentMontage, PlayRate);
 
	UE_LOG(LogTemp, Log, TEXT("[SetPlayRate] PlayRate : %.2f"), PlayRate);
}
