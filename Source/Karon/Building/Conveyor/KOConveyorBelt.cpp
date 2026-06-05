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
    ForwardWorld = GetActorForwardVector().GetSafeNormal2D();
    if (ForwardWorld.IsNearlyZero())
    {
        ForwardWorld = FVector::ForwardVector;
    }

    const FIntPoint Forward = WorldDirToGridStep(ForwardWorld);
    // M1 직선 벨트: 입구/출구 모두 forward 축.
    InDir  = Forward;
    OutDir = Forward;

    if (const UKOGridSubsystem* Grid = GetWorld() ? GetWorld()->GetSubsystem<UKOGridSubsystem>() : nullptr)
    {
        MyCell   = Grid->WorldToGridPosition(GetActorLocation());
        CellSize = Grid->GetCellSize();
    }
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

    const FVector CellCenter = GetActorLocation();
    const FVector EntryEdge  = CellCenter - ForwardWorld * (CellSize * 0.5f) + FVector(0, 0, 30.f);
    const float   SlotSpacing = CellSize / static_cast<float>(SlotCount);

    for (int32 i = 0; i < SlotCount; ++i)
    {
        if (!Slots[i].IsValid())
        {
            continue;
        }
        const float Along = (static_cast<float>(i) + 0.5f + MoveAccumulator) * SlotSpacing;
        const FVector Pos = EntryEdge + ForwardWorld * Along;
        DrawDebugSphere(World, Pos, SlotSpacing * 0.35f, 8, FColor::Yellow, false, -1.f, 0, 1.f);
    }
}
