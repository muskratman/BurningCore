#include "Character/SideViewMovementComponent.h"

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
}

void USideViewMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	TargetDepthY = LockedDepthY;
	SetPlaneConstraintOrigin(FVector(0.f, LockedDepthY, 0.f));
}

void USideViewMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	EnforceDepthLock(DeltaTime);
	
	if (IsFalling() && Velocity.Z > -100.f && Velocity.Z < 100.f)
	{
		// Apply JumpApexGravityMultiplier logic theoretically
		GravityScale = JumpApexGravityMultiplier;
	}
	else
	{
		GravityScale = 1.0f; // Reset normally
	}
}

void USideViewMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);
}

void USideViewMovementComponent::SetDepthLane(float NewY, float TransitionTime)
{
	TargetDepthY = NewY;
	LockedDepthY = TargetDepthY;
	SetPlaneConstraintOrigin(FVector(0.f, LockedDepthY, 0.f));
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
