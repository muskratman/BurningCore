#include "Platformer/Environment/PlatformerLightningBase.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "UObject/ConstructorHelpers.h"

APlatformerLightningBase::APlatformerLightningBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(Root);
	DirectionalLight->SetMobility(EComponentMobility::Movable);
	DirectionalLight->SetRelativeLocation(FVector(0.0f, 0.0f, 400.0f));
	DirectionalLight->SetRelativeRotation(FRotator(-49.51711f, -10.308601f, 112.360672f));
	DirectionalLight->SetIntensity(6.0f);
	DirectionalLight->SetAtmosphereSunLight(true);
	DirectionalLight->SetAtmosphereSunLightIndex(0);

	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"));
	ExponentialHeightFog->SetupAttachment(Root);
	ExponentialHeightFog->SetMobility(EComponentMobility::Movable);
	ExponentialHeightFog->SetRelativeLocation(FVector(-5600.0f, -50.0f, -6850.0f));

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);
	SkyAtmosphere->SetMobility(EComponentMobility::Movable);
	SkyAtmosphere->SetRelativeLocation(FVector(0.0f, 0.0f, -6000.0f));

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetRelativeLocation(FVector(0.0f, 0.0f, 600.0f));
	SkyLight->SetIntensity(1.0f);
	SkyLight->SetRealTimeCapture(true);

	SkySphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphereMesh"));
	SkySphereMesh->SetupAttachment(Root);
	SkySphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkySphereMesh->SetCastShadow(false);
	SkySphereMesh->SetCanEverAffectNavigation(false);
	SkySphereMesh->SetRelativeScale3D(FVector(400.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkySphereMeshAsset(TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere"));
	if (SkySphereMeshAsset.Succeeded())
	{
		SkySphereMesh->SetStaticMesh(SkySphereMeshAsset.Object);
	}

	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"));
	VolumetricCloud->SetupAttachment(Root);
	VolumetricCloud->SetMobility(EComponentMobility::Movable);
}
