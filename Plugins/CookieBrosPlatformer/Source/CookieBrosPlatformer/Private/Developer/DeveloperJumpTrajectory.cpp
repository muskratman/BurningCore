#include "Developer/DeveloperJumpTrajectory.h"

#include "AbilitySystemComponent.h"
#include "Character/PlatformerCharacterBase.h"
#include "Character/SideViewMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GAS/Abilities/GA_PlatformerJump.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
struct FJumpTrajectoryPreviewSample
{
	float Time = 0.0f;
	FVector Position = FVector::ZeroVector;
};

float SimulateJumpTrajectoryBrakingVelocity(
	float Velocity,
	float DeltaTime,
	float Friction,
	float BrakingDeceleration)
{
	if (FMath::IsNearlyZero(Velocity) || DeltaTime <= UE_KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float FrictionTerm = FMath::Max(0.0f, Friction);
	const float BrakingTerm = FMath::Max(0.0f, BrakingDeceleration);
	if (FMath::IsNearlyZero(FrictionTerm) && FMath::IsNearlyZero(BrakingTerm))
	{
		return Velocity;
	}

	const float ReverseAcceleration = BrakingTerm * -FMath::Sign(Velocity);
	const float UpdatedVelocity = Velocity + (((-FrictionTerm) * Velocity) + ReverseAcceleration) * DeltaTime;
	if ((UpdatedVelocity * Velocity) <= 0.0f)
	{
		return 0.0f;
	}

	return UpdatedVelocity;
}

float ResolveJumpTrajectoryAirControl(
	const UCharacterMovementComponent* MovementComponent,
	float HorizontalVelocity)
{
	if (!MovementComponent)
	{
		return 0.0f;
	}

	float TickAirControl = FMath::Max(MovementComponent->AirControl, 0.0f);
	if (TickAirControl > 0.0f
		&& MovementComponent->AirControlBoostMultiplier > 0.0f
		&& FMath::Square(FMath::Abs(HorizontalVelocity)) < FMath::Square(MovementComponent->AirControlBoostVelocityThreshold))
	{
		TickAirControl = FMath::Min(1.0f, MovementComponent->AirControlBoostMultiplier * TickAirControl);
	}

	return TickAirControl;
}

float SimulateJumpTrajectoryHorizontalVelocity(
	const UCharacterMovementComponent* MovementComponent,
	float CurrentHorizontalVelocity,
	float DeltaTime,
	bool bSimulateForwardInput,
	bool bSuppressBraking)
{
	if (!MovementComponent || DeltaTime <= UE_KINDA_SMALL_NUMBER)
	{
		return FMath::Max(CurrentHorizontalVelocity, 0.0f);
	}

	const float MaxWalkSpeed = FMath::Max(MovementComponent->MaxWalkSpeed, 0.0f);
	const float MaxAcceleration = FMath::Max(MovementComponent->GetMaxAcceleration(), 0.0f);
	const float TickAirControl = bSimulateForwardInput
		? ResolveJumpTrajectoryAirControl(MovementComponent, CurrentHorizontalVelocity)
		: 0.0f;
	const float EffectiveForwardAcceleration = FMath::Min(MaxAcceleration, MaxAcceleration * TickAirControl);
	const bool bHasForwardAcceleration = EffectiveForwardAcceleration > UE_KINDA_SMALL_NUMBER;
	const bool bVelocityOverMaxSpeed = CurrentHorizontalVelocity > MaxWalkSpeed;

	float UpdatedHorizontalVelocity = CurrentHorizontalVelocity;
	if (!bHasForwardAcceleration || bVelocityOverMaxSpeed)
	{
		const float AppliedBrakingFriction = bSuppressBraking
			? 0.0f
			: FMath::Max(MovementComponent->FallingLateralFriction, 0.0f) * FMath::Max(MovementComponent->BrakingFrictionFactor, 0.0f);
		const float AppliedBrakingDeceleration = bSuppressBraking
			? 0.0f
			: FMath::Max(MovementComponent->BrakingDecelerationFalling, 0.0f);
		const float PreviousHorizontalVelocity = UpdatedHorizontalVelocity;
		UpdatedHorizontalVelocity = SimulateJumpTrajectoryBrakingVelocity(
			UpdatedHorizontalVelocity,
			DeltaTime,
			AppliedBrakingFriction,
			AppliedBrakingDeceleration);

		if (bVelocityOverMaxSpeed
			&& UpdatedHorizontalVelocity < MaxWalkSpeed
			&& bHasForwardAcceleration
			&& PreviousHorizontalVelocity > 0.0f)
		{
			UpdatedHorizontalVelocity = MaxWalkSpeed;
		}
	}

	if (bHasForwardAcceleration)
	{
		const float MaxInputSpeed = bVelocityOverMaxSpeed
			? UpdatedHorizontalVelocity
			: MaxWalkSpeed;
		UpdatedHorizontalVelocity = FMath::Min(
			UpdatedHorizontalVelocity + (EffectiveForwardAcceleration * DeltaTime),
			MaxInputSpeed);
	}

	return FMath::Max(UpdatedHorizontalVelocity, 0.0f);
}

FVector SampleJumpTrajectoryPositionAtTime(
	const TArray<FJumpTrajectoryPreviewSample>& Samples,
	float SampleTime)
{
	if (Samples.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	if (SampleTime <= Samples[0].Time)
	{
		return Samples[0].Position;
	}

	for (int32 SampleIndex = 1; SampleIndex < Samples.Num(); ++SampleIndex)
	{
		const FJumpTrajectoryPreviewSample& PreviousSample = Samples[SampleIndex - 1];
		const FJumpTrajectoryPreviewSample& CurrentSample = Samples[SampleIndex];
		if (SampleTime <= CurrentSample.Time)
		{
			const float InterpAlpha = FMath::GetRangePct(PreviousSample.Time, CurrentSample.Time, SampleTime);
			return FMath::Lerp(PreviousSample.Position, CurrentSample.Position, InterpAlpha);
		}
	}

	return Samples.Last().Position;
}
}

ADeveloperJumpTrajectory::ADeveloperJumpTrajectory()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	StandingJumpTrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("StandingJumpTrajectorySpline"));
	StandingJumpTrajectorySpline->SetupAttachment(Root);
	StandingJumpTrajectorySpline->SetClosedLoop(false);
	StandingJumpTrajectorySpline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StandingJumpTrajectorySpline->SetDrawDebug(false);
	StandingJumpTrajectorySpline->SetHiddenInGame(true);
	StandingJumpTrajectorySpline->SetVisibility(false);

	MaxWalkSpeedJumpTrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("MaxWalkSpeedJumpTrajectorySpline"));
	MaxWalkSpeedJumpTrajectorySpline->SetupAttachment(Root);
	MaxWalkSpeedJumpTrajectorySpline->SetClosedLoop(false);
	MaxWalkSpeedJumpTrajectorySpline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MaxWalkSpeedJumpTrajectorySpline->SetDrawDebug(false);
	MaxWalkSpeedJumpTrajectorySpline->SetHiddenInGame(true);
	MaxWalkSpeedJumpTrajectorySpline->SetVisibility(false);

	SetActorEnableCollision(false);
}

