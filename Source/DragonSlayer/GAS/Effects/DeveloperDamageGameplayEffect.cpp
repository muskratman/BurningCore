#include "GAS/Effects/DeveloperDamageGameplayEffect.h"

#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"

UDeveloperDamageGameplayEffect::UDeveloperDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo DeveloperDamageModifier;
	DeveloperDamageModifier.Attribute = UPlatformerCharacterAttributeSet::GetIncomingDamageAttribute();
	DeveloperDamageModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat DeveloperSetByCallerMagnitude;
	DeveloperSetByCallerMagnitude.DataTag = GetDeveloperDamageSetByCallerTag();
	DeveloperDamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DeveloperSetByCallerMagnitude);

	Modifiers.Add(DeveloperDamageModifier);
}

FGameplayTag UDeveloperDamageGameplayEffect::GetDeveloperDamageSetByCallerTag()
{
	return FGameplayTag::RequestGameplayTag(FName(TEXT("Developer.Combat.Damage")));
}
