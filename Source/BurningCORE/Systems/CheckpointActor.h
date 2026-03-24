#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "GameplayTagContainer.h"
#include "CheckpointActor.generated.h"

class UBoxComponent;
class UNiagaraComponent;

/**
 * ACheckpointActor
 * Shrine that the player interacts with to save progress and respawn.
 */
UCLASS()
class BURNINGCORE_API ACheckpointActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	ACheckpointActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FGameplayTag CheckpointID;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Checkpoint")
	FText CheckpointName;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerZone;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> ShrineVFX;

	bool bIsActive = false;
};
