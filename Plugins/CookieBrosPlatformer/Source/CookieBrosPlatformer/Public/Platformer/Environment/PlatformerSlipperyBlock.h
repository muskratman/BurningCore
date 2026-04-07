#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerSurfaceBlockBase.h"
#include "PlatformerSlipperyBlock.generated.h"

class ACharacter;

USTRUCT()
struct FPlatformerSlipperyMovementState
{
	GENERATED_BODY()

	UPROPERTY()
	float GroundFriction = 8.0f;

	UPROPERTY()
	float BrakingFrictionFactor = 2.0f;

	UPROPERTY()
	float BrakingDecelerationWalking = 2048.0f;
};

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerSlipperyBlock : public APlatformerSurfaceBlockBase
{
	GENERATED_BODY()

public:
	APlatformerSlipperyBlock();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void HandleCharacterEnteredSurface(ACharacter* Character) override;
	virtual void HandleCharacterLeftSurface(ACharacter* Character) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slippery Block|Movement", meta=(ClampMin=0.0))
	float GroundFrictionOverride = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slippery Block|Movement", meta=(ClampMin=0.0))
	float BrakingFrictionFactorOverride = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slippery Block|Movement", meta=(ClampMin=0.0, Units="cm/s^2"))
	float BrakingDecelerationWalkingOverride = 96.0f;

	TMap<TWeakObjectPtr<ACharacter>, FPlatformerSlipperyMovementState> CachedMovementStates;

	void RestoreCharacter(ACharacter* Character);
};
