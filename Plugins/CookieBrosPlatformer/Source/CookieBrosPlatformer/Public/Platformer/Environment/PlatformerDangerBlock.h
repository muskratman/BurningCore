#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerSurfaceBlockBase.h"
#include "PlatformerDangerBlock.generated.h"

class UGameplayEffect;

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerDangerBlock : public APlatformerSurfaceBlockBase
{
	GENERATED_BODY()

public:
	APlatformerDangerBlock();

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage", meta=(ClampMin=0.0))
	float DamageAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage", meta=(ClampMin=0.0, Units="s"))
	float DamageCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage")
	bool bLaunchCharacters = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage", meta=(ClampMin=0.0, Units="cm/s"))
	float HorizontalKnockback = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Danger Block|Damage", meta=(ClampMin=0.0, Units="cm/s"))
	float VerticalKnockback = 250.0f;

	TMap<TWeakObjectPtr<AActor>, float> LastDamageTimeByActor;

	void TryDamageCharacter(ACharacter* Character);
	bool CanDamageActor(AActor* Actor) const;
};
