#include "Platformer/Environment/PlatformerCheckpoint.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Platformer/Systems/PlatformerCheckpointSubsystem.h"
#include "UObject/ConstructorHelpers.h"

APlatformerCheckpoint::APlatformerCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerZone->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerZone->SetGenerateOverlapEvents(true);
	TriggerZone->SetBoxExtent(FVector(100.0f, 100.0f, 150.0f));
	RootComponent = TriggerZone;

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointMesh"));
	CheckpointMesh->SetupAttachment(TriggerZone);
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CheckpointMesh->SetGenerateOverlapEvents(false);
	CheckpointMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CheckpointStaticMesh(TEXT("/CookieBrosPlatformer/Meshes/Platform_Checkpoint.Platform_Checkpoint"));
	if (CheckpointStaticMesh.Succeeded())
	{
		CheckpointMesh->SetStaticMesh(CheckpointStaticMesh.Object);
	}

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &APlatformerCheckpoint::OnTriggerZoneBeginOverlap);
}

void APlatformerCheckpoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshCheckpointLayout();
}

void APlatformerCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UPlatformerCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UPlatformerCheckpointSubsystem>())
		{
			CheckpointSubsystem->RegisterCheckpoint(this);
		}
	}
}

void APlatformerCheckpoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UPlatformerCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UPlatformerCheckpointSubsystem>())
		{
			CheckpointSubsystem->UnregisterCheckpoint(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void APlatformerCheckpoint::Interact_Implementation(AActor* Interactor)
{
	ActivateCheckpoint(Interactor);
}

bool APlatformerCheckpoint::CanInteract_Implementation(AActor* Interactor) const
{
	return IsValid(Interactor);
}

FText APlatformerCheckpoint::GetInteractionPrompt_Implementation() const
{
	return CheckpointName.IsEmpty()
		? NSLOCTEXT("CookieBrosPlatformer", "CheckpointPromptFallback", "Activate checkpoint")
		: FText::Format(NSLOCTEXT("CookieBrosPlatformer", "CheckpointPrompt", "Rest at {0}"), CheckpointName);
}

void APlatformerCheckpoint::ActivateCheckpoint(AActor* Activator)
{
	if (UWorld* World = GetWorld())
	{
		if (UPlatformerCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UPlatformerCheckpointSubsystem>())
		{
			CheckpointSubsystem->ActivateCheckpoint(this, Activator);
			return;
		}
	}

	SetCheckpointActive(true);
}

void APlatformerCheckpoint::SetCheckpointActive(bool bInActive)
{
	bIsActive = bInActive;
}

FTransform APlatformerCheckpoint::GetRespawnTransform() const
{
	return RespawnTransformOffset * GetActorTransform();
}

void APlatformerCheckpoint::RefreshCheckpointLayout()
{
	if (!TriggerZone || !CheckpointMesh)
	{
		return;
	}

	const UStaticMesh* StaticMesh = CheckpointMesh->GetStaticMesh();
	if (!StaticMesh)
	{
		return;
	}

	const FVector MeshScale = CheckpointMesh->GetRelativeScale3D();
	const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
	const float TriggerBottomZ = -TriggerZone->GetUnscaledBoxExtent().Z;
	const float MeshBottomZ = (MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z) * MeshScale.Z;
	FVector MeshRelativeLocation = CheckpointMesh->GetRelativeLocation();
	MeshRelativeLocation.Z = TriggerBottomZ - MeshBottomZ;
	CheckpointMesh->SetRelativeLocation(MeshRelativeLocation);
}

void APlatformerCheckpoint::OnTriggerZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bActivateOnOverlap || !IsValid(OtherActor) || !OtherActor->IsA<APlatformerCharacterBase>())
	{
		return;
	}

	ActivateCheckpoint(OtherActor);
}
