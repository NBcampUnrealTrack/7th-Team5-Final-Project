// Copyright Karon Team 5. All Rights Reserved.

#include "Building/Conveyor/KOConveyorBelt.h"

#include "Subsystem/KOConveyorSubsystem.h"
#include "Subsystem/KOGridSubsystem.h"
#include "Components/ActorComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

namespace
{
    // 월드 방향을 축 정렬 그리드 스텝(±1,0)/(0,±1)으로 변환.
    FIntPoint WorldDirToGridStep(const FVector& Dir)
    {
        if (FMath::Abs(Dir.X) >= FMath::Abs(Dir.Y))
        {
            return FIntPoint(Dir.X >= 0.f ? 1 : -1, 0);
        }
        return FIntPoint(0, Dir.Y >= 0.f ? 1 : -1);
    }

    // 그리드 스텝을 월드 단위 방향으로(그리드 X/Y = 월드 X/Y 직접 매핑).
    FVector GridStepToWorldDir(const FIntPoint& Step)
    {
        return FVector(static_cast<float>(Step.X), static_cast<float>(Step.Y), 0.f).GetSafeNormal();
    }
}

AKOConveyorBelt::AKOConveyorBelt()
{
    PrimaryActorTick.bCanEverTick = false; // 서브시스템이 구동.
}

void AKOConveyorBelt::BeginPlay()
{
    Super::BeginPlay();

    SlotCount = FMath::Max(1, SlotCount);
    Slots.SetNum(SlotCount); // 기본값 = 빈 아이템(NAME_None).
    MoveAccumulator = 0.f;

    RecomputePortDirections();

    if (UKOConveyorSubsystem* Subsystem = UKOConveyorSubsystem::Get(this))
    {
        Subsystem->RegisterBelt(this);
    }
}

void AKOConveyorBelt::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UKOConveyorSubsystem* Subsystem = UKOConveyorSubsystem::Get(this))
    {
        Subsystem->UnregisterBelt(this);
    }
    Super::EndPlay(Reason);
}

void AKOConveyorBelt::RecomputePortDirections()
{
    FVector FwdWorld   = GetActorForwardVector().GetSafeNormal2D();
    FVector RightWorld = GetActorRightVector().GetSafeNormal2D();
    if (FwdWorld.IsNearlyZero())   { FwdWorld   = FVector::ForwardVector; }
    if (RightWorld.IsNearlyZero()) { RightWorld = FVector::RightVector; }

    const FIntPoint Forward = WorldDirToGridStep(FwdWorld);   // 로컬 +X 다리
    const FIntPoint Side    = WorldDirToGridStep(RightWorld); // 로컬 +Y 다리

    if (Shape == EKOBeltShape::Corner)
    {
        // ㄱ자: 두 열린 다리 = +Forward(로컬 +X), +Side(로컬 +Y).
        // 흐름은 이 두 다리를 잇고, bCornerFlip 은 입/출구만 교환한다 → 같은 L 메시로 좌/우 코너 모두 표현(거울 메시 불필요).
        if (!bCornerFlip)
        {
            InDir  = -Side;     // 입구 이웃 = MyCell - InDir = MyCell + Side
            OutDir =  Forward;  // 출구 이웃 = MyCell + Forward
        }
        else
        {
            InDir  = -Forward;  // 입구 이웃 = MyCell + Forward
            OutDir =  Side;     // 출구 이웃 = MyCell + Side
        }
    }
    else
    {
        // 직선: 입구=-Forward 셀, 출구=+Forward 셀.
        InDir  = Forward;
        OutDir = Forward;
    }

    // 디버그/비주얼용: 중심에서 입구/출구 이웃을 향하는 월드 방향.
    EntryDirWorld = GridStepToWorldDir(-InDir);
    ExitDirWorld  = GridStepToWorldDir(OutDir);

    if (const UKOGridSubsystem* Grid = GetWorld() ? GetWorld()->GetSubsystem<UKOGridSubsystem>() : nullptr)
    {
        MyCell   = Grid->WorldToGridPosition(GetActorLocation());
        CellSize = Grid->GetCellSize();
    }
}

void AKOConveyorBelt::SetCornerFlip(bool bInFlip)
{
    if (bCornerFlip != bInFlip)
    {
        bCornerFlip = bInFlip;
        RecomputePortDirections();
    }
}

void AKOConveyorBelt::ApplyPlacementFlow(bool bManualFlipFallback)
{
    if (Shape == EKOBeltShape::Corner)
    {
        bool bAutoFlip = false;
        bCornerFlip = TryResolveCornerFlipFromNeighbors(bAutoFlip) ? bAutoFlip : bManualFlipFallback;
    }
    RecomputePortDirections();
}

