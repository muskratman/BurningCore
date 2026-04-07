#include "Platformer/Environment/PlatformerVanishingBlock.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APlatformerVanishingBlock::APlatformerVanishingBlock()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerVanishingBlock.PlatformerVanishingBlock")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BlockMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BlockMeshLayoutRoot"));
	BlockMeshLayoutRoot->SetupAttachment(Root);

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
	BlockMesh->SetupAttachment(BlockMeshLayoutRoot);
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockMesh->SetCollisionObjectType(ECC_WorldStatic);
	BlockMesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BlockMesh->SetStaticMesh(CubeMesh.Object);
	}

	TriggerVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TriggerVolumeLayoutRoot"));
	TriggerVolumeLayoutRoot->SetupAttachment(Root);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(TriggerVolumeLayoutRoot);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerVanishingBlock::OnTriggerVolumeBeginOverlap);
}

void APlatformerVanishingBlock::BeginPlay()
{
	Super::BeginPlay();

	SetBlockActive(bStartVisible);

	if (bAutoCycle)
	{
		const float FirstDelay = InitialDelay > 0.0f ? InitialDelay : (bCurrentlyVisible ? VisibleDuration : HiddenDuration);
		if (FirstDelay <= 0.0f)
		{
			if (bCurrentlyVisible)
			{
				HideBlock();
			}
			else
			{
				ShowBlock();
			}
		}
		else
		{
			if (bCurrentlyVisible)
			{
				GetWorldTimerManager().SetTimer(StateTimerHandle, this, &APlatformerVanishingBlock::HideBlock, FirstDelay, false);
			}
			else
			{
				GetWorldTimerManager().SetTimer(StateTimerHandle, this, &APlatformerVanishingBlock::ShowBlock, FirstDelay, false);
			}
		}
	}
}

void APlatformerVanishingBlock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlatformerEnvironment::ApplyRelativeTransform(
		BlockMeshLayoutRoot,
		FVector(0.0f, 0.0f, BlockSize.Z * 0.5f),
		FRotator::ZeroRotator,
		BlockSize / 100.0f,
		BlockMeshTransformOffset);

	TriggerVolume->SetBoxExtent(FVector(BlockSize.X * 0.45f, BlockSize.Y * 0.45f, 24.0f));
	PlatformerEnvironment::ApplyRelativeTransform(
		TriggerVolumeLayoutRoot,
		FVector(0.0f, 0.0f, BlockSize.Z + 16.0f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		TriggerVolumeTransformOffset);
}

void APlatformerVanishingBlock::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bTriggerOnCharacterOverlap || !bCurrentlyVisible || !Cast<ACharacter>(OtherActor))
	{
		return;
	}

	if (VisibleDuration <= 0.0f)
	{
		HideBlock();
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(StateTimerHandle))
	{
		GetWorldTimerManager().SetTimer(StateTimerHandle, this, &APlatformerVanishingBlock::HideBlock, VisibleDuration, false);
	}
}

void APlatformerVanishingBlock::ShowBlock()
{
	SetBlockActive(true);

	if (bAutoCycle && VisibleDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StateTimerHandle, this, &APlatformerVanishingBlock::HideBlock, VisibleDuration, false);
	}
}

void APlatformerVanishingBlock::HideBlock()
{
	SetBlockActive(false);

	if (HiddenDuration <= 0.0f)
	{
		if (bAutoCycle)
		{
			ShowBlock();
		}
		return;
	}

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &APlatformerVanishingBlock::ShowBlock, HiddenDuration, false);
}

void APlatformerVanishingBlock::SetBlockActive(bool bNewVisible)
{
	bCurrentlyVisible = bNewVisible;
	BlockMesh->SetHiddenInGame(!bNewVisible);
	BlockMesh->SetCollisionEnabled(bNewVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	TriggerVolume->SetCollisionEnabled(bNewVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}
