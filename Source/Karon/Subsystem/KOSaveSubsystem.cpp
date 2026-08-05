#include "KOSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Game/Save/KOSaveGame.h"
#include "Game/KOPlayerController.h"
#include "Game/KOPlayerState.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Build/KOBuildUIComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "EngineUtils.h"
#include "KOEnemyDataSubsystem.h"
#include "KOGridSubsystem.h"
#include "KOQuestGuideSubsystem.h"
#include "KOUnlockSubsystem.h"
#include "Building/KOBaseBuilding.h"
#include "Building/Conveyor/KOConveyorBelt.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Cluster/KOEnemyCluster.h"
#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01GimmickPillar.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "MapActor/KOItemDropActor.h"
#include "UI/Map/FOW/KOFogManagerSubsystem.h"
#include "HAL/PlatformTime.h"

const FString UKOSaveSubsystem::DefaultSlotName = TEXT("KaronSaveSlot");

UKOSaveSubsystem* UKOSaveSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext || !GEngine)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContext,
		EGetWorldErrorMode::LogAndReturnNull
	);

	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UKOSaveSubsystem>() : nullptr;
}

bool UKOSaveSubsystem::SaveCurrentGameInternal(bool bIgnoreCombatRestriction)
{
	if (!bIgnoreCombatRestriction && !CanSaveOrLoad())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 저장 실패: 전투 중에는 저장할 수 없습니다."));

		// 나중에 UI 메시지 띄우고 싶으면 여기서 토스트/알림 호출
		// ShowSaveLoadBlockedMessage();

		return false;
	}
	
	UWorld* World = GetWorld();
	if (!World) { return false; } 
	
	UKOSaveGame* SaveData = Cast<UKOSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UKOSaveGame::StaticClass())
	);

	if (!SaveData)
	{
		return false;
	}

	AKOPlayerController* PC = GetKOPlayerController();
	if (!PC)
	{
		return false;
	}

	// 플레이어 위치 저장
	if (APawn* Pawn = PC->GetPawn())
	{
		SaveData->bHasPlayerTransform = true;
		SaveData->PlayerTransform = Pawn->GetActorTransform();
	}
	
	// 플레이어 체력 저장
	if (AKOPlayerState* PS = PC->GetPlayerState<AKOPlayerState>())
	{
		SaveData->PlayerStatus.bHasHealth = true;
		SaveData->PlayerStatus.Health = PS->GetHealthForSave();
	}

	// 인벤토리 저장
	if (UKOInventoryComponent* Inventory = GetPlayerInventory(PC))
	{
		SaveData->InventorySlots = Inventory->GetSlots();
	}

	// 장착 무기 저장
	if (UKOEquipmentComponent* Equipment = GetPlayerEquipment(PC))
	{
		const FName WeaponItemId = Equipment->GetCurrentWeaponItemId();

		if (!WeaponItemId.IsNone())
		{
			SaveData->EquippedWeaponItemId = WeaponItemId;
			SaveData->EquippedWeaponSlot = Equipment->GetCurrentWeaponSlot();
		}
		else
		{
			SaveData->EquippedWeaponItemId = NAME_None;
			SaveData->EquippedWeaponSlot = EWeaponSlot::Holster;
		}
		
		// 방어구 저장
		SaveData->EquippedArmorItemIds = Equipment->GetEquippedArmorItemIds();
	}
	
	// 건설 퀵슬롯 저장
    if (UKOBuildUIComponent* BuildUI = GetPlayerBuildUI(PC))
    {
    	SaveData->BuildQuickSlots = BuildUI->GetBuildQuickSlotsForSave();
    }
	
	// 설비 저장
	SaveData->Buildings.Empty();
	for (TActorIterator<AKOBaseBuilding> It(World); It; ++It)
	{
		AKOBaseBuilding* Building = *It;
		if (!Building)
		{
			continue;
		}

		const FName FactoryId = Building->GetFactoryId();
		if (FactoryId.IsNone())
		{
			continue;
		}

		FKOSavedBuilding SavedBuilding;
		SavedBuilding.FactoryId = FactoryId;
		SavedBuilding.Transform = Building->GetActorTransform();
		
		// 그리드 점유 정보 저장
		if (UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>())
		{
			FIntPoint Anchor;
			FIntPoint Size;

			if (GridSub->TryGetOccupiedAreaForActor(Building, Anchor, Size))
			{
				SavedBuilding.GridAnchor = Anchor;
				SavedBuilding.GridSize = Size;
			}
		}
		
		// input, output, 제작 진행도, 레시피 저장
		if (UKOFactoryProcessorComponent* Processor = Building->FindComponentByClass<UKOFactoryProcessorComponent>())
		{
			SavedBuilding.ProcessorState.SelectedRecipeId = Processor->GetSelectedRecipe();
			SavedBuilding.ProcessorState.InputBuffer = Processor->GetInputBuffer();
			SavedBuilding.ProcessorState.OutputBuffer = Processor->GetOutputBuffer();
			SavedBuilding.ProcessorState.ActiveRecipeId = Processor->GetActiveRecipeId();
			SavedBuilding.ProcessorState.CurrentCycleSeconds = Processor->GetCurrentCycleSecondsForSave();
			SavedBuilding.ProcessorState.Progress = Processor->GetProgressSecondsForSave();
		}

		// 압력 상태 저장
		if (UKOEnergyProducerComponent* Producer = Building->FindComponentByClass<UKOEnergyProducerComponent>())
		{
			SavedBuilding.ProducerState.FuelItemId = Producer->GetFuelItemId();
			SavedBuilding.ProducerState.FuelCount = Producer->GetFuelCount();
			SavedBuilding.ProducerState.FuelDebt = Producer->GetFuelDebtForSave();
			SavedBuilding.ProducerState.bHasActiveFuel = Producer->HasActiveFuelForSave();
		}
		
		// 컨베이어 상태 저장
		if (AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(Building))
		{
			Belt->GetConveyorStateForSave(
				SavedBuilding.ConveyorState.SlotItemIds,
				SavedBuilding.ConveyorState.MoveAccumulator,
				SavedBuilding.ConveyorState.bCornerFlip,
				SavedBuilding.ConveyorState.bStraightReverse
			);
			
			// Output 포트 바인딩 저장
			FIntPoint BoundMachineGridAnchor;
			int32 BoundOutputPortIndex = INDEX_NONE;
			FName BoundOutputItemId = NAME_None;
			bool bHasSelectedOutputPort = false;

			if (Belt->GetOutputPortBindingForSave(
				BoundMachineGridAnchor,
				BoundOutputPortIndex,
				BoundOutputItemId,
				bHasSelectedOutputPort
			))
			{
				SavedBuilding.ConveyorState.bHasOutputBinding = true;
				SavedBuilding.ConveyorState.BoundOutputMachineGridAnchor = BoundMachineGridAnchor;
				SavedBuilding.ConveyorState.BoundOutputPortIndex = BoundOutputPortIndex;
				SavedBuilding.ConveyorState.BoundOutputItemId = BoundOutputItemId;
				SavedBuilding.ConveyorState.bHasSelectedOutputPort = bHasSelectedOutputPort;
			}
		}

		SaveData->Buildings.Add(SavedBuilding);
	}
	
	// 해금 태그 저장
	if (UKOUnlockSubsystem* UnlockSubsystem = UKOUnlockSubsystem::Get(this))
	{
		SaveData->OwnedUnlockTags = UnlockSubsystem->GetOwnedUnlockTags();
	}
	
	// 몬스터 상태 저장
	SaveData->Monsters.Empty();
	for (TActorIterator<AKOBaseEnemy> It(World); It; ++It)
	{
		AKOBaseEnemy* Monster = *It;
		if (!Monster || Monster->GetMonsterSaveId().IsNone() || Monster->IsDeadForSave())
		{
			continue;
		}

		FKOSavedMonster SavedMonster;
		SavedMonster.MonsterSaveId = Monster->GetMonsterSaveId();
		SavedMonster.ClusterSaveId = Monster->GetClusterSaveIdForSave();
		SavedMonster.MonsterClassPath = FSoftClassPath(Monster->GetClass());
		SavedMonster.Transform = Monster->GetActorTransform();
		SavedMonster.Level = Monster->GetEnemyLevelForSave();

		SaveData->Monsters.Add(SavedMonster);
	}

	SaveData->DeadMonsterIds = DeadMonsterIds.Array();
	
	// 보스 상태 저장
	SaveData->Bosses.Empty();
	for (TActorIterator<AKOBossBase> It(World); It; ++It)
	{
		AKOBossBase* Boss = *It;
		if (!Boss)
		{
			continue;
		}

		const FName BossSaveId = Boss->GetBossSaveId();
		if (BossSaveId.IsNone())
		{
			continue;
		}

		FKOSavedBoss SavedBoss;
		SavedBoss.BossSaveId = BossSaveId;
		SavedBoss.BossClassPath = FSoftClassPath(Boss->GetClass());
		SavedBoss.Transform = Boss->GetActorTransform();
		SavedBoss.bWasAlive = !Boss->IsDeadForSave();

		SaveData->Bosses.Add(SavedBoss);
	}
	
	// 보스 기믹 기둥 상태 저장
	SaveData->BossPillars.Empty();

	for (TActorIterator<AKOBossCH01GimmickPillar> It(World); It; ++It)
	{
		AKOBossCH01GimmickPillar* Pillar = *It;
		if (!Pillar)
		{
			continue;
		}

		const FName PillarSaveId = Pillar->GetPillarSaveId();

		if (PillarSaveId.IsNone())
		{
			continue;
		}

		FKOSavedBossPillar SavedPillar;
		SavedPillar.PillarSaveId = PillarSaveId;
		SavedPillar.bWasBroken = Pillar->IsBrokenForSave();

		SaveData->BossPillars.Add(SavedPillar);
	}
	
	// 스킬 저장
	if (UKOSkillSubsystem* SkillSubsystem = GetPlayerSkillSubsystem(PC))
	{
		SkillSubsystem->GetSkillStateForSave(SaveData->SkillState.UnlockedSkillIds);
		SkillSubsystem->GetSkillQuickSlotsForSave(SaveData->SkillState.SkillQuickSlots);
	}
	
	// 퀘스트 진행도 저장
	if (UKOQuestGuideSubsystem* QuestGuide = GetQuestGuideSubsystem())
	{
		QuestGuide->GetQuestGuideStateForSave(
			SaveData->QuestGuideState.CurrentQuestId,
			SaveData->QuestGuideState.CurrentProgress,
			SaveData->QuestGuideState.CompletedQuestIds
		);
	}
	
	// 채집물 저장
	SaveData->CollectedItemDropIds = CollectedItemDropIds.Array();
	// 파괴된 액터 저장
	SaveData->DestroyedActorIds=DestroyedActorIds.Array();
	// 장비 제작 이력 저장
	SaveData->CraftedEquipmentIds = CraftedEquipmentIds.Array();
	
	// 안개 저장
	if (UKOFogManagerSubsystem* FogSubsystem = UKOFogManagerSubsystem::Get(PC))
	{
		const bool bFogSaved = FogSubsystem->GetFogStateForSave(
				SaveData->FogState.ExploredPixels,
				SaveData->FogState.SizeX,
				SaveData->FogState.SizeY
			);

		if (!bFogSaved)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 안개 상태를 저장하지 못했습니다."));
		}
	}
	
	// 활성화된 화톳불 저장
	if (UKOTeleportSubsystem* TeleportSubsystem = GetPlayerTeleportSubsystem(PC))
	{
		SaveData->ActivatedBonfireIds = TeleportSubsystem->GetActivatedBonfireSet().Array();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 저장 성공"));
	return UGameplayStatics::SaveGameToSlot(SaveData, DefaultSlotName, DefaultUserIndex);
}

