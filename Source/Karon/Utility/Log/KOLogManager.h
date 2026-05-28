#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"

// ==========================================================
//                         사용 예시
// ==========================================================
// KO_LOG     (GAS,    Log,     TEXT("ASC initialized"));
// KO_LOGS    (GAS,    Ability, Log,     TEXT("GA activated: %s"), *Name);
// KO_LOGS    (GAS,    Attr,    Warning, TEXT("Health 0"));
// KO_LOGS    (Char,   Hero,    Log,     TEXT("Possessed"));
// KO_LOGS    (Char,   Monster, Log,     TEXT("Spawned"));
// KO_LOG_FUNC(Combat, Hit,     Log,     TEXT("Damage: %f"), Dmg);
// KO_LOG_IF  (HP < 20.f, Combat, LowHP, Warning, TEXT("Low HP: %f"), HP);
// KO_SCREEN  (Combat, Hit, FColor::Red, 3.f, TEXT("Damage: %f"), Dmg);

// ==========================================================
//                      Log Categories 
// ==========================================================
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Char,     Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_GAS,      Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Combat,   Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Movement, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Input,    Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_UI,       Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Item,     Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Data,     Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_AI,       Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogKO_Factory,  Log, All); 

// ==========================================================
//                            Enum
// ==========================================================
enum class ELogCategory : uint8
{
	Char,
	GAS,
	Combat,
	Factory, 
	Movement,
	Input,
	UI,
	Item,
	Data,
	AI,
	COUNT
};

enum class ELogGroup : uint8
{
	Character,
	AbilitySystem,
	// [NOTE] All 은 GroupMap 에 등록하지 않고 SetGroupEnabled 에서 코드로 처리.
  //        GroupMap 순회 로직이 추가되더라도 All 이 빠지지 않도록 이 주석으로 명시.
	All
};

struct FLogCategoryEntry
{
	FLogCategoryBase*   Category;
	ELogVerbosity::Type DefaultVerbosity; // 비활성화-> 활성화시 복원할 Verbosity
	ELogVerbosity::Type CurrentVerbosity; // SetVerbosity 가 DefaultVerbosity 를 오염시키지 않도록 분리
};

// ==========================================================
//                         Log Manager
// ==========================================================
class KARON_API FKOLogManager
{
public:
	static FKOLogManager& Get();

	void SetEnabled(ELogCategory Category, bool bEnable);
	bool SetEnabledByName(const FString& CategoryName, bool bEnable);
	void SetGroupEnabled(ELogGroup Group, bool bEnable);
	
	
	void SetVerbosity(ELogCategory Category, ELogVerbosity::Type Verbosity);
	void ToggleAll(bool bEnable);

	FORCEINLINE bool IsEnabled(ELogCategory Category) const
	{
		return bEnabled[static_cast<uint8>(Category)];
	}

	void DumpStatus() const;

private:
	FKOLogManager();

	void Register(
		ELogCategory Category, FLogCategoryBase* LogCategory,
		const FString& Name, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	
	bool bEnabled[static_cast<uint8>(ELogCategory::COUNT)]; 
	FLogCategoryEntry CategoryEntries[static_cast<uint8>(ELogCategory::COUNT)]; 

	
	TMap<FString,      ELogCategory> NameToCategoryMap;
	TMap<ELogCategory, FString>      CategoryToNameMap;
	TMap<ELogGroup,    TArray<ELogCategory>> GroupMap;
};

// ==========================================================
//                       기본 매크로
// ==========================================================

// Sub 없이 카테고리만
#define KO_LOG(Category, Verbosity, Format, ...) \
do { \
if (FKOLogManager::Get().IsEnabled(ELogCategory::Category)) \
	UE_LOG(LogKO_##Category, Verbosity, Format, ##__VA_ARGS__); \
} while(0)

// Sub 카테고리 포함 - [Sub] 접두어 출력
#define KO_LOGS(Category, Sub, Verbosity, Format, ...) \
do { \
if (FKOLogManager::Get().IsEnabled(ELogCategory::Category)) \
	UE_LOG(LogKO_##Category, Verbosity, TEXT("[" #Sub "] ") Format, ##__VA_ARGS__); \
} while(0)

// 함수명 + 라인 포함
#define KO_LOG_FUNC(Category, Sub, Verbosity, Format, ...) \
do { \
if (FKOLogManager::Get().IsEnabled(ELogCategory::Category)) \
	UE_LOG(LogKO_##Category, Verbosity, TEXT("[" #Sub "][%s:%d] ") Format, \
		*FString(__FUNCTION__), __LINE__, ##__VA_ARGS__); \
} while(0)

// 조건부
#define KO_LOG_IF(Condition, Category, Sub, Verbosity, Format, ...) \
do { \
if ((Condition) && FKOLogManager::Get().IsEnabled(ELogCategory::Category)) \
	UE_LOG(LogKO_##Category, Verbosity, TEXT("[" #Sub "] ") Format, ##__VA_ARGS__); \
} while(0)

// 스크린 출력
#define KO_SCREEN(Category, Sub, Color, Duration, Format, ...) \
do { \
if (GEngine && FKOLogManager::Get().IsEnabled(ELogCategory::Category)) \
GEngine->AddOnScreenDebugMessage(\
	(int32)ELogCategory::Category, Duration, Color, \
	FString::Printf(TEXT("[" #Sub "] ") Format, ##__VA_ARGS__)); \
} while(0)
