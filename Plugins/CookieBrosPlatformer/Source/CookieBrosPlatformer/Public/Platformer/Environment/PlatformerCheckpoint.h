#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interfaces/Interactable.h"
#include "PlatformerCheckpoint.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/**
 * Thin environment actor that marks a respawn point.
 * Runtime registration, save, and respawn flow live in UPlatformerCheckpointSubsystem.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerCheckpoint : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APlatformerCheckpoint();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void ActivateCheckpoint(AActor* Activator);

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	void SetCheckpointActive(bool bInActive);

	UFUNCTION(BlueprintPure, Category="Checkpoint")
	FTransform GetRespawnTransform() const;

	FORCEINLINE bool IsCheckpointActive() const { return bIsActive; }
	FORCEINLINE UBoxComponent* GetTriggerZone() const { return TriggerZone; }
	FORCEINLINE UStaticMeshComponent* GetCheckpointMesh() const { return CheckpointMesh; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FGameplayTag CheckpointID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FText CheckpointName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	bool bActivateOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FTransform RespawnTransformOffset = FTransform::Identity;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Checkpoint")
	bool bIsActive = false;

private:
	void RefreshCheckpointLayout();

	UFUNCTION()
	void OnTriggerZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