bool AKOConveyorBelt::TryResolveCornerFlipFromNeighbors(bool& OutFlip) const
{
    const UKOGridSubsystem* Grid = GetWorld() ? GetWorld()->GetSubsystem<UKOGridSubsystem>() : nullptr;
    if (!Grid)
    {
        return false;
    }
    const FIntPoint Cell = Grid->WorldToGridPosition(GetActorLocation());

    FVector FwdWorld   = GetActorForwardVector().GetSafeNormal2D();
    FVector RightWorld = GetActorRightVector().GetSafeNormal2D();
    if (FwdWorld.IsNearlyZero())   { FwdWorld   = FVector::ForwardVector; }
    if (RightWorld.IsNearlyZero()) { RightWorld = FVector::RightVector; }

    const FIntPoint Forward = WorldDirToGridStep(FwdWorld);   // LegA = +Forward
    const FIntPoint Side    = WorldDirToGridStep(RightWorld); // LegB = +Side

    // 이웃 1칸을 분류: +1 = 나에게 공급(업스트림), -1 = 내가 공급(다운스트림), 0 = 모호/없음.
    auto Classify = [&](const FIntPoint& NeighborCell) -> int32
    {
        AActor* Actor = GetActorAtCell(NeighborCell);
        if (!Actor)
        {
            return 0;
        }
        if (const AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(Actor))
        {
            if (Belt->OutputsToCell(Cell))  { return +1; } // 이웃 벨트가 나를 향해 출력 → 업스트림
            if (Belt->InputsFromCell(Cell)) { return -1; } // 이웃 벨트가 나에게서 입력 → 다운스트림
            return 0;
        }
        // 머신: 단방향 포트만 있으면 방향 확정, 양쪽(Processor) 또는 없음이면 모호.
        const bool bHasSource = ResolveSource(Actor) != nullptr;
        const bool bHasSink   = ResolveSink(Actor)   != nullptr;
        if (bHasSource && !bHasSink) { return +1; } // 출력만 → 나에게 공급
        if (bHasSink && !bHasSource) { return -1; } // 입력만 → 내가 공급
        return 0;
    };

    const int32 RoleA = Classify(Cell + Forward); // LegA(+Forward)
    const int32 RoleB = Classify(Cell + Side);    // LegB(+Side)

    // flip=false: 입구=+Side(LegB), 출구=+Forward(LegA).
    // flip=true : 입구=+Forward(LegA), 출구=+Side(LegB).
    const bool bWantFalse = (RoleB > 0) || (RoleA < 0); // Side 가 업스트림 또는 Forward 가 다운스트림
    const bool bWantTrue  = (RoleA > 0) || (RoleB < 0); // Forward 가 업스트림 또는 Side 가 다운스트림

    if (bWantFalse && !bWantTrue) { OutFlip = false; return true; }
    if (bWantTrue && !bWantFalse) { OutFlip = true;  return true; }
    return false; // 양쪽 충돌 또는 단서 없음 → 수동 폴백.
}

void AKOConveyorBelt::AdvanceBelt(float DeltaTime)
{
    if (SlotsPerSecond > 0.f)
    {
        MoveAccumulator += SlotsPerSecond * DeltaTime;

        // 한 프레임에 여러 칸 전진할 수 있으나 슬롯 수를 넘지 않게 가드.
        int32 GuardSteps = SlotCount + 1;
        while (MoveAccumulator >= 1.f && GuardSteps-- > 0)
        {
            StepOnce();
            MoveAccumulator -= 1.f;
        }
    }

    if (bDrawSlotsDebug)
    {
        DrawSlotsDebug();
    }
}

void AKOConveyorBelt::StepOnce()
{
    const int32 TailIdx = SlotCount - 1;

    // 1) tail → 다운스트림 머신 sink push. (다운스트림이 벨트면 그쪽이 pull 하도록 skip)
    if (Slots[TailIdx].IsValid())
    {
        AActor* DownActor = GetActorAtCell(MyCell + OutDir);
        if (DownActor && !DownActor->IsA(AKOConveyorBelt::StaticClass()))
        {
            if (IKOItemSink* Sink = ResolveSink(DownActor))
            {
                if (Sink->CanAcceptItem(Slots[TailIdx]) && Sink->PushItem(Slots[TailIdx]))
                {
                    Slots[TailIdx].Reset();
                }
            }
        }
    }

    // 2) 앞(출구)쪽으로 한 칸 압축 전진. tail→head 순회로 각 아이템이 최대 1칸만 이동.
    for (int32 i = TailIdx; i > 0; --i)
    {
        if (!Slots[i].IsValid() && Slots[i - 1].IsValid())
        {
            Slots[i] = Slots[i - 1];
            Slots[i - 1].Reset();
        }
    }

    // 3) head 가 비었으면 업스트림 source 에서 pull.
    if (!Slots[0].IsValid())
    {
        AActor* UpActor = GetActorAtCell(MyCell - InDir);
        if (IKOItemSource* Src = ResolveSource(UpActor))
        {
            FKOConveyorItem Pulled;
            if (Src->PopOutputItem(Pulled))
            {
                Slots[0] = Pulled;
            }
        }
    }
}