void ADeveloperJumpTrajectory::BeginPlay()
{
	Super::BeginPlay();

	ApplyJumpTrajectorySplineStyle();
	ApplyJumpTrajectorySplineVisibility();
}

void ADeveloperJumpTrajectory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawJumpTrajectoryPreview();
}

void ADeveloperJumpTrajectory::InitializeAttachedPreview(APlatformerCharacterBase* InSourceCharacter)
{
	SourceCharacter = InSourceCharacter;
	bIsSnapshotCopy = false;
}

void ADeveloperJumpTrajectory::SetShowJumpTrajectoryPreview(bool bInShowJumpTrajectoryPreview)
{
	bShowJumpTrajectoryPreview = bInShowJumpTrajectoryPreview;
	ApplyJumpTrajectorySplineVisibility();
}

void ADeveloperJumpTrajectory::RefreshTrajectoryPreview()
{
	ApplyJumpTrajectorySplineStyle();
	ApplyJumpTrajectorySplineVisibility();

	if (!SourceCharacter.IsValid())
	{
		return;
	}

	RefreshJumpTrajectorySpline(StandingJumpTrajectorySpline, 0.0f, true);

	if (MaxWalkSpeedJumpTrajectorySpline)
	{
		MaxWalkSpeedJumpTrajectorySpline->ClearSplinePoints(true);
	}
}

ADeveloperJumpTrajectory* ADeveloperJumpTrajectory::SpawnSnapshotCopy() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADeveloperJumpTrajectory* SnapshotTrajectory = World->SpawnActor<ADeveloperJumpTrajectory>(
		GetClass(),
		GetActorTransform(),
		SpawnParameters);
	if (!SnapshotTrajectory)
	{
		return nullptr;
	}

	SnapshotTrajectory->bIsSnapshotCopy = true;
	SnapshotTrajectory->SourceCharacter = nullptr;
	SnapshotTrajectory->bShowJumpTrajectoryPreview = true;
	SnapshotTrajectory->JumpTrajectoryPreviewPointCount = JumpTrajectoryPreviewPointCount;
	SnapshotTrajectory->JumpTrajectoryPreviewSimulationStep = JumpTrajectoryPreviewSimulationStep;
	SnapshotTrajectory->JumpTrajectoryPreviewMaxDuration = JumpTrajectoryPreviewMaxDuration;
	SnapshotTrajectory->JumpTrajectoryPreviewSplineWidth = JumpTrajectoryPreviewSplineWidth;
	SnapshotTrajectory->JumpTrajectoryPreviewLineThickness = JumpTrajectoryPreviewLineThickness;
	SnapshotTrajectory->StandingJumpTrajectoryPreviewColor = StandingJumpTrajectoryPreviewColor;
	SnapshotTrajectory->MaxWalkSpeedJumpTrajectoryPreviewColor = MaxWalkSpeedJumpTrajectoryPreviewColor;
	SnapshotTrajectory->ApplyJumpTrajectorySplineStyle();
	SnapshotTrajectory->ApplyJumpTrajectorySplineVisibility();
	CopySplinePointsTo(SnapshotTrajectory);

	return SnapshotTrajectory;
}

const UGA_PlatformerJump* ADeveloperJumpTrajectory::FindGrantedJumpAbility() const
{
	const APlatformerCharacterBase* ResolvedCharacter = SourceCharacter.Get();
	if (!ResolvedCharacter)
	{
		return nullptr;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ResolvedCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = AbilitySystemComponent->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities)
	{
		if (const UGA_PlatformerJump* JumpAbility = Cast<UGA_PlatformerJump>(AbilitySpec.Ability))
		{
			return JumpAbility;
		}
	}

	return nullptr;
}

void ADeveloperJumpTrajectory::RefreshJumpTrajectorySpline(
	USplineComponent* TargetSpline,
	float PreJumpHorizontalSpeed,
	bool bSimulateForwardInput) const
{
	if (!TargetSpline)
	{
		return;
	}

	const TArray<FVector> PreviewPoints = BuildJumpTrajectoryPreviewPoints(
		PreJumpHorizontalSpeed,
		bSimulateForwardInput);
	if (PreviewPoints.Num() < 2)
	{
		TargetSpline->ClearSplinePoints(true);
		return;
	}

	TargetSpline->SetSplinePoints(PreviewPoints, ESplineCoordinateSpace::Local, false);
	for (int32 PointIndex = 0; PointIndex < PreviewPoints.Num(); ++PointIndex)
	{
		TargetSpline->SetSplinePointType(PointIndex, ESplinePointType::Linear, false);
	}

	TargetSpline->UpdateSpline();
}

