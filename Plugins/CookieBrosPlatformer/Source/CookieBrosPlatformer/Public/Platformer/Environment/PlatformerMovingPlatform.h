// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Character/PlatformerInteractable.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "PlatformerMovingPlatform.generated.h"

class UBoxComponent;
class UPlatformerDropThroughPlatformComponent;
class USceneComponent;
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
 * Unified native moving platform driven by UPlatformerPathComponent route data.
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
	void SetMovementPathPoints(const TArray<FPlatformerPathPoint>& InPathPoints);

	UFUNCTION(BlueprintCallable, Category="Moving Platform")
	virtual void ResetInteraction();

	FORCEINLINE UStaticMeshComponent* GetPlatformMesh() const { return PlatformMesh; }
	FORCEINLINE const FVector& GetPlatformSize() const { return PlatformSize; }
	FORCEINLINE UPlatformerPathComponent* GetMovementPathComponent() const { return MovementPathComponent; }
	const TArray<FPlatformerPathPoint>& GetMovementPathPoints() const;
	FVector GetMovementPathPointWorldLocation(int32 PointIndex) const;
	int32 GetLastMovementPathPointIndex() const;
	FORCEINLINE bool IsAtPointA() const { return !IsMoving() && CurrentPathPointIndex == 0; }
	FORCEINLINE bool IsAtPointB() const { return !IsMoving() && CurrentPathPointIndex == GetLastMovementPathPointIndex(); }
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
	TObjectPtr<USceneComponent> DropThroughTopCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DropThroughTopCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DropThroughBottomCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DropThroughBottomCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlatformerDropThroughPlatformComponent> DropThroughPlatformComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlatformerPathComponent> MovementPathComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Shape")
	FVector PlatformSize = FVector(250.0f, 250.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Components")
	FPlatformerComponentTransformOffset PlatformMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=1.0, Units="cm/s"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement", meta=(ClampMin=0.0, Units="cm"))
	float ArrivalTolerance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement")
	bool bStartAtPointB = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Moving Platform|Movement")
	bool bAutoStart = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	TArray<FVector> CachedPathPointLocations;
	EPlatformerMoverState MoveState = EPlatformerMoverState::IdleAtPointA;
	float PauseTimer = 0.0f;
	int32 CurrentPathPointIndex = 0;
	int32 TargetPathPointIndex = INDEX_NONE;
	int32 ActivePathStartPointIndex = 0;
	bool bPendingInitialDeparture = false;
	bool bPathRunActive = false;

	void MoveTowards(float DeltaTime);
	void EnterPauseAtCurrentPathPoint();
	void AdvancePause(float DeltaTime);
	void StartMovingAwayFromCurrentPoint();
	void ContinuePathAfterPause();
	void StartMovingToPathPoint(int32 InTargetPointIndex);
	float GetPathPointDelay(int32 PointIndex) const;
	float GetPathSegmentSpeedScale(int32 StartPointIndex) const;

	virtual void HandleReachedPointA();
	virtual void HandleReachedPointB();
	virtual void HandlePauseFinishedAtPointA();
	virtual void HandlePauseFinishedAtPointB();

	void StartMovingToPointA();
	void StartMovingToPointB();
	void RefreshMovingPlatformLayout();
#if WITH_EDITOR
	void RebaseEditorPathStartToActorLocation();
#endif
	void EnsureMovementPathPoints();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Moving Platform", meta=(DisplayName="Move to Target"))
	void BP_MoveToTarget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Moving Platform", meta=(DisplayName="On Movement Started"))
	void BP_OnMovementStarted();
};
