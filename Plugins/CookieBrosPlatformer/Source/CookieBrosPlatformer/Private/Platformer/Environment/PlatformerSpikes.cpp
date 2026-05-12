#include "Platformer/Environment/PlatformerSpikes.h"

#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);

	InstancedSpikeMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedSpikeMesh"));
	InstancedSpikeMesh->SetupAttachment(MeshLayoutRoot);
	InstancedSpikeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	InstancedSpikeMesh->SetCollisionObjectType(ECC_WorldStatic);
	InstancedSpikeMesh->SetCollisionResponseToAllChannels(ECR_Block);
	InstancedSpikeMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SpikeMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Spikes.SM_Platform_Spikes"));
	if (SpikeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SpikeMesh.Object);
		InstancedSpikeMesh->SetStaticMesh(SpikeMesh.Object);
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

	PlatformerEnvironment::ApplyRelativeTransform(
		MeshLayoutRoot,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector,
		MeshTransformOffset);

	UStaticMesh* SpikeStaticMesh = Mesh ? Mesh->GetStaticMesh() : nullptr;

	if (Mesh)
	{
		Mesh->SetVisibility(false, true);
		Mesh->SetHiddenInGame(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (InstancedSpikeMesh)
	{
		InstancedSpikeMesh->ClearInstances();

		if (SpikeStaticMesh)
		{
			if (InstancedSpikeMesh->GetStaticMesh() != SpikeStaticMesh)
			{
				InstancedSpikeMesh->SetStaticMesh(SpikeStaticMesh);
			}

			const FVector MeshSize = (SpikeStaticMesh->GetBounds().BoxExtent * 2.0f).ComponentMax(FVector(1.0f, 1.0f, 1.0f));
			const float InstanceWidth = MeshSize.X;
			const int32 InstanceCount = FMath::Max(1, FMath::CeilToInt(FMath::Max(SpikeSize.X, 1.0f) / InstanceWidth));
			const float VisualWidth = InstanceWidth * static_cast<float>(InstanceCount);
			const float InstanceDepth = MeshSize.Y;
			const int32 InstanceDepthCount = FMath::Max(1, FMath::CeilToInt(FMath::Max(SpikeSize.Y, 1.0f) / InstanceDepth));
			const float VisualDepth = InstanceDepth * static_cast<float>(InstanceDepthCount);
			const FVector InstanceScale(
				1.0f,
				1.0f,
				FMath::Max(SpikeSize.Z, 1.0f) / MeshSize.Z);
			const float LocalStartX = (-0.5f * VisualWidth) + (0.5f * InstanceWidth);
			const float LocalStartY = (-0.5f * VisualDepth) + (0.5f * InstanceDepth);

			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
			{
				for (int32 InstanceDepthIndex = 0; InstanceDepthIndex < InstanceDepthCount; ++InstanceDepthIndex)
				{
					const FTransform InstanceTransform(
						FRotator::ZeroRotator,
						FVector(LocalStartX + (InstanceIndex * InstanceWidth), LocalStartY + (InstanceDepthIndex * InstanceDepth), 0.0f),
						InstanceScale);
					InstancedSpikeMesh->AddInstance(InstanceTransform);
				}
			}
		}
	}

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
