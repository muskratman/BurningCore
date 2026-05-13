#include "Platformer/Character/DragonDashMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "Combat/PlatformerCombatCharacterBase.h"
#include "Character/DragonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Data/DragonDashBounceDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/DragonDashBounceSurfaceComponent.h"
#include "Systems/DragonDashEnemyTargetComponent.h"
#include "TimerManager.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

namespace
{
	constexpr float WallBounceNormalThreshold = 0.75f;
	constexpr float VerticalBounceNormalThreshold = 0.75f;
	constexpr float EnemyRepeatHitLockout = 0.15f;
}

UDragonDashMovementComponent::UDragonDashMovementComponent()
{
}

void UDragonDashMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UDragonDashMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DragonDashState == EDragonDashState::AerialDecision)
	{
		UpdateAerialDecisionWindow();
	}
}

void UDragonDashMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (IsDragonDashActive())
	{
		PhysDragonDash(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UDragonDashMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Walking)
	{
		bHasUsedAirDash = false;
		if (DragonDashState == EDragonDashState::AerialDecision)
		{
			EndAerialDecisionWindow();
		}
	}

	if (IsDragonDashActive() && MovementMode != MOVE_Custom)
	{
		FinishDragonDash(EDragonDashFinishReason::Interrupted);
	}
}

void UDragonDashMovementComponent::SetDragonDashConfig(UDragonDashBounceDataAsset* InDashConfig)
{
	DragonDashConfig = InDashConfig;
}

bool UDragonDashMovementComponent::CanStartDragonDash() const
{
	if (!CharacterOwner || !UpdatedComponent || IsDragonDashActive())
	{
		return false;
	}

	if (GetWorldTimeSafe() < ActiveDashRecoveryEndTime)
	{
		return false;
	}

	return IsMovingOnGround() || !bHasUsedAirDash;
}

bool UDragonDashMovementComponent::StartDragonDash(const FDragonDashRequest& Request)
{
	if (!CanStartDragonDash())
	{
		return false;
	}

	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	FVector DashDirection = BuildDashDirection(Request);
	if (DashDirection.IsNearlyZero())
	{
		return false;
	}

	const bool bStartedInAir = !IsMovingOnGround();
	if (DragonDashState == EDragonDashState::AerialDecision)
	{
		EndAerialDecisionWindow();
	}

	if (!bStartedInAir && DashDirection.Z < 0.0f)
	{
		DashDirection.Z = 0.0f;
		DashDirection = QuantizeDashDirection(DashDirection);
	}

	EnterDragonDash(
		bStartedInAir ? EDragonDashState::AirDash : EDragonDashState::GroundDash,
		DashDirection,
		bStartedInAir,
		Tuning.DashSpeed,
		Tuning.DashDistance);

	if (bStartedInAir)
	{
		bHasUsedAirDash = true;
	}

	return true;
}

