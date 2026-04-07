#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GameplayEffectExtension.h"

UPlatformerCharacterAttributeSet::UPlatformerCharacterAttributeSet()
	: Health(10.0f)
	, MaxHealth(10.0f)
	, BaseDamage(1.0f)
	, AttackSpeed(1.0f)
	, MeleeAttackDamage(1.0f)
	, MeleeAttackDelay(0.0f)
	, RangeBaseAttackDamage(1.0f)
	, RangeChargedAttackDamage(1.0f)
	, RangeAttackDelay(0.0f)
	, MoveSpeed(600.0f)
	, JumpHeight(600.0f)
	, IncomingDamage(0.0f)
{
}

void UPlatformerCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetBaseDamageAttribute() || Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMeleeAttackDamageAttribute()
		|| Attribute == GetRangeBaseAttackDamageAttribute()
		|| Attribute == GetRangeChargedAttackDamageAttribute()
		|| Attribute == GetMeleeAttackDelayAttribute()
		|| Attribute == GetRangeAttackDelayAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMoveSpeedAttribute() || Attribute == GetJumpHeightAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UPlatformerCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalIncomingDamage > 0.0f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
}
