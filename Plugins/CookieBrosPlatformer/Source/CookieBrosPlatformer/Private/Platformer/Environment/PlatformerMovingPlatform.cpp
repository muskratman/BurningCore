// Copyright Epic Games, Inc. All Rights Reserved.

#include "Platformer/Environment/PlatformerMovingPlatform.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "Platformer/Environment/PlatformerDropThroughPlatformComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerMovingPlatform::APlatformerMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	bAutoStart = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerMovingPlatform.PlatformerMovingPlatform")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PlatformMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlatformMeshLayoutRoot"));
	PlatformMeshLayoutRoot->SetupAttachment(Root);

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PlatformMeshLayoutRoot);
	PlatformMesh->SetMobility(EComponentMobility::Movable);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionObjectType(ECC_WorldDynamic);
	PlatformMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlatformMesh->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(CubeMesh.Object);
	}

	DropThroughTopCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropThroughTopCheckLayoutRoot"));
	DropThroughTopCheckLayoutRoot->SetupAttachment(Root);

	DropThroughTopCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DropThroughTopCheckBox"));
	DropThroughTopCheckBox->SetupAttachment(DropThroughTopCheckLayoutRoot);

	DropThroughBottomCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropThroughBottomCheckLayoutRoot"));
	DropThroughBottomCheckLayoutRoot->SetupAttachment(Root);

	DropThroughBottomCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DropThroughBottomCheckBox"));
	DropThroughBottomCheckBox->SetupAttachment(DropThroughBottomCheckLayoutRoot);

	DropThroughPlatformComponent = CreateDefaultSubobject<UPlatformerDropThroughPlatformComponent>(TEXT("DropThroughPlatform"));
	DropThroughPlatformComponent->InitializeDropThroughPlatform(
		PlatformMesh,
		DropThroughTopCheckLayoutRoot,
		DropThroughTopCheckBox,
		DropThroughBottomCheckLayoutRoot,
		DropThroughBottomCheckBox);

	MovementPathComponent = CreateDefaultSubobject<UPlatformerPathComponent>(TEXT("MovementPath"));
	MovementPathComponent->SetupAttachment(Root);
	MovementPathComponent->SetPathPoints({
		FPlatformerPathPoint(FVector::ZeroVector, 0.5f, 1.0f),
		FPlatformerPathPoint(FVector(500.0f, 0.0f, 0.0f), 0.5f, 1.0f)
	});
}

void APlatformerMovingPlatform::SetPlatformSize(const FVector& InPlatformSize)
{
	PlatformSize = InPlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	RefreshMovingPlatformLayout();
}

void APlatformerMovingPlatform::SetMovementPathPoints(const TArray<FPlatformerPathPoint>& InPathPoints)
{
	if (!MovementPathComponent)
	{
		return;
	}

	TArray<FPlatformerPathPoint> ResolvedPathPoints = InPathPoints;
	if (ResolvedPathPoints.Num() < 2)
	{
		ResolvedPathPoints.SetNum(2);
	}

	if (ResolvedPathPoints[1].PointLocation.IsNearlyZero())
	{
		ResolvedPathPoints[1].PointLocation = FVector(500.0f, 0.0f, 0.0f);
	}

	MovementPathComponent->SetPathPoints(ResolvedPathPoints);
}

const TArray<FPlatformerPathPoint>& APlatformerMovingPlatform::GetMovementPathPoints() const
{
	static const TArray<FPlatformerPathPoint> EmptyPathPoints;
	return MovementPathComponent ? MovementPathComponent->GetPathPoints() : EmptyPathPoints;
}

FVector APlatformerMovingPlatform::GetMovementPathPointWorldLocation(int32 PointIndex) const
{
	return MovementPathComponent ? MovementPathComponent->GetPathPointWorldLocation(PointIndex) : GetActorLocation();
}

int32 APlatformerMovingPlatform::GetLastMovementPathPointIndex() const
{
	const TArray<FPlatformerPathPoint>& PathPoints = GetMovementPathPoints();
	return FMath::Max(PathPoints.Num() - 1, 0);
}

void APlatformerMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	EnsureMovementPathPoints();
	CachedPathPointLocations.Reset(GetMovementPathPoints().Num());
	for (int32 PointIndex = 0; PointIndex < GetMovementPathPoints().Num(); ++PointIndex)
	{
		CachedPathPointLocations.Add(GetMovementPathPointWorldLocation(PointIndex));
	}

	CurrentPathPointIndex = bStartAtPointB ? GetLastMovementPathPointIndex() : 0;
	TargetPathPointIndex = INDEX_NONE;
	ActivePathStartPointIndex = CurrentPathPointIndex;
	MoveState = CurrentPathPointIndex == 0 ? EPlatformerMoverState::IdleAtPointA : EPlatformerMoverState::IdleAtPointB;
	SetActorLocation(CachedPathPointLocations.IsValidIndex(CurrentPathPointIndex)
		? CachedPathPointLocations[CurrentPathPointIndex]
		: GetActorLocation());

	if (bAutoStart)
	{
		bPendingInitialDeparture = true;
		bPathRunActive = true;
		EnterPauseAtCurrentPathPoint();
	}
}

