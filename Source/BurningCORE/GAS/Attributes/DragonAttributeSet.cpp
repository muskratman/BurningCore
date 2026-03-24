#include "GAS/Attributes/DragonAttributeSet.h"
#include "GameplayEffectExtension.h"

UDragonAttributeSet::UDragonAttributeSet()
	: Health(10.0f)
	, MaxHealth(10.0f)
	, OverdriveEnergy(0.0f)
	, MaxOverdriveEnergy(100.0f)
	, BaseDamage(1.0f)
	, ChargeMultiplier(1.5f)
	, IncomingDamage(0.0f)
{
}

void UDragonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetOverdriveEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxOverdriveEnergy());
	}
}

void UDragonAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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

			if (GetHealth() <= 0.0f)
			{
				// Character dead, handle in blueprint or ASC tag
			}
		}
	}
}
