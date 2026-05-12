#include "Platformer/Environment/PlatformerLadder.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerLadder::APlatformerLadder()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerLadder.PlatformerLadder")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	LadderMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LadderMeshLayoutRoot"));
	LadderMeshLayoutRoot->SetupAttachment(Root);

	LadderMeshInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("LadderMeshInstances"));
	LadderMeshInstances->SetupAttachment(LadderMeshLayoutRoot);
	LadderMeshInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LadderMeshInstances->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Ladder.SM_Platform_Ladder"));
	if (CubeMesh.Succeeded())
	{
		LadderMeshInstances->SetStaticMesh(CubeMesh.Object);
	}

	ClimbVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ClimbVolumeLayoutRoot"));
	ClimbVolumeLayoutRoot->SetupAttachment(Root);

	ClimbVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ClimbVolume"));
	ClimbVolume->SetupAttachment(ClimbVolumeLayoutRoot);
	ClimbVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClimbVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ClimbVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClimbVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ClimbVolume->SetGenerateOverlapEvents(true);

	ClimbVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerLadder::OnClimbVolumeBeginOverlap);
	ClimbVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerLadder::OnClimbVolumeEndOverlap);
}

void APlatformerLadder::SetLadderSize(const FVector& InLadderSize)
{
	LadderSize = InLadderSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	RefreshLadderLayout();
}

void APlatformerLadder::SetClimbVolumeTransformOffset(const FPlatformerComponentTransformOffset& InTransformOffset)
{
	ClimbVolumeTransformOffset = InTransformOffset;
	RefreshLadderLayout();
}

void APlatformerLadder::SetSnapCharacterDepthToLadder(bool bInSnapCharacterDepthToLadder)
{
	bSnapCharacterDepthToLadder = bInSnapCharacterDepthToLadder;
}

bool APlatformerLadder::CanCharacterEnterLadder(const APlatformerCharacterBase* Character, float DesiredClimbInput) const
{
	return IsValid(Character);
}

void APlatformerLadder::HandleCharacterEnteredLadder(APlatformerCharacterBase* Character, float DesiredClimbInput)
{
}

void APlatformerLadder::HandleCharacterExitedAtTop(APlatformerCharacterBase* Character)
{
}

void APlatformerLadder::OnCharacterMounted(APlatformerCharacterBase* Character)
{
}

void APlatformerLadder::OnCharacterDismountedBottom(APlatformerCharacterBase* Character)
{
}

float APlatformerLadder::GetClimbBottomWorldZ() const
{
	return ClimbVolume ? ClimbVolume->Bounds.GetBox().Min.Z : GetActorLocation().Z;
}

float APlatformerLadder::GetClimbTopWorldZ() const
{
	return ClimbVolume ? ClimbVolume->Bounds.GetBox().Max.Z : GetActorLocation().Z + LadderSize.Z;
}

void APlatformerLadder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshLadderLayout();
}

void APlatformerLadder::RefreshLadderLayout()
{
	const FVector ResolvedLadderSize = LadderSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	FPlatformerComponentTransformOffset ResolvedVisualOffset = LadderMeshTransformOffset;
	ResolvedVisualOffset.RelativeScale3D = FVector::OneVector;

	PlatformerEnvironment::ApplyRelativeTransform(
		LadderMeshLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedLadderSize.Z * 0.5f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		ResolvedVisualOffset);

	if (LadderMeshInstances)
	{
		LadderMeshInstances->ClearInstances();

		if (const UStaticMesh* LadderStaticMesh = LadderMeshInstances->GetStaticMesh())
		{
			const FVector MeshSize = (LadderStaticMesh->GetBounds().BoxExtent * 2.0f).ComponentMax(FVector(1.0f, 1.0f, 1.0f));
			const FVector InstanceScale(
				ResolvedLadderSize.X / MeshSize.X,
				1.0f,
				1.0f);
			const float EffectiveTileHeight = MeshSize.Z;
			const int32 TileCount = FMath::Max(1, FMath::CeilToInt(ResolvedLadderSize.Z / EffectiveTileHeight));
			const float LocalStartZ = (-0.5f * ResolvedLadderSize.Z) + (0.5f * EffectiveTileHeight);
			const float LocalStepZ = EffectiveTileHeight;

			for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
			{
				const FTransform InstanceTransform(
					FRotator::ZeroRotator,
					FVector(0.0f, 0.0f, LocalStartZ + (TileIndex * LocalStepZ)),
					InstanceScale);
				LadderMeshInstances->AddInstance(InstanceTransform);
			}
		}
	}

	if (ClimbVolume)
	{
		ClimbVolume->SetBoxExtent(ResolvedLadderSize * 0.5f);
	}

	PlatformerEnvironment::ApplyRelativeTransform(
		ClimbVolumeLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedLadderSize.Z * 0.5f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		ClimbVolumeTransformOffset);
}

void APlatformerLadder::BeginPlay()
{
	Super::BeginPlay();
	SyncOverlappingCharacters();
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

void APlatformerLadder::SyncOverlappingCharacters()
{
	if (!ClimbVolume)
	{
		return;
	}

	ClimbVolume->UpdateOverlaps();

	TArray<AActor*> OverlappingCharacters;
	ClimbVolume->GetOverlappingActors(OverlappingCharacters, APlatformerCharacterBase::StaticClass());
	for (AActor* OverlappingActor : OverlappingCharacters)
	{
		if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OverlappingActor))
		{
			PlatformerCharacter->NotifyLadderAvailable(this);
		}
	}
}
