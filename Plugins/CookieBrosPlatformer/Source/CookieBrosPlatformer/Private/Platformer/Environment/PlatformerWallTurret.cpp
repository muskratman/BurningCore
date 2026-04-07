#include "Platformer/Environment/PlatformerWallTurret.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "Platformer/Environment/PlatformerHazardProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APlatformerWallTurret::APlatformerWallTurret()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerWallTurret.PlatformerWallTurret")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	TurretMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretMeshLayoutRoot"));
	TurretMeshLayoutRoot->SetupAttachment(Root);

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(TurretMeshLayoutRoot);
	TurretMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TurretMesh->SetCollisionObjectType(ECC_WorldStatic);
	TurretMesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TurretMesh->SetStaticMesh(CubeMesh.Object);
	}

	MuzzlePointLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePointLayoutRoot"));
	MuzzlePointLayoutRoot->SetupAttachment(Root);

	MuzzlePoint = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(MuzzlePointLayoutRoot);

	ActivationVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ActivationVolumeLayoutRoot"));
	ActivationVolumeLayoutRoot->SetupAttachment(Root);

	ActivationVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationVolume"));
	ActivationVolume->SetupAttachment(ActivationVolumeLayoutRoot);
	ActivationVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivationVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ActivationVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivationVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ActivationVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerWallTurret::OnActivationVolumeBeginOverlap);
	ActivationVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerWallTurret::OnActivationVolumeEndOverlap);
}

void APlatformerWallTurret::BeginPlay()
{
	Super::BeginPlay();

	if (FireInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &APlatformerWallTurret::FireProjectile, FireInterval, true, FireInterval);
	}
}

void APlatformerWallTurret::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlatformerEnvironment::ApplyRelativeTransform(
		TurretMeshLayoutRoot,
		FVector(0.0f, 0.0f, 50.0f),
		FRotator::ZeroRotator,
		FVector(0.8f, 0.8f, 1.0f),
		TurretMeshTransformOffset);

	ActivationVolume->SetBoxExtent(ActivationExtent);

	PlatformerEnvironment::ApplyRelativeTransform(
		MuzzlePointLayoutRoot,
		FVector(-MuzzleOffset, 0.0f, 50.0f),
		FRotator(0.0f, 180.0f, 0.0f),
		FVector::OneVector,
		MuzzlePointTransformOffset);

	PlatformerEnvironment::ApplyRelativeTransform(
		ActivationVolumeLayoutRoot,
		FVector(-ActivationExtent.X, 0.0f, 50.0f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		ActivationVolumeTransformOffset);
}

void APlatformerWallTurret::OnActivationVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		TrackedTarget = Character;
	}
}

void APlatformerWallTurret::OnActivationVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (TrackedTarget.Get() == OtherActor)
	{
		TrackedTarget.Reset();
	}
}

void APlatformerWallTurret::FireProjectile()
{
	if (!IsAllowedToFire() || !ProjectileClass)
	{
		return;
	}

	const FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
	FRotator SpawnRotation = MuzzlePoint->GetComponentRotation();

	if (bAimAtTrackedTarget)
	{
		if (ACharacter* TargetCharacter = ResolveTrackedTarget())
		{
			SpawnRotation = (TargetCharacter->GetActorLocation() - SpawnLocation).Rotation();
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APlatformerHazardProjectile* Projectile = GetWorld()->SpawnActor<APlatformerHazardProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (!Projectile)
	{
		return;
	}

	Projectile->InitializeProjectile(ProjectileSpeed, ProjectileLifetime, ProjectileDamage, ProjectileDamageEffectClass, this);
}

bool APlatformerWallTurret::IsAllowedToFire() const
{
	if (bRequireTargetInRange)
	{
		return ResolveTrackedTarget() != nullptr;
	}

	return bAutoActivate;
}

ACharacter* APlatformerWallTurret::ResolveTrackedTarget() const
{
	return TrackedTarget.IsValid() ? TrackedTarget.Get() : nullptr;
}
