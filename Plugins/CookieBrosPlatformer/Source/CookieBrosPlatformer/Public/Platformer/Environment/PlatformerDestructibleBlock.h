#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Damageable.h"
#include "Platformer/Environment/PlatformerBlock.h"
#include "PlatformerDestructibleBlock.generated.h"

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerDestructibleBlock : public APlatformerBlock, public IDamageable
{
	GENERATED_BODY()

public:
	APlatformerDestructibleBlock();

	virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult) override;
	virtual bool IsAlive() const override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destructible Block", meta=(ClampMin=1.0))
	float MaxHitPoints = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destructible Block", meta=(ClampMin=0.0))
	float DamagePerHit = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destructible Block")
	bool bDestroyActorOnBreak = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destructible Block")
	bool bRespawnAfterBreak = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destructible Block", meta=(ClampMin=0.0, Units="s"))
	float RespawnDelay = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destructible Block")
	float CurrentHitPoints = 3.0f;

	FTimerHandle RespawnTimerHandle;

	void ApplyPointDamage(float DamageAmount);

	UFUNCTION()
	void HandleBroken();

	UFUNCTION()
	void RespawnBlock();
};
