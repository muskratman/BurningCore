#include "Platformer/Environment/PlatformerSpikes.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerSpikes::APlatformerSpikes()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerSpikes.PlatformerSpikes")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshLayoutRoot"));
	MeshLayoutRoot->SetupAttachment(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(MeshLayoutRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	DamageVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DamageVolumeLayoutRoot"));
	DamageVolumeLayoutRoot->SetupAttachment(Root);

	DamageVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageVolume"));
	DamageVolume->SetupAttachment(DamageVolumeLayoutRoot);
	DamageVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageVolume->SetCollisionObjectType(ECC_WorldDynamic);
	DamageVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DamageVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerSpikes::OnDamageVolumeBeginOverlap);
	DamageVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerSpikes::OnDamageVolumeEndOverlap);
}

void APlatformerSpikes::SetSpikeSize(const FVector& InSpikeSize)
{
	SpikeSize = InSpikeSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerSpikes::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector MeshScale = FVector(
		FMath::Max(SpikeSize.X, 1.0f) / 100.0f,
		FMath::Max(SpikeSize.Y, 1.0f) / 100.0f,
		FMath::Max(SpikeSize.Z, 1.0f) / 100.0f);

	PlatformerEnvironment::ApplyRelativeTransform(
		MeshLayoutRoot,
		FVector(0.0f, 0.0f, SpikeSize.Z * 0.5f),
		FRotator::ZeroRotator,
		MeshScale,
		MeshTransformOffset);

	DamageVolume->SetBoxExtent(SpikeSize * 0.5f);
	PlatformerEnvironment::ApplyRelativeTransform(
		DamageVolumeLayoutRoot,
		FVector(0.0f, 0.0f, SpikeSize.Z * 0.5f),
		FRotator::ZeroRotator,
		FVector::OneVector,
		DamageVolumeTransformOffset);
}

void APlatformerSpikes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (auto It = OverlappingActors.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		TryDamageActor(It->Get());
	}
}

void APlatformerSpikes::OnDamageVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	OverlappingActors.Add(OtherActor);
	TryDamageActor(OtherActor);
}

void APlatformerSpikes::OnDamageVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingActors.Remove(OtherActor);
	}
}

void APlatformerSpikes::TryDamageActor(AActor* OtherActor)
{
	if (!CanDamageActor(OtherActor))
	{
		return;
	}

	FHitResult HitResult;
	HitResult.Location = OtherActor->GetActorLocation();
	HitResult.ImpactPoint = OtherActor->GetActorLocation();
	HitResult.ImpactNormal = FVector::UpVector;

	if (PlatformerEnvironment::ApplyConfiguredDamage(this, OtherActor, DamageEffectClass, DamageAmount, HitResult))
	{
		LastDamageTimeByActor.Add(OtherActor, GetWorld()->GetTimeSeconds());

		if (bLaunchCharacters)
		{
			if (ACharacter* Character = Cast<ACharacter>(OtherActor))
			{
				const FVector AwayDirection = FVector(Character->GetActorLocation().X - GetActorLocation().X, Character->GetActorLocation().Y - GetActorLocation().Y, 0.0f).GetSafeNormal();
				const FVector LaunchVelocity = (AwayDirection * HorizontalKnockback) + (FVector::UpVector * VerticalKnockback);
				Character->LaunchCharacter(LaunchVelocity, true, true);
			}
		}
	}
}

bool APlatformerSpikes::CanDamageActor(AActor* OtherActor) const
{
	if (!OtherActor || OtherActor == this)
	{
		return false;
	}

	if (const float* LastDamageTime = LastDamageTimeByActor.Find(OtherActor))
	{
		return (GetWorld()->GetTimeSeconds() - *LastDamageTime) >= DamageCooldown;
	}

	return true;
}
