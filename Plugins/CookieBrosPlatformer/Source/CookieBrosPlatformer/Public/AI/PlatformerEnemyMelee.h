#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyBase.h"
#include "PlatformerEnemyMelee.generated.h"

/**
 * APlatformerEnemyMelee
 * Generic close-range enemy shell.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerEnemyMelee : public APlatformerEnemyBase
{
	GENERATED_BODY()

public:
	APlatformerEnemyMelee(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditDefaultsOnly, Category="Combat|Melee", meta=(ClampMin=0.0, Units="cm"))
	float MeleeTraceDistance = 140.0f;

	UPROPERTY(EditDefaultsOnly, Category="Combat|Melee", meta=(ClampMin=0.0, Units="cm"))
	float MeleeTraceRadius = 45.0f;

	virtual float GetAttackRange() const override;
	virtual float GetAttackCooldown() const override;
	virtual float GetAttackDamageAmount() const override;
	virtual bool PerformAttack(APlatformerCombatCharacterBase* TargetActor) override;
};
