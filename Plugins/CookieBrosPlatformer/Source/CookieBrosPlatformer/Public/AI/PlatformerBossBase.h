#pragma once

#include "CoreMinimal.h"
#include "AI/PlatformerEnemyBase.h"
#include "PlatformerBossBase.generated.h"

/**
 * APlatformerBossBase
 * Generic multi-phase platformer boss shell with encounter activation hooks.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerBossBase : public APlatformerEnemyBase
{
	GENERATED_BODY()

public:
	APlatformerBossBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss")
	FText BossEncounterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss", meta=(ClampMin=1, UIMin=1))
	int32 TotalPhases = 1;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Boss")
	void TransitionToPhase(int32 NextPhase);

protected:
	virtual void BeginPlay() override;
	virtual float GetDefaultMaxHealth() const override;
	virtual float GetProjectileMaxDistance() const override;
	virtual float GetHealthWidgetVerticalPadding() const override;

	int32 CurrentPhase = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(ClampMin=0.0, Units="cm"))
	float BossHealthWidgetVerticalPadding = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Projectile", meta=(ClampMin=0.0, Units="cm"))
	float BossProjectileMaxDistance = 1500.0f;
};
