#include "Character/SideViewMovementComponent.h"

#include "Traversal/PlatformerTraversalMovementComponent.h"

USideViewMovementComponent::USideViewMovementComponent()
{
	bConstrainToPlane = true;
	bSnapToPlaneAtStart = true;
	SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f)); // Lock Y axis
	SetPlaneConstraintOrigin(FVector::ZeroVector);

	AirControl = 0.85f; // Strong air control
	BrakingFrictionFactor = 2.0f;
	FallingLateralFriction = 3.0f;

	TargetDepthY = LockedDepthY;
	DefaultGravityScale = GravityScale;
	bHasExternalGravityScaleOverride = false;
	ExternalGravityScaleOverride = GravityScale;
}

void USideViewMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	TargetDepthY = LockedDepthY;
	SetPlaneConstraintOrigin(FVector(0.f, LockedDepthY, 0.f));
	DefaultGravityScale = GravityScale;
}

void USideViewMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	EnforceDepthLock(DeltaTime);

	float DesiredGravityScale = DefaultGravityScale;
	if (IsFalling() && Velocity.Z > -100.f && Velocity.Z < 100.f)
	{
		DesiredGravityScale = DefaultGravityScale * JumpApexGravityMultiplier;
	}

	if (bHasExternalGravityScaleOverride)
	{
		DesiredGravityScale = ExternalGravityScaleOverride;
	}

	GravityScale = DesiredGravityScale;
}

void USideViewMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (ShouldSuppressFallingJumpBraking())
	{
		const float SavedBrakingFrictionFactor = BrakingFrictionFactor;
		BrakingFrictionFactor = 0.0f;
		Super::CalcVelocity(DeltaTime, Friction, bFluid, 0.0f);
		BrakingFrictionFactor = SavedBrakingFrictionFactor;
		UpdateJumpHorizontalSpeedProtection();
		return;
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	UpdateJumpHorizontalSpeedProtection();
}

void USideViewMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);
}

void USideViewMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode != MOVE_Falling)
	{
		ClearJumpHorizontalSpeedProtection();
	}
}

void USideViewMovementComponent::SetDepthLane(float NewY, float TransitionTime)
{
	TargetDepthY = NewY;
	LockedDepthY = TargetDepthY;
	SetPlaneConstraintOrigin(FVector(0.f, LockedDepthY, 0.f));
}

void USideViewMovementComponent::SetExternalGravityScaleOverride(float NewGravityScale)
{
	bHasExternalGravityScaleOverride = true;
	ExternalGravityScaleOverride = FMath::Max(NewGravityScale, 0.0f);
	GravityScale = ExternalGravityScaleOverride;
}

void USideViewMovementComponent::ClearExternalGravityScaleOverride()
{
	bHasExternalGravityScaleOverride = false;
	ExternalGravityScaleOverride = DefaultGravityScale;
	GravityScale = DefaultGravityScale;
}

void USideViewMovementComponent::SetBaseGravityScale(float NewGravityScale)
{
	DefaultGravityScale = FMath::Max(NewGravityScale, 0.0f);

	if (!bHasExternalGravityScaleOverride)
	{
		GravityScale = DefaultGravityScale;
	}
}

void USideViewMovementComponent::EnforceDepthLock(float DeltaTime)
{
	if (UpdatedComponent)
	{
		FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
		if (!FMath::IsNearlyEqual(CurrentLocation.Y, LockedDepthY, 1.0f))
		{
			CurrentLocation.Y = LockedDepthY;
			UpdatedComponent->SetWorldLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void USideViewMovementComponent::NotifyJumpHorizontalSpeedApplied(float HorizontalSpeed, float DirectionSign)
{
	if (HorizontalSpeed <= MaxWalkSpeed || FMath::IsNearlyZero(DirectionSign))
	{
		ClearJumpHorizontalSpeedProtection();
		return;
	}

	bHasJumpHorizontalSpeedProtection = true;
	ProtectedJumpDirectionSign = FMath::Sign(DirectionSign);
	UpdateJumpHorizontalSpeedProtection();
}

bool USideViewMovementComponent::ShouldSuppressFallingJumpBraking() const
{
	if (!bHasJumpHorizontalSpeedProtection || !IsFalling())
	{
		return false;
	}

	if (FMath::IsNearlyZero(ProtectedJumpDirectionSign))
	{
		return false;
	}

	if (!IsJumpHorizontalSpeedInputHeld())
	{
		return false;
	}

	const float CurrentAlignedSpeed = Velocity.X * ProtectedJumpDirectionSign;
	return CurrentAlignedSpeed > MaxWalkSpeed;
}

bool USideViewMovementComponent::IsJumpHorizontalSpeedInputHeld() const
{
	if (FMath::IsNearlyZero(ProtectedJumpDirectionSign))
	{
		return false;
	}

	float HorizontalInput = 0.0f;

	if (const UPlatformerTraversalMovementComponent* TraversalMovementComponent = Cast<UPlatformerTraversalMovementComponent>(this))
	{
		HorizontalInput = TraversalMovementComponent->GetTraversalInputVector().X;
		if (FMath::IsNearlyZero(HorizontalInput))
		{
			return false;
		}

		return FMath::Sign(HorizontalInput) == ProtectedJumpDirectionSign;
	}

	HorizontalInput = Acceleration.X;

	if (FMath::IsNearlyZero(HorizontalInput))
	{
		return false;
	}

	return FMath::Sign(HorizontalInput) == ProtectedJumpDirectionSign;
}

void USideViewMovementComponent::ClearJumpHorizontalSpeedProtection()
{
	bHasJumpHorizontalSpeedProtection = false;
	ProtectedJumpDirectionSign = 0.0f;
}

void USideViewMovementComponent::UpdateJumpHorizontalSpeedProtection()
{
	if (!bHasJumpHorizontalSpeedProtection)
	{
		return;
	}

	if (!IsFalling() || FMath::IsNearlyZero(ProtectedJumpDirectionSign))
	{
		ClearJumpHorizontalSpeedProtection();
		return;
	}

	if (!IsJumpHorizontalSpeedInputHeld())
	{
		ClearJumpHorizontalSpeedProtection();
		return;
	}

	const float CurrentAlignedSpeed = Velocity.X * ProtectedJumpDirectionSign;
	if (CurrentAlignedSpeed <= MaxWalkSpeed)
	{
		ClearJumpHorizontalSpeedProtection();
	}
}