bool UDragonDashMovementComponent::CancelDragonDashForJump()
{
	if (DragonDashState != EDragonDashState::GroundDash
		|| bActiveDashStartedInAir
		|| FMath::Abs(ActiveDashDirection.Z) > KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	const float PreservedSpeed = FMath::Min(FMath::Abs(Velocity.X), Tuning.DashSpeed) * Tuning.JumpCancelMomentumScale;
	const float PreservedVelocityX = FMath::Sign(ActiveDashDirection.X) * PreservedSpeed;
	FinishDragonDash(EDragonDashFinishReason::JumpCanceled);
	SetMovementMode(MOVE_Walking);
	Velocity.X = PreservedVelocityX;
	return true;
}

void UDragonDashMovementComponent::ResetDragonAirDash()
{
	bHasUsedAirDash = false;
}

bool UDragonDashMovementComponent::IsDragonDashActive() const
{
	return DragonDashState == EDragonDashState::GroundDash
		|| DragonDashState == EDragonDashState::AirDash
		|| DragonDashState == EDragonDashState::BounceDash;
}

float UDragonDashMovementComponent::GetDragonDashElapsedTime() const
{
	return IsDragonDashActive() ? FMath::Max(GetWorldTimeSafe() - ActiveDashStartTime, 0.0f) : 0.0f;
}

float UDragonDashMovementComponent::GetDragonDashProgressAlpha() const
{
	if (!IsDragonDashActive())
	{
		return 0.0f;
	}

	const float TimeAlpha = ActiveDashDuration > 0.0f
		? FMath::Clamp(GetDragonDashElapsedTime() / ActiveDashDuration, 0.0f, 1.0f)
		: 1.0f;
	const float DistanceTravelled = UpdatedComponent
		? FVector::Dist(UpdatedComponent->GetComponentLocation(), ActiveDashStartLocation)
		: 0.0f;
	const float DistanceAlpha = ActiveDashDistance > 0.0f
		? FMath::Clamp(DistanceTravelled / ActiveDashDistance, 0.0f, 1.0f)
		: 1.0f;
	return FMath::Max(TimeAlpha, DistanceAlpha);
}

const FDragonDashBounceTuning& UDragonDashMovementComponent::GetDashTuning() const
{
	return DragonDashConfig ? DragonDashConfig->Tuning : FallbackDashTuning;
}

FVector UDragonDashMovementComponent::BuildDashDirection(const FDragonDashRequest& Request) const
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	FVector2D InputVector = Request.InputVector;
	if (InputVector.SizeSquared() < FMath::Square(Tuning.DirectionInputDeadZone))
	{
		const float FacingSign = CharacterOwner ? FMath::Sign(CharacterOwner->GetActorForwardVector().X) : 1.0f;
		InputVector.X = FMath::IsNearlyZero(FacingSign) ? 1.0f : FacingSign;
		InputVector.Y = 0.0f;
	}

	if (FMath::Abs(InputVector.Y) < Tuning.DiagonalInputThreshold)
	{
		InputVector.Y = 0.0f;
	}

	FVector Direction(InputVector.X, 0.0f, InputVector.Y);
	if (Direction.IsNearlyZero())
	{
		const float FacingSign = CharacterOwner ? FMath::Sign(CharacterOwner->GetActorForwardVector().X) : 1.0f;
		Direction.X = FMath::IsNearlyZero(FacingSign) ? 1.0f : FacingSign;
	}

	return QuantizeDashDirection(Direction);
}

FVector UDragonDashMovementComponent::QuantizeDashDirection(const FVector& Direction) const
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	const float HorizontalSign = FMath::Abs(Direction.X) >= Tuning.DirectionInputDeadZone ? FMath::Sign(Direction.X) : 0.0f;
	const float VerticalSign = FMath::Abs(Direction.Z) >= Tuning.DirectionInputDeadZone ? FMath::Sign(Direction.Z) : 0.0f;

	FVector QuantizedDirection(HorizontalSign, 0.0f, VerticalSign);
	if (QuantizedDirection.IsNearlyZero())
	{
		const float FacingSign = CharacterOwner ? FMath::Sign(CharacterOwner->GetActorForwardVector().X) : 1.0f;
		QuantizedDirection.X = FMath::IsNearlyZero(FacingSign) ? 1.0f : FacingSign;
	}

	return QuantizedDirection.GetSafeNormal();
}

float UDragonDashMovementComponent::CalculateDashDuration(float Distance, float Speed) const
{
	return Speed > KINDA_SMALL_NUMBER ? FMath::Max(Distance, 0.0f) / Speed : KINDA_SMALL_NUMBER;
}

