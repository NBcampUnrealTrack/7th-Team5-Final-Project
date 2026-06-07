// Copyright Karon Team 5. All Rights Reserved.

#include "Building/Conveyor/KOConveyorBelt.h"

#include "Subsystem/KOConveyorSubsystem.h"
#include "Subsystem/KOGridSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Utility/Log/KOLogManager.h"
#include "Components/ActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
static TAutoConsoleVariable<int32> CVarKOConveyorDrawSlots(
    TEXT("ko.Conveyor.DrawSlots"),
    -1,
    TEXT("벨트 슬롯 디버그 스피어. -1: 인스턴스 설정 사용, 0: 강제 off, 1: 강제 on"),
    ECVF_Cheat
);
#endif

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

    // FIntPoint 는 단항 - 연산자가 없어 수동 음수화.
    FIntPoint NegateStep(const FIntPoint& Step)
    {
        return FIntPoint(-Step.X, -Step.Y);
    }
}

AKOConveyorBelt::AKOConveyorBelt()
{
    PrimaryActorTick.bCanEverTick = false; // 서브시스템이 구동.

    // AKOBaseBuilding 은 루트를 만들지 않는다. 전용 씬 루트를 둬서 런타임에 생성하는
    // 아이템 ISM 들의 부착 부모를 보장한다(루트가 없으면 첫 씬 컴포넌트가 루트로 승격됨).
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    // 아이템 비주얼은 메시별 ISM 으로 BeginPlay 시 지연 생성(GetOrCreateISMForMesh).
}

void AKOConveyorBelt::BeginPlay()
{
    Super::BeginPlay();

    SlotCount = FMath::Max(1, SlotCount);
    Slots.SetNum(SlotCount); // 기본값 = 빈 아이템(NAME_None).
    MoveAccumulator = 0.f;

    RecomputePortDirections();
    SetupItemVisual();

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
            InDir  = NegateStep(Side); // 입구 이웃 = MyCell - InDir = MyCell + Side
            OutDir = Forward;          // 출구 이웃 = MyCell + Forward
        }
        else
        {
            InDir  = NegateStep(Forward); // 입구 이웃 = MyCell + Forward
            OutDir = Side;                // 출구 이웃 = MyCell + Side
        }
    }
    else
    {
        // 직선: 흐름 축은 Forward. bStraightReverse 면 정/역 반전(입구=+Forward, 출구=-Forward).
        const FIntPoint Flow = bStraightReverse ? NegateStep(Forward) : Forward;
        InDir  = Flow;
        OutDir = Flow;
    }

    // 디버그/비주얼용: 중심에서 입구/출구 이웃을 향하는 월드 방향.
    EntryDirWorld = GridStepToWorldDir(NegateStep(InDir));
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
        const bool bAuto = TryResolveCornerFlipFromNeighbors(bAutoFlip);
        bCornerFlip = bAuto ? bAutoFlip : bManualFlipFallback;
        KO_LOGS(Factory, Conveyor, Log,
            TEXT("ApplyPlacementFlow: 벨트='%s' Corner → %s (최종 flip=%d, 수동폴백값=%d)"),
            *GetName(),
            bAuto ? TEXT("자동추론 성공") : TEXT("모호/단서없음 → 수동폴백"),
            bCornerFlip ? 1 : 0, bManualFlipFallback ? 1 : 0);
    }
    else
    {
        bool bAutoReverse = false;
        const bool bAuto = TryResolveStraightFlowFromNeighbors(bAutoReverse);
        bStraightReverse = bAuto ? bAutoReverse : false; // 모호하면 배치 방향 그대로.
        KO_LOGS(Factory, Conveyor, Log,
            TEXT("ApplyPlacementFlow: 벨트='%s' Straight → %s (최종 reverse=%d)"),
            *GetName(),
            bAuto ? TEXT("자동추론 성공") : TEXT("모호/단서없음 → 배치방향 유지"),
            bStraightReverse ? 1 : 0);
    }

    RecomputePortDirections();

    KO_LOGS(Factory, Conveyor, Log,
        TEXT("  → 확정: cell=(%d,%d) InDir=(%d,%d) OutDir=(%d,%d) | 입구이웃셀=(%d,%d) 출구이웃셀=(%d,%d)"),
        MyCell.X, MyCell.Y, InDir.X, InDir.Y, OutDir.X, OutDir.Y,
        (MyCell - InDir).X, (MyCell - InDir).Y, (MyCell + OutDir).X, (MyCell + OutDir).Y);
}

