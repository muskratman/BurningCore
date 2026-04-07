#include "Platformer/Environment/PlatformerTeleporter.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerTeleporter::APlatformerTeleporter()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerTeleporter.PlatformerTeleporter")));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);

	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerTeleporter::OnTriggerVolumeBeginOverlap);

	TeleporterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleporterMesh"));
	TeleporterMesh->SetupAttachment(RootComponent);
	TeleporterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		TeleporterMesh->SetStaticMesh(CylinderMesh.Object);
	}

	ExitPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitPoint"));
	ExitPoint->SetupAttachment(RootComponent);
	ExitPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f));

	ExitTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTriggerVolume"));
	ExitTriggerVolume->SetupAttachment(ExitPoint);
	ExitTriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitTriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ExitTriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitTriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExitTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerTeleporter::OnExitTriggerVolumeBeginOverlap);

	ExitTeleporterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExitTeleporterMesh"));
	ExitTeleporterMesh->SetupAttachment(ExitPoint);
	ExitTeleporterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (CylinderMesh.Succeeded())
	{
		ExitTeleporterMesh->SetStaticMesh(CylinderMesh.Object);
	}
}

void APlatformerTeleporter::RegisterArrival(AActor* Actor)
{
	MarkActorTeleported(Actor);
}

void APlatformerTeleporter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	TriggerVolume->SetBoxExtent(TriggerExtent);
	ExitTriggerVolume->SetBoxExtent(TriggerExtent);
}

void APlatformerTeleporter::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !CanTeleportActor(OtherActor))
	{
		return;
	}

	TeleportActorFromPointA(OtherActor);
}

void APlatformerTeleporter::OnExitTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bTwoSidedTeleport || !OtherActor || !CanTeleportActor(OtherActor))
	{
		return;
	}

	TeleportActorFromPointB(OtherActor);
}

void APlatformerTeleporter::TeleportActorFromPointA(AActor* Actor)
{
	APlatformerTeleporter* Destination = DestinationTeleporter ? DestinationTeleporter.Get() : this;
	if (Destination == nullptr)
	{
		return;
	}

	TeleportActorToResolvedDestination(Actor, Destination->GetExitLocation(), Destination->GetExitRotation(), Destination);
}

void APlatformerTeleporter::TeleportActorFromPointB(AActor* Actor)
{
	TeleportActorToResolvedDestination(Actor, GetEntryLocation(), GetEntryRotation(), this);
}

void APlatformerTeleporter::TeleportActorToResolvedDestination(AActor* Actor, const FVector& DestinationLocation, const FRotator& DestinationRotation, APlatformerTeleporter* DestinationSettingsOwner)
{
	if (Actor == nullptr || DestinationSettingsOwner == nullptr)
	{
		return;
	}

	DestinationSettingsOwner->RegisterArrival(Actor);
	MarkActorTeleported(Actor);

	Actor->TeleportTo(DestinationLocation, DestinationRotation, false, true);

	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (!DestinationSettingsOwner->bKeepVelocity)
			{
				MovementComponent->StopMovementImmediately();
			}

			if (DestinationSettingsOwner->ExitSpeed > 0.0f)
			{
				MovementComponent->Velocity = DestinationRotation.Vector() * DestinationSettingsOwner->ExitSpeed;
			}
		}
	}
}

bool APlatformerTeleporter::CanTeleportActor(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	if (const float* LastTeleportTime = RecentlyTeleportedActors.Find(Actor))
	{
		return (GetWorld()->GetTimeSeconds() - *LastTeleportTime) >= TeleportCooldown;
	}

	return true;
}

void APlatformerTeleporter::MarkActorTeleported(AActor* Actor)
{
	if (Actor)
	{
		RecentlyTeleportedActors.Add(Actor, GetWorld()->GetTimeSeconds());
	}
}

FVector APlatformerTeleporter::GetEntryLocation() const
{
	return GetActorLocation();
}

FRotator APlatformerTeleporter::GetEntryRotation() const
{
	return GetActorRotation();
}

FVector APlatformerTeleporter::GetExitLocation() const
{
	return ExitPoint->GetComponentLocation();
}

FRotator APlatformerTeleporter::GetExitRotation() const
{
	return ExitPoint->GetComponentRotation();
}
