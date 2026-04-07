#include "Platformer/Environment/PlatformerGravityVolume.h"

#include "Character/SideViewMovementComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

APlatformerGravityVolume::APlatformerGravityVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerGravityVolume.PlatformerGravityVolume")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	VolumeMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VolumeMeshLayoutRoot"));
	VolumeMeshLayoutRoot->SetupAttachment(Root);

	VolumeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolumeMesh"));
	VolumeMesh->SetupAttachment(VolumeMeshLayoutRoot);
	VolumeMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VolumeMesh->SetCollisionObjectType(ECC_WorldDynamic);
	VolumeMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	VolumeMesh->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		VolumeMesh->SetStaticMesh(CubeMesh.Object);
	}

	VolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VolumeLayoutRoot"));
	VolumeLayoutRoot->SetupAttachment(Root);

	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetupAttachment(VolumeLayoutRoot);

	Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Volume->SetCollisionObjectType(ECC_WorldDynamic);
	Volume->SetCollisionResponseToAllChannels(ECR_Ignore);
	Volume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Volume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerGravityVolume::OnVolumeBeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &APlatformerGravityVolume::OnVolumeEndOverlap);
}

void APlatformerGravityVolume::SetVolumeSize(const FVector& InVolumeSize)
{
	VolumeSize = InVolumeSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerGravityVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector ResolvedVolumeSize = VolumeSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const FVector BoxExtent = ResolvedVolumeSize * 0.5f;
	const FVector ComponentLocation(0.0f, 0.0f, BoxExtent.Z);
	const FVector MeshLocation = bVolumeMeshUsesBottomPivot ? FVector::ZeroVector : ComponentLocation;

	PlatformerEnvironment::ApplyRelativeTransform(
		VolumeMeshLayoutRoot,
		MeshLocation,
		FRotator::ZeroRotator,
		ResolvedVolumeSize / 100.0f,
		VolumeMeshTransformOffset);

	Volume->SetBoxExtent(BoxExtent);
	PlatformerEnvironment::ApplyRelativeTransform(
		VolumeLayoutRoot,
		ComponentLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		VolumeTransformOffset);
}

void APlatformerGravityVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (auto It = AffectedCharacters.CreateIterator(); It; ++It)
	{
		ACharacter* Character = It->Key.Get();
		if (!Character)
		{
			It.RemoveCurrent();
			continue;
		}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (MaxGravityVelocity <= 0.0f)
			{
				continue;
			}

			FVector Velocity = MovementComponent->Velocity;
			if (GravityScaleOverride >= 0.0f && Velocity.Z < -MaxGravityVelocity)
			{
				Velocity.Z = -MaxGravityVelocity;
				MovementComponent->Velocity = Velocity;
			}
			else if (GravityScaleOverride < 0.0f && Velocity.Z > MaxGravityVelocity)
			{
				Velocity.Z = MaxGravityVelocity;
				MovementComponent->Velocity = Velocity;
			}
		}
	}
}

void APlatformerGravityVolume::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character || AffectedCharacters.Contains(Character))
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		FPlatformerGravityVolumeState SavedState;
		SavedState.GravityScale = MovementComponent->GravityScale;
		SavedState.MovementMode = MovementComponent->MovementMode;
		if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			SavedState.bHadExternalGravityScaleOverride = SideViewMovementComponent->HasExternalGravityScaleOverride();
			SavedState.ExternalGravityScaleOverride = SideViewMovementComponent->GetExternalGravityScaleOverride();
			SideViewMovementComponent->SetExternalGravityScaleOverride(GravityScaleOverride);
		}
		else
		{
			MovementComponent->GravityScale = GravityScaleOverride;
		}

		AffectedCharacters.Add(Character, SavedState);
		if (bUseFlyingMovementMode)
		{
			MovementComponent->SetMovementMode(MOVE_Flying);
		}
	}
}

void APlatformerGravityVolume::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		RestoreCharacter(Character);
	}
}

void APlatformerGravityVolume::RestoreCharacter(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}

	const FPlatformerGravityVolumeState* SavedState = AffectedCharacters.Find(Character);
	if (!SavedState)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			if (SavedState->bHadExternalGravityScaleOverride)
			{
				SideViewMovementComponent->SetExternalGravityScaleOverride(SavedState->ExternalGravityScaleOverride);
			}
			else
			{
				SideViewMovementComponent->ClearExternalGravityScaleOverride();
			}
		}
		else
		{
			MovementComponent->GravityScale = SavedState->GravityScale;
		}
		MovementComponent->SetMovementMode(SavedState->MovementMode);
	}

	AffectedCharacters.Remove(Character);
}
