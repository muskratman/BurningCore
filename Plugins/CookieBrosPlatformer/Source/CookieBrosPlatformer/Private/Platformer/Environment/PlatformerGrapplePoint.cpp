#include "Platformer/Environment/PlatformerGrapplePoint.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerGrapplePoint::APlatformerGrapplePoint()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerGrapplePoint.PlatformerGrapplePoint")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GrappleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrappleMesh"));
	GrappleMesh->SetupAttachment(Root);
	GrappleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		GrappleMesh->SetStaticMesh(SphereMesh.Object);
	}

	GrappleAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("GrappleAnchor"));
	GrappleAnchor->SetupAttachment(Root);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(Root);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APlatformerGrapplePoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DetectionSphere->SetSphereRadius(FMath::Max(DetectionRadius, 1.0f));
}

FVector APlatformerGrapplePoint::GetGrappleLocation() const
{
	return GrappleAnchor->GetComponentLocation();
}

bool APlatformerGrapplePoint::CanBeGrappled() const
{
	return bEnabled;
}
