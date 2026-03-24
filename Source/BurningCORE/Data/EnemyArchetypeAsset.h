#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EnemyArchetypeAsset.generated.h"

class AEnemyBase;
class UStateTree;
class UGameplayAbility;

/**
 * UEnemyArchetypeAsset
 * Data asset containing stats, behavior, and abilities for an enemy type.
 */
UCLASS()
class BURNINGCORE_API UEnemyArchetypeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Identity")
	FGameplayTag EnemyTypeTag;
    
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSubclassOf<AEnemyBase> EnemyClass;
    
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float BaseHealth = 3.0f;
    
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float BaseDamage = 1.0f;
    
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float MoveSpeed = 400.0f;
    
	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UStateTree> BehaviorTree;
    
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
    
	UPROPERTY(EditDefaultsOnly, Category="Rewards")
	int32 CurrencyDrop = 10;
    
	UPROPERTY(EditDefaultsOnly, Category="Rewards")
	float OverdriveEnergyDrop = 5.0f;
    
	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	TArray<FGameplayTag> Immunities;
};
