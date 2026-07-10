#pragma once

class KOTelemetryType
{
public:
	
};


// 워커 스레드로 넘길 값 타입
// 스레드-세이프를 위해 UObjectPtr을 사용하지 않음
struct FKOTelemetryEvent
{
	FString  SessionId;
	double   GameTime = 0.0;
	FName    EventType;
	FName    EnemyTag;
	int32    EnemyLevel = 0;
	FString  AbilityName;
	float	 Value = 0.f;
	float	 HealthPercentAfter=0.f;
	FVector  Position = FVector::ZeroVector;
	FString  Extra;
};