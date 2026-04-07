#include "Platformer/Environment/PlatformerBlock.h"

#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerBlock::APlatformerBlock()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerBlock.PlatformerBlock")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BlockMeshLayoutRoot"));
	BlockMeshLayoutRoot->SetupAttachment(Root);

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
	BlockMesh->SetupAttachment(BlockMeshLayoutRoot);
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockMesh->SetCollisionObjectType(ECC_WorldStatic);
	BlockMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BlockMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FullSizeMesh = CubeMesh.Object;
		BlockMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void APlatformerBlock::SetBlockSize(const FVector& InBlockSize)
{
	BlockSize = InBlockSize.ComponentMax(FVector::ZeroVector);
}

void APlatformerBlock::SetBlockMeshVariant(EPlatformerBlockMeshVariant InBlockMeshVariant)
{
	BlockMeshVariant = InBlockMeshVariant;
}

UStaticMesh* APlatformerBlock::ResolveBlockStaticMesh() const
{
	switch (BlockMeshVariant)
	{
	case EPlatformerBlockMeshVariant::HalfSize:
		return HalfSizeMesh != nullptr ? HalfSizeMesh : FullSizeMesh;

	case EPlatformerBlockMeshVariant::FullSize:
	default:
		return FullSizeMesh;
	}
}

void APlatformerBlock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UStaticMesh* ResolvedBlockMesh = ResolveBlockStaticMesh())
	{
		BlockMesh->SetStaticMesh(ResolvedBlockMesh);
	}

	PlatformerEnvironment::ApplyRelativeTransform(
		BlockMeshLayoutRoot,
		FVector(0.0f, 0.0f, BlockSize.Z * 0.5f),
		FRotator::ZeroRotator,
		BlockSize / 100.0f,
		BlockMeshTransformOffset);
}
