#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerBlockBase.h"
#include "PlatformerYokuBlocks.generated.h"

/**
 * Cyclic appearing/disappearing platform block used for timing-based traversal sections.
 */
UENUM()
enum class EPlatformerYokuBlockTransitionState : uint8
{
	IdleAtVisible = 0,
	IdleAtHidden,
	MovingToVisible,
	MovingToHidden
};

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerYokuBlocks : public APlatformerBlockBase
{
	GENERATED_BODY()

public:
	APlatformerYokuBlocks();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yoku Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float InitialDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yoku Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float ShowDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yoku Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float HidenDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yoku Block|Behaviour", meta=(ClampMin=0.0, Units="cm/s"))
	float HideSpeed = 300.0f;

	UPROPERTY(Transient)
	bool bIsBlockVisible = true;

	UPROPERTY(Transient)
	FVector VisibleActorLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector HiddenActorLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	EPlatformerYokuBlockTransitionState TransitionState = EPlatformerYokuBlockTransitionState::IdleAtVisible;

	FTimerHandle StateTimerHandle;

	UFUNCTION()
	void ShowBlock();

	UFUNCTION()
	void HideBlock();

	void SetBlockVisibleState(bool bNewVisible);
	void UpdateHiddenActorLocation();
	void StartBlockTransition(bool bMoveToVisible);
	void FinishBlockTransition();
	void ScheduleNextStateTransition(float Delay, void (APlatformerYokuBlocks::*NextStateFunction)());
};