AKOPlayerController* UKOSaveSubsystem::GetKOPlayerController() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return Cast<AKOPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
}

UKOInventoryComponent* UKOSaveSubsystem::GetPlayerInventory(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	// 네 구조에서는 InventoryComponent가 PlayerController에 붙어 있음
	return PC->FindComponentByClass<UKOInventoryComponent>();
}

UKOEquipmentComponent* UKOSaveSubsystem::GetPlayerEquipment(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	// EquipmentComponent는 캐릭터/Pawn에 붙어 있음
	return Pawn->FindComponentByClass<UKOEquipmentComponent>();
}

UKOBuildUIComponent* UKOSaveSubsystem::GetPlayerBuildUI(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	return PC->FindComponentByClass<UKOBuildUIComponent>();
}

UKOSkillSubsystem* UKOSaveSubsystem::GetPlayerSkillSubsystem(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<UKOSkillSubsystem>();
	}

	return UKOSkillSubsystem::Get(PC);
}

UKOQuestGuideSubsystem* UKOSaveSubsystem::GetQuestGuideSubsystem() const
{
	return UKOQuestGuideSubsystem::Get(this);
}

UKOTeleportSubsystem* UKOSaveSubsystem::GetPlayerTeleportSubsystem(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UKOTeleportSubsystem>();
}

