// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Character/PlatformerInteractable.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerMovingPlatform.generated.h"

class UArrowComponent;
class USceneComponent;
class USplineComponent;
class UStaticMeshComponent;
class UTexture2D;

UENUM(BlueprintType)
enum class EPlatformerMoverState : uint8
{
	IdleAtPointA,
	MovingToPointB,
	IdleAtPointB,
	MovingToPointA
};

/**
 * Unified native moving platform with optional auto-start, point delay and repeatable ping-pong motion.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerMovingPlatform : public AActor, public IPlatformerInteractable
{
	GENERATED_BODY()

public:
	APlatformerMovingPlatform();
	virtual void Tick(float DeltaTime) override;
	virtual void Interaction(AActor* Interactor) override;
	void SetPlatformSize(const FVector& InPlatformSize);

	UFUNCTION(BlueprintCallable, Category="Moving Platform")
	virtual void ResetInteraction();

	FORCEINLINE UStaticMeshComponent* GetPlatformMesh() const { return PlatformMesh; }
	FORCEINLINE const FVector& GetPlatformSize() const { return PlatformSize; }
	FORCEINLINE bool IsAtPointA() const { return MoveState == EPlatformerMoverState::IdleAtPointA; }
	FORCEINLINE bool IsAtPointB() const { return MoveState == EPlatformerMoverState::IdleAtPointB; }
	FORCEINLINE bool IsMoving() const
	{
		return MoveState == EPlatformerMoverState::MovingToPointA || MoveState == EPlatformerMoverState::MovingToPointB;
	}

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> PlatformMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> PointALayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UArrowComponent> PointA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> PointBLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UArrowComponent> PointB;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Editor")
	TObjectPtr<UStaticMeshComponent> PointBPreviewMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Editor")
	TObjectPtr<USplineComponent> MovementPathSpline;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Shape")
	FVector PlatformSize = FVector(250.0f, 250.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FPlatformerComponentTransformOffset PlatformMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FVector PointABaseRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FVector PointBBaseRelativeLocation = FVector(500.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FPlatformerComponentTransformOffset PointATransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FPlatformerComponentTransformOffset PointBTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=1.0, Units="cm/s"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=0.0, Units="s"))
	float PointADelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=0.0, Units="s"))
	float PointBDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=0.0, Units="cm"))
	float ArrivalTolerance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement")
	bool bStartAtPointB = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement")
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement")
	bool bIsRepeatable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	UPROPERTY()
	float PointDelay = 0.5f;

	FVector CachedPointALocation = FVector::ZeroVector;
	FVector CachedPointBLocation = FVector::ZeroVector;
	EPlatformerMoverState MoveState = EPlatformerMoverState::IdleAtPointA;
	float PauseTimer = 0.0f;
	bool bPendingInitialDeparture = false;

	void MoveTowards(const FVector& TargetLocation, float DeltaTime, EPlatformerMoverState ArrivalState);
	void EnterPauseState(EPlatformerMoverState NewState);
	void AdvancePause(float DeltaTime);
	void StartMovingAwayFromCurrentPoint();

	virtual void HandleReachedPointA();
	virtual void HandleReachedPointB();
	virtual void HandlePauseFinishedAtPointA();
	virtual void HandlePauseFinishedAtPointB();

	void StartMovingToPointA();
	void StartMovingToPointB();
	void RefreshMovingPlatformLayout();
#if WITH_EDITOR
	void RebaseEditorPointAToActorLocation();
#endif
#if WITH_EDITORONLY_DATA
	void RefreshEditorPreviewComponents();
#endif

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Moving Platform", meta=(DisplayName="Move to Target"))
	void BP_MoveToTarget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Moving Platform", meta=(DisplayName="On Movement Started"))
	void BP_OnMovementStarted();
};