int32 AKOConveyorBelt::ClassifyNeighbor(const FIntPoint& MyCellAbs, const FIntPoint& NeighborCell) const
{
    AActor* Actor = GetActorAtCell(NeighborCell);
    if (!Actor)
    {
        return 0;
    }
    if (const AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(Actor))
    {
        if (Belt->OutputsToCell(MyCellAbs))  { return +1; } // 이웃 벨트가 나를 향해 출력 → 업스트림
        if (Belt->InputsFromCell(MyCellAbs)) { return -1; } // 이웃 벨트가 나에게서 입력 → 다운스트림
        return 0;
    }
    // 머신: 단방향 포트만 있으면 방향 확정, 양쪽(Processor) 또는 없음이면 모호.
    const bool bHasSource = ResolveSource(Actor) != nullptr;
    const bool bHasSink   = ResolveSink(Actor)   != nullptr;
    if (bHasSource && !bHasSink) { return +1; } // 출력만 → 나에게 공급
    if (bHasSink && !bHasSource) { return -1; } // 입력만 → 내가 공급
    return 0;
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

    const FIntPoint CellA = Cell + Forward; // LegA(+Forward)
    const FIntPoint CellB = Cell + Side;    // LegB(+Side)
    const int32 RoleA = ClassifyNeighbor(Cell, CellA);
    const int32 RoleB = ClassifyNeighbor(Cell, CellB);

    // 역할 문자열(+1=업스트림/공급, -1=다운스트림/수취, 0=모호).
    auto RoleStr = [](int32 R) { return R > 0 ? TEXT("업스트림(+1)") : (R < 0 ? TEXT("다운스트림(-1)") : TEXT("모호(0)")); };
    KO_LOGS(Factory, Conveyor, Log,
        TEXT("[FlowInfer] 벨트='%s' cell=(%d,%d) | LegA(+Fwd) cell=(%d,%d) actor='%s' → %s | LegB(+Side) cell=(%d,%d) actor='%s' → %s"),
        *GetName(), Cell.X, Cell.Y,
        CellA.X, CellA.Y, *GetNameSafe(GetActorAtCell(CellA)), RoleStr(RoleA),
        CellB.X, CellB.Y, *GetNameSafe(GetActorAtCell(CellB)), RoleStr(RoleB));

    // flip=false: 입구=+Side(LegB), 출구=+Forward(LegA).
    // flip=true : 입구=+Forward(LegA), 출구=+Side(LegB).
    const bool bWantFalse = (RoleB > 0) || (RoleA < 0); // Side 가 업스트림 또는 Forward 가 다운스트림
    const bool bWantTrue  = (RoleA > 0) || (RoleB < 0); // Forward 가 업스트림 또는 Side 가 다운스트림

    if (bWantFalse && !bWantTrue) { OutFlip = false; return true; }
    if (bWantTrue && !bWantFalse) { OutFlip = true;  return true; }
    return false; // 양쪽 충돌 또는 단서 없음 → 수동 폴백.
}

