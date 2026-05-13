#pragma once

#include "CoreMinimal.h"
#include "Data/DragonDashBounceDataAsset.h"
#include "TimerManager.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "DragonDashMovementComponent.generated.h"

class UDragonDashBounceDataAsset;
class UDragonDashBounceSurfaceComponent;
class UDragonDashEnemyTargetComponent;

UENUM(BlueprintType)
enum class EDragonDashState : uint8
{
	None,
	GroundDash,
	AirDash,
	BounceDash,
	AerialDecision
};

UENUM(BlueprintType)
enum class EDragonDashFinishReason : uint8
{
	Completed,
	Interrupted,
	JumpCanceled,
	Blocked
};

USTRUCT(BlueprintType)
struct FDragonDashRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Dragon Dash")
	FVector2D InputVector = FVector2D::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDragonDashStateChanged, EDragonDashState, PreviousState, EDragonDashState, NewState);

/**
 * Dragon-specific dash executor. Keeps reusable traversal behavior available
 * through UPlatformerTraversalMovementComponent while owning diagonal dash,
 * air dash, bounce, and drift rules in project code.
 */
UCLASS()
class DRAGONSLAYER_API UDragonDashMovementComponent : public UPlatformerTraversalMovementComponent
{
	GENERATED_BODY()

public:
	UDragonDashMovementComponent();

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UFUNCTION(BlueprintCallable, Category="Dragon Dash")
	void SetDragonDashConfig(UDragonDashBounceDataAsset* InDashConfig);

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	UDragonDashBounceDataAsset* GetDragonDashConfig() const { return DragonDashConfig; }

	UFUNCTION(BlueprintCallable, Category="Dragon Dash")
	bool StartDragonDash(const FDragonDashRequest& Request);

	UFUNCTION(BlueprintCallable, Category="Dragon Dash")
	bool CancelDragonDashForJump();

	UFUNCTION(BlueprintCallable, Category="Dragon Dash")
	void ResetDragonAirDash();

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool CanStartDragonDash() const;

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool IsDragonDashActive() const;

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool IsInAerialDecisionWindow() const { return DragonDashState == EDragonDashState::AerialDecision; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	bool HasUsedDragonAirDash() const { return bHasUsedAirDash; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	EDragonDashState GetDragonDashState() const { return DragonDashState; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetDragonDashElapsedTime() const;

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetDragonDashDuration() const { return ActiveDashDuration; }

	UFUNCTION(BlueprintPure, Category="Dragon Dash")
	float GetDragonDashProgressAlpha() const;

	UPROPERTY(BlueprintAssignable, Category="Dragon Dash")
	FOnDragonDashStateChanged OnDragonDashStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash")
	TObjectPtr<UDragonDashBounceDataAsset> DragonDashConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dragon Dash|Fallback")
	FDragonDashBounceTuning FallbackDashTuning;

private:
	const FDragonDashBounceTuning& GetDashTuning() const;
	FVector BuildDashDirection(const FDragonDashRequest& Request) const;
	FVector QuantizeDashDirection(const FVector& Direction) const;
	float CalculateDashDuration(float Distance, float Speed) const;
	void EnterDragonDash(EDragonDashState NewState, const FVector& Direction, bool bStartedInAir, float Speed, float Distance);
	void FinishDragonDash(EDragonDashFinishReason FinishReason);
	void EnterAerialDecisionWindow(AActor* EnemyActor);
	void EndAerialDecisionWindow();
	void PhysDragonDash(float DeltaTime, int32 Iterations);
	void UpdateAerialDecisionWindow();
	bool TryBounceFromHit(const FHitResult& Hit);
	bool TryGetBounceSurface(const FHitResult& Hit, UDragonDashBounceSurfaceComponent*& OutSurface) const;
	bool TryHandleEnemyDashContact(const FVector& MoveDelta);
	bool TryResolveEnemyDashTarget(const FVector& MoveDelta, FHitResult& OutHit, AActor*& OutTarget, UDragonDashEnemyTargetComponent*& OutTargetComponent) const;
	bool IsValidEnemyDashTarget(AActor* TargetActor, const UDragonDashEnemyTargetComponent* TargetComponent) const;
	bool TryHandleResolvedEnemyDashTarget(const FHitResult& EnemyHit, AActor* EnemyTarget, UDragonDashEnemyTargetComponent* TargetComponent);
	bool ApplyEnemyDashDamage(AActor* TargetActor, const FHitResult& Hit, const UDragonDashEnemyTargetComponent* TargetComponent) const;
	void EnterEnemyBounce(AActor* TargetActor, const FHitResult& Hit, UDragonDashEnemyTargetComponent* TargetComponent);
	bool BeginEnemyHitStop(AActor* TargetActor, const UDragonDashEnemyTargetComponent* TargetComponent);
	void CompleteEnemyHitStop();
	void RestoreEnemyHitStopTimeDilation();
	void SetDragonDashState(EDragonDashState NewState);
	void AddDashTagAndCue();
	void RemoveDashTag();
	void ExecuteCue(const FGameplayTag& CueTag) const;
	float GetWorldTimeSafe() const;

	EDragonDashState DragonDashState = EDragonDashState::None;
	FVector ActiveDashDirection = FVector::ForwardVector;
	FVector ActiveDashStartLocation = FVector::ZeroVector;
	float ActiveDashSpeed = 0.0f;
	float ActiveDashDistance = 0.0f;
	float ActiveDashDuration = 0.0f;
	float ActiveDashStartTime = 0.0f;
	float ActiveDashRecoveryEndTime = -1.0f;
	float AerialDecisionWindowEndTime = -1.0f;
	float PreAerialDecisionGravityScale = 1.0f;
	bool bActiveDashStartedInAir = false;
	bool bActiveDashFromEnemyBounce = false;
	bool bPendingAerialDecisionAfterEnemyBounce = false;
	bool bHasAerialDecisionGravityOverride = false;
	bool bDashHitStopActive = false;
	bool bHasUsedAirDash = false;
	int32 ActiveDashBounceCount = 0;
	TWeakObjectPtr<AActor> LastEnemyBounceTarget;
	float LastEnemyBounceIgnoreEndTime = -1.0f;
	FTimerHandle EnemyHitStopTimerHandle;
	TWeakObjectPtr<AActor> EnemyHitStopTarget;
	float PreEnemyHitStopDragonTimeDilation = 1.0f;
	float PreEnemyHitStopTargetTimeDilation = 1.0f;
};
