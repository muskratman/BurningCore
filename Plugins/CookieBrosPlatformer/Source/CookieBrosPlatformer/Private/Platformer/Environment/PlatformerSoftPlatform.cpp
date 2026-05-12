// Copyright Epic Games, Inc. All Rights Reserved.

#include "Platformer/Environment/PlatformerSoftPlatform.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerSoftPlatform::APlatformerSoftPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerSoftPlatform.PlatformerSoftPlatform")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshLayoutRoot"));
	MeshLayoutRoot->SetupAttachment(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(MeshLayoutRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Cube.SM_Platform_Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	PlatformCollisionLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlatformCollisionLayoutRoot"));
	PlatformCollisionLayoutRoot->SetupAttachment(Root);

	PlatformCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlatformCollisionBox"));
	PlatformCollisionBox->SetupAttachment(PlatformCollisionLayoutRoot);
	PlatformCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlatformCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	PlatformCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlatformCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PlatformCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APlatformerSoftPlatform::OnTopCheckBeginOverlap);
	PlatformCollisionBox->OnComponentEndOverlap.AddDynamic(this, &APlatformerSoftPlatform::OnTopCheckEndOverlap);

	CollisionCheckLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionCheckLayoutRoot"));
	CollisionCheckLayoutRoot->SetupAttachment(Root);

	CollisionCheckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Check Box"));
	CollisionCheckBox->SetupAttachment(CollisionCheckLayoutRoot);
	CollisionCheckBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionCheckBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionCheckBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionCheckBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionCheckBox->OnComponentBeginOverlap.AddDynamic(this, &APlatformerSoftPlatform::OnBottomCheckBeginOverlap);
	CollisionCheckBox->OnComponentEndOverlap.AddDynamic(this, &APlatformerSoftPlatform::OnBottomCheckEndOverlap);
}

void APlatformerSoftPlatform::SetPlatformSize(const FVector& InPlatformSize)
{
	PlatformSize = InPlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerSoftPlatform::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FVector ResolvedPlatformSize = PlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	const FVector PlatformCollisionExtent(
		ResolvedPlatformSize.X * 0.5f,
		ResolvedPlatformSize.Y * 0.5f,
		FMath::Max(PlatformCollisionHeight * 0.5f, 1.0f));
	const FVector CheckExtent(
		ResolvedPlatformSize.X * 0.5f,
		ResolvedPlatformSize.Y * 0.5f,
		FMath::Max(UnderPlatformTriggerHeight * 0.5f, 1.0f));

	PlatformerEnvironment::ApplyRelativeTransform(
		MeshLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedPlatformSize.Z * 0.5f),
		FRotator::ZeroRotator,
		ResolvedPlatformSize / 100.0f,
		MeshTransformOffset);

	PlatformCollisionBox->SetBoxExtent(PlatformCollisionExtent);
	PlatformerEnvironment::ApplyRelativeTransform(
		PlatformCollisionLayoutRoot,
		FVector(0.0f, 0.0f, ResolvedPlatformSize.Z + PlatformCollisionExtent.Z),
		FRotator::ZeroRotator,
		FVector::OneVector,
		PlatformCollisionTransformOffset);

	CollisionCheckBox->SetBoxExtent(CheckExtent);
	PlatformerEnvironment::ApplyRelativeTransform(
		CollisionCheckLayoutRoot,
		FVector(0.0f, 0.0f, -CheckExtent.Z),
		FRotator::ZeroRotator,
		FVector::OneVector,
		CollisionCheckTransformOffset);
}

void APlatformerSoftPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClearInvalidCharacterSet(CharactersAbovePlatform);
	ClearInvalidCharacterSet(CharactersBelowPlatform);
}

void APlatformerSoftPlatform::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CharactersAbovePlatform.Empty();
	CharactersBelowPlatform.Empty();
	Super::EndPlay(EndPlayReason);
}

void APlatformerSoftPlatform::OnTopCheckBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersAbovePlatform.Add(Character);
	}
}

void APlatformerSoftPlatform::OnTopCheckEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersAbovePlatform.Remove(Character);
		if (!CharactersBelowPlatform.Contains(Character))
		{
			SetCharacterIgnoreComponentWhenMoving(Character, false);
		}
	}
}

void APlatformerSoftPlatform::OnBottomCheckBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersBelowPlatform.Add(Character);
		SetCharacterIgnoreComponentWhenMoving(Character, true);
	}
}

void APlatformerSoftPlatform::OnBottomCheckEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersBelowPlatform.Remove(Character);
		if (!CharactersAbovePlatform.Contains(Character))
		{
			SetCharacterIgnoreComponentWhenMoving(Character, false);
		}
	}
}

bool APlatformerSoftPlatform::RequestCharacterDropThrough(ACharacter* Character)
{
	if (Character && CharactersAbovePlatform.Contains(Character))
	{
		StartIgnoringCharacter(Character, true);
		return true;
	}
	return false;
}

void APlatformerSoftPlatform::SetCharacterIgnoreComponentWhenMoving(ACharacter* Character, bool bShouldIgnore)
{
	if (UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(Character->GetRootComponent()))
	{
		RootPrimitiveComponent->IgnoreComponentWhenMoving(Mesh, bShouldIgnore);
	}
}

void APlatformerSoftPlatform::ClearInvalidCharacterSet(TSet<TWeakObjectPtr<ACharacter>>& CharacterSet)
{
	for (auto It = CharacterSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void APlatformerSoftPlatform::StartIgnoringCharacter(ACharacter* Character, bool bForceDownwardDrop)
{
	if (Character == nullptr)
	{
		return;
	}

	SetCharacterIgnoreComponentWhenMoving(Character, true);

	if (bForceDownwardDrop)
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			FVector Velocity = MovementComponent->Velocity;
			Velocity.Z = FMath::Min(Velocity.Z, -DropThroughDownwardSpeed);
			MovementComponent->Velocity = Velocity;
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}
