#include "Platformer/Environment/PlatformerLedgeGrab.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Traversal/PlatformerTraversalTypes.h"

namespace PlatformerLedgeGrabPrivate
{
	constexpr float FloorStandTolerance = 2.0f;
}

APlatformerLedgeGrab::APlatformerLedgeGrab()
{
	LedgeTriggerLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LedgeTriggerLayoutRoot"));
	LedgeTriggerLayoutRoot->SetupAttachment(Root);

	LedgeTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LedgeTriggerBox"));
	LedgeTriggerBox->SetupAttachment(LedgeTriggerLayoutRoot);
	LedgeTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LedgeTriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	LedgeTriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LedgeTriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LedgeTriggerBox->SetCanEverAffectNavigation(false);

	LedgeTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlatformerLedgeGrab::OnLedgeTriggerBeginOverlap);
	LedgeTriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlatformerLedgeGrab::OnLedgeTriggerEndOverlap);
}

void APlatformerLedgeGrab::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector TriggerExtent(
		FMath::Max((BlockSize.X * 0.5f) + TriggerHorizontalPadding, 1.0f),
		FMath::Max(BlockSize.Y * 0.5f, 1.0f),
		FMath::Max(TriggerVerticalExtent, 1.0f));
	LedgeTriggerBox->SetBoxExtent(TriggerExtent);

	PlatformerEnvironment::ApplyRelativeTransform(
		LedgeTriggerLayoutRoot,
		FVector(0.0f, 0.0f, BlockSize.Z),
		FRotator::ZeroRotator,
		FVector::OneVector,
		LedgeTriggerTransformOffset);
}

bool APlatformerLedgeGrab::TryBuildTraversalTargets(
	const ACharacter& Character,
	float DirectionSign,
	float TargetDepthY,
	const FPlatformerLedgeTraversalSettings& LedgeSettings,
	FVector& OutHangLocation,
	FVector& OutClimbTargetLocation,
	FVector& OutWallNormal) const
{
	if (!LedgeTriggerBox || FMath::IsNearlyZero(DirectionSign))
	{
		return false;
	}

	if (!LedgeTriggerBox->IsOverlappingActor(const_cast<ACharacter*>(&Character)))
	{
		return false;
	}

	const UCapsuleComponent* CapsuleComponent = Character.GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return false;
	}

	const float CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	const FVector CharacterLocation = Character.GetActorLocation();
	const float BlockHalfWidth = BlockSize.X * 0.5f;
	const float TopSurfaceZ = GetActorLocation().Z + BlockSize.Z;
	const float FeetZ = CharacterLocation.Z - CapsuleHalfHeight;
	const float LedgeHeight = TopSurfaceZ - FeetZ;
	if (LedgeHeight < LedgeSettings.MinHangHeight || LedgeHeight > LedgeSettings.MaxReachHeight)
	{
		return false;
	}

	const float WallFaceX = GetActorLocation().X - (DirectionSign * BlockHalfWidth);
	if ((DirectionSign > 0.0f && CharacterLocation.X >= WallFaceX)
		|| (DirectionSign < 0.0f && CharacterLocation.X <= WallFaceX))
	{
		return false;
	}

	OutWallNormal = FVector(-FMath::Sign(DirectionSign), 0.0f, 0.0f);

	OutHangLocation = FVector(
		WallFaceX + OutWallNormal.X * (CapsuleRadius - LedgeSettings.HangForwardOffset),
		TargetDepthY,
		TopSurfaceZ - CapsuleHalfHeight + LedgeSettings.HangVerticalOffset);

	OutClimbTargetLocation = FVector(
		OutHangLocation.X + DirectionSign * LedgeSettings.TopPointHorizontalOffset,
		TargetDepthY,
		TopSurfaceZ + CapsuleHalfHeight + PlatformerLedgeGrabPrivate::FloorStandTolerance);

	return true;
}

void APlatformerLedgeGrab::OnLedgeTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		PlatformerCharacter->NotifyLedgeGrabAvailable(this);
	}
}

void APlatformerLedgeGrab::OnLedgeTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(OtherActor))
	{
		PlatformerCharacter->NotifyLedgeGrabUnavailable(this);
	}
}
