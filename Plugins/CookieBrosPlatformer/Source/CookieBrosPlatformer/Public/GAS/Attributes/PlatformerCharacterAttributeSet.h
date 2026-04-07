#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlatformerCharacterAttributeSet.generated.h"

#define PLATFORMER_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UPlatformerCharacterAttributeSet
 * Generic stat sheet for a playable platformer character or a reusable combatant base.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPlatformerCharacterAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData Health;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData MaxHealth;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData BaseDamage;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, BaseDamage)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData AttackSpeed;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData MeleeAttackDamage;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, MeleeAttackDamage)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData MeleeAttackDelay;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, MeleeAttackDelay)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData RangeBaseAttackDamage;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, RangeBaseAttackDamage)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData RangeChargedAttackDamage;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, RangeChargedAttackDamage)

	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData RangeAttackDelay;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, RangeAttackDelay)

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	FGameplayAttributeData MoveSpeed;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	FGameplayAttributeData JumpHeight;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, JumpHeight)

	UPROPERTY(BlueprintReadOnly, Category="Meta")
	FGameplayAttributeData IncomingDamage;
	PLATFORMER_ATTRIBUTE_ACCESSORS(UPlatformerCharacterAttributeSet, IncomingDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