void UDragonDashMovementComponent::EnterDragonDash(
	EDragonDashState NewState,
	const FVector& Direction,
	bool bStartedInAir,
	float Speed,
	float Distance)
{
	ActiveDashDirection = Direction.GetSafeNormal();
	ActiveDashStartLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	ActiveDashSpeed = FMath::Max(Speed, 0.0f);
	ActiveDashDistance = FMath::Max(Distance, 0.0f);
	ActiveDashDuration = CalculateDashDuration(ActiveDashDistance, ActiveDashSpeed);
	ActiveDashStartTime = GetWorldTimeSafe();
	bActiveDashStartedInAir = bStartedInAir;
	bActiveDashFromEnemyBounce = false;
	bPendingAerialDecisionAfterEnemyBounce = false;
	ActiveDashBounceCount = NewState == EDragonDashState::BounceDash ? ActiveDashBounceCount : 0;
	if (NewState == EDragonDashState::GroundDash || NewState == EDragonDashState::AirDash)
	{
		LastEnemyBounceTarget.Reset();
		LastEnemyBounceIgnoreEndTime = -1.0f;
	}

	Velocity = ActiveDashDirection * ActiveDashSpeed;
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::Dash));
	SetDragonDashState(NewState);
	AddDashTagAndCue();
}

void UDragonDashMovementComponent::FinishDragonDash(EDragonDashFinishReason FinishReason)
{
	if (bDashHitStopActive)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(EnemyHitStopTimerHandle);
		}
		RestoreEnemyHitStopTimeDilation();
	}

	const bool bShouldEnterAerialDecision = FinishReason == EDragonDashFinishReason::Completed
		&& bActiveDashFromEnemyBounce
		&& bPendingAerialDecisionAfterEnemyBounce
		&& !IsMovingOnGround();

	if (!bShouldEnterAerialDecision)
	{
		ActiveDashRecoveryEndTime = GetWorldTimeSafe() + GetDashTuning().DashRecovery;
	}
	RemoveDashTag();

	if (bShouldEnterAerialDecision)
	{
		EnterAerialDecisionWindow(LastEnemyBounceTarget.Get());
		return;
	}

	Velocity = FVector::ZeroVector;
	SetDragonDashState(EDragonDashState::None);
	if (MovementMode == MOVE_Custom)
	{
		SetMovementMode(IsMovingOnGround() ? MOVE_Walking : MOVE_Falling);
	}
}

void UDragonDashMovementComponent::EnterAerialDecisionWindow(AActor* EnemyActor)
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	if (Tuning.AerialDecisionWindowDuration <= 0.0f)
	{
		SetDragonDashState(EDragonDashState::None);
		SetMovementMode(MOVE_Falling);
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = CharacterOwner ? CharacterOwner->GetCharacterMovement() : nullptr)
	{
		PreAerialDecisionGravityScale = MovementComponent->GravityScale;
		MovementComponent->GravityScale = Tuning.AerialDecisionGravityScale;
		bHasAerialDecisionGravityOverride = true;
	}

	LastEnemyBounceTarget = EnemyActor;
	LastEnemyBounceIgnoreEndTime = GetWorldTimeSafe() + EnemyRepeatHitLockout;
	AerialDecisionWindowEndTime = GetWorldTimeSafe() + Tuning.AerialDecisionWindowDuration;
	Velocity.Z = FMath::Max(Velocity.Z, -80.0f);
	SetDragonDashState(EDragonDashState::AerialDecision);
	SetMovementMode(MOVE_Falling);
}

void UDragonDashMovementComponent::EndAerialDecisionWindow()
{
	if (bHasAerialDecisionGravityOverride)
	{
		if (UCharacterMovementComponent* MovementComponent = CharacterOwner ? CharacterOwner->GetCharacterMovement() : nullptr)
		{
			MovementComponent->GravityScale = PreAerialDecisionGravityScale;
		}
		bHasAerialDecisionGravityOverride = false;
	}

	AerialDecisionWindowEndTime = -1.0f;
	if (DragonDashState == EDragonDashState::AerialDecision)
	{
		SetDragonDashState(EDragonDashState::None);
	}
}

