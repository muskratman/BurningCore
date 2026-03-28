// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

void ASideScrollingCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// ensure the view target is a pawn
	APawn* TargetPawn = Cast<APawn>(OutVT.Target);

	// is our target valid?
	if (IsValid(TargetPawn))
	{
		// Default values
		FRotator TargetRotation = FRotator(0.0f, -90.0f, 0.0f);
		float TargetFOV = 65.0f;
		FVector IdealCameraLocation = TargetPawn->GetActorLocation() + FVector(0.0f, CurrentZoom, CameraZOffset);

		// 1. Try to get the "Ideal" position from the Pawn's components
		if (USpringArmComponent* SpringArm = TargetPawn->FindComponentByClass<USpringArmComponent>())
		{
			// The SpringArm already knows where the camera "should" be based on its settings in BP
			IdealCameraLocation = SpringArm->GetSocketLocation(USpringArmComponent::SocketName);
			TargetRotation = SpringArm->GetSocketRotation(USpringArmComponent::SocketName);
		}
		
		if (UCameraComponent* CameraComp = TargetPawn->FindComponentByClass<UCameraComponent>())
		{
			TargetFOV = CameraComp->FieldOfView;
			// If there's no SpringArm but a Camera exists, we use its relative offset as well
			if (!TargetPawn->FindComponentByClass<USpringArmComponent>())
			{
				IdealCameraLocation = CameraComp->GetComponentLocation();
				TargetRotation = CameraComp->GetComponentRotation();
			}
		}

		// 2. Set the view target FOV and rotation
		OutVT.POV.Rotation = TargetRotation;
		OutVT.POV.FOV = TargetFOV;

		// 3. Cache the current location for interpolation
		FVector CurrentCameraLocation = GetCameraLocation();
		FVector CurrentActorLocation = TargetPawn->GetActorLocation();

		// 4. Do first-time setup
		if (bSetup)
		{
			bSetup = false;
			OutVT.POV.Location = IdealCameraLocation;
			CurrentZ = OutVT.POV.Location.Z;
			return;
		}

		// check if the camera needs to update its height
		bool bZUpdate = false;

		// is the character moving vertically?
		if (FMath::IsNearlyZero(TargetPawn->GetVelocity().Z))
		{
			// determine if we need to do a height update
			bZUpdate = FMath::IsNearlyEqual(CurrentZ, CurrentCameraLocation.Z, 25.0f);

		} else {

			// run a trace below the character to determine if we need to do a height update
			FHitResult OutHit;

			const FVector End = CurrentActorLocation + FVector(0.0f, 0.0f, -1000.0f);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(TargetPawn);

			// only update height if we're not about to hit ground
			bZUpdate = !GetWorld()->LineTraceSingleByChannel(OutHit, CurrentActorLocation, End, ECC_Visibility, QueryParams);

		}

		// do we need to do a height update?
		if (bZUpdate)
		{
			// set the height goal from the ideal camera location (respects SpringArm/Camera BP settings)
			CurrentZ = IdealCameraLocation.Z;

		} else {

			// are we close enough to the target height?
			if (FMath::IsNearlyEqual(CurrentZ, IdealCameraLocation.Z, 100.0f))
			{
				// set the height goal from the ideal camera location
				CurrentZ = IdealCameraLocation.Z;

			} else {

				// blend the height towards the ideal camera location
				CurrentZ = FMath::FInterpTo(CurrentZ, IdealCameraLocation.Z, DeltaTime, CameraZInterpSpeed);
				
			}

		}

		// 5. Calculate look-ahead horizontal offset
		float FacingDir = TargetPawn->GetActorForwardVector().X;
		float TargetHorizontalOffset = 0.0f;

		// If facing a clear direction, set the target offset
		if (FMath::Abs(FacingDir) > 0.1f)
		{
			TargetHorizontalOffset = (FacingDir > 0.0f) ? HorizontalOffset : -HorizontalOffset;
		}
		else
		{
			// If no clear direction, keep the current offset to avoid snapping to center
			TargetHorizontalOffset = CurrentHorizontalOffset;
		}

		CurrentHorizontalOffset = FMath::FInterpTo(CurrentHorizontalOffset, TargetHorizontalOffset, DeltaTime, HorizontalOffsetInterpSpeed);

		// clamp the X axis to the min and max camera bounds, including the look-ahead offset
		float TargetX = FMath::Clamp(IdealCameraLocation.X + CurrentHorizontalOffset, CameraXMinBounds, CameraXMaxBounds);

		// blend towards the new camera location and update the output
		// We keep the Ideal Y (depth) from the SpringArm, but clamp X and smooth Z
		FVector TargetCameraLocation(TargetX, IdealCameraLocation.Y, CurrentZ);

		OutVT.POV.Location = FMath::VInterpTo(CurrentCameraLocation, TargetCameraLocation, DeltaTime, CameraInterpSpeed);
	}
}