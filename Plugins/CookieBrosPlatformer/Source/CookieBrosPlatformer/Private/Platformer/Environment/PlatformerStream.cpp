#include "Platformer/Environment/PlatformerStream.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerStream::APlatformerStream()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerStream.PlatformerStream")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	VolumeMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VolumeMeshLayoutRoot"));
	VolumeMeshLayoutRoot->SetupAttachment(Root);

	VolumeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolumeMesh"));
	VolumeMesh->SetupAttachment(VolumeMeshLayoutRoot);
	VolumeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	Volume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerStream::OnVolumeBeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &APlatformerStream::OnVolumeEndOverlap);
}

void APlatformerStream::SetVolumeSize(const FVector& InVolumeSize)
{
	VolumeSize = InVolumeSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerStream::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector ResolvedVolumeSize = VolumeSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const FVector BoxExtent = ResolvedVolumeSize * 0.5f;
	const FVector ComponentLocation(0.0f, 0.0f, BoxExtent.Z);
	const FVector MeshLocation = bVolumeMeshUsesBottomPivot ? FVector::ZeroVector : ComponentLocation;

	Volume->SetBoxExtent(BoxExtent);

	PlatformerEnvironment::ApplyRelativeTransform(
		VolumeMeshLayoutRoot,
		MeshLocation,
		FRotator::ZeroRotator,
		ResolvedVolumeSize / 100.0f,
		VolumeMeshTransformOffset);

	PlatformerEnvironment::ApplyRelativeTransform(
		VolumeLayoutRoot,
		ComponentLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		VolumeTransformOffset);
}

void APlatformerStream::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector WorldDelta = GetActorTransform().TransformVectorNoScale(LocalFlowVelocity) * DeltaTime;

	for (auto It = AffectedCharacters.CreateIterator(); It; ++It)
	{
		ACharacter* Character = It->Key.Get();
		if (Character == nullptr)
		{
			It.RemoveCurrent();
			continue;
		}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (!bAffectAirborneCharacters && !MovementComponent->IsMovingOnGround())
			{
				continue;
			}
		}

		Character->AddActorWorldOffset(WorldDelta, true);
	}
}

void APlatformerStream::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (const TPair<TWeakObjectPtr<ACharacter>, FPlatformerStreamMovementState>& Pair : AffectedCharacters)
	{
		if (Pair.Key.IsValid())
		{
			RestoreCharacter(Pair.Key.Get());
		}
	}

	AffectedCharacters.Empty();
	Super::EndPlay(EndPlayReason);
}

void APlatformerStream::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if ((Character == nullptr) || AffectedCharacters.Contains(Character))
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		FPlatformerStreamMovementState SavedState;
		SavedState.MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		SavedState.MaxAcceleration = MovementComponent->MaxAcceleration;
		SavedState.BrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		AffectedCharacters.Add(Character, SavedState);

		MovementComponent->MaxWalkSpeed *= MovementSpeedMultiplier;
		MovementComponent->MaxAcceleration *= MovementSpeedMultiplier;
		MovementComponent->BrakingDecelerationWalking *= MovementSpeedMultiplier;
	}
}

void APlatformerStream::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		RestoreCharacter(Character);
	}
}

void APlatformerStream::RestoreCharacter(ACharacter* Character)
{
	if (Character == nullptr)
	{
		return;
	}

	const FPlatformerStreamMovementState* SavedState = AffectedCharacters.Find(Character);
	if (SavedState == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = SavedState->MaxWalkSpeed;
		MovementComponent->MaxAcceleration = SavedState->MaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = SavedState->BrakingDecelerationWalking;
	}

	AffectedCharacters.Remove(Character);
}