void UDragonDashMovementComponent::PhysDragonDash(float DeltaTime, int32 Iterations)
{
	if (!UpdatedComponent)
	{
		FinishDragonDash(EDragonDashFinishReason::Interrupted);
		return;
	}

	if (bDashHitStopActive)
	{
		Velocity = FVector::ZeroVector;
		return;
	}

	if (DeltaTime <= 0.0f)
	{
		FinishDragonDash(EDragonDashFinishReason::Interrupted);
		return;
	}

	const float ElapsedTime = GetWorldTimeSafe() - ActiveDashStartTime;
	const float TravelDistance = FVector::Dist(UpdatedComponent->GetComponentLocation(), ActiveDashStartLocation);
	if (ElapsedTime >= ActiveDashDuration || TravelDistance >= ActiveDashDistance)
	{
		FinishDragonDash(EDragonDashFinishReason::Completed);
		return;
	}

	const float RemainingDistance = FMath::Max(ActiveDashDistance - TravelDistance, 0.0f);
	const FVector MoveDelta = ActiveDashDirection * FMath::Min(ActiveDashSpeed * DeltaTime, RemainingDistance);

	if (TryHandleEnemyDashContact(MoveDelta))
	{
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		AActor* HitActor = Hit.GetActor();
		UDragonDashEnemyTargetComponent* TargetComponent = HitActor
			? HitActor->FindComponentByClass<UDragonDashEnemyTargetComponent>()
			: nullptr;
		if (IsValidEnemyDashTarget(HitActor, TargetComponent)
			&& TryHandleResolvedEnemyDashTarget(Hit, HitActor, TargetComponent))
		{
			return;
		}
	}

	if (Hit.IsValidBlockingHit() && !TryBounceFromHit(Hit))
	{
		FinishDragonDash(EDragonDashFinishReason::Blocked);
		return;
	}

	Velocity = ActiveDashDirection * ActiveDashSpeed;
}

void UDragonDashMovementComponent::UpdateAerialDecisionWindow()
{
	if (GetWorldTimeSafe() >= AerialDecisionWindowEndTime)
	{
		EndAerialDecisionWindow();
		return;
	}

	if (GetTraversalInputVector().Y < -GetDashTuning().DirectionInputDeadZone)
	{
		EndAerialDecisionWindow();
	}
}

bool UDragonDashMovementComponent::TryBounceFromHit(const FHitResult& Hit)
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	if (ActiveDashBounceCount >= Tuning.MaxBouncesPerDash)
	{
		return false;
	}

	UDragonDashBounceSurfaceComponent* BounceSurface = nullptr;
	const bool bHasBounceSurface = TryGetBounceSurface(Hit, BounceSurface);
	FVector BounceDirection = FVector::ZeroVector;
	if (BounceSurface && BounceSurface->TryGetBounceDirection(BounceDirection))
	{
		BounceDirection = QuantizeDashDirection(BounceDirection);
	}
	else
	{
		const bool bWallBounce = FMath::Abs(Hit.Normal.X) >= WallBounceNormalThreshold;
		const bool bVerticalBounce = FMath::Abs(Hit.Normal.Z) >= VerticalBounceNormalThreshold;
		const bool bFloorBounce = Hit.Normal.Z >= VerticalBounceNormalThreshold;
		if (bFloorBounce && !bHasBounceSurface)
		{
			return false;
		}

		if (!bWallBounce && !bVerticalBounce)
		{
			return false;
		}

		BounceDirection = ActiveDashDirection - 2.0f * FVector::DotProduct(ActiveDashDirection, Hit.Normal) * Hit.Normal;
		BounceDirection = QuantizeDashDirection(BounceDirection);
	}

	if (BounceDirection.IsNearlyZero())
	{
		return false;
	}

	++ActiveDashBounceCount;
	ActiveDashDirection = BounceDirection;
	ActiveDashStartLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	ActiveDashStartTime = GetWorldTimeSafe();
	ActiveDashDistance = FMath::Max(
		ActiveDashDistance,
		Tuning.MinRemainingDistanceAfterBounce);

	if (BounceSurface)
	{
		ActiveDashSpeed *= BounceSurface->GetSpeedMultiplier();
		ActiveDashDistance *= BounceSurface->GetDistanceMultiplier();
		if (BounceSurface->ShouldResetDragonAirDash())
		{
			ResetDragonAirDash();
		}
	}

	ActiveDashDuration = CalculateDashDuration(ActiveDashDistance, ActiveDashSpeed);
	Velocity = ActiveDashDirection * ActiveDashSpeed;
	SetDragonDashState(EDragonDashState::BounceDash);
	ExecuteCue(Tuning.BounceCueTag);
	return true;
}