bool UKOSaveSubsystem::SaveCurrentGame()
{
	return SaveCurrentGameInternal(false);
}

bool UKOSaveSubsystem::SaveCheckpoint()
{
	return SaveCurrentGameInternal(true);
}

bool UKOSaveSubsystem::LoadCurrentGame()
{
	if (!CanSaveOrLoad())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 로드 실패: 전투 중에는 로드할 수 없습니다."));

		return false;
	}
	
	if (!DoesSaveExist())
	{
		return false;
	}

	UKOSaveGame* SaveData = Cast<UKOSaveGame>(
		UGameplayStatics::LoadGameFromSlot(DefaultSlotName, DefaultUserIndex)
	);

	if (!SaveData)
	{
		return false;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AKOPlayerController* PC = GetKOPlayerController();
	if (!PC)
	{
		return false;
	}

	// 플레이어 위치 로드
	if (SaveData->bHasPlayerTransform)
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorTransform(SaveData->PlayerTransform);
		}
	}
	
	// 플레이어 체력 로드
	if (SaveData->PlayerStatus.bHasHealth)
	{
		if (AKOPlayerState* PS = PC->GetPlayerState<AKOPlayerState>())
		{
			PS->LoadHealthFromSave(SaveData->PlayerStatus.Health);
		}
	}
	
	// 사망 상태 복구
	if (APawn* Pawn = PC->GetPawn())
	{
		if (AKOCharacterBase* Character = Cast<AKOCharacterBase>(Pawn))
		{
			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();

			const bool bHasDeadTag = ASC && ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead);

			if (Character->bIsDead || bHasDeadTag)
			{
				Character->RestoreAliveStateFromLoad();

				if (AKOHeroCharacter* HeroCharacter = Cast<AKOHeroCharacter>(Character))
				{
					HeroCharacter->ReactivateOverClockAfterRespawn();
				}
			}
		}
	}

	// 인벤토리 로드
	if (UKOInventoryComponent* Inventory = GetPlayerInventory(PC))
	{
		Inventory->LoadSlotsFromSave(SaveData->InventorySlots);
	}

	// 장착 무기 로드
	if (UKOEquipmentComponent* Equipment = GetPlayerEquipment(PC))
	{
		if (SaveData->EquippedWeaponItemId.IsNone())
		{
			Equipment->UnequipWeapon();
		}
		else
		{
			UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(PC);
			if (!LoadSub)
			{
				Equipment->UnequipWeapon();
			}
			else if (UKOWeaponDefinition* WeaponDef =
				LoadSub->ResolveWeaponDefinitionByItemId(SaveData->EquippedWeaponItemId))
			{
				Equipment->RestoreWeaponFromSave(
				SaveData->EquippedWeaponItemId,
				WeaponDef,
				SaveData->EquippedWeaponSlot
			);
			}
			else
			{
				Equipment->UnequipWeapon();
			}
		}
		
		// 방어구 로드
		Equipment->LoadArmorFromSave(SaveData->EquippedArmorItemIds);
	}
	
	// 건설 퀵슬롯 로드
    if (UKOBuildUIComponent* BuildUI = GetPlayerBuildUI(PC))
    {
    	BuildUI->CloseBuildMenu();
    
    	BuildUI->LoadBuildQuickSlotsFromSave(SaveData->BuildQuickSlots);
    }
	
	// 설비 로드
	UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>();
		
	// 기존 설비 제거
	for (TActorIterator<AKOBaseBuilding> It(World); It; ++It)
	{
		AKOBaseBuilding* Building = *It;
		if (!Building)
		{
			continue;
		}
		
		if (GridSub)
		{
			GridSub->FreeAreaByActor(Building);
		}

		Building->Destroy();
	}

	// 저장된 설비 다시 스폰
	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (LoadSub)
	{
		TArray<AKOBaseBuilding*> SpawnedBuildings;
		TArray<FKOSavedBuilding> SpawnedBuildingSaveData;
		
		for (const FKOSavedBuilding& SavedBuilding : SaveData->Buildings)
		{
			if (SavedBuilding.FactoryId.IsNone())
			{
				continue;
			}

			UClass* BuildingClass = LoadSub->ResolveBuildingClass(SavedBuilding.FactoryId);

			if (!BuildingClass)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[SaveLoad] BuildingClass 로드 실패: FactoryId=%s"),
					*SavedBuilding.FactoryId.ToString()
				);
				continue;
			}

			AKOBaseBuilding* NewBuilding = World->SpawnActor<AKOBaseBuilding>(
				BuildingClass,
				SavedBuilding.Transform
			);
			
			if (!NewBuilding)
			{
				continue;
			}
			
			NewBuilding->InitializeBuildingData(SavedBuilding.FactoryId);
			
			// 그리드 점유 복원
			if (GridSub)
			{
				GridSub->OccupyArea(SavedBuilding.GridAnchor, SavedBuilding.GridSize, NewBuilding);
			}

			// 설비 내부 상태 복원
			if (UKOFactoryProcessorComponent* Processor =
					NewBuilding->FindComponentByClass<UKOFactoryProcessorComponent>())
			{
				Processor->LoadProcessorStateFromSave(
					SavedBuilding.ProcessorState.SelectedRecipeId,
					SavedBuilding.ProcessorState.InputBuffer,
					SavedBuilding.ProcessorState.OutputBuffer,
					SavedBuilding.ProcessorState.ActiveRecipeId,
					SavedBuilding.ProcessorState.CurrentCycleSeconds,
					SavedBuilding.ProcessorState.Progress
				);
			}

			if (UKOEnergyProducerComponent* Producer =
					NewBuilding->FindComponentByClass<UKOEnergyProducerComponent>())
			{
				Producer->LoadFuelFromSave(
					SavedBuilding.ProducerState.FuelItemId,
					SavedBuilding.ProducerState.FuelCount,
					SavedBuilding.ProducerState.FuelDebt,
					SavedBuilding.ProducerState.bHasActiveFuel
				);
			}
			
			// 컨베이어 이동 아이템 / 방향 복원
			if (AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(NewBuilding))
			{
				Belt->LoadConveyorStateFromSave(
					SavedBuilding.ConveyorState.SlotItemIds,
					SavedBuilding.ConveyorState.MoveAccumulator,
					SavedBuilding.ConveyorState.bCornerFlip,
					SavedBuilding.ConveyorState.bStraightReverse
				);
			}
			
			SpawnedBuildings.Add(NewBuilding);
			SpawnedBuildingSaveData.Add(SavedBuilding);
		}
		
		// 모든 건물이 스폰되고 그리드 점유가 끝난 뒤 Output 포트 바인딩 복원
		for (int32 i = 0; i < SpawnedBuildings.Num(); ++i)
		{
			AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(SpawnedBuildings[i]);
			if (!Belt)
			{
				continue;
			}

			const FKOSavedConveyorState& ConveyorState = SpawnedBuildingSaveData[i].ConveyorState;

			if (!ConveyorState.bHasOutputBinding)
			{
				continue;
			}

			if (!GridSub)
			{
				continue;
			}

			AActor* MachineActor = GridSub->GetOccupyingActorAt(ConveyorState.BoundOutputMachineGridAnchor);

			AKOBaseBuilding* BoundMachine = Cast<AKOBaseBuilding>(MachineActor);
			if (!BoundMachine)
			{
				continue;
			}

			Belt->LoadOutputPortBindingFromSave(
				BoundMachine,
				ConveyorState.BoundOutputPortIndex,
				ConveyorState.BoundOutputItemId,
				ConveyorState.bHasSelectedOutputPort
			);
		}
	}
	
	// 장비 제작 이력 로드
	CraftedEquipmentIds.Reset();

	for (const FName& EquipmentId : SaveData->CraftedEquipmentIds)
	{
		if (!EquipmentId.IsNone())
		{
			CraftedEquipmentIds.Add(EquipmentId);
		}
	}
	
	// 해금 태그 로드
	if (UKOUnlockSubsystem* UnlockSubsystem = UKOUnlockSubsystem::Get(this))
	{
		UnlockSubsystem->RestoreUnlockTags(SaveData->OwnedUnlockTags);
	}
	
	// 몬스터 상태 로드
	DeadMonsterIds.Reset();
	for (const FName& MonsterSaveId : SaveData->DeadMonsterIds)
	{
		if (!MonsterSaveId.IsNone())
		{
			DeadMonsterIds.Add(MonsterSaveId);
		}
	}

	// 현재 몬스터 제거
	for (TActorIterator<AKOBaseEnemy> It(World); It; ++It)
	{
		if (AKOBaseEnemy* Monster = *It)
		{
			if (!Monster->GetMonsterSaveId().IsNone())
			{
				Monster->Destroy();
			}
		}
	}

	// 클러스터 초기화
	for (TActorIterator<AKOEnemyCluster> It(World); It; ++It)
	{
		if (AKOEnemyCluster* Cluster = *It)
		{
			Cluster->ResetClusterForLoad();
		}
	}
	
	// 저장된 몬스터 다시 스폰
	UKOEnemyDataSubsystem* EnemyDataSubsystem = UKOEnemyDataSubsystem::Get(this);
	for (const FKOSavedMonster& SavedMonster : SaveData->Monsters)
	{
		if (SavedMonster.MonsterSaveId.IsNone())
		{
			continue;
		}

		UClass* MonsterClass = SavedMonster.MonsterClassPath.TryLoadClass<AKOBaseEnemy>();
		if (!MonsterClass)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[SaveLoad] MonsterClass 로드 실패: MonsterSaveId=%s"),
				*SavedMonster.MonsterSaveId.ToString()
			);
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AKOBaseEnemy* NewMonster = World->SpawnActor<AKOBaseEnemy>(
			MonsterClass,
			SavedMonster.Transform,
			SpawnParams
		);

		if (!NewMonster)
		{
			continue;
		}

		NewMonster->SetMonsterSaveInfoForLoad(SavedMonster.ClusterSaveId, SavedMonster.MonsterSaveId);

		if (EnemyDataSubsystem)
		{
			NewMonster->SetupEnemy(EnemyDataSubsystem, SavedMonster.Level);
		}

		NewMonster->RestoreMonsterFromSave(SavedMonster.Transform);

		for (TActorIterator<AKOEnemyCluster> It(World); It; ++It)
		{
			AKOEnemyCluster* Cluster = *It;
			if (!Cluster)
			{
				continue;
			}

			if (Cluster->GetClusterSaveId() == SavedMonster.ClusterSaveId)
			{
				Cluster->RegisterSpawnedEnemyForLoad(NewMonster);
				break;
			}
		}
	}
	
	for (TActorIterator<AKOEnemyCluster> It(World); It; ++It)
	{
		AKOEnemyCluster* Cluster = *It;
		if (!Cluster)
		{
			continue;
		}

		if (Cluster->GetClusterSaveId().IsNone())
		{
			continue;
		}

		if (Cluster->GetSpawnedEnemiesCountForLoad() <= 0)
		{
			Cluster->ScheduleRespawnForLoad();
		}
	}
	
	// 보스 상태 로드
	for (const FKOSavedBoss& SavedBoss : SaveData->Bosses)
	{
		if (SavedBoss.BossSaveId.IsNone())
		{
			continue;
		}

		AKOBossBase* TargetBoss = nullptr;

		for (TActorIterator<AKOBossBase> It(World); It; ++It)
		{
			AKOBossBase* Boss = *It;
			if (!Boss)
			{
				continue;
			}

			if (Boss->GetBossSaveId() == SavedBoss.BossSaveId)
			{
				TargetBoss = Boss;
				break;
			}
		}

		// 보스가 Destroy되어 월드에 없으면 다시 스폰
		if (!TargetBoss)
		{
			UClass* BossClass = SavedBoss.BossClassPath.TryLoadClass<AKOBossBase>();
			if (!BossClass)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[SaveLoad] BossClass 로드 실패: BossSaveId=%s"),
					*SavedBoss.BossSaveId.ToString()
				);
				continue;
			}

			TargetBoss = World->SpawnActor<AKOBossBase>(BossClass, SavedBoss.Transform);

			if (!TargetBoss)
			{
				continue;
			}

			TargetBoss->SetBossSaveIdForLoad(SavedBoss.BossSaveId);
		}

		TargetBoss->RestoreBossFromSave(SavedBoss.Transform, SavedBoss.bWasAlive);
	}
	
	// 보스 기믹 기둥 로드
	TMap<FName, bool> SavedPillarStateMap;

	for (const FKOSavedBossPillar& SavedPillar : SaveData->BossPillars)
	{
		if (SavedPillar.PillarSaveId.IsNone())
		{
			continue;
		}

		SavedPillarStateMap.Add(SavedPillar.PillarSaveId, SavedPillar.bWasBroken);
	}

	for (TActorIterator<AKOBossCH01GimmickPillar> It(World); It; ++It)
	{
		AKOBossCH01GimmickPillar* Pillar = *It;
		if (!Pillar)
		{
			continue;
		}

		const FName PillarSaveId = Pillar->GetPillarSaveId();

		if (PillarSaveId.IsNone())
		{
			continue;
		}

		const bool* SavedBrokenState = SavedPillarStateMap.Find(PillarSaveId);

		if (SavedBrokenState)
		{
			Pillar->RestoreFromSave(*SavedBrokenState);
		}
		else
		{
			Pillar->RestoreFromSave(false);
		}
	}
	
	// 스킬 로드
	if (UKOSkillSubsystem* SkillSubsystem = GetPlayerSkillSubsystem(PC))
	{
		SkillSubsystem->LoadSkillStateFromSave(SaveData->SkillState.UnlockedSkillIds);
		SkillSubsystem->LoadSkillQuickSlotsFromSave(SaveData->SkillState.SkillQuickSlots);
	}
	
	// 퀘스트 진행도 로드
	if (UKOQuestGuideSubsystem* QuestGuide = GetQuestGuideSubsystem())
	{
		QuestGuide->LoadQuestGuideStateFromSave(
			SaveData->QuestGuideState.CurrentQuestId,
			SaveData->QuestGuideState.CurrentProgress,
			SaveData->QuestGuideState.CompletedQuestIds
		);
	}
	
	// 파괴된 액터 컨테이너 로드
	DestroyedActorIds.Reset();

	for (const FName& DestroyedId : SaveData->DestroyedActorIds)
	{
		if (!DestroyedId.IsNone())
		{
			DestroyedActorIds.Add(DestroyedId);
		}
	}
	
	// 채집물 로드
	CollectedItemDropIds.Reset();

	for (const FName& DropSaveId : SaveData->CollectedItemDropIds)
	{
		if (!DropSaveId.IsNone())
		{
			CollectedItemDropIds.Add(DropSaveId);
		}
	}
	
	for (TActorIterator<AKOItemDropActor> It(World); It; ++It)
	{
		AKOItemDropActor* ItemDrop = *It;
		if (!ItemDrop)
		{
			continue;
		}

		const FName DropSaveId = ItemDrop->GetDropSaveId();

		if (!DropSaveId.IsNone() && CollectedItemDropIds.Contains(DropSaveId))
		{
			ItemDrop->ApplyCollectedFromSave();
		}
		else
		{
			ItemDrop->ApplyAvailableFromSave();
		}
	}
	
	// 안개 로드
	if (UKOFogManagerSubsystem* FogSubsystem = UKOFogManagerSubsystem::Get(PC))
	{
		FogSubsystem->LoadFogStateFromSave(
			SaveData->FogState.ExploredPixels,
			SaveData->FogState.SizeX,
			SaveData->FogState.SizeY
		);
	}
	
	// 활성화된 화톳불 로드
	if (UKOTeleportSubsystem* TeleportSubsystem = GetPlayerTeleportSubsystem(PC))
	{
		TSet<FName> ActivatedBonfires;

		for (const FName& BonfireId : SaveData->ActivatedBonfireIds)
		{
			if (!BonfireId.IsNone())
			{
				ActivatedBonfires.Add(BonfireId);
			}
		}

		TeleportSubsystem->SetActivatedBonfireSet(ActivatedBonfires);
	}

	UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 로드 성공"));
	return true;
}