AActor* AKOConveyorBelt::GetActorAtCell(const FIntPoint& Cell) const
{
    const UKOGridSubsystem* Grid = GetWorld() ? GetWorld()->GetSubsystem<UKOGridSubsystem>() : nullptr;
    return Grid ? Grid->GetOccupyingActorAt(Cell) : nullptr;
}

IKOItemSource* AKOConveyorBelt::ResolveSource(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }
    if (IKOItemSource* AsActor = Cast<IKOItemSource>(Actor))
    {
        return AsActor;
    }
    TInlineComponentArray<UActorComponent*> Components(Actor);
    for (UActorComponent* Comp : Components)
    {
        if (IKOItemSource* AsComp = Cast<IKOItemSource>(Comp))
        {
            return AsComp;
        }
    }
    return nullptr;
}

IKOItemSink* AKOConveyorBelt::ResolveSink(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }
    if (IKOItemSink* AsActor = Cast<IKOItemSink>(Actor))
    {
        return AsActor;
    }
    TInlineComponentArray<UActorComponent*> Components(Actor);
    for (UActorComponent* Comp : Components)
    {
        if (IKOItemSink* AsComp = Cast<IKOItemSink>(Comp))
        {
            return AsComp;
        }
    }
    return nullptr;
}

// ─── IKOItemSource (tail) ────────────────────────────────────────────────────
bool AKOConveyorBelt::PeekOutputItem(FKOConveyorItem& OutItem) const
{
    const int32 TailIdx = SlotCount - 1;
    if (Slots.IsValidIndex(TailIdx) && Slots[TailIdx].IsValid())
    {
        OutItem = Slots[TailIdx];
        return true;
    }
    return false;
}

bool AKOConveyorBelt::PopOutputItem(FKOConveyorItem& OutItem)
{
    const int32 TailIdx = SlotCount - 1;
    if (Slots.IsValidIndex(TailIdx) && Slots[TailIdx].IsValid())
    {
        OutItem = Slots[TailIdx];
        Slots[TailIdx].Reset();
        return true;
    }
    return false;
}

// ─── IKOItemSink (head) ──────────────────────────────────────────────────────
bool AKOConveyorBelt::CanAcceptItem(const FKOConveyorItem& Item) const
{
    return Item.IsValid() && Slots.IsValidIndex(0) && !Slots[0].IsValid();
}

bool AKOConveyorBelt::PushItem(const FKOConveyorItem& Item)
{
    if (CanAcceptItem(Item))
    {
        Slots[0] = Item;
        return true;
    }
    return false;
}

void AKOConveyorBelt::DrawSlotsDebug() const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector CellCenter = GetActorLocation() + FVector(0, 0, 30.f);
    const FVector EntryEdge   = CellCenter + EntryDirWorld * (CellSize * 0.5f);
    const FVector ExitEdge    = CellCenter + ExitDirWorld  * (CellSize * 0.5f);
    const float   SlotRadius  = (CellSize / static_cast<float>(SlotCount)) * 0.35f;

    // T(0~1)을 입구 모서리→중심→출구 모서리 경로 위 점으로. 코너면 중심에서 꺾이고, 직선이면 일직선.
    auto PathPoint = [&](float T) -> FVector
    {
        return (T <= 0.5f)
            ? FMath::Lerp(EntryEdge, CellCenter, T * 2.f)
            : FMath::Lerp(CellCenter, ExitEdge, (T - 0.5f) * 2.f);
    };

    for (int32 i = 0; i < SlotCount; ++i)
    {
        if (!Slots[i].IsValid())
        {
            continue;
        }
        // 시뮬은 이산 슬롯이지만 비주얼은 MoveAccumulator 로 슬롯 간 보간.
        const float T = FMath::Clamp((static_cast<float>(i) + 0.5f + MoveAccumulator) / static_cast<float>(SlotCount), 0.f, 1.f);
        DrawDebugSphere(World, PathPoint(T), SlotRadius, 8, FColor::Yellow, false, -1.f, 0, 1.f);
    }
}
