#pragma once
#include "SQLiteDatabase.h"
#include "Data/Type/KOTelemetryType.h"

class FKOTelemetryWriter : public FRunnable
{
public:
	FKOTelemetryWriter(const FString& InDbPath,
					   TQueue<FKOTelemetryEvent, EQueueMode::Spsc>& InQueue);
	virtual ~FKOTelemetryWriter() override;

	virtual bool   Init() override;     
	virtual uint32 Run() override;     
	virtual void   Stop() override;    

private:
	void FlushBatch();

	FString DbPath;
	TQueue<FKOTelemetryEvent, EQueueMode::Spsc>& Queue;   
	FSQLiteDatabase Db;
	TUniquePtr<FSQLitePreparedStatement> InsertStmt;      
	FRunnableThread* Thread = nullptr;
	FThreadSafeBool  bStopRequested = false;
	static constexpr float FlushIntervalSec = 0.5f;
	static constexpr int32 MaxBatch = 512;
	
};
