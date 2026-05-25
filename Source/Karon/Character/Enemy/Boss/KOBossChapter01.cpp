#include "KOBossChapter01.h"
 
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
 
AKOBossChapter01::AKOBossChapter01()
{
}
 
void AKOBossChapter01::BeginPlay()
{
	Super::BeginPlay();
}

void AKOBossChapter01::OnBossInitialized()
{
	// 코어 머티리얼 슬롯을 Dynamic으로 교체
	if (GetMesh())
	{
		CoreMID = GetMesh()->CreateDynamicMaterialInstance(
			CoreMaterialIndex
		);
	}
 
	// 시작 시 코어 꺼진 상태 보장
	CloseCore();
}
 
// 페이즈 전환
void AKOBossChapter01::OnPhaseChanged(int32 NewPhase)
{
	if (NewPhase == 2)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[BossChapter01] 페이즈2 진입"));
 
		// TODO: 페이즈2 처리
		// 이동속도 증가 GE 적용
		// BT 블랙보드 키 변경으로 패턴 전환 IsPhase02 = true
	}
}
 
// 그로기 진입 > 코어 개방 
void AKOBossChapter01::OnGroggyBegin()
{
	if (bIsDead) return;
 
	UE_LOG(LogTemp, Log,
		TEXT("[BossChapter01] 그로기 진입 - 코어 개방"));
 
	OpenCore();
 
	// CoreOpenDuration 후 자동으로 코어 닫힘
	GetWorldTimerManager().SetTimer(
		CoreCloseTimerHandle,
		this,
		&AKOBossChapter01::OnGroggyEnd,
		CoreOpenDuration,
		false
	);
}
 
// 그로기 종료 > 코어 닫기
void AKOBossChapter01::OnGroggyEnd()
{
	if (bIsDead) return;
 
	UE_LOG(LogTemp, Log,
		TEXT("[BossChapter01] 그로기 종료 - 코어 닫힘"));
 
	// 타이머가 남아있으면 취소
	GetWorldTimerManager().ClearTimer(CoreCloseTimerHandle);
 
	CloseCore();
 
	// TODO: 그로기 종료 후 보스 복귀 처리
	// BT 블랙보드 bIsGroggy = false
}
 
// 사망
void AKOBossChapter01::OnBossDeath()
{
	if (bIsDead) return;
	bIsDead = true;
 
	UE_LOG(LogTemp, Log,
		TEXT("[BossChapter01] 보스 사망"));
 
	// 진행 중인 타이머 정리
	GetWorldTimerManager().ClearTimer(CoreCloseTimerHandle);
 
	// TODO: 사망 처리
	// AI 정지
	// 사망 몽타주 재생
	// 클리어 이벤트 발생
}
 
// 코어 개방
void AKOBossChapter01::OpenCore()
{
	bCoreOpen = true;
 
	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue(
			"EmissiveIntensity",
			CoreEmissiveIntensity
		);
	}
}
 
// 코어 닫기
void AKOBossChapter01::CloseCore()
{
	bCoreOpen = false;
 
	if (CoreMID)
	{
		CoreMID->SetScalarParameterValue(
			"EmissiveIntensity",
			0.f
		);
	}
}