void APlatformerMovingPlatform::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshMovingPlatformLayout();
}

void APlatformerMovingPlatform::RefreshMovingPlatformLayout()
{
	const FVector ResolvedPlatformSize = PlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));

	PlatformerEnvironment::ApplyRelativeTransform(
		PlatformMeshLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedPlatformSize.Z * 0.5f),
		FRotator::ZeroRotator,
		ResolvedPlatformSize / 100.0f,
		PlatformMeshTransformOffset);

	if (DropThroughPlatformComponent)
	{
		DropThroughPlatformComponent->RefreshDropThroughPlatformLayout(ResolvedPlatformSize);
	}

	EnsureMovementPathPoints();
}

void APlatformerMovingPlatform::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void APlatformerMovingPlatform::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (!bFinished)
	{
		return;
	}

	RebaseEditorPathStartToActorLocation();
	RefreshMovingPlatformLayout();
}

void APlatformerMovingPlatform::RebaseEditorPathStartToActorLocation()
{
	if (!MovementPathComponent)
	{
		return;
	}

	TArray<FPlatformerPathPoint> PathPoints = MovementPathComponent->GetPathPoints();
	if (!PathPoints.IsValidIndex(0) || PathPoints[0].PointLocation.IsNearlyZero())
	{
		return;
	}

	Modify();
	const FVector RebaseOffset = PathPoints[0].PointLocation;
	for (FPlatformerPathPoint& PathPoint : PathPoints)
	{
		PathPoint.PointLocation -= RebaseOffset;
	}
	MovementPathComponent->SetPathPoints(PathPoints);
}
#endif

void APlatformerMovingPlatform::EnsureMovementPathPoints()
{
	if (!MovementPathComponent)
	{
		return;
	}

	TArray<FPlatformerPathPoint> PathPoints = MovementPathComponent->GetPathPoints();
	const int32 PreviousNum = PathPoints.Num();
	if (PathPoints.Num() < 2)
	{
		PathPoints.SetNum(2);
	}
	if (PreviousNum == 0)
	{
		PathPoints[0] = FPlatformerPathPoint(FVector::ZeroVector, 0.5f, 1.0f);
	}
	if (PathPoints[1].PointLocation.IsNearlyZero())
	{
		PathPoints[1].PointLocation = FVector(500.0f, 0.0f, 0.0f);
	}
	MovementPathComponent->SetPathPoints(PathPoints);
}

void APlatformerMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (MoveState)
	{
	case EPlatformerMoverState::MovingToPointB:
	case EPlatformerMoverState::MovingToPointA:
		MoveTowards(DeltaTime);
		break;

	case EPlatformerMoverState::IdleAtPointA:
	case EPlatformerMoverState::IdleAtPointB:
		AdvancePause(DeltaTime);
		break;
	}
}

void APlatformerMovingPlatform::Interaction(AActor* Interactor)
{
	if (IsMoving())
	{
		return;
	}

	bPendingInitialDeparture = false;
	StartMovingAwayFromCurrentPoint();
}

void APlatformerMovingPlatform::ResetInteraction()
{
	PauseTimer = 0.0f;
	bPendingInitialDeparture = false;
	bPathRunActive = false;
}

void APlatformerMovingPlatform::MoveTowards(float DeltaTime)
{
	if (!CachedPathPointLocations.IsValidIndex(TargetPathPointIndex))
	{
		EnterPauseAtCurrentPathPoint();
		return;
	}

	const FVector TargetLocation = CachedPathPointLocations[TargetPathPointIndex];
	const FVector CurrentLocation = GetActorLocation();
	const float ResolvedMoveSpeed = FMath::Max(MoveSpeed * GetPathSegmentSpeedScale(ActivePathStartPointIndex), 1.0f);
	const FVector NextLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, ResolvedMoveSpeed);

	SetActorLocation(NextLocation);

	if (FVector::DistSquared(NextLocation, TargetLocation) <= FMath::Square(ArrivalTolerance))
	{
		SetActorLocation(TargetLocation);
		CurrentPathPointIndex = TargetPathPointIndex;
		TargetPathPointIndex = INDEX_NONE;
		EnterPauseAtCurrentPathPoint();
	}
}

