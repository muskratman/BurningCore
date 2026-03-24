#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "BossBase.generated.h"

/**
 * ABossBase
 * Enlarged, multi-phase enemy type that registers itself with the GameMode for UI and progression purposes.
 */
UCLASS()
class BURNINGCORE_API ABossBase : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	ABossBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss")
	FText BossEncounterName;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss")
	int32 TotalPhases = 1;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Boss")
	void TransitionToPhase(int32 NextPhase);

protected:
	virtual void BeginPlay() override;

	int32 CurrentPhase = 1;
};