TArray<FVector> ADeveloperJumpTrajectory::BuildJumpTrajectoryPreviewPoints(
	float PreJumpHorizontalSpeed,
	bool bSimulateForwardInput) const
{
	TArray<FVector> PreviewPoints;

	const APlatformerCharacterBase* ResolvedCharacter = SourceCharacter.Get();
	if (!ResolvedCharacter)
	{
		return PreviewPoints;
	}

	const UCharacterMovementComponent* MovementComponent = ResolvedCharacter->GetCharacterMovement();
	const UCapsuleComponent* CharacterCapsule = ResolvedCharacter->GetCapsuleComponent();
	if (!MovementComponent || !CharacterCapsule || JumpTrajectoryPreviewPointCount < 2)
	{
		return PreviewPoints;
	}

	const float TimeStep = FMath::Max(JumpTrajectoryPreviewSimulationStep, UE_KINDA_SMALL_NUMBER);
	const float MaxDuration = FMath::Max(JumpTrajectoryPreviewMaxDuration, TimeStep);
	const int32 MaxSimulationSteps = FMath::Max(FMath::CeilToInt(MaxDuration / TimeStep), 1);
	const float CapsuleHalfHeight = CharacterCapsule->GetScaledCapsuleHalfHeight();
	const USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent);
	const UGA_PlatformerJump* JumpAbility = FindGrantedJumpAbility();

	const float JumpMaxHoldTime = JumpAbility ? FMath::Max(JumpAbility->GetJumpMaxHoldTime(), 0.0f) : 0.0f;
	const float InitialHorizontalSpeed = JumpAbility
		? JumpAbility->ResolvePreviewHorizontalJumpSpeed(ResolvedCharacter, FMath::Max(PreJumpHorizontalSpeed, 0.0f), 1.0f)
		: FMath::Max(PreJumpHorizontalSpeed, 0.0f);
	const float JumpZVelocity = FMath::Max(MovementComponent->JumpZVelocity, 0.0f);
	const float BaseGravityScale = SideViewMovementComponent
		? FMath::Max(SideViewMovementComponent->GetBaseGravityScale(), 0.0f)
		: FMath::Max(MovementComponent->GravityScale, 0.0f);
	const float ApexGravityMultiplier = SideViewMovementComponent
		? FMath::Max(SideViewMovementComponent->JumpApexGravityMultiplier, 0.0f)
		: 1.0f;
	const float WorldGravityZ = GetWorld() ? GetWorld()->GetGravityZ() : -980.0f;
	const float BaseGravityZ = WorldGravityZ * BaseGravityScale;
	const float MaxWalkSpeed = FMath::Max(MovementComponent->MaxWalkSpeed, 0.0f);

	float SimulatedTime = 0.0f;
	float JumpForceTimeRemaining = JumpMaxHoldTime;
	float HorizontalVelocity = InitialHorizontalSpeed;
	float VerticalVelocity = JumpZVelocity;
	FVector SimulatedDisplacement = FVector::ZeroVector;
	bool bHasJumpHorizontalSpeedProtection = InitialHorizontalSpeed > MaxWalkSpeed;

	TArray<FJumpTrajectoryPreviewSample> TrajectorySamples;
	TrajectorySamples.Reserve(MaxSimulationSteps + 1);
	TrajectorySamples.Add({0.0f, FVector(0.0f, 0.0f, -CapsuleHalfHeight)});

	for (int32 StepIndex = 0; StepIndex < MaxSimulationSteps; ++StepIndex)
	{
		const float PreviousHorizontalVelocity = HorizontalVelocity;
		float PreviousVerticalVelocity = VerticalVelocity;
		if (JumpForceTimeRemaining > 0.0f)
		{
			PreviousVerticalVelocity = FMath::Max(PreviousVerticalVelocity, JumpZVelocity);
		}

		const bool bSuppressBraking = bHasJumpHorizontalSpeedProtection && PreviousHorizontalVelocity > MaxWalkSpeed;
		HorizontalVelocity = SimulateJumpTrajectoryHorizontalVelocity(
			MovementComponent,
			PreviousHorizontalVelocity,
			TimeStep,
			bSimulateForwardInput,
			bSuppressBraking);

		if (bHasJumpHorizontalSpeedProtection && HorizontalVelocity <= MaxWalkSpeed)
		{
			bHasJumpHorizontalSpeedProtection = false;
		}

		float CurrentGravityZ = BaseGravityZ;
		if (PreviousVerticalVelocity > -100.0f && PreviousVerticalVelocity < 100.0f)
		{
			CurrentGravityZ = BaseGravityZ * ApexGravityMultiplier;
		}

		VerticalVelocity = PreviousVerticalVelocity + (CurrentGravityZ * TimeStep);

		if (JumpForceTimeRemaining > 0.0f)
		{
			JumpForceTimeRemaining = FMath::Max(JumpForceTimeRemaining - TimeStep, 0.0f);
		}

		SimulatedDisplacement += FVector(
			0.5f * (PreviousHorizontalVelocity + HorizontalVelocity) * TimeStep,
			0.0f,
			0.5f * (PreviousVerticalVelocity + VerticalVelocity) * TimeStep);
		SimulatedTime += TimeStep;

		TrajectorySamples.Add({
			SimulatedTime,
			FVector(SimulatedDisplacement.X, 0.0f, SimulatedDisplacement.Z - CapsuleHalfHeight)
		});

		if (SimulatedDisplacement.Z <= 0.0f && VerticalVelocity <= 0.0f)
		{
			if (TrajectorySamples.Num() >= 2)
			{
				FJumpTrajectoryPreviewSample& LandingSample = TrajectorySamples.Last();
				const FJumpTrajectoryPreviewSample& PreviousSample = TrajectorySamples[TrajectorySamples.Num() - 2];
				if (!FMath::IsNearlyEqual(PreviousSample.Position.Z, LandingSample.Position.Z))
				{
					const float LandingAlpha = FMath::Clamp(
						FMath::GetRangePct(
							PreviousSample.Position.Z,
							LandingSample.Position.Z,
							-CapsuleHalfHeight),
						0.0f,
						1.0f);
					LandingSample.Position = FMath::Lerp(PreviousSample.Position, LandingSample.Position, LandingAlpha);
					LandingSample.Time = FMath::Lerp(PreviousSample.Time, LandingSample.Time, LandingAlpha);
				}

				LandingSample.Position.Z = -CapsuleHalfHeight;
			}

			break;
		}
	}

	if (TrajectorySamples.Num() == 0)
	{
		return PreviewPoints;
	}

	const float FinalTime = FMath::Max(TrajectorySamples.Last().Time, 0.0f);
	PreviewPoints.Reserve(JumpTrajectoryPreviewPointCount);

	for (int32 PointIndex = 0; PointIndex < JumpTrajectoryPreviewPointCount; ++PointIndex)
	{
		const float PointAlpha = JumpTrajectoryPreviewPointCount > 1
			? static_cast<float>(PointIndex) / static_cast<float>(JumpTrajectoryPreviewPointCount - 1)
			: 0.0f;
		const float SampleTime = FinalTime * PointAlpha;
		PreviewPoints.Add(SampleJumpTrajectoryPositionAtTime(TrajectorySamples, SampleTime));
	}

	return PreviewPoints;
}

void ADeveloperJumpTrajectory::ApplyJumpTrajectorySplineStyle()
{
	auto ApplyStyleToSpline = [this](USplineComponent* JumpTrajectorySpline, const FLinearColor& PreviewColor)
	{
		if (!JumpTrajectorySpline)
		{
			return;
		}

		JumpTrajectorySpline->SetUnselectedSplineSegmentColor(PreviewColor);
		JumpTrajectorySpline->SetSelectedSplineSegmentColor(PreviewColor);
		JumpTrajectorySpline->ScaleVisualizationWidth = JumpTrajectoryPreviewSplineWidth;
	};

	ApplyStyleToSpline(StandingJumpTrajectorySpline, StandingJumpTrajectoryPreviewColor);
	ApplyStyleToSpline(MaxWalkSpeedJumpTrajectorySpline, MaxWalkSpeedJumpTrajectoryPreviewColor);
}

void ADeveloperJumpTrajectory::ApplyJumpTrajectorySplineVisibility() const
{
	const bool bShouldShowJumpTrajectoryPreview = bShowJumpTrajectoryPreview && GetWorld() && GetWorld()->IsGameWorld();

	auto ApplyVisibilityToSpline = [](USplineComponent* JumpTrajectorySpline, bool bShouldShowSpline)
	{
		if (!JumpTrajectorySpline)
		{
			return;
		}

		JumpTrajectorySpline->SetDrawDebug(bShouldShowSpline);
		JumpTrajectorySpline->SetHiddenInGame(!bShouldShowSpline);
		JumpTrajectorySpline->SetVisibility(bShouldShowSpline);
	};

	ApplyVisibilityToSpline(StandingJumpTrajectorySpline, bShouldShowJumpTrajectoryPreview);
	ApplyVisibilityToSpline(MaxWalkSpeedJumpTrajectorySpline, false);
}

