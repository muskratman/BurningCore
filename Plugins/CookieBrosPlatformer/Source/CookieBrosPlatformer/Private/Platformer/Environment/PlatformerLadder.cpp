#include "Platformer/Environment/PlatformerLadder.h"

#include "Character/PlatformerCharacterBase.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerLadder::APlatformerLadder()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerLadder.PlatformerLadder")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	LadderMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LadderMeshLayoutRoot"));
	LadderMeshLayoutRoot->SetupAttachment(Root);

	LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
	LadderMesh->SetupAttachment(LadderMeshLayoutRoot);
	LadderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		LadderMesh->SetStaticMesh(CubeMesh.Object);
	}

	ClimbVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ClimbVolumeLayoutRoot"));
	ClimbVolumeLayoutRoot->SetupAttachment(Root);

	ClimbVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ClimbVolume"));
	ClimbVolume->SetupAttachment(ClimbVolumeLayoutRoot);
	ClimbVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClimbVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ClimbVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClimbVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ClimbVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerLadder::OnClimbVolumeBeginOverlap);
	ClimbVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerLadder::OnClimbVolumeEndOverlap);
}

void APlatformerLadder::SetLadderSize(const FVector& InLadderSize)
{
	LadderSize = InLadderSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerLadder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlatformerEnvironment::ApplyRelativeTransform(
		LadderMeshLayoutRoot,
		FVector(0.0f, 0.0f, LadderSize.Z * 0.5f),
		FRotator::ZeroRotator,
		LadderSize / 100.0f,
		LadderMeshTransformOffset);

	ClimbVolume->SetBoxExtent(LadderSize * 0.5f);
	PlatformerEnvironment::ApplyRelativeTransform(
		ClimbVolumeLayoutRoot,
		FVector(0.0f, 0.0f, LadderSize.Z * 0.5f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		ClimbVolumeTransformOffset);
}

void APlatformerLadder::OnClimbVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		PlatformerCharacter->NotifyLadderAvailable(this);
	}
}

void APlatformerLadder::OnClimbVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		PlatformerCharacter->NotifyLadderUnavailable(this);
	}
}
