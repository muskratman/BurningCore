#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "EnemyAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UEnemyAttributeSet
 * Simplified stat sheet for hostile characters. Health and Damage only.
 */
UCLASS()
class BURNINGCORE_API UEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UEnemyAttributeSet();

	// Vitals
	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, Health)
    
	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, MaxHealth)

	// Combat
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData BaseDamage;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, BaseDamage)

	// Meta (incoming damage)
	UPROPERTY(BlueprintReadOnly, Category="Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, IncomingDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