bool UDragonDashMovementComponent::TryHandleEnemyDashContact(const FVector& MoveDelta)
{
	FHitResult EnemyHit;
	AActor* EnemyTarget = nullptr;
	UDragonDashEnemyTargetComponent* TargetComponent = nullptr;
	if (!TryResolveEnemyDashTarget(MoveDelta, EnemyHit, EnemyTarget, TargetComponent))
	{
		return false;
	}

	return TryHandleResolvedEnemyDashTarget(EnemyHit, EnemyTarget, TargetComponent);
}

bool UDragonDashMovementComponent::TryHandleResolvedEnemyDashTarget(
	const FHitResult& EnemyHit,
	AActor* EnemyTarget,
	UDragonDashEnemyTargetComponent* TargetComponent)
{
	if (TargetComponent && TargetComponent->HasAntiDash())
	{
		ExecuteCue(GetDashTuning().AntiDashCueTag);
		FinishDragonDash(EDragonDashFinishReason::Blocked);
		return true;
	}

	if (!ApplyEnemyDashDamage(EnemyTarget, EnemyHit, TargetComponent))
	{
		return false;
	}

	EnterEnemyBounce(EnemyTarget, EnemyHit, TargetComponent);
	return true;
}

bool UDragonDashMovementComponent::TryResolveEnemyDashTarget(
	const FVector& MoveDelta,
	FHitResult& OutHit,
	AActor*& OutTarget,
	UDragonDashEnemyTargetComponent*& OutTargetComponent) const
{
	OutTarget = nullptr;
	OutTargetComponent = nullptr;

	UWorld* World = GetWorld();
	if (!World || !CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	const UCapsuleComponent* CapsuleComponent = CharacterOwner ? CharacterOwner->GetCapsuleComponent() : nullptr;
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + MoveDelta;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DragonDashEnemyHit), false, CharacterOwner);
	QueryParams.bFindInitialOverlaps = true;

	TArray<FHitResult> Hits;
	const FCollisionShape DashCollisionShape = CapsuleComponent
		? FCollisionShape::MakeCapsule(
			CapsuleComponent->GetScaledCapsuleRadius() + Tuning.EnemyHitSweepRadius,
			CapsuleComponent->GetScaledCapsuleHalfHeight())
		: FCollisionShape::MakeSphere(Tuning.EnemyHitSweepRadius);
	const bool bHit = World->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		DashCollisionShape,
		QueryParams);
	if (!bHit)
	{
		return false;
	}

	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (const FHitResult& CandidateHit : Hits)
	{
		AActor* CandidateActor = CandidateHit.GetActor();
		UDragonDashEnemyTargetComponent* CandidateComponent = CandidateActor
			? CandidateActor->FindComponentByClass<UDragonDashEnemyTargetComponent>()
			: nullptr;
		if (!IsValidEnemyDashTarget(CandidateActor, CandidateComponent))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Start, CandidateHit.ImpactPoint);
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			OutHit = CandidateHit;
			OutTarget = CandidateActor;
			OutTargetComponent = CandidateComponent;
		}
	}

	return OutTarget != nullptr;
}

