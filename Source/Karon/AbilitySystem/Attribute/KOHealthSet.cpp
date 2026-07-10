#include "KOHealthSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Projectile/KOEnemyProjectileActor.h"
#include "Utility/Messaging/KOMessageTypes.h"


UKOHealthSet::UKOHealthSet()
{
	// MetaData는 x 
	InitDamage(0.f);
	InitHealing(0.f);
}

// Base 값 변경 전 - Clamp 만 
void UKOHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());

	if (Attribute == GetMaxHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

// Current값 변경 전 - 클램핑 + Max변경 시 비율 조정
void UKOHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	
	if (Attribute == GetMaxHealthAttribute())
		AdjustCurrentForMaxChange(
			Health, MaxHealth,
			NewValue,
			GetHealthAttribute()
		);	
}

// Base 영구 변경 후 - 레벨업/장비/세이브
void UKOHealthSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetHealthAttribute())
		OnHealthBaseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
		OnMaxHealthBaseChanged.Broadcast(OldValue, NewValue);
}

// Current값 변경 후 - UI/이벤트
void UKOHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	// 메타 데이터에 경우 처리 x 
	if (Attribute == GetDamageAttribute() || Attribute == GetHealingAttribute()) return;
	
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetHealthAttribute())
		OnHealthChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxHealthAttribute())
		OnMaxHealthChanged.Broadcast(OldValue, NewValue);
}

void UKOHealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// Handle Damage 
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		HandleDamage(Data);
	}
	
	// Handle Heal
	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		HandleHealing(Data);
	}
}

void UKOHealthSet::HandleDamage(const FGameplayEffectModCallbackData& Data)
{
	FKOEffectContext Context = CacheEffectContext(Data);
		
	UAbilitySystemComponent* ASC = Context.TargetASC; 
	if (!ASC) return; 
	
	float DamageAmount = GetDamage();
	
	// 무적이면 데미지 0 
	if (ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Invincible))
	{
		DamageAmount = 0.f;
	}
		
	float NewHealth = FMath::Clamp(
		GetHealth() - DamageAmount, 
		0.f, 
		GetMaxHealth()
	);
		
	SetHealth(NewHealth);
	SetDamage(0.f); // 메타 데이터 초기화 
		
	// 사망 처리 
	if (NewHealth <= 0.f)
	{
		HandleDeath(Data); 
	}
	
	
	const AActor* InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();
	if (const AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(InstigatorActor))
	{
		const float MaxHP = GetMaxHealth();
		const float HealthPctAfter = (MaxHP > 0.f) ? (NewHealth / MaxHP) : 0.f;
		
		//Telemetry로 전송
		FKOTelemetryCombatMessage CombatMessage;
		const AKOEnemyProjectileActor* ProjectileActor =Cast<AKOEnemyProjectileActor>(Data.EffectSpec.GetContext().GetEffectCauser());
		if (ProjectileActor)
		{
			CombatMessage.EnemyTag           = ProjectileActor->GetProjectileTag().GetTagName();
		}
		else
		{
			CombatMessage.EnemyTag           = Enemy->EnemyNameTag.GetTagName();
		}
		
		CombatMessage.EnemyLevel         = Enemy->EnemyLevel;
		CombatMessage.Value				  = DamageAmount;     
		CombatMessage.HealthPercentAfter = HealthPctAfter;
		CombatMessage.Position			= Context.TargetCharacter->GetActorLocation();
		
		if (const UGameplayAbility* Ability =Data.EffectSpec.GetContext().GetAbilityInstance_NotReplicated())
		{
			CombatMessage.AbilityName = Ability->GetClass()->GetName();
		}
		
		UGMRouterSubsystem::BroadcastMessage(GetWorld(),
			KOGameplayTags::Event_Telemetry_Combat,
			FInstancedStruct::Make(CombatMessage));
	}
}

void UKOHealthSet::HandleDeath(const FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent(); 
	if (ASC)
	{
		// Ability 호출 (몽타주 재생 / GE_Death 적용) 
		FGameplayEventData EventData;
		EventData.Target = ASC->GetAvatarActor();
		EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator();
		
		 ASC->HandleGameplayEvent(KOGameplayTags::Event_Death, &EventData);
	}
}

void UKOHealthSet::HandleHealing(const FGameplayEffectModCallbackData& Data)
{
	float NewHealth = FMath::Clamp(
		GetHealth() + GetHealing(), 
		0.f,
		GetMaxHealth()
	);
		
	SetHealth(NewHealth);
	SetHealing(0.f); // 메타 데이터 초기화 
}
