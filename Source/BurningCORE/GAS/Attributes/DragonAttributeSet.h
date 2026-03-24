#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DragonAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UDragonAttributeSet
 * Health, Overdrive, and Combat stats for the Dragon character.
 */
UCLASS()
class BURNINGCORE_API UDragonAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDragonAttributeSet();

	// Vitals
	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, Health)
    
	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, MaxHealth)
    
	// Overdrive Resource
	UPROPERTY(BlueprintReadOnly, Category="Overdrive")
	FGameplayAttributeData OverdriveEnergy;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, OverdriveEnergy)
    
	UPROPERTY(BlueprintReadOnly, Category="Overdrive")
	FGameplayAttributeData MaxOverdriveEnergy;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, MaxOverdriveEnergy)
    
	// Combat
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData BaseDamage;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, BaseDamage)
    
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	FGameplayAttributeData ChargeMultiplier;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, ChargeMultiplier)
    
	// Meta (incoming damage, used for pre-processing)
	UPROPERTY(BlueprintReadOnly, Category="Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UDragonAttributeSet, IncomingDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