bool UDragonDashMovementComponent::IsValidEnemyDashTarget(AActor* TargetActor, const UDragonDashEnemyTargetComponent* TargetComponent) const
{
	if (!TargetActor || TargetActor == CharacterOwner)
	{
		return false;
	}

	if (TargetActor == LastEnemyBounceTarget.Get() && GetWorldTimeSafe() < LastEnemyBounceIgnoreEndTime)
	{
		return false;
	}

	if (TargetComponent && !TargetComponent->AllowsDashBounce() && !TargetComponent->HasAntiDash())
	{
		return false;
	}

	const APlatformerCombatCharacterBase* CombatTarget = Cast<APlatformerCombatCharacterBase>(TargetActor);
	return TargetComponent || (CombatTarget && CombatTarget->IsAlive());
}

bool UDragonDashMovementComponent::ApplyEnemyDashDamage(
	AActor* TargetActor,
	const FHitResult& Hit,
	const UDragonDashEnemyTargetComponent* TargetComponent) const
{
	const ADragonCharacter* DragonCharacter = Cast<ADragonCharacter>(CharacterOwner);
	if (!DragonCharacter || !TargetActor)
	{
		return false;
	}

	const float DamageMultiplier = TargetComponent ? TargetComponent->GetDamageMultiplier() : 1.0f;
	const float DamageAmount = GetDashTuning().EnemyBounceDamage * DamageMultiplier;
	if (DamageAmount <= 0.0f)
	{
		return true;
	}

	return DragonCharacter->ApplyCombatDamageToActor(TargetActor, DamageAmount, Hit);
}

void UDragonDashMovementComponent::EnterEnemyBounce(
	AActor* TargetActor,
	const FHitResult& Hit,
	UDragonDashEnemyTargetComponent* TargetComponent)
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	FVector BounceDirection = QuantizeDashDirection(-ActiveDashDirection);
	if (BounceDirection.IsNearlyZero())
	{
		BounceDirection = FVector::UpVector;
	}

	const bool bEnemyBounceAirborne = !IsMovingOnGround() || FMath::Abs(BounceDirection.Z) > KINDA_SMALL_NUMBER;
	const float DashSpeed = ActiveDashSpeed;
	const float DashDistance = ActiveDashDistance;
	const float DashDuration = ActiveDashDuration;

	ActiveDashDirection = BounceDirection;
	ActiveDashSpeed = DashSpeed;
	ActiveDashDistance = DashDistance;
	ActiveDashDuration = DashDuration;
	ActiveDashStartLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	ActiveDashStartTime = GetWorldTimeSafe();
	ActiveDashBounceCount = 0;
	bActiveDashStartedInAir = bEnemyBounceAirborne;
	bActiveDashFromEnemyBounce = true;
	bPendingAerialDecisionAfterEnemyBounce = bEnemyBounceAirborne;
	LastEnemyBounceTarget = TargetActor;
	LastEnemyBounceIgnoreEndTime = GetWorldTimeSafe() + EnemyRepeatHitLockout;

	if (!TargetComponent || TargetComponent->ShouldResetAirDash())
	{
		ResetDragonAirDash();
	}

	if (TargetComponent)
	{
		TargetComponent->BP_OnDragonDashHit(CharacterOwner, Hit);
	}

	ExecuteCue(Tuning.EnemyBounceCueTag);
	SetDragonDashState(EDragonDashState::BounceDash);
	if (!BeginEnemyHitStop(TargetActor, TargetComponent))
	{
		Velocity = ActiveDashDirection * ActiveDashSpeed;
	}
}