bool AKOConveyorBelt::TryResolveStraightFlowFromNeighbors(bool& OutReverse) const
{
    const UKOGridSubsystem* Grid = GetWorld() ? GetWorld()->GetSubsystem<UKOGridSubsystem>() : nullptr;
    if (!Grid)
    {
        return false;
    }
    const FIntPoint Cell = Grid->WorldToGridPosition(GetActorLocation());

    FVector FwdWorld = GetActorForwardVector().GetSafeNormal2D();
    if (FwdWorld.IsNearlyZero()) { FwdWorld = FVector::ForwardVector; }
    const FIntPoint Forward = WorldDirToGridStep(FwdWorld);

    // 기본(정방향): 입구=뒤(-Forward), 출구=앞(+Forward).
    const FIntPoint FrontCell = Cell + Forward; // 앞(기본 출구쪽)
    const FIntPoint BackCell  = Cell - Forward; // 뒤(기본 입구쪽)
    const int32 RoleFront = ClassifyNeighbor(Cell, FrontCell);
    const int32 RoleBack  = ClassifyNeighbor(Cell, BackCell);

    auto RoleStr = [](int32 R) { return R > 0 ? TEXT("업스트림(+1)") : (R < 0 ? TEXT("다운스트림(-1)") : TEXT("모호(0)")); };
    KO_LOGS(Factory, Conveyor, Log,
        TEXT("[FlowInfer-S] 벨트='%s' cell=(%d,%d) | 앞(+Fwd) cell=(%d,%d) actor='%s' → %s | 뒤(-Fwd) cell=(%d,%d) actor='%s' → %s"),
        *GetName(), Cell.X, Cell.Y,
        FrontCell.X, FrontCell.Y, *GetNameSafe(GetActorAtCell(FrontCell)), RoleStr(RoleFront),
        BackCell.X, BackCell.Y, *GetNameSafe(GetActorAtCell(BackCell)), RoleStr(RoleBack));

    // 정방향 유지: 뒤가 업스트림 또는 앞이 다운스트림.
    // 역방향 반전: 앞이 업스트림 또는 뒤가 다운스트림.
    const bool bWantForward = (RoleBack > 0)  || (RoleFront < 0);
    const bool bWantReverse = (RoleFront > 0) || (RoleBack  < 0);

    if (bWantForward && !bWantReverse) { OutReverse = false; return true; }
    if (bWantReverse && !bWantForward) { OutReverse = true;  return true; }
    return false; // 양쪽 충돌 또는 단서 없음 → 배치 방향 유지.
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

    // 기본 비주얼: ISM 인스턴스 갱신.
    UpdateItemVisual();

    bool bShouldDraw = bDrawSlotsDebug;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // 콘솔 CVar 로 디버그 스피어 전역 강제 on/off 가능(-1 이면 인스턴스 설정 유지).
    const int32 DrawMode = CVarKOConveyorDrawSlots.GetValueOnGameThread();
    if (DrawMode >= 0)
    {
        bShouldDraw = (DrawMode > 0);
    }
#endif
    if (bShouldDraw)
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

FVector AKOConveyorBelt::ComputeSlotWorldPos(float T) const
{
    // 입구 모서리→중심→출구 모서리 경로. 코너면 중심에서 꺾이고, 직선이면 일직선.
    const FVector Center    = GetActorLocation();
    const FVector EntryEdge = Center + EntryDirWorld * (CellSize * 0.5f);
    const FVector ExitEdge  = Center + ExitDirWorld  * (CellSize * 0.5f);
    return (T <= 0.5f)
        ? FMath::Lerp(EntryEdge, Center, T * 2.f)
        : FMath::Lerp(Center, ExitEdge, (T - 0.5f) * 2.f);
}

void AKOConveyorBelt::DrawSlotsDebug() const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector ZBump(0, 0, 30.f);
    const float   SlotRadius = (CellSize / static_cast<float>(SlotCount)) * 0.35f;

    for (int32 i = 0; i < SlotCount; ++i)
    {
        if (!Slots[i].IsValid())
        {
            continue;
        }
        // 시뮬은 이산 슬롯이지만 비주얼은 MoveAccumulator 로 슬롯 간 보간.
        const float T = FMath::Clamp((static_cast<float>(i) + 0.5f + MoveAccumulator) / static_cast<float>(SlotCount), 0.f, 1.f);
        DrawDebugSphere(World, ComputeSlotWorldPos(T) + ZBump, SlotRadius, 8, FColor::Yellow, false, -1.f, 0, 1.f);
    }
}

