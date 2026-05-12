#include "Platformer/Environment/PlatformerBlockBase.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerBlockBase::APlatformerBlockBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BlockMeshLayoutRoot"));
	BlockMeshLayoutRoot->SetupAttachment(Root);

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
	BlockMesh->SetupAttachment(BlockMeshLayoutRoot);
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockMesh->SetCollisionObjectType(ECC_WorldStatic);
	BlockMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BlockMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Cube.SM_Platform_Cube"));
	if (CubeMesh.Succeeded())
	{
		BlockMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void APlatformerBlockBase::SetBlockSize(const FVector& InBlockSize)
{
	BlockSize = InBlockSize.ComponentMax(FVector::ZeroVector);
	RefreshBlockLayout();
}

void APlatformerBlockBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshBlockLayout();
}

void APlatformerBlockBase::RefreshBlockLayout()
{
	PlatformerEnvironment::ApplyRelativeTransform(
		BlockMeshLayoutRoot,
		FVector(0.0f, 0.0f, BlockSize.Z * 0.5f),
		FRotator::ZeroRotator,
		BlockSize / 100.0f,
		BlockMeshTransformOffset);
}
