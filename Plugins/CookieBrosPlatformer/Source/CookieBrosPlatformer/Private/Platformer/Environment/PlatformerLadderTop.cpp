#include "Platformer/Environment/PlatformerLadderTop.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerDropThroughPlatformComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerLadderTop::APlatformerLadderTop()
{
	TopBlockMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TopBlockMeshLayoutRoot"));
	TopBlockMeshLayoutRoot->SetupAttachment(Root);

	TopBlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopBlockMesh"));
	TopBlockMesh->SetupAttachment(TopBlockMeshLayoutRoot);
	TopBlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TopBlockMesh->SetCollisionObjectType(ECC_WorldStatic);
	TopBlockMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TopBlockMesh->SetGenerateOverlapEvents(true);
	TopBlockMesh->SetCanEverAffectNavigation(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TopBlockMesh->SetStaticMesh(CubeMesh.Object);
	}

	TopBlockDropThroughLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TopBlockDropThroughLayoutRoot"));
	TopBlockDropThroughLayoutRoot->SetupAttachment(Root);

	TopBlockDropThroughTopCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TopBlockDropThroughTopCheckLayoutRoot"));
	TopBlockDropThroughTopCheckLayoutRoot->SetupAttachment(TopBlockDropThroughLayoutRoot);

	TopBlockDropThroughTopCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TopBlockDropThroughTopCheckBox"));
	TopBlockDropThroughTopCheckBox->SetupAttachment(TopBlockDropThroughTopCheckLayoutRoot);

	TopBlockDropThroughBottomCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TopBlockDropThroughBottomCheckLayoutRoot"));
	TopBlockDropThroughBottomCheckLayoutRoot->SetupAttachment(TopBlockDropThroughLayoutRoot);

	TopBlockDropThroughBottomCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TopBlockDropThroughBottomCheckBox"));
	TopBlockDropThroughBottomCheckBox->SetupAttachment(TopBlockDropThroughBottomCheckLayoutRoot);

	TopBlockDropThroughPlatformComponent = CreateDefaultSubobject<UPlatformerDropThroughPlatformComponent>(TEXT("TopBlockDropThroughPlatform"));
	TopBlockDropThroughPlatformComponent->InitializeDropThroughPlatform(
		TopBlockMesh,
		TopBlockDropThroughTopCheckLayoutRoot,
		TopBlockDropThroughTopCheckBox,
		TopBlockDropThroughBottomCheckLayoutRoot,
		TopBlockDropThroughBottomCheckBox);

	TopEntryVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TopEntryVolume"));
	TopEntryVolume->SetupAttachment(Root);
	TopEntryVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TopEntryVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TopEntryVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TopEntryVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TopEntryVolume->SetGenerateOverlapEvents(true);
	TopEntryVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerLadderTop::OnTopEntryVolumeBeginOverlap);
	TopEntryVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerLadderTop::OnTopEntryVolumeEndOverlap);
}

bool APlatformerLadderTop::RequestCharacterDropThrough(ACharacter* Character)
{
	return bTopSectionEnabled && TopBlockDropThroughPlatformComponent
		? TopBlockDropThroughPlatformComponent->RequestCharacterDropThrough(Character)
		: false;
}

void APlatformerLadderTop::SetTopBlockDepth(float InTopBlockDepth)
{
	TopBlockDepth = FMath::Max(InTopBlockDepth, 1.0f);
	RefreshLadderLayout();
}

void APlatformerLadderTop::SetTopSectionEnabled(bool bInTopSectionEnabled)
{
	if (bTopSectionEnabled == bInTopSectionEnabled)
	{
		ApplyTopSectionEnabled();
		return;
	}

	bTopSectionEnabled = bInTopSectionEnabled;
	ApplyTopSectionEnabled();
}

bool APlatformerLadderTop::CanCharacterEnterLadder(const APlatformerCharacterBase* Character, float DesiredClimbInput) const
{
	if (!Super::CanCharacterEnterLadder(Character, DesiredClimbInput))
	{
		return false;
	}

	if (!bTopSectionEnabled)
	{
		return true;
	}

	// Block re-entry from above going upward — character is already on top.
	return !IsCharacterOnTopBlock(Character) || DesiredClimbInput < 0.0f;
}

void APlatformerLadderTop::HandleCharacterEnteredLadder(APlatformerCharacterBase* Character, float DesiredClimbInput)
{
	Super::HandleCharacterEnteredLadder(Character, DesiredClimbInput);
	// Collision bypass is handled by OnCharacterMounted, which is called from
	// SetLadderState(Mounting) after the character has been snapped to the ladder.
}

