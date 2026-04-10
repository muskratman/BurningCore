#pragma once

#include "CoreMinimal.h"
#include "Character/SideViewMovementComponent.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "PlatformerTraversalMovementComponent.generated.h"

class UPlatformerTraversalConfigDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlatformerTraversalStateChanged,
	EPlatformerTraversalState,
	PreviousState,
	EPlatformerTraversalState,
	NewState);

/**
 * Extended side-view movement component with data-driven traversal states.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerTraversalMovementComponent : public USideViewMovementComponent
{
	GENERATED_BODY()

public:
	UPlatformerTraversalMovementComponent();

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void SetTraversalConfig(UPlatformerTraversalConfigDataAsset* InTraversalConfig);

	UFUNCTION(BlueprintPure, Category="Traversal")
	UPlatformerTraversalConfigDataAsset* GetTraversalConfig() const { return TraversalConfig; }

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void SetTraversalEnabled(bool bInTraversalEnabled);

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsTraversalEnabled() const { return bTraversalEnabled; }

	void SetDefaultLedgeSettings(const FPlatformerLedgeTraversalSettings& InSettings);
	void SetDefaultSlideDashSettings(const FPlatformerSlideDashSettings& InSettings);
	void SetDefaultWallSettings(const FPlatformerWallTraversalSettings& InSettings);
	void SetDeveloperTraversalSettingsOverride(
		const FPlatformerLedgeTraversalSettings& InLedgeSettings,
		const FPlatformerSlideDashSettings& InSlideDashSettings,
		const FPlatformerWallTraversalSettings& InWallSettings);
	void ClearDeveloperTraversalSettingsOverride();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void SetTraversalInputVector(FVector2D InTraversalInputVector);

	UFUNCTION(BlueprintPure, Category="Traversal")
	FVector2D GetTraversalInputVector() const { return TraversalInputVector; }

	UFUNCTION(BlueprintCallable, Category="Traversal")
	bool HandleTraversalJumpPressed();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	bool HandleTraversalCrouchPressed();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void HandleTraversalJumpReleased();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void HandleTraversalCrouchReleased();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	bool StartSlideDash();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void CancelTraversal();

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsTraversalStateActive() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsInCustomTraversalMode() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsHangingOnLedge() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsWallSliding() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsSlideDashing() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsInSlideDashRecovery() const;

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsAttackBlockedByTraversal() const;

	UPROPERTY(BlueprintAssignable, Category="Traversal")
	FOnPlatformerTraversalStateChanged OnTraversalStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Traversal|Runtime")
	bool bTraversalEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Traversal|Runtime")
	TObjectPtr<UPlatformerTraversalConfigDataAsset> TraversalConfig;

private:
	struct FCachedCapsuleState
	{
		float UnscaledRadius = 0.0f;
		float UnscaledHalfHeight = 0.0f;
		float ShapeScale = 1.0f;
		bool bValid = false;
	};

	void CacheCapsuleState();
	void UpdateAutomaticTraversal();
	void UpdateTraversalState(EPlatformerTraversalState NewState);
	void AddPersistentCue(const FGameplayTag& CueTag);
	void RemovePersistentCue();
	void ExecuteCue(const FGameplayTag& CueTag) const;
	void AddLooseTag(const FGameplayTag& Tag) const;
	void RemoveLooseTag(const FGameplayTag& Tag) const;
	FGameplayTag GetStateTagForTraversalState(EPlatformerTraversalState State) const;

	float GetWorldTimeSafe() const;
	float GetTraversalDirectionSign() const;
	const FPlatformerLedgeTraversalSettings& GetLedgeSettings() const;
	const FPlatformerSlideDashSettings& GetSlideDashSettings() const;
	const FPlatformerWallTraversalSettings& GetWallSettings() const;
	bool ShouldUseFallingTraversal() const;
	bool RestoreCapsuleSizeIfPossible();
	void ApplySlideDashCapsule();
	bool CanUseFullCapsuleAt(const FVector& TestLocation) const;
	bool HasWalkableFloorBelow(const FVector& TestLocation) const;

	bool TryFindLedgeGrab(FVector& OutHangLocation, FVector& OutClimbTargetLocation, FVector& OutWallNormal) const;
	bool TryFindWallSlide(FVector& OutWallNormal, FVector& OutAnchorLocation, AActor*& OutWallActor) const;

	void EnterLedgeHang(const FVector& HangLocation, const FVector& ClimbTargetLocation, const FVector& WallNormal);
	void StartLedgeClimb();
	void DropFromLedge();
	void PhysLedgeHang(float DeltaTime, int32 Iterations);
	void PhysLedgeClimb(float DeltaTime, int32 Iterations);

	void EnterWallSlide(const FVector& WallNormal, const FVector& AnchorLocation, AActor* WallActor);
	void ExitWallSlide(bool bSetToFallingMode);
	void PhysWallSlide(float DeltaTime, int32 Iterations);
	void PerformWallKick();

	void FinishSlideDash(bool bInterrupted, bool bForceFallingMode);
	void PhysSlideDash(float DeltaTime, int32 Iterations);

	FVector2D TraversalInputVector = FVector2D::ZeroVector;
	EPlatformerTraversalState TraversalState = EPlatformerTraversalState::None;
	FPlatformerLedgeTraversalSettings DefaultLedgeSettings;
	FPlatformerSlideDashSettings DefaultSlideDashSettings;
	FPlatformerWallTraversalSettings DefaultWallSettings;
	bool bHasDeveloperTraversalSettingsOverride = false;
	FPlatformerLedgeTraversalSettings DeveloperLedgeSettingsOverride;
	FPlatformerSlideDashSettings DeveloperSlideDashSettingsOverride;
	FPlatformerWallTraversalSettings DeveloperWallSettingsOverride;

	FCachedCapsuleState CachedCapsuleState;
	bool bDashCapsuleApplied = false;
	bool bPendingCapsuleRestore = false;

	FVector LedgeHangLocation = FVector::ZeroVector;
	FVector LedgeClimbTargetLocation = FVector::ZeroVector;
	FVector LedgeWallNormal = FVector::ZeroVector;
	float LedgeClimbStartTime = 0.0f;
	float LedgeClimbDuration = 0.0f;
	float LedgeReleaseLockoutEndTime = -1.0f;

	FVector WallSlideNormal = FVector::ZeroVector;
	FVector WallSlideAnchorLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> WallSlideActor;
	TWeakObjectPtr<AActor> LastWallKickActor;
	FVector LastWallKickNormal = FVector::ZeroVector;
	float WallSlideStartTime = 0.0f;
	float SameWallReattachEndTime = -1.0f;

	FVector SlideDashStartLocation = FVector::ZeroVector;
	float SlideDashStartTime = 0.0f;
	float SlideDashDirectionSign = 1.0f;
	float SlideDashRecoveryEndTime = -1.0f;
	float FallingStartTime = -1.0f;

	FGameplayTag ActivePersistentCueTag;
};
