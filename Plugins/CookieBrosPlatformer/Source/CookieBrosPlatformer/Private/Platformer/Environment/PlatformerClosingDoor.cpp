#include "Platformer/Environment/PlatformerClosingDoor.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerClosingDoor::APlatformerClosingDoor()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerBlock.PlatformerBlock")));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Cube.SM_Platform_Cube"));
	if (CubeMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(CubeMesh.Object);
	}

	TriggerVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TriggerVolumeLayoutRoot"));
	TriggerVolumeLayoutRoot->SetupAttachment(Root);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(TriggerVolumeLayoutRoot);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetCanEverAffectNavigation(false);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerClosingDoor::OnTriggerBeginOverlap);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerClosingDoor::OnTriggerEndOverlap);

	PlatformSize = FVector(100.0f, 100.0f, 100.0f);
	SetMovementPathPoints({
		FPlatformerPathPoint(FVector::ZeroVector),
		FPlatformerPathPoint(FVector(0.0f, 0.0f, -300.0f))
	});
	MoveSpeed = 600.0f;
	bAutoStart = false;
}

void APlatformerClosingDoor::SetTriggerSize(const FVector& InTriggerSize)
{
	TriggerSize = InTriggerSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerClosingDoor::BeginPlay()
{
	Super::BeginPlay();

	OverlappingCharacters.Empty();
	OverlapEntryLocations.Empty();
	bHasPendingPointATriggerTraversal = false;
	bHasBeenTriggered = bStartAtPointB;
	SetTriggerEnabled(!bHasBeenTriggered && !IsAtPointB());
}

void APlatformerClosingDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TriggerVolume->SetBoxExtent(TriggerSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f)) * 0.5f);
	PlatformerEnvironment::ApplyRelativeTransform(
		TriggerVolumeLayoutRoot,
		TriggerBaseRelativeLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		TriggerVolumeTransformOffset);
}

void APlatformerClosingDoor::Interaction(AActor* /*Interactor*/)
{
	if (OverlappingCharacters.Num() == 0 && bHasPendingPointATriggerTraversal)
	{
		TryTriggerDoor();
	}
}

void APlatformerClosingDoor::ResetInteraction()
{
	Super::ResetInteraction();

	ClearInvalidOverlappingCharacters();
	bHasPendingPointATriggerTraversal = false;

	if (IsAtPointA() && !IsMoving())
	{
		bHasBeenTriggered = false;
		SetTriggerEnabled(true);
		return;
	}

	SetTriggerEnabled(false);
}

void APlatformerClosingDoor::HandleReachedPointB()
{
	Super::HandleReachedPointB();

	OverlappingCharacters.Empty();
	OverlapEntryLocations.Empty();
	bHasPendingPointATriggerTraversal = false;
	SetTriggerEnabled(false);
}

void APlatformerClosingDoor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlatformerCharacterBase* TriggeringCharacter = nullptr;
	if (!IsValidTriggeringCharacter(OtherActor, TriggeringCharacter))
	{
		return;
	}

	OverlappingCharacters.Add(TriggeringCharacter);
	OverlapEntryLocations.Add(TWeakObjectPtr<APlatformerCharacterBase>(TriggeringCharacter), TriggeringCharacter->GetActorLocation());
}

void APlatformerClosingDoor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlatformerCharacterBase* TriggeringCharacter = nullptr;
	if (!IsValidTriggeringCharacter(OtherActor, TriggeringCharacter))
	{
		return;
	}

	const FVector ExitLocation = TriggeringCharacter->GetActorLocation();
	if (DidCharacterPassInClosingDirection(TriggeringCharacter, ExitLocation))
	{
		bHasPendingPointATriggerTraversal = true;
	}

	OverlappingCharacters.Remove(TriggeringCharacter);
	OverlapEntryLocations.Remove(TWeakObjectPtr<APlatformerCharacterBase>(TriggeringCharacter));
	ClearInvalidOverlappingCharacters();

	if (OverlappingCharacters.Num() == 0 && bHasPendingPointATriggerTraversal)
	{
		TryTriggerDoor();
	}
}

bool APlatformerClosingDoor::IsValidTriggeringCharacter(AActor* OtherActor, APlatformerCharacterBase*& OutCharacter) const
{
	OutCharacter = Cast<APlatformerCharacterBase>(OtherActor);
	if (OutCharacter == nullptr)
	{
		return false;
	}

	if (bOnlyPlayerControlledCharacters && !OutCharacter->IsPlayerControlled())
	{
		OutCharacter = nullptr;
		return false;
	}

	return true;
}