bool UKOSaveSubsystem::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(DefaultSlotName, DefaultUserIndex);
}

bool UKOSaveSubsystem::DeleteSave()
{
	bLobbyLoadRequested = false;
	
	if (!DoesSaveExist())
	{
		return false;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(DefaultSlotName, DefaultUserIndex);

	if (bDeleted)
	{
		CollectedItemDropIds.Reset();
		DeadMonsterIds.Reset();
		DestroyedActorIds.Reset();
		CraftedEquipmentIds.Reset();
	}

	return bDeleted;
}

void UKOSaveSubsystem::RequestLobbyLoad()
{
	bLobbyLoadRequested = true;
}

bool UKOSaveSubsystem::ConsumeLobbyLoadRequest()
{
	const bool bRequested = bLobbyLoadRequested;
	bLobbyLoadRequested = false;

	return bRequested;
}

void UKOSaveSubsystem::NotifyActorTargetingPlayer(AActor* SourceActor)
{
	if (!IsValid(SourceActor))
	{
		return;
	}

	ActorsTargetingPlayer.Add(SourceActor);

	// 다시 전투가 시작됐으므로 기존 해제 예정 시간 초기화
	CombatUnlockRealTimeSeconds = 0.0;
}

void UKOSaveSubsystem::NotifyActorStoppedTargetingPlayer(AActor* SourceActor)
{
	if (IsValid(SourceActor))
	{
		ActorsTargetingPlayer.Remove(SourceActor);
	}

	// 죽었거나 Destroy된 액터 정리
	for (auto It = ActorsTargetingPlayer.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	if (ActorsTargetingPlayer.Num() > 0)
	{
		return;
	}

	// 게임 일시정지와 무관한 실제 시간 기준으로 해제 시점 설정
	CombatUnlockRealTimeSeconds = FPlatformTime::Seconds() + static_cast<double>(SaveLoadUnlockDelayAfterCombat);
}

bool UKOSaveSubsystem::CanSaveOrLoad() const
{
	// 현재 플레이어를 타겟으로 삼는 적이 있으면 불가능
	if (ActorsTargetingPlayer.Num() > 0)
	{
		return false;
	}

	// 전투 종료 대기시간이 설정되지 않았다면 가능
	if (CombatUnlockRealTimeSeconds <= 0.0)
	{
		return true;
	}
	const double CurrentRealTime = FPlatformTime::Seconds();
	const double RemainingTime = FMath::Max(0.0, CombatUnlockRealTimeSeconds - CurrentRealTime);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SaveLoad] 전투 종료까지 %.2f초"),
		RemainingTime
	);

	// 실제 시간이 해제 예정 시간을 지났는지 검사
	return FPlatformTime::Seconds() >= CombatUnlockRealTimeSeconds;
}