void APlatformerMovingPlatform::EnterPauseAtCurrentPathPoint()
{
	MoveState = CurrentPathPointIndex == 0 ? EPlatformerMoverState::IdleAtPointA : EPlatformerMoverState::IdleAtPointB;
	PauseTimer = 0.0f;

	if (CurrentPathPointIndex == 0)
	{
		HandleReachedPointA();
	}
	else if (CurrentPathPointIndex == GetLastMovementPathPointIndex())
	{
		HandleReachedPointB();
	}

	const float ResolvedPointDelay = GetPathPointDelay(CurrentPathPointIndex);
	if (ResolvedPointDelay > 0.0f)
	{
		PauseTimer = ResolvedPointDelay;
		return;
	}

	if (CurrentPathPointIndex == 0)
	{
		HandlePauseFinishedAtPointA();
	}
	else if (CurrentPathPointIndex == GetLastMovementPathPointIndex())
	{
		HandlePauseFinishedAtPointB();
	}
	else
	{
		ContinuePathAfterPause();
	}
}

void APlatformerMovingPlatform::AdvancePause(float DeltaTime)
{
	if (PauseTimer <= 0.0f)
	{
		return;
	}

	PauseTimer = FMath::Max(0.0f, PauseTimer - DeltaTime);
	if (PauseTimer > 0.0f)
	{
		return;
	}

	if (CurrentPathPointIndex == 0)
	{
		HandlePauseFinishedAtPointA();
	}
	else if (CurrentPathPointIndex == GetLastMovementPathPointIndex())
	{
		HandlePauseFinishedAtPointB();
	}
	else
	{
		ContinuePathAfterPause();
	}
}

void APlatformerMovingPlatform::StartMovingAwayFromCurrentPoint()
{
	bPathRunActive = true;
	if (CurrentPathPointIndex >= GetLastMovementPathPointIndex())
	{
		StartMovingToPointA();
		return;
	}

	StartMovingToPathPoint(CurrentPathPointIndex + 1);
}

void APlatformerMovingPlatform::HandleReachedPointA()
{
}

void APlatformerMovingPlatform::HandleReachedPointB()
{
}

void APlatformerMovingPlatform::HandlePauseFinishedAtPointA()
{
	const bool bShouldStartDeparture = bPendingInitialDeparture || bPathRunActive;
	bPendingInitialDeparture = false;
	if (bShouldStartDeparture)
	{
		ContinuePathAfterPause();
	}
}

void APlatformerMovingPlatform::HandlePauseFinishedAtPointB()
{
	const bool bShouldStartDeparture = bPendingInitialDeparture || bPathRunActive;
	bPendingInitialDeparture = false;
	if (bShouldStartDeparture)
	{
		ContinuePathAfterPause();
	}
}

void APlatformerMovingPlatform::StartMovingToPointA()
{
	bPathRunActive = false;
	StartMovingToPathPoint(0);
}

void APlatformerMovingPlatform::StartMovingToPointB()
{
	bPathRunActive = false;
	StartMovingToPathPoint(GetLastMovementPathPointIndex());
}

void APlatformerMovingPlatform::ContinuePathAfterPause()
{
	const int32 LastPointIndex = GetLastMovementPathPointIndex();
	if (CurrentPathPointIndex < LastPointIndex)
	{
		StartMovingToPathPoint(CurrentPathPointIndex + 1);
		return;
	}

	if (MovementPathComponent && MovementPathComponent->ShouldRepeatPath())
	{
		bPathRunActive = true;
		StartMovingToPathPoint(0);
		return;
	}

	bPathRunActive = false;
}

void APlatformerMovingPlatform::StartMovingToPathPoint(int32 InTargetPointIndex)
{
	if (!CachedPathPointLocations.IsValidIndex(InTargetPointIndex) || InTargetPointIndex == CurrentPathPointIndex)
	{
		return;
	}

	PauseTimer = 0.0f;
	ActivePathStartPointIndex = CurrentPathPointIndex;
	TargetPathPointIndex = InTargetPointIndex;
	MoveState = TargetPathPointIndex < CurrentPathPointIndex
		? EPlatformerMoverState::MovingToPointA
		: EPlatformerMoverState::MovingToPointB;
	BP_MoveToTarget();
	BP_OnMovementStarted();
}

float APlatformerMovingPlatform::GetPathPointDelay(int32 PointIndex) const
{
	const TArray<FPlatformerPathPoint>& PathPoints = GetMovementPathPoints();
	return PathPoints.IsValidIndex(PointIndex) ? FMath::Max(PathPoints[PointIndex].PointDelay, 0.0f) : 0.0f;
}

float APlatformerMovingPlatform::GetPathSegmentSpeedScale(int32 StartPointIndex) const
{
	const TArray<FPlatformerPathPoint>& PathPoints = GetMovementPathPoints();
	return PathPoints.IsValidIndex(StartPointIndex) ? FMath::Max(PathPoints[StartPointIndex].SpeedScale, 0.01f) : 1.0f;
}