void APlatformerLadderTop::OnCharacterMounted(APlatformerCharacterBase* Character)
{
	// Disable ALL collision on the top block so the character can pass through
	// it in both directions (climbing up from below, descending from above).
	//
	// SetCollisionEnabled(NoCollision) is used rather than per-channel ECR
	// because it bypasses Blueprint collision preset overrides and is guaranteed
	// to make the mesh invisible to EVERY query — SafeMoveUpdatedComponent
	// sweeps, FindFloor floor detection, and physics contacts.
	if (bTopSectionEnabled && TopBlockMesh)
	{
		TopBlockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APlatformerLadderTop::HandleCharacterExitedAtTop(APlatformerCharacterBase* Character)
{
	Super::HandleCharacterExitedAtTop(Character);

	// SnapCharacterToLadderTopSurface (called just before this) has already
	// placed the character at the top surface via TeleportPhysics, so
	// re-enabling collision here is safe: the capsule sits on top of the block,
	// not inside it. SetMovementMode(MOVE_Walking) will run FindFloor next tick
	// and detect the block as a valid floor.
	if (bTopSectionEnabled && TopBlockMesh)
	{
		TopBlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void APlatformerLadderTop::OnCharacterDismountedBottom(APlatformerCharacterBase* Character)
{
	// Restore full collision after a bottom or sideways dismount.
	if (bTopSectionEnabled && TopBlockMesh)
	{
		TopBlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void APlatformerLadderTop::RefreshLadderLayout()
{
	Super::RefreshLadderLayout();

	const FVector ResolvedLadderSize = LadderSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	RefreshTopLadderMeshLayout(ResolvedLadderSize);
	RefreshTopLadderClimbVolumeLayout(ResolvedLadderSize);

	const float TopBlockHeight = GetResolvedTopBlockHeight(ResolvedLadderSize);
	const float TopBlockBottomZ = FMath::Max(ResolvedLadderSize.Z - TopBlockHeight, 0.0f);
	const FVector TopBlockSize(ResolvedLadderSize.X, FMath::Max(TopBlockDepth, 1.0f), TopBlockHeight);

	if (TopBlockMeshLayoutRoot)
	{
		TopBlockMeshLayoutRoot->SetRelativeLocation(FVector::ZeroVector);
		TopBlockMeshLayoutRoot->SetRelativeRotation(FRotator::ZeroRotator);
		TopBlockMeshLayoutRoot->SetRelativeScale3D(FVector::OneVector);
	}

	if (TopBlockMesh)
	{
		const FBoxSphereBounds MeshBounds = TopBlockMesh->GetStaticMesh()
			? TopBlockMesh->GetStaticMesh()->GetBounds()
			: FBoxSphereBounds(FVector::ZeroVector, FVector(50.0f), 50.0f);
		const FVector MeshSize = (MeshBounds.BoxExtent * 2.0f).ComponentMax(FVector(1.0f, 1.0f, 1.0f));
		const FVector MeshScale = TopBlockSize / MeshSize;
		const float MeshBottomZ = (MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z) * MeshScale.Z;
		FPlatformerComponentTransformOffset ResolvedTopBlockMeshTransformOffset = TopBlockMeshTransformOffset;
		ResolvedTopBlockMeshTransformOffset.RelativeLocation.Y = 0.0f;
		const float TopBlockCenterY = ClimbVolumeTransformOffset.RelativeLocation.Y;

		PlatformerEnvironment::ApplyRelativeTransform(
			TopBlockMesh,
			FVector(
				-MeshBounds.Origin.X * MeshScale.X,
				TopBlockCenterY - (MeshBounds.Origin.Y * MeshScale.Y),
				TopBlockBottomZ - MeshBottomZ),
			FRotator::ZeroRotator,
			MeshScale,
			ResolvedTopBlockMeshTransformOffset);
	}

	if (TopBlockDropThroughLayoutRoot)
	{
		TopBlockDropThroughLayoutRoot->SetRelativeLocation(FVector(0.0f, ClimbVolumeTransformOffset.RelativeLocation.Y, TopBlockBottomZ));
		TopBlockDropThroughLayoutRoot->SetRelativeRotation(FRotator::ZeroRotator);
		TopBlockDropThroughLayoutRoot->SetRelativeScale3D(FVector::OneVector);
	}

	if (TopBlockDropThroughPlatformComponent)
	{
		TopBlockDropThroughPlatformComponent->RefreshDropThroughPlatformLayout(TopBlockSize);
	}

	if (TopEntryVolume)
	{
		const float ResolvedEntryHeight = FMath::Max(TopEntryVolumeHeight, 1.0f);
		TopEntryVolume->SetBoxExtent(FVector(TopBlockSize.X, TopBlockSize.Y, ResolvedEntryHeight) * 0.5f);
		TopEntryVolume->SetRelativeLocation(FVector(0.0f, ClimbVolumeTransformOffset.RelativeLocation.Y, ResolvedLadderSize.Z + ResolvedEntryHeight * 0.5f));
	}

	ApplyTopSectionEnabled();
}

void APlatformerLadderTop::ApplyTopSectionEnabled()
{
	auto ApplySceneEnabled = [this](USceneComponent* Component)
	{
		if (Component)
		{
			Component->SetHiddenInGame(!bTopSectionEnabled, true);
			Component->SetVisibility(bTopSectionEnabled, true);
		}
	};

	auto ApplyPrimitiveEnabled = [this, &ApplySceneEnabled](UPrimitiveComponent* Component, ECollisionEnabled::Type EnabledCollision)
	{
		if (Component)
		{
			ApplySceneEnabled(Component);
			Component->SetCollisionEnabled(bTopSectionEnabled ? EnabledCollision : ECollisionEnabled::NoCollision);
			Component->SetGenerateOverlapEvents(bTopSectionEnabled);
		}
	};

	ApplySceneEnabled(TopBlockMeshLayoutRoot);
	ApplyPrimitiveEnabled(TopBlockMesh, ECollisionEnabled::QueryAndPhysics);
	ApplySceneEnabled(TopBlockDropThroughLayoutRoot);
	ApplySceneEnabled(TopBlockDropThroughTopCheckLayoutRoot);
	ApplyPrimitiveEnabled(TopBlockDropThroughTopCheckBox, ECollisionEnabled::QueryOnly);
	ApplySceneEnabled(TopBlockDropThroughBottomCheckLayoutRoot);
	ApplyPrimitiveEnabled(TopBlockDropThroughBottomCheckBox, ECollisionEnabled::QueryOnly);
	ApplyPrimitiveEnabled(TopEntryVolume, ECollisionEnabled::QueryOnly);
}

bool APlatformerLadderTop::IsCharacterOnTopBlock(const APlatformerCharacterBase* Character) const
{
	if (!bTopSectionEnabled || !IsValid(Character))
	{
		return false;
	}

	float CharacterFeetZ = Character->GetActorLocation().Z;
	if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
	{
		CharacterFeetZ -= CapsuleComponent->GetScaledCapsuleHalfHeight();
	}

	const float TopSurfaceZ = GetActorLocation().Z + LadderSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f)).Z;
	return CharacterFeetZ >= TopSurfaceZ - FMath::Max(TopBlockFeetTolerance, 0.0f);
}

float APlatformerLadderTop::GetResolvedTopBlockHeight(const FVector& ResolvedLadderSize) const
{
	return FMath::Clamp(
		ResolvedLadderSize.X * FMath::Max(TopBlockHeightRatio, 0.01f),
		1.0f,
		ResolvedLadderSize.Z);
}

void APlatformerLadderTop::RefreshTopLadderMeshLayout(const FVector& ResolvedLadderSize)
{
	FPlatformerComponentTransformOffset ResolvedVisualOffset = LadderMeshTransformOffset;
	ResolvedVisualOffset.RelativeScale3D = FVector::OneVector;

	PlatformerEnvironment::ApplyRelativeTransform(
		LadderMeshLayoutRoot,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		ResolvedVisualOffset);

	if (!LadderMeshInstances)
	{
		return;
	}

	LadderMeshInstances->ClearInstances();

	const UStaticMesh* LadderStaticMesh = LadderMeshInstances->GetStaticMesh();
	if (!LadderStaticMesh)
	{
		return;
	}

	const FBoxSphereBounds MeshBounds = LadderStaticMesh->GetBounds();
	const FVector MeshSize = (MeshBounds.BoxExtent * 2.0f).ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const FVector InstanceScale(ResolvedLadderSize.X / MeshSize.X, 1.0f, 1.0f);
	const float EffectiveTileHeight = MeshSize.Z * InstanceScale.Z;
	const int32 TileCount = FMath::Max(1, FMath::CeilToInt(ResolvedLadderSize.Z / EffectiveTileHeight));
	const float MeshBottomZ = (MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z) * InstanceScale.Z;

	for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
	{
		const float DesiredInstanceBottomZ = TileIndex * EffectiveTileHeight;
		const FTransform InstanceTransform(
			FRotator::ZeroRotator,
			FVector(0.0f, 0.0f, DesiredInstanceBottomZ - MeshBottomZ),
			InstanceScale);
		LadderMeshInstances->AddInstance(InstanceTransform);
	}
}

void APlatformerLadderTop::RefreshTopLadderClimbVolumeLayout(const FVector& ResolvedLadderSize)
{
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

void APlatformerLadderTop::OnTopEntryVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bTopSectionEnabled)
	{
		return;
	}

	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		// "Push up" mechanic: if the character is actively climbing this ladder
		// upward when their capsule enters the top entry zone, finish the climb
		// by teleporting them onto the top platform. This bypasses the fragile
		// TopFinish FSM timer / animation timing and is the canonical signal
		// that the character has reached the top.
		if (PlatformerCharacter->GetActiveLadder() == this &&
			PlatformerCharacter->IsOnLadder() &&
			PlatformerCharacter->GetLadderClimbInput() > 0.0f)
		{
			PlatformerCharacter->FinishLadderClimbAtTop(this);
			return;
		}

		// Otherwise this is a character walking onto the top platform from the
		// side or above — register the ladder as available so a "down" press
		// can start a descent.
		PlatformerCharacter->NotifyLadderAvailable(this);
	}
}

void APlatformerLadderTop::OnTopEntryVolumeEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!bTopSectionEnabled)
	{
		return;
	}

	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		if (PlatformerCharacter->GetActiveLadder() == this)
		{
			return;
		}

		PlatformerCharacter->NotifyLadderUnavailable(this);
	}
}
