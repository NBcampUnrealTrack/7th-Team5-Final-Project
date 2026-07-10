#include "FKOTelemetryWriter.h"

FKOTelemetryWriter::FKOTelemetryWriter(const FString& InDbPath, TQueue<FKOTelemetryEvent, EQueueMode::Spsc>& InQueue)
	:Queue(InQueue)
{
	DbPath=InDbPath;
	Thread=FRunnableThread::Create(this,TEXT("FKOTelemetryWriter"));
}

FKOTelemetryWriter::~FKOTelemetryWriter()
{
	if (Thread)
	{
		// Stop() 호출 + Run() 종료까지 대기(마지막 FlushBatch 보장)
		Thread->Kill(true); 
		delete Thread;
		Thread = nullptr;
	}
	InsertStmt.Reset();  
	Db.Close();
}

bool FKOTelemetryWriter::Init()
{
	// DB 파일 생성 후 열기
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(DbPath), true);
	if (!Db.Open(*DbPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
		return false;  

	// 스키마 생성
	Db.Execute(TEXT(
		"CREATE TABLE IF NOT EXISTS events ("
		//PK
		"  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  session_id   TEXT NOT NULL,"
		"  game_time    REAL NOT NULL,"
		"  event_type   TEXT NOT NULL,"
		"  enemy_tag    TEXT,"
		"  enemy_level  INTEGER,"
		"  ability_name TEXT,"
		"  value        REAL,"
		"  health_percent_after  REAL,"
		"  pos_x REAL, pos_y REAL, pos_z REAL,"
		"  extra        TEXT"
		");"));
	Db.Execute(TEXT("CREATE INDEX IF NOT EXISTS idx_type    ON events(event_type);"));
	Db.Execute(TEXT("CREATE INDEX IF NOT EXISTS idx_pattern ON events(enemy_tag, ability_name);"));

	// INSERT 문 1회 준비 → FlushBatch에서 Reset()으로 재사용 (매번 SQL 파싱 안 함)
	// 바인딩 이름(@session …)은 FlushBatch의 SetBindingValueByName과 1:1로 맞춘다
	InsertStmt = MakeUnique<FSQLitePreparedStatement>(Db, TEXT(
		"INSERT INTO events"
		"  (session_id, game_time, event_type, enemy_tag, enemy_level,"
		"   ability_name, value, health_percent_after, pos_x, pos_y, pos_z, extra)"
		"VALUES"
		"  (@session, @time, @type, @enemy, @level,"
		"   @ability, @value, @health_percent_after, @x, @y, @z, @extra);"),
		ESQLitePreparedStatementFlags::Persistent);

	return true;
}

uint32 FKOTelemetryWriter::Run()
{
	while (!bStopRequested)
	{
		FlushBatch();
		FPlatformProcess::Sleep(FlushIntervalSec);
	}
	// 남은 큐도 배치
	FlushBatch();   
	return 0;
}

void FKOTelemetryWriter::Stop()
{
	bStopRequested = true;
}

void FKOTelemetryWriter::FlushBatch()
{
	FKOTelemetryEvent E;
	int32 Count = 0;

	Db.Execute(TEXT("BEGIN TRANSACTION;"));
	while (Count < MaxBatch && Queue.Dequeue(E))
	{
		InsertStmt->Reset();
		InsertStmt->SetBindingValueByName(TEXT("@session"), E.SessionId);
		InsertStmt->SetBindingValueByName(TEXT("@time"),    E.GameTime);
		InsertStmt->SetBindingValueByName(TEXT("@type"),    E.EventType.ToString());
		InsertStmt->SetBindingValueByName(TEXT("@enemy"),   E.EnemyTag.ToString());
		InsertStmt->SetBindingValueByName(TEXT("@level"),   E.EnemyLevel);
		InsertStmt->SetBindingValueByName(TEXT("@ability"), E.AbilityName);
		InsertStmt->SetBindingValueByName(TEXT("@value"),   E.Value);
		InsertStmt->SetBindingValueByName(TEXT("@health_percent_after"),E.HealthPercentAfter);
		InsertStmt->SetBindingValueByName(TEXT("@x"), E.Position.X);
		InsertStmt->SetBindingValueByName(TEXT("@y"), E.Position.Y);
		InsertStmt->SetBindingValueByName(TEXT("@z"), E.Position.Z);
		InsertStmt->SetBindingValueByName(TEXT("@extra"), E.Extra);
		InsertStmt->Execute();
		++Count;
	}
	Db.Execute(TEXT("COMMIT;"));
}
