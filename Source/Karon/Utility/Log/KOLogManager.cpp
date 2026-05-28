#include "KOLogManager.h"

DEFINE_LOG_CATEGORY(LogKO_Char);
DEFINE_LOG_CATEGORY(LogKO_GAS);
DEFINE_LOG_CATEGORY(LogKO_Combat);
DEFINE_LOG_CATEGORY(LogKO_Movement);
DEFINE_LOG_CATEGORY(LogKO_Input);
DEFINE_LOG_CATEGORY(LogKO_UI);
DEFINE_LOG_CATEGORY(LogKO_Item);
DEFINE_LOG_CATEGORY(LogKO_Data);
DEFINE_LOG_CATEGORY(LogKO_AI);
DEFINE_LOG_CATEGORY(LogKO_Factory);


FKOLogManager& FKOLogManager::Get()
{
	static FKOLogManager Instance;
	return Instance;
}

FKOLogManager::FKOLogManager()
{
	for (bool& b : bEnabled) { b = true; } 
	FMemory::Memset(CategoryEntries, 0, sizeof(CategoryEntries));

	Register(ELogCategory::Char,     &LogKO_Char,     TEXT("Char"));
	Register(ELogCategory::GAS,      &LogKO_GAS,      TEXT("GAS"));
	Register(ELogCategory::Combat,   &LogKO_Combat,   TEXT("Combat"));
	Register(ELogCategory::Movement, &LogKO_Movement, TEXT("Movement"));
	Register(ELogCategory::Input,    &LogKO_Input,    TEXT("Input"));
	Register(ELogCategory::UI,       &LogKO_UI,       TEXT("UI"));
	Register(ELogCategory::Item,     &LogKO_Item,     TEXT("Item"));
	Register(ELogCategory::Data,     &LogKO_Data,     TEXT("Data"));
	Register(ELogCategory::AI,       &LogKO_AI,       TEXT("AI"));
	Register(ELogCategory::Factory,  &LogKO_Factory,  TEXT("Factory"));

	// 그룹 정의
	GroupMap.Add(
		ELogGroup::Character,
		{ ELogCategory::Char }
	);
	
	GroupMap.Add(
		ELogGroup::AbilitySystem, 
		{ 
			ELogCategory::GAS,
			ELogCategory::Combat,
			ELogCategory::Movement
		}
	);
}

void FKOLogManager::Register(
	ELogCategory Category, 
	FLogCategoryBase* LogCategory, 
	const FString& Name,
	ELogVerbosity::Type Verbosity)
{
	const uint8 Index = static_cast<uint8>(Category);
	CategoryEntries[Index] = { LogCategory, Verbosity, Verbosity };
	NameToCategoryMap.Add(Name, Category);
	CategoryToNameMap.Add(Category, Name);
}


void FKOLogManager::SetEnabled(ELogCategory Category, bool bEnable)
{
	const uint8 Index = static_cast<uint8>(Category);
	bEnabled[Index] = bEnable;

	FLogCategoryEntry& Entry = CategoryEntries[Index];
	Entry.Category->SetVerbosity(
		bEnable ? Entry.CurrentVerbosity  : ELogVerbosity::NoLogging
	);
}

bool FKOLogManager::SetEnabledByName(const FString& CategoryName, bool bEnable)
{
	if (const ELogCategory* Found = NameToCategoryMap.Find(CategoryName))
	{
		SetEnabled(*Found, bEnable);
		return true; 
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[KOLogManager] Unknown category name: %s"), *CategoryName); 
	return false; 
}

void FKOLogManager::SetGroupEnabled(ELogGroup Group, bool bEnable)
{
	if (Group == ELogGroup::All)
	{
		ToggleAll(bEnable);
		return;
	}

	if (const TArray<ELogCategory>* Categories = GroupMap.Find(Group))
	{
		for (ELogCategory Cat : *Categories)
		{
			SetEnabled(Cat, bEnable);
		}
	}
}

void FKOLogManager::SetVerbosity(ELogCategory Category, ELogVerbosity::Type Verbosity)
{
	const uint8 Index = static_cast<uint8>(Category);
	FLogCategoryEntry& Entry = CategoryEntries[Index];
	Entry.CurrentVerbosity  = Verbosity;

	if (bEnabled[Index])
	{
		Entry.Category->SetVerbosity(Verbosity);
	}
}

void FKOLogManager::ToggleAll(bool bEnable)
{
	for (uint8 i = 0; i < static_cast<uint8>(ELogCategory::COUNT); ++i)
	{
		SetEnabled(static_cast<ELogCategory>(i), bEnable);
	}
}

void FKOLogManager::DumpStatus() const
{
	for (uint8 i = 0; i < static_cast<uint8>(ELogCategory::COUNT); ++i)
	{
		const ELogCategory Category = static_cast<ELogCategory>(i);
		const FString* Name = CategoryToNameMap.Find(Category);
		
		UE_LOG(LogTemp, Log, TEXT("[KOLogManager] %-10s : %s"),
			Name ? **Name : TEXT("Unknown"),
			bEnabled[i] ? TEXT("ON") : TEXT("OFF")
		);
	}
}