UStaticMesh* AKOConveyorBelt::GetFallbackMesh()
{
    if (ItemMesh)
    {
        return ItemMesh;
    }
    if (!CachedFallbackCube)
    {
        // 폴백조차 없으면 엔진 기본 큐브(에셋 없어도 동작).
        CachedFallbackCube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    return CachedFallbackCube;
}

UStaticMesh* AKOConveyorBelt::ResolveItemMesh(FName ItemId)
{
    if (const TObjectPtr<UStaticMesh>* Cached = ItemMeshCache.Find(ItemId))
    {
        return *Cached;
    }

    UStaticMesh* Mesh = nullptr;
    if (UKOLoadSubsystem* Load = UKOLoadSubsystem::Get(this))
    {
        Mesh = Load->ResolveItemMesh(ItemId); // DT 의 WorldMesh (미지정/실패 시 nullptr).
    }
    if (!Mesh)
    {
        Mesh = GetFallbackMesh();
    }

    ItemMeshCache.Add(ItemId, Mesh);
    return Mesh;
}

UInstancedStaticMeshComponent* AKOConveyorBelt::GetOrCreateISMForMesh(UStaticMesh* Mesh)
{
    if (!Mesh)
    {
        return nullptr;
    }
    if (const TObjectPtr<UInstancedStaticMeshComponent>* Found = MeshToISM.Find(Mesh))
    {
        return *Found;
    }

    // 런타임 ISM 생성·등록. 인스턴스 transform 은 월드 공간으로 갱신하므로 부착 부모는 정렬 용도.
    UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(this);
    ISM->SetMobility(EComponentMobility::Movable);
    ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ISM->SetCastShadow(false);
    ISM->SetCanEverAffectNavigation(false);
    ISM->RegisterComponent();
    if (USceneComponent* Root = GetRootComponent())
    {
        ISM->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
    }
    ISM->SetStaticMesh(Mesh);

    // 메시 바운드로 균일 스케일 산출(슬롯 간격 비례).
    const float MeshExtent  = FMath::Max(Mesh->GetBounds().BoxExtent.GetMax(), 1.f);
    const float SlotSpacing = CellSize / static_cast<float>(FMath::Max(SlotCount, 1));
    const float Scale = (SlotSpacing * ItemVisualScale) / MeshExtent;

    // 슬롯 수만큼 인스턴스 풀 미리 생성(매 틱 add/remove 회피). 초기엔 스케일 0 으로 숨김.
    const FTransform Hidden(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
    for (int32 i = 0; i < SlotCount; ++i)
    {
        ISM->AddInstance(Hidden);
    }

    MeshToISM.Add(Mesh, ISM);
    MeshToScale.Add(Mesh, Scale);
    return ISM;
}

void AKOConveyorBelt::SetupItemVisual()
{
    // 폴백 메시용 ISM 을 미리 만들어 풀을 준비(미등록/메시 미지정 아이템도 즉시 표시).
    // 개별 아이템 메시용 ISM 은 해당 아이템이 처음 등장할 때 UpdateItemVisual 에서 지연 생성.
    GetOrCreateISMForMesh(GetFallbackMesh());
}

void AKOConveyorBelt::UpdateItemVisual()
{
    const FVector ZBump(0, 0, ItemZOffset);

    // 1) 점유 슬롯의 메시를 먼저 해석해 필요한 ISM 을 보장(신규 메시면 지연 생성).
    for (int32 i = 0; i < SlotCount; ++i)
    {
        if (Slots[i].IsValid())
        {
            GetOrCreateISMForMesh(ResolveItemMesh(Slots[i].ItemId));
        }
    }

    // 2) 모든 메시 ISM 을 순회하며 각 슬롯 인스턴스를 갱신.
    //    슬롯 i 의 아이템 메시 == 이 ISM 의 메시면 경로 위치에 표시, 아니면 스케일 0 으로 숨김.
    for (const TPair<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>>& Pair : MeshToISM)
    {
        UInstancedStaticMeshComponent* ISM = Pair.Value;
        if (!ISM || ISM->GetInstanceCount() < SlotCount)
        {
            continue;
        }
        const float Scale = MeshToScale.FindRef(Pair.Key);

        for (int32 i = 0; i < SlotCount; ++i)
        {
            FTransform Xf;
            const bool bShow = Slots[i].IsValid() && (ResolveItemMesh(Slots[i].ItemId) == Pair.Key);
            if (bShow)
            {
                // 시뮬은 이산이지만 비주얼은 MoveAccumulator 로 슬롯 간 연속 보간.
                const float T = FMath::Clamp((static_cast<float>(i) + 0.5f + MoveAccumulator) / static_cast<float>(SlotCount), 0.f, 1.f);
                Xf.SetLocation(ComputeSlotWorldPos(T) + ZBump);
                Xf.SetScale3D(FVector(Scale));
            }
            else
            {
                Xf.SetScale3D(FVector::ZeroVector); // 빈 슬롯 / 다른 메시 슬롯은 숨김.
            }
            // transform-only 갱신, 마지막에 ISM 당 한 번만 렌더상태 갱신.
            ISM->UpdateInstanceTransform(i, Xf, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ false, /*bTeleport*/ true);
        }

        ISM->MarkRenderStateDirty();
    }
}

int32 AKOConveyorBelt::GetOccupiedSlotCount() const
{
    int32 Count = 0;
    for (const FKOConveyorItem& Slot : Slots)
    {
        if (Slot.IsValid())
        {
            ++Count;
        }
    }
    return Count;
}

FString AKOConveyorBelt::DescribeForDebug() const
{
    const TCHAR* ShapeStr = (Shape == EKOBeltShape::Corner) ? TEXT("Corner") : TEXT("Straight");
    return FString::Printf(
        TEXT("%s flip=%d cell=(%d,%d) in=(%d,%d) out=(%d,%d) items=%d/%d"),
        ShapeStr, bCornerFlip ? 1 : 0,
        MyCell.X, MyCell.Y, InDir.X, InDir.Y, OutDir.X, OutDir.Y,
        GetOccupiedSlotCount(), SlotCount
    );
}
