#include "Platformer/Environment/PlatformerYokuBlocks.h"

#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

APlatformerYokuBlocks::APlatformerYokuBlocks()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerVanishingBlock.PlatformerVanishingBlock")));
}

void APlatformerYokuBlocks::BeginPlay()
{
	Super::BeginPlay();

	VisibleActorLocation = GetActorLocation();
	UpdateHiddenActorLocation();

	SetBlockVisibleState(false);
	SetActorLocation(HiddenActorLocation);
	TransitionState = EPlatformerYokuBlockTransitionState::IdleAtHidden;

	if (InitialDelay > 0.0f)
	{
		ScheduleNextStateTransition(InitialDelay, &APlatformerYokuBlocks::ShowBlock);
	}
	else
	{
		ShowBlock();
	}
}

void APlatformerYokuBlocks::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TransitionState != EPlatformerYokuBlockTransitionState::MovingToVisible
		&& TransitionState != EPlatformerYokuBlockTransitionState::MovingToHidden)
	{
		SetActorTickEnabled(false);
		return;
	}

	const FVector TargetLocation = TransitionState == EPlatformerYokuBlockTransitionState::MovingToVisible
		? VisibleActorLocation
		: HiddenActorLocation;
	const FVector UpdatedLocation = HideSpeed > 0.0f
		? FMath::VInterpConstantTo(GetActorLocation(), TargetLocation, DeltaTime, HideSpeed)
		: TargetLocation;
	SetActorLocation(UpdatedLocation);

	if (UpdatedLocation.Equals(TargetLocation, 0.1f))
	{
		SetActorLocation(TargetLocation);
		FinishBlockTransition();
	}
}

void APlatformerYokuBlocks::ShowBlock()
{
	StartBlockTransition(true);
}

void APlatformerYokuBlocks::HideBlock()
{
	StartBlockTransition(false);
}

void APlatformerYokuBlocks::SetBlockVisibleState(bool bNewVisible)
{
	bIsBlockVisible = bNewVisible;

	if (UStaticMeshComponent* PlatformMesh = BlockMesh)
	{
		PlatformMesh->SetHiddenInGame(false);
		PlatformMesh->SetCollisionEnabled(bNewVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

void APlatformerYokuBlocks::UpdateHiddenActorLocation()
{
	const float HiddenTravelDistance = FMath::Max(BlockSize.Y, 100.0f);
	HiddenActorLocation = VisibleActorLocation - FVector(0.0f, HiddenTravelDistance, 0.0f);
}

void APlatformerYokuBlocks::StartBlockTransition(bool bMoveToVisible)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StateTimerHandle);
	}

	if (bMoveToVisible)
	{
		SetBlockVisibleState(false);
		TransitionState = EPlatformerYokuBlockTransitionState::MovingToVisible;
	}
	else
	{
		SetBlockVisibleState(false);
		TransitionState = EPlatformerYokuBlockTransitionState::MovingToHidden;
	}

	SetActorTickEnabled(true);

	if (HideSpeed <= 0.0f)
	{
		SetActorLocation(bMoveToVisible ? VisibleActorLocation : HiddenActorLocation);
		FinishBlockTransition();
	}
}

void APlatformerYokuBlocks::FinishBlockTransition()
{
	switch (TransitionState)
	{
	case EPlatformerYokuBlockTransitionState::MovingToVisible:
		TransitionState = EPlatformerYokuBlockTransitionState::IdleAtVisible;
		SetBlockVisibleState(true);
		ScheduleNextStateTransition(ShowDuration, &APlatformerYokuBlocks::HideBlock);
		break;

	case EPlatformerYokuBlockTransitionState::MovingToHidden:
		TransitionState = EPlatformerYokuBlockTransitionState::IdleAtHidden;
		SetBlockVisibleState(false);
		ScheduleNextStateTransition(HidenDuration, &APlatformerYokuBlocks::ShowBlock);
		break;

	case EPlatformerYokuBlockTransitionState::IdleAtVisible:
	case EPlatformerYokuBlockTransitionState::IdleAtHidden:
	default:
		break;
	}

	SetActorTickEnabled(false);
}

void APlatformerYokuBlocks::ScheduleNextStateTransition(float Delay, void (APlatformerYokuBlocks::*NextStateFunction)())
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StateTimerHandle);

		if (Delay <= 0.0f)
		{
			FTimerDelegate NextStateDelegate;
			NextStateDelegate.BindUObject(this, NextStateFunction);
			World->GetTimerManager().SetTimerForNextTick(NextStateDelegate);
			return;
		}

		World->GetTimerManager().SetTimer(StateTimerHandle, this, NextStateFunction, Delay, false);
	}
}