void ADeveloperJumpTrajectory::DrawJumpTrajectoryPreview() const
{
	if (!bShowJumpTrajectoryPreview)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	DrawJumpTrajectorySplinePreview(
		StandingJumpTrajectorySpline,
		StandingJumpTrajectoryPreviewColor.ToFColor(true));
}

void ADeveloperJumpTrajectory::DrawJumpTrajectorySplinePreview(
	const USplineComponent* JumpTrajectorySpline,
	const FColor& PreviewColor) const
{
	if (!JumpTrajectorySpline || JumpTrajectorySpline->GetNumberOfSplinePoints() < 2)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float SplineLength = JumpTrajectorySpline->GetSplineLength();
	const int32 DebugSegmentCount = FMath::Max(JumpTrajectoryPreviewPointCount * 4, 16);
	FVector PreviousWorldLocation = JumpTrajectorySpline->GetLocationAtDistanceAlongSpline(
		0.0f,
		ESplineCoordinateSpace::World);

	for (int32 SegmentIndex = 1; SegmentIndex <= DebugSegmentCount; ++SegmentIndex)
	{
		const float SegmentDistance = SplineLength * (static_cast<float>(SegmentIndex) / static_cast<float>(DebugSegmentCount));
		const FVector CurrentWorldLocation = JumpTrajectorySpline->GetLocationAtDistanceAlongSpline(
			SegmentDistance,
			ESplineCoordinateSpace::World);
		DrawDebugLine(
			World,
			PreviousWorldLocation,
			CurrentWorldLocation,
			PreviewColor,
			false,
			0.0f,
			0,
			JumpTrajectoryPreviewLineThickness);
		PreviousWorldLocation = CurrentWorldLocation;
	}
}

void ADeveloperJumpTrajectory::CopySplinePointsTo(ADeveloperJumpTrajectory* TargetTrajectory) const
{
	if (!TargetTrajectory)
	{
		return;
	}

	CopySplinePoints(TargetTrajectory->StandingJumpTrajectorySpline, StandingJumpTrajectorySpline);
	CopySplinePoints(TargetTrajectory->MaxWalkSpeedJumpTrajectorySpline, MaxWalkSpeedJumpTrajectorySpline);
}

void ADeveloperJumpTrajectory::CopySplinePoints(
	USplineComponent* TargetSpline,
	const USplineComponent* SourceSpline)
{
	if (!TargetSpline || !SourceSpline)
	{
		return;
	}

	const int32 PointCount = SourceSpline->GetNumberOfSplinePoints();
	if (PointCount < 2)
	{
		TargetSpline->ClearSplinePoints(true);
		return;
	}

	TArray<FVector> LocalPoints;
	LocalPoints.Reserve(PointCount);

	for (int32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
	{
		LocalPoints.Add(SourceSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local));
	}

	TargetSpline->SetSplinePoints(LocalPoints, ESplineCoordinateSpace::Local, false);
	for (int32 PointIndex = 0; PointIndex < PointCount; ++PointIndex)
	{
		TargetSpline->SetSplinePointType(
			PointIndex,
			SourceSpline->GetSplinePointType(PointIndex),
			false);
	}

	TargetSpline->UpdateSpline();
}
