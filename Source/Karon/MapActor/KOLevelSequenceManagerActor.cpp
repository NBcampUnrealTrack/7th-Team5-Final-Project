// Fill out your copyright notice in the Description page of Project Settings.


#include "KOLevelSequenceManagerActor.h"

#include "Data/KODataTableTypes.h"


// Called when the game starts or when spawned
void AKOLevelSequenceManagerActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (LevelSequenceDataTable)
	{
		const TMap<FName, uint8*>& DM_RowMap = LevelSequenceDataTable->GetRowMap();
		
		for (const auto& Pair : DM_RowMap)
		{
			FName RowName = Pair.Key;
			FKOLevelSequenceRow* RowData = reinterpret_cast<FKOLevelSequenceRow*>(Pair.Value);
			if (RowData)
			{	
				LevelSequenceDataTableMap.Add(RowName, RowData->LevelSequence);
			}
		}
	}
}


