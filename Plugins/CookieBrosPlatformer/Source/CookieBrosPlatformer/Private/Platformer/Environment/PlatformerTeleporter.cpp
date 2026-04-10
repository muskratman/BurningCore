#include "Platformer/Environment/PlatformerTeleporter.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
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

#if WITH_EDITORONLY_DATA
	TeleporterIdText = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("TeleporterIdText"));
	if (TeleporterIdText)
	{
		TeleporterIdText->SetupAttachment(RootComponent);
		TeleporterIdText->SetRelativeLocation(FVector(0.0f, 0.0f, 105.0f));
		TeleporterIdText->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
		TeleporterIdText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		TeleporterIdText->SetTextRenderColor(FColor(90, 200, 255));
		TeleporterIdText->SetWorldSize(48.0f);
		TeleporterIdText->SetHiddenInGame(true);
		TeleporterIdText->SetIsVisualizationComponent(true);
		TeleporterIdText->bIsEditorOnly = true;
	}

	ExitIdText = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("ExitIdText"));
	if (ExitIdText)
	{
		ExitIdText->SetupAttachment(RootComponent);
		ExitIdText->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
		ExitIdText->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
		ExitIdText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		ExitIdText->SetTextRenderColor(FColor(255, 190, 90));
		ExitIdText->SetWorldSize(40.0f);
		ExitIdText->SetHiddenInGame(true);
		ExitIdText->SetIsVisualizationComponent(true);
		ExitIdText->bIsEditorOnly = true;
	}
#endif
}

void APlatformerTeleporter::RegisterArrival(AActor* Actor)
{
	MarkActorTeleported(Actor);
}

void APlatformerTeleporter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	TriggerVolume->SetBoxExtent(TriggerExtent);

#if WITH_EDITORONLY_DATA
	RefreshEditorLabels();
#endif
}

void APlatformerTeleporter::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !CanTeleportActor(OtherActor))
	{
		return;
	}

	TeleportActorFromPointA(OtherActor);
}

void APlatformerTeleporter::TeleportActorFromPointA(AActor* Actor)
{
	APlatformerTeleporter* Destination = ResolveExitTeleporter();
	if (Destination == nullptr)
	{
		return;
	}

	TeleportActorToResolvedDestination(Actor, Destination->GetEntryLocation(), Destination->GetEntryRotation(), Destination);
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

APlatformerTeleporter* APlatformerTeleporter::ResolveExitTeleporter() const
{
	APlatformerTeleporter* ResolvedExitTeleporter = ExitTeleporter.Get();
	return (ResolvedExitTeleporter != nullptr && ResolvedExitTeleporter != this) ? ResolvedExitTeleporter : nullptr;
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

#if WITH_EDITORONLY_DATA
void APlatformerTeleporter::RefreshEditorLabels()
{
	if (TeleporterIdText)
	{
		const bool bHasTeleporterId = !TeleporterId.IsEmpty();
		TeleporterIdText->SetVisibility(bHasTeleporterId);
		TeleporterIdText->SetText(bHasTeleporterId
			? FText::FromString(FString::Printf(TEXT("TeleporterId: %s"), *TeleporterId))
			: FText::GetEmpty());
	}

	if (ExitIdText)
	{
		const FString ResolvedExitIdText = GetResolvedExitIdText();
		const bool bHasExitId = !ResolvedExitIdText.IsEmpty();
		ExitIdText->SetVisibility(bHasExitId);
		ExitIdText->SetText(bHasExitId ? FText::FromString(ResolvedExitIdText) : FText::GetEmpty());
	}
}

FString APlatformerTeleporter::GetResolvedExitIdText() const
{
	const APlatformerTeleporter* ResolvedExitTeleporter = ResolveExitTeleporter();
	if (ResolvedExitTeleporter == nullptr)
	{
		return FString();
	}

	return ResolvedExitTeleporter->TeleporterId.IsEmpty()
		? FString(TEXT("ExitId: <Unset>"))
		: FString::Printf(TEXT("ExitId: %s"), *ResolvedExitTeleporter->TeleporterId);
}
#endif