void UKOSaveSubsystem::ForceEndCombat()
{
	ActorsTargetingPlayer.Reset();
	CombatUnlockRealTimeSeconds = 0.0;

	UE_LOG(LogTemp, Warning, TEXT("[SaveLoad] 플레이어 사망으로 전투 상태 강제 종료"));
}

void UKOSaveSubsystem::MarkItemDropCollected(FName DropSaveId)
{
	if (DropSaveId.IsNone())
	{
		return;
	}

	CollectedItemDropIds.Add(DropSaveId);
}

void UKOSaveSubsystem::MarkMonsterDead(FName MonsterSaveId)
{
	if (MonsterSaveId.IsNone())
	{
		return;
	}

	DeadMonsterIds.Add(MonsterSaveId);
}

void UKOSaveSubsystem::MarkDestoryedActor(FName ActorSaveId)
{
	if (ActorSaveId.IsNone())
	{
		return;
	}

	DestroyedActorIds.Add(ActorSaveId);
}

bool UKOSaveSubsystem::CheckIsDestroyedActor(FName ActorSaveId)
{
	return DestroyedActorIds.Contains(ActorSaveId);
}

void UKOSaveSubsystem::MarkEquipmentCrafted(FName EquipmentId)
{
	if (EquipmentId.IsNone())
	{
		return;
	}

	CraftedEquipmentIds.Add(EquipmentId);
}

bool UKOSaveSubsystem::HasCraftedEquipment(FName EquipmentId) const
{
	if (EquipmentId.IsNone())
	{
		return false;
	}

	return CraftedEquipmentIds.Contains(EquipmentId);
}