FVector APlatformerClosingDoor::GetTriggerPassDirection() const
{
	const FVector TriggerLocation = TriggerVolume ? TriggerVolume->GetComponentLocation() : GetActorLocation() + TriggerBaseRelativeLocation;
	const FVector PointALocation = GetMovementPathPointWorldLocation(0);

	FVector Direction = TriggerLocation - PointALocation;
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero())
	{
		return Direction.GetSafeNormal();
	}

	Direction = GetActorForwardVector();
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero())
	{
		return Direction.GetSafeNormal();
	}

	return FVector::ForwardVector;
}

float APlatformerClosingDoor::GetTriggerHalfExtentAlongDirection(const FVector& Direction) const
{
	if (!TriggerVolume)
	{
		const FVector FallbackExtent = TriggerSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f)) * 0.5f;
		return FMath::Max3(FallbackExtent.X, FallbackExtent.Y, FallbackExtent.Z);
	}

	const FVector BoxExtent = TriggerVolume->GetScaledBoxExtent();
	const FTransform& TriggerTransform = TriggerVolume->GetComponentTransform();
	return
		FMath::Abs(FVector::DotProduct(TriggerTransform.GetUnitAxis(EAxis::X), Direction)) * BoxExtent.X +
		FMath::Abs(FVector::DotProduct(TriggerTransform.GetUnitAxis(EAxis::Y), Direction)) * BoxExtent.Y +
		FMath::Abs(FVector::DotProduct(TriggerTransform.GetUnitAxis(EAxis::Z), Direction)) * BoxExtent.Z;
}

bool APlatformerClosingDoor::DidCrossTriggerInClosingDirection(const FVector& EntryLocation, const FVector& ExitLocation) const
{
	const FVector PassDirection = GetTriggerPassDirection();
	const FVector TriggerLocation = TriggerVolume ? TriggerVolume->GetComponentLocation() : GetActorLocation() + TriggerBaseRelativeLocation;
	const float HalfExtentAlongPassDirection = GetTriggerHalfExtentAlongDirection(PassDirection);

	const float EntrySignedDistance = FVector::DotProduct(EntryLocation - TriggerLocation, PassDirection);
	const float ExitSignedDistance = FVector::DotProduct(ExitLocation - TriggerLocation, PassDirection);

	FVector HorizontalTravel = ExitLocation - EntryLocation;
	HorizontalTravel.Z = 0.0f;
	const float TravelAlongPassDirection = FVector::DotProduct(HorizontalTravel, PassDirection);

	return
		EntrySignedDistance <= -HalfExtentAlongPassDirection &&
		ExitSignedDistance >= HalfExtentAlongPassDirection &&
		TravelAlongPassDirection > 1.0f;
}

bool APlatformerClosingDoor::DidCharacterPassInClosingDirection(APlatformerCharacterBase* TriggeringCharacter, const FVector& ExitLocation) const
{
	if (!TriggeringCharacter)
	{
		return false;
	}

	const FVector* EntryLocation = OverlapEntryLocations.Find(TWeakObjectPtr<APlatformerCharacterBase>(TriggeringCharacter));
	if (!EntryLocation)
	{
		return false;
	}

	return DidCrossTriggerInClosingDirection(*EntryLocation, ExitLocation);
}

void APlatformerClosingDoor::TryTriggerDoor()
{
	if (bHasBeenTriggered || IsMoving() || IsAtPointB())
	{
		return;
	}

	bHasBeenTriggered = true;
	OverlappingCharacters.Empty();
	OverlapEntryLocations.Empty();
	bHasPendingPointATriggerTraversal = false;
	SetTriggerEnabled(false);
	StartMovingToPointB();
}

void APlatformerClosingDoor::SetTriggerEnabled(bool bEnabled)
{
	if (TriggerVolume == nullptr)
	{
		return;
	}

	TriggerVolume->SetGenerateOverlapEvents(bEnabled);
	TriggerVolume->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void APlatformerClosingDoor::ClearInvalidOverlappingCharacters()
{
	for (auto It = OverlappingCharacters.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			OverlapEntryLocations.Remove(*It);
			It.RemoveCurrent();
		}
	}
}
