#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerMovingPlatform.h"
#include "PlatformerTriggeredLift.generated.h"

class UBoxComponent;
class ACharacter;
class UPrimitiveComponent;
class USceneComponent;

/**
 * Lift that starts moving when a character enters its trigger volume.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerTriggeredLift : public APlatformerMovingPlatform
{
	GENERATED_BODY()

public:
	APlatformerTriggeredLift();
	void SetTriggerSize(const FVector& InTriggerSize);

	FORCEINLINE const FVector& GetTriggerSize() const { return TriggerSize; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void HandlePauseFinishedAtPointB() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TriggerVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lift|Trigger")
	FVector TriggerSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lift|Components")
	FPlatformerComponentTransformOffset TriggerVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lift|Behaviour")
	bool bAutoReturnToPointA = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lift|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float AutoReturnDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lift|Behaviour")
	bool bAllowManualReturnFromPointB = true;

	TSet<TWeakObjectPtr<ACharacter>> OccupyingCharacters;
	FTimerHandle ReturnTimerHandle;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleReturnTimerElapsed();

	void ClearInvalidOccupants();
	void ScheduleReturnToPointA();
};