bool UDragonDashMovementComponent::BeginEnemyHitStop(AActor* TargetActor, const UDragonDashEnemyTargetComponent* TargetComponent)
{
	const FDragonDashBounceTuning& Tuning = GetDashTuning();
	if (!TargetActor || Tuning.HitStopDuration <= 0.0f)
	{
		return false;
	}

	if (TargetComponent && !TargetComponent->ShouldTriggerHitStop())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	RestoreEnemyHitStopTimeDilation();
	World->GetTimerManager().ClearTimer(EnemyHitStopTimerHandle);

	bDashHitStopActive = true;
	EnemyHitStopTarget = TargetActor;
	PreEnemyHitStopDragonTimeDilation = CharacterOwner ? CharacterOwner->CustomTimeDilation : 1.0f;
	PreEnemyHitStopTargetTimeDilation = TargetActor->CustomTimeDilation;

	const float HitStopDilation = FMath::Clamp(Tuning.HitStopTimeDilation, 0.0f, 1.0f);
	if (CharacterOwner)
	{
		CharacterOwner->CustomTimeDilation = HitStopDilation;
	}
	TargetActor->CustomTimeDilation = HitStopDilation;
	Velocity = FVector::ZeroVector;

	World->GetTimerManager().SetTimer(
		EnemyHitStopTimerHandle,
		this,
		&UDragonDashMovementComponent::CompleteEnemyHitStop,
		Tuning.HitStopDuration,
		false);
	return true;
}

void UDragonDashMovementComponent::CompleteEnemyHitStop()
{
	RestoreEnemyHitStopTimeDilation();

	if (DragonDashState != EDragonDashState::BounceDash || !UpdatedComponent)
	{
		return;
	}

	ActiveDashStartLocation = UpdatedComponent->GetComponentLocation();
	ActiveDashStartTime = GetWorldTimeSafe();
	Velocity = ActiveDashDirection * ActiveDashSpeed;
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::Dash));
}

void UDragonDashMovementComponent::RestoreEnemyHitStopTimeDilation()
{
	if (!bDashHitStopActive)
	{
		return;
	}

	if (CharacterOwner)
	{
		CharacterOwner->CustomTimeDilation = PreEnemyHitStopDragonTimeDilation;
	}
	if (EnemyHitStopTarget.IsValid())
	{
		EnemyHitStopTarget->CustomTimeDilation = PreEnemyHitStopTargetTimeDilation;
	}

	bDashHitStopActive = false;
	EnemyHitStopTarget.Reset();
}

bool UDragonDashMovementComponent::TryGetBounceSurface(const FHitResult& Hit, UDragonDashBounceSurfaceComponent*& OutSurface) const
{
	OutSurface = nullptr;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return false;
	}

	OutSurface = HitActor->FindComponentByClass<UDragonDashBounceSurfaceComponent>();
	return OutSurface && OutSurface->AllowsDragonDashBounce();
}

void UDragonDashMovementComponent::SetDragonDashState(EDragonDashState NewState)
{
	if (DragonDashState == NewState)
	{
		return;
	}

	const EDragonDashState PreviousState = DragonDashState;
	DragonDashState = NewState;
	OnDragonDashStateChanged.Broadcast(PreviousState, NewState);
}

void UDragonDashMovementComponent::AddDashTagAndCue()
{
	if (ADragonCharacter* DragonCharacter = Cast<ADragonCharacter>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = DragonCharacter->GetAbilitySystemComponent())
		{
			AbilitySystemComponent->AddLooseGameplayTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
		}
	}

	ExecuteCue(GetDashTuning().DashCueTag);
}

void UDragonDashMovementComponent::RemoveDashTag()
{
	if (ADragonCharacter* DragonCharacter = Cast<ADragonCharacter>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = DragonCharacter->GetAbilitySystemComponent())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
		}
	}
}

void UDragonDashMovementComponent::ExecuteCue(const FGameplayTag& CueTag) const
{
	if (!CueTag.IsValid())
	{
		return;
	}

	if (const ADragonCharacter* DragonCharacter = Cast<ADragonCharacter>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = DragonCharacter->GetAbilitySystemComponent())
		{
			FGameplayCueParameters CueParameters;
			CueParameters.Location = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : DragonCharacter->GetActorLocation();
			AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
		}
	}
}

float UDragonDashMovementComponent::GetWorldTimeSafe() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}
