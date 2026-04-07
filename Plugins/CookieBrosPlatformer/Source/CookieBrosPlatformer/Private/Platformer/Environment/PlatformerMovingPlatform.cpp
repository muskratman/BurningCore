// Copyright Epic Games, Inc. All Rights Reserved.

#include "Platformer/Environment/PlatformerMovingPlatform.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
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

	PointALayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PointALayoutRoot"));
	PointALayoutRoot->SetupAttachment(Root);

	PointA = CreateDefaultSubobject<UArrowComponent>(TEXT("PointA"));
	PointA->SetupAttachment(PointALayoutRoot);
	PointA->ArrowSize = 1.2f;

	PointBLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PointBLayoutRoot"));
	PointBLayoutRoot->SetupAttachment(Root);

	PointB = CreateDefaultSubobject<UArrowComponent>(TEXT("PointB"));
	PointB->SetupAttachment(PointBLayoutRoot);
	PointB->ArrowSize = 1.2f;
}

void APlatformerMovingPlatform::SetPlatformSize(const FVector& InPlatformSize)
{
	PlatformSize = InPlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	CachedPointALocation = PointA->GetComponentLocation();
	CachedPointBLocation = PointB->GetComponentLocation();

	MoveState = bStartAtPointB ? EPlatformerMoverState::IdleAtPointB : EPlatformerMoverState::IdleAtPointA;
	SetActorLocation(bStartAtPointB ? CachedPointBLocation : CachedPointALocation);

	if (bAutoStart)
	{
		bPendingInitialDeparture = true;
		EnterPauseState(MoveState);
	}
}

void APlatformerMovingPlatform::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlatformerEnvironment::ApplyRelativeTransform(
		PlatformMeshLayoutRoot,
		FVector(0.0f, 0.0f, PlatformSize.Z * 0.5f),
		FRotator::ZeroRotator,
		PlatformSize / 100.0f,
		PlatformMeshTransformOffset);

	PlatformerEnvironment::ApplyRelativeTransform(
		PointALayoutRoot,
		PointABaseRelativeLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		PointATransformOffset);

	PlatformerEnvironment::ApplyRelativeTransform(
		PointBLayoutRoot,
		PointBBaseRelativeLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		PointBTransformOffset);
}

void APlatformerMovingPlatform::PostLoad()
{
	Super::PostLoad();

	const bool bUsesDefaultPerPointDelays =
		FMath::IsNearlyEqual(PointADelay, 0.5f) &&
		FMath::IsNearlyEqual(PointBDelay, 0.5f);

	if (bUsesDefaultPerPointDelays && !FMath::IsNearlyEqual(PointDelay, 0.5f))
	{
		PointADelay = PointDelay;
		PointBDelay = PointDelay;
	}
}

void APlatformerMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (MoveState)
	{
	case EPlatformerMoverState::MovingToPointB:
		MoveTowards(CachedPointBLocation, DeltaTime, EPlatformerMoverState::IdleAtPointB);
		break;

	case EPlatformerMoverState::MovingToPointA:
		MoveTowards(CachedPointALocation, DeltaTime, EPlatformerMoverState::IdleAtPointA);
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
}

void APlatformerMovingPlatform::MoveTowards(const FVector& TargetLocation, float DeltaTime, EPlatformerMoverState ArrivalState)
{
	const FVector CurrentLocation = GetActorLocation();
	const FVector NextLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);

	SetActorLocation(NextLocation);

	if (FVector::DistSquared(NextLocation, TargetLocation) <= FMath::Square(ArrivalTolerance))
	{
		SetActorLocation(TargetLocation);
		EnterPauseState(ArrivalState);
	}
}

void APlatformerMovingPlatform::EnterPauseState(EPlatformerMoverState NewState)
{
	MoveState = NewState;
	PauseTimer = 0.0f;

	if (NewState == EPlatformerMoverState::IdleAtPointA)
	{
		HandleReachedPointA();
	}
	else if (NewState == EPlatformerMoverState::IdleAtPointB)
	{
		HandleReachedPointB();
	}

	const float ResolvedPointDelay =
		MoveState == EPlatformerMoverState::IdleAtPointA ? PointADelay :
		MoveState == EPlatformerMoverState::IdleAtPointB ? PointBDelay :
		0.0f;

	if (ResolvedPointDelay > 0.0f)
	{
		PauseTimer = ResolvedPointDelay;
		return;
	}

	if (MoveState == EPlatformerMoverState::IdleAtPointA)
	{
		HandlePauseFinishedAtPointA();
	}
	else if (MoveState == EPlatformerMoverState::IdleAtPointB)
	{
		HandlePauseFinishedAtPointB();
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

	if (MoveState == EPlatformerMoverState::IdleAtPointA)
	{
		HandlePauseFinishedAtPointA();
	}
	else if (MoveState == EPlatformerMoverState::IdleAtPointB)
	{
		HandlePauseFinishedAtPointB();
	}
}

void APlatformerMovingPlatform::StartMovingAwayFromCurrentPoint()
{
	if (IsAtPointA())
	{
		StartMovingToPointB();
	}
	else if (IsAtPointB())
	{
		StartMovingToPointA();
	}
}

void APlatformerMovingPlatform::HandleReachedPointA()
{
}

void APlatformerMovingPlatform::HandleReachedPointB()
{
}

void APlatformerMovingPlatform::HandlePauseFinishedAtPointA()
{
	const bool bShouldStartDeparture = bPendingInitialDeparture || bIsRepeatable;
	bPendingInitialDeparture = false;
	if (bShouldStartDeparture)
	{
		StartMovingToPointB();
	}
}

void APlatformerMovingPlatform::HandlePauseFinishedAtPointB()
{
	const bool bShouldStartDeparture = bPendingInitialDeparture || bIsRepeatable;
	bPendingInitialDeparture = false;
	if (bShouldStartDeparture)
	{
		StartMovingToPointA();
	}
}

void APlatformerMovingPlatform::StartMovingToPointA()
{
	PauseTimer = 0.0f;
	MoveState = EPlatformerMoverState::MovingToPointA;
	BP_MoveToTarget();
	BP_OnMovementStarted();
}

void APlatformerMovingPlatform::StartMovingToPointB()
{
	PauseTimer = 0.0f;
	MoveState = EPlatformerMoverState::MovingToPointB;
	BP_MoveToTarget();
	BP_OnMovementStarted();
}
