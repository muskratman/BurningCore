#include "Character/SideViewMovementComponent.h"

#include "Character/PlatformerCharacterBase.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"

namespace
{
	constexpr float ChangeDirectionYawDegrees = 180.0f;
}

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

	bOrientRotationToMovement = false;
	bUseControllerDesiredRotation = false;
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

bool USideViewMovementComponent::RequestTurnAround()
{
	if (bTurning) { return false; }
	bTurning = true;
	bFacingRight = !bFacingRight;
	return true;
}

void USideViewMovementComponent::SetChangeDirectionSpeed(float NewChangeDirectionSpeed)
{
	if (NewChangeDirectionSpeed <= KINDA_SMALL_NUMBER)
	{
		TurnAroundSeconds = 0.0f;
		return;
	}

	TurnAroundSeconds = ChangeDirectionYawDegrees / NewChangeDirectionSpeed;
}

float USideViewMovementComponent::GetChangeDirectionSpeed() const
{
	const float EffectiveTurnAroundSeconds = FMath::Max(TurnAroundSeconds, 0.001f);
	return ChangeDirectionYawDegrees / EffectiveTurnAroundSeconds;
}

void USideViewMovementComponent::AddInputVector(FVector WorldVector, bool bForce)
{
	bool bIsRightMotion = WorldVector.X > 0.0f;
	
	// Only intervene if movement is primarily along X axis
	if (FMath::Abs(WorldVector.X) > KINDA_SMALL_NUMBER)
	{
		if (bIsRightMotion == bFacingRight)
		{
			Super::AddInputVector(WorldVector, bForce);
		}
		else if (!bTurning)
		{
			RequestTurnAround();
		}
	}
	else
	{
		Super::AddInputVector(WorldVector, bForce);
	}
}

void USideViewMovementComponent::StartNewPhysics(float deltaTime, int32 Iterations)
{
	if (CharacterOwner)
	{
		APlatformerCharacterBase* PlatformerChar = Cast<APlatformerCharacterBase>(CharacterOwner);
		const bool bIsInLadderPose =
			PlatformerChar && (PlatformerChar->IsOnLadder() || PlatformerChar->IsLadderTopFinishActive());

		if (!bIsInLadderPose)
		{
			FRotator CurrentRot = UpdatedComponent->GetComponentRotation();
			FRotator TargetRot = bFacingRight ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);

			float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);

			if (FMath::Abs(DeltaYaw) > KINDA_SMALL_NUMBER)
			{
				float TurnRate = ChangeDirectionYawDegrees / FMath::Max(TurnAroundSeconds, 0.001f);
				
				if (FMath::Abs(DeltaYaw) > 170.0f)
				{
					// Keep 180-degree turns on the gameplay-facing side of the camera plane.
					float Sign = bFacingRight ? -1.0f : 1.0f;
					float Step = Sign * TurnRate * deltaTime;
					CurrentRot.Yaw += Step;
					CurrentRot.Normalize();
				}
				else
				{
					CurrentRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, deltaTime, TurnRate);
				}

				FHitResult Hit(1.0f);
				SafeMoveUpdatedComponent(FVector::ZeroVector, CurrentRot.Quaternion(), false, Hit);
			}
			else
			{
				bTurning = false;
			}
		}
	}

	Super::StartNewPhysics(deltaTime, Iterations);
}
