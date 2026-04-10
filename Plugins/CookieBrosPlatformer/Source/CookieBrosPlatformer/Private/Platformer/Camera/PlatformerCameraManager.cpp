// Copyright Epic Games, Inc. All Rights Reserved.

#include "Platformer/Camera/PlatformerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/PlatformerDeveloperSettingsSubsystem.h"
#include "Engine/GameInstance.h"

void APlatformerCameraManager::BeginPlay()
{
	Super::BeginPlay();
	LoadAndApplyDeveloperCameraManagerSettings();
}

void APlatformerCameraManager::ResolveBaseCameraPose(const APawn& TargetPawn, FVector& OutLocation, FRotator& OutRotation, float& OutFOV) const
{
	OutLocation = TargetPawn.GetActorLocation() + FVector(0.0f, CurrentZoom, CameraZOffset);
	OutRotation = FallbackRotation;
	OutFOV = 65.0f;

	if (const UCameraComponent* CameraComp = TargetPawn.FindComponentByClass<UCameraComponent>())
	{
		OutLocation = CameraComp->GetComponentLocation();
		OutRotation = CameraComp->GetComponentRotation();
		OutFOV = CameraComp->FieldOfView;
		return;
	}

	if (const USpringArmComponent* SpringArm = TargetPawn.FindComponentByClass<USpringArmComponent>())
	{
		OutLocation = SpringArm->GetSocketLocation(USpringArmComponent::SocketName);
		OutRotation = SpringArm->GetSocketRotation(USpringArmComponent::SocketName);
	}
}

FVector2D APlatformerCameraManager::ResolveMovementDirection2D(const FVector& Velocity) const
{
	const FVector2D MovementInPlane(Velocity.X, Velocity.Z);
	return MovementInPlane.GetSafeNormal();
}

void APlatformerCameraManager::ApplyDeveloperCameraManagerSettings(const FDeveloperPlatformerCameraManagerSettings& DeveloperCameraManagerSettings)
{
	IdleSpeedThreshold = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold * 100.0f);
	HorizontalOffset = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset);
	HorizontalOffsetInterpSpeedStart = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart);
	HorizontalOffsetInterpSpeedEnd = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd);
	VerticalOffset = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset);
	VerticalOffsetInterpSpeed = FMath::Max(0.0f, DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed);

	CurrentHorizontalOffset = FMath::Clamp(CurrentHorizontalOffset, -HorizontalOffset, HorizontalOffset);
	CurrentVerticalOffset = FMath::Clamp(CurrentVerticalOffset, -VerticalOffset, VerticalOffset);
}

FDeveloperPlatformerCameraManagerSettings APlatformerCameraManager::CaptureDeveloperCameraManagerSettings() const
{
	FDeveloperPlatformerCameraManagerSettings DeveloperCameraManagerSettings;
	DeveloperCameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold = IdleSpeedThreshold / 100.0f;
	DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffset = HorizontalOffset;
	DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart = HorizontalOffsetInterpSpeedStart;
	DeveloperCameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd = HorizontalOffsetInterpSpeedEnd;
	DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffset = VerticalOffset;
	DeveloperCameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed = VerticalOffsetInterpSpeed;
	return DeveloperCameraManagerSettings;
}

void APlatformerCameraManager::LoadAndApplyDeveloperCameraManagerSettings()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem =
			GameInstance->GetSubsystem<UPlatformerDeveloperSettingsSubsystem>())
		{
			FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
			if (DeveloperSettingsSubsystem->TryLoadCurrentSnapshot(DeveloperSettingsSnapshot))
			{
				ApplyDeveloperCameraManagerSettings(DeveloperSettingsSnapshot.CameraManagerSettings);
			}
		}
	}
}

void APlatformerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	APawn* TargetPawn = Cast<APawn>(OutVT.Target);
	if (!IsValid(TargetPawn))
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	FVector BaseCameraLocation = FVector::ZeroVector;
	FRotator BaseCameraRotation = FRotator::ZeroRotator;
	float TargetFOV = 65.0f;
	ResolveBaseCameraPose(*TargetPawn, BaseCameraLocation, BaseCameraRotation, TargetFOV);

	OutVT.POV.Rotation = BaseCameraRotation;
	OutVT.POV.FOV = TargetFOV;

	const FVector Velocity = TargetPawn->GetVelocity();
	float TargetHorizontalLookAhead = 0.0f;
	float TargetVerticalLookAhead = 0.0f;

	if (!Velocity.IsNearlyZero(IdleSpeedThreshold))
	{
		const FVector2D MovementDirection = ResolveMovementDirection2D(Velocity);
		TargetHorizontalLookAhead = MovementDirection.X * HorizontalOffset;
		TargetVerticalLookAhead = MovementDirection.Y * VerticalOffset;
	}

	const float HorizontalInterpSpeed = FMath::IsNearlyZero(TargetHorizontalLookAhead, KINDA_SMALL_NUMBER)
		? HorizontalOffsetInterpSpeedEnd
		: HorizontalOffsetInterpSpeedStart;
	CurrentHorizontalOffset = FMath::FInterpTo(CurrentHorizontalOffset, TargetHorizontalLookAhead, DeltaTime, HorizontalInterpSpeed);
	CurrentVerticalOffset = FMath::FInterpTo(CurrentVerticalOffset, TargetVerticalLookAhead, DeltaTime, VerticalOffsetInterpSpeed);

	OutVT.POV.Location = BaseCameraLocation + FVector(CurrentHorizontalOffset, 0.0f, CurrentVerticalOffset);
}
