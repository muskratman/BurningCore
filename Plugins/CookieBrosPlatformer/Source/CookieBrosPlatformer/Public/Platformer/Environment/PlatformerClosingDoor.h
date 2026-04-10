#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerMovingPlatform.h"
#include "PlatformerClosingDoor.generated.h"

class APlatformerCharacterBase;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

/**
 * One-shot overhead door that drops down to seal a passage after the player clears its trigger.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerClosingDoor : public APlatformerMovingPlatform
{
	GENERATED_BODY()

public:
	APlatformerClosingDoor();

	virtual void Interaction(AActor* Interactor) override;
	virtual void ResetInteraction() override;

	void SetTriggerSize(const FVector& InTriggerSize);

	FORCEINLINE const FVector& GetTriggerSize() const { return TriggerSize; }
	FORCEINLINE const FVector& GetTriggerBaseRelativeLocation() const { return TriggerBaseRelativeLocation; }
	FORCEINLINE bool HasBeenTriggered() const { return bHasBeenTriggered; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void HandleReachedPointB() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TriggerVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Closing Door|Trigger")
	FVector TriggerSize = FVector(200.0f, 200.0f, 220.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Closing Door|Trigger")
	FVector TriggerBaseRelativeLocation = FVector(150.0f, 0.0f, -150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Closing Door|Components")
	FPlatformerComponentTransformOffset TriggerVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Closing Door|Behaviour")
	bool bTriggerOnEndOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Closing Door|Behaviour")
	bool bOnlyPlayerControlledCharacters = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Closing Door|State")
	bool bHasBeenTriggered = false;

	TSet<TWeakObjectPtr<APlatformerCharacterBase>> OverlappingCharacters;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsValidTriggeringCharacter(AActor* OtherActor, APlatformerCharacterBase*& OutCharacter) const;
	void TryTriggerDoor();
	void SetTriggerEnabled(bool bEnabled);
	void ClearInvalidOverlappingCharacters();
};
