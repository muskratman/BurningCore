#include "Platformer/Environment/PlatformerHalfBlock.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerDropThroughPlatformComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerHalfBlock::APlatformerHalfBlock()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerBlock.PlatformerBlock")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PlatformMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlatformMeshLayoutRoot"));
	PlatformMeshLayoutRoot->SetupAttachment(Root);

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PlatformMeshLayoutRoot);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionObjectType(ECC_WorldStatic);
	PlatformMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlatformMesh->SetGenerateOverlapEvents(true);
	PlatformMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlatformBottomMesh(TEXT("/CookieBrosPlatformer/Meshes/Platform_Bottom.Platform_Bottom"));
	if (PlatformBottomMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(PlatformBottomMesh.Object);
	}

	DropThroughLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropThroughLayoutRoot"));
	DropThroughLayoutRoot->SetupAttachment(Root);

	DropThroughTopCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropThroughTopCheckLayoutRoot"));
	DropThroughTopCheckLayoutRoot->SetupAttachment(DropThroughLayoutRoot);

	DropThroughTopCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DropThroughTopCheckBox"));
	DropThroughTopCheckBox->SetupAttachment(DropThroughTopCheckLayoutRoot);

	DropThroughBottomCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropThroughBottomCheckLayoutRoot"));
	DropThroughBottomCheckLayoutRoot->SetupAttachment(DropThroughLayoutRoot);

	DropThroughBottomCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DropThroughBottomCheckBox"));
	DropThroughBottomCheckBox->SetupAttachment(DropThroughBottomCheckLayoutRoot);

	DropThroughPlatformComponent = CreateDefaultSubobject<UPlatformerDropThroughPlatformComponent>(TEXT("DropThroughPlatform"));
	DropThroughPlatformComponent->InitializeDropThroughPlatform(
		PlatformMesh,
		DropThroughTopCheckLayoutRoot,
		DropThroughTopCheckBox,
		DropThroughBottomCheckLayoutRoot,
		DropThroughBottomCheckBox);
}

void APlatformerHalfBlock::SetPlatformSize(const FVector& InPlatformSize)
{
	PlatformSize = InPlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	RefreshHalfBlockLayout();
}

void APlatformerHalfBlock::SetHalfTop(bool bInHalfTop)
{
	bHalfTop = bInHalfTop;
	RefreshHalfBlockLayout();
}

bool APlatformerHalfBlock::RequestCharacterDropThrough(ACharacter* Character)
{
	return DropThroughPlatformComponent
		? DropThroughPlatformComponent->RequestCharacterDropThrough(Character)
		: false;
}

void APlatformerHalfBlock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshHalfBlockLayout();
}

void APlatformerHalfBlock::RefreshHalfBlockLayout()
{
	const FVector ResolvedPlatformSize = PlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const float HalfBlockHeight = GetResolvedHalfBlockHeight();
	const float HalfBlockBottomZ = GetResolvedHalfBlockBottomZ();
	const FVector HalfBlockSize(ResolvedPlatformSize.X, ResolvedPlatformSize.Y, HalfBlockHeight);

	PlatformerEnvironment::ApplyRelativeTransform(
		PlatformMesh,
		FVector(0.0f, 0.0f, (bHalfTop ? 0.5f : -0.5f) * ResolvedPlatformSize.Z),
		FRotator::ZeroRotator,
		ResolvedPlatformSize / 100.0f,
		PlatformMeshTransformOffset);

	if (DropThroughLayoutRoot)
	{
		DropThroughLayoutRoot->SetRelativeLocation(FVector(0.0f, 0.0f, HalfBlockBottomZ));
		DropThroughLayoutRoot->SetRelativeRotation(FRotator::ZeroRotator);
		DropThroughLayoutRoot->SetRelativeScale3D(FVector::OneVector);
	}

	if (DropThroughPlatformComponent)
	{
		DropThroughPlatformComponent->RefreshDropThroughPlatformLayout(HalfBlockSize);
	}
}

float APlatformerHalfBlock::GetResolvedHalfBlockHeight() const
{
	const FVector ResolvedPlatformSize = PlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	return ResolvedPlatformSize.Z * FMath::Clamp(HalfBlockHeightRatio, 0.01f, 1.0f);
}

float APlatformerHalfBlock::GetResolvedHalfBlockBottomZ() const
{
	const FVector ResolvedPlatformSize = PlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const float HalfBlockHeight = GetResolvedHalfBlockHeight();
	return bHalfTop ? (ResolvedPlatformSize.Z - HalfBlockHeight) : 0.0f;
}
