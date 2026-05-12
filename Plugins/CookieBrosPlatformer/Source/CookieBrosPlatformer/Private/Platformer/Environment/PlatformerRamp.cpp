#include "Platformer/Environment/PlatformerRamp.h"

#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerRamp::APlatformerRamp()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerRamp.PlatformerRamp")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	RampMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RampMeshLayoutRoot"));
	RampMeshLayoutRoot->SetupAttachment(Root);

	RampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RampMesh"));
	RampMesh->SetupAttachment(RampMeshLayoutRoot);
	RampMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RampMesh->SetCollisionObjectType(ECC_WorldStatic);
	RampMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RampMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Ramp45Mesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Ramp_45.SM_Platform_Ramp_45"));
	if (Ramp45Mesh.Succeeded())
	{
		DefaultRampMesh = Ramp45Mesh.Object;
		RampMesh->SetStaticMesh(Ramp45Mesh.Object);

		FPlatformerRampMeshVariant& Variant = RampMeshVariants.AddDefaulted_GetRef();
		Variant.AngleDegrees = 45;
		Variant.StaticMesh = Ramp45Mesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Ramp15Mesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Ramp15.SM_Platform_Ramp15"));
	if (Ramp15Mesh.Succeeded())
	{
		FPlatformerRampMeshVariant& Variant = RampMeshVariants.AddDefaulted_GetRef();
		Variant.AngleDegrees = 15;
		Variant.StaticMesh = Ramp15Mesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Ramp25Mesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Ramp25.SM_Platform_Ramp25"));
	if (Ramp25Mesh.Succeeded())
	{
		FPlatformerRampMeshVariant& Variant = RampMeshVariants.AddDefaulted_GetRef();
		Variant.AngleDegrees = 25;
		Variant.StaticMesh = Ramp25Mesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Ramp30Mesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Ramp30.SM_Platform_Ramp30"));
	if (Ramp30Mesh.Succeeded())
	{
		FPlatformerRampMeshVariant& Variant = RampMeshVariants.AddDefaulted_GetRef();
		Variant.AngleDegrees = 30;
		Variant.StaticMesh = Ramp30Mesh.Object;
	}
}

void APlatformerRamp::SetRampSize(const FVector& InRampSize)
{
	RampSize = InRampSize.ComponentMax(FVector::ZeroVector);
}

void APlatformerRamp::SetRampAngleDegrees(int32 InRampAngleDegrees)
{
	RampAngleDegrees = FMath::Max(1, InRampAngleDegrees);
}

UStaticMesh* APlatformerRamp::ResolveRampStaticMesh() const
{
	for (const FPlatformerRampMeshVariant& Variant : RampMeshVariants)
	{
		if ((Variant.AngleDegrees == RampAngleDegrees) && (Variant.StaticMesh != nullptr))
		{
			return Variant.StaticMesh;
		}
	}

	return DefaultRampMesh;
}

void APlatformerRamp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UStaticMesh* ResolvedRampMesh = ResolveRampStaticMesh())
	{
		RampMesh->SetStaticMesh(ResolvedRampMesh);
	}

	PlatformerEnvironment::ApplyRelativeTransform(
		RampMeshLayoutRoot,
		FVector(0.0f, 0.0f, RampSize.Z * 0.5f),
		FRotator::ZeroRotator,
		RampSize / 100.0f,
		RampMeshTransformOffset);
	RampMesh->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Increase, 89.0f));
}
