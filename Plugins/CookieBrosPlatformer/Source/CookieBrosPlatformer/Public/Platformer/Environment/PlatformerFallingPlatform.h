#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerSurfaceBlockBase.h"
#include "PlatformerFallingPlatform.generated.h"

class ACharacter;

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerFallingPlatform : public APlatformerSurfaceBlockBase
{
	GENERATED_BODY()

public:
	APlatformerFallingPlatform();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void HandleCharacterEnteredSurface(ACharacter* Character) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Falling Platform", meta=(ClampMin=0.0, Units="s"))
	float FallDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Falling Platform", meta=(ClampMin=0.0, Units="cm/s"))
	float FallingSpeed = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Falling Platform")
	bool bRespawnAfterFall = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Falling Platform", meta=(ClampMin=0.0, Units="s"))
	float RespawnDelay = 3.0f;

	FVector InitialActorLocation = FVector::ZeroVector;
	bool bPendingFall = false;
	bool bFalling = false;
	FTimerHandle FallTimerHandle;
	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void StartFalling();

	UFUNCTION()
	void RespawnPlatform();
};
