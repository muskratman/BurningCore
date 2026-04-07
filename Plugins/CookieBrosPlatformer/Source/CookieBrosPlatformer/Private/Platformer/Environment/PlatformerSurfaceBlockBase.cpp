#include "Platformer/Environment/PlatformerSurfaceBlockBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"

APlatformerSurfaceBlockBase::APlatformerSurfaceBlockBase()
{
	SurfaceVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SurfaceVolumeLayoutRoot"));
	SurfaceVolumeLayoutRoot->SetupAttachment(Root);

	SurfaceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SurfaceVolume"));
	SurfaceVolume->SetupAttachment(SurfaceVolumeLayoutRoot);
	SurfaceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SurfaceVolume->SetCollisionObjectType(ECC_WorldDynamic);
	SurfaceVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	SurfaceVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SurfaceVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerSurfaceBlockBase::OnSurfaceVolumeBeginOverlap);
	SurfaceVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerSurfaceBlockBase::OnSurfaceVolumeEndOverlap);
}

void APlatformerSurfaceBlockBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector ResolvedBlockSize = GetBlockSize().ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const FVector2D ResolvedCoverage(
		FMath::Clamp(SurfaceCoverage.X, 0.01f, 1.0f),
		FMath::Clamp(SurfaceCoverage.Y, 0.01f, 1.0f));
	const FVector SurfaceExtent(
		ResolvedBlockSize.X * 0.5f * ResolvedCoverage.X,
		ResolvedBlockSize.Y * 0.5f * ResolvedCoverage.Y,
		FMath::Max(SurfaceTriggerHeight * 0.5f, 1.0f));

	SurfaceVolume->SetBoxExtent(SurfaceExtent);

	PlatformerEnvironment::ApplyRelativeTransform(
		SurfaceVolumeLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedBlockSize.Z + SurfaceExtent.Z),
		FRotator::ZeroRotator,
		FVector::OneVector,
		SurfaceVolumeTransformOffset);
}

void APlatformerSurfaceBlockBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OverlappingCharacters.Empty();
	Super::EndPlay(EndPlayReason);
}

void APlatformerSurfaceBlockBase::HandleCharacterEnteredSurface(ACharacter* Character)
{
}

void APlatformerSurfaceBlockBase::HandleCharacterLeftSurface(ACharacter* Character)
{
}

void APlatformerSurfaceBlockBase::ClearInvalidOverlappingCharacters()
{
	for (auto It = OverlappingCharacters.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

bool APlatformerSurfaceBlockBase::IsCharacterStandingOnSurface(const ACharacter* Character) const
{
	if (Character == nullptr)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if ((MovementComponent == nullptr) || !MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	const UPrimitiveComponent* MovementBase = MovementComponent->GetMovementBase();
	return (MovementBase != nullptr) && (MovementBase->GetOwner() == this);
}

void APlatformerSurfaceBlockBase::OnSurfaceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (!OverlappingCharacters.Contains(Character))
		{
			OverlappingCharacters.Add(Character);
			HandleCharacterEnteredSurface(Character);
		}
	}
}

void APlatformerSurfaceBlockBase::OnSurfaceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		if (OverlappingCharacters.Remove(Character) > 0)
		{
			HandleCharacterLeftSurface(Character);
		}
	}
}
