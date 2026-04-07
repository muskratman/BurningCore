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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
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

	UpdateIgnoredCharacters();
	ClearInvalidCharacterSet(CharactersAbovePlatform);
	ClearInvalidCharacterSet(CharactersBelowPlatform);

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : CharactersBelowPlatform)
	{
		ACharacter* Character = CharacterPtr.Get();
		if (Character == nullptr || IgnoredCharactersUntilTime.Contains(Character))
		{
			continue;
		}

		if (const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (MovementComponent->Velocity.Z > 0.0f)
			{
				StartIgnoringCharacter(Character, JumpThroughIgnoreDuration, false);
			}
		}
	}

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : CharactersAbovePlatform)
	{
		ACharacter* Character = CharacterPtr.Get();
		if ((Character == nullptr) || IgnoredCharactersUntilTime.Contains(Character) || !IsCharacterStandingOnPlatform(Character))
		{
			continue;
		}

		if (IsCharacterRequestingDropThrough(Character))
		{
			StartIgnoringCharacter(Character, DropThroughIgnoreDuration, true);
		}
	}
}

void APlatformerSoftPlatform::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<TWeakObjectPtr<ACharacter>> IgnoredCharacters;
	IgnoredCharactersUntilTime.GetKeys(IgnoredCharacters);

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : IgnoredCharacters)
	{
		if (CharacterPtr.IsValid())
		{
			StopIgnoringCharacter(CharacterPtr.Get());
		}
	}

	IgnoredCharactersUntilTime.Empty();
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
	}
}

void APlatformerSoftPlatform::OnBottomCheckBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersBelowPlatform.Add(Character);
	}
}

void APlatformerSoftPlatform::OnBottomCheckEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		CharactersBelowPlatform.Remove(Character);
	}
}

bool APlatformerSoftPlatform::IsCharacterStandingOnPlatform(const ACharacter* Character) const
{
	if (Character == nullptr)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if ((MovementComponent == nullptr) || !MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	const UPrimitiveComponent* MovementBase = MovementComponent->GetMovementBase();
	return (MovementBase != nullptr) && (MovementBase->GetOwner() == this);
}

bool APlatformerSoftPlatform::IsCharacterRequestingDropThrough(const ACharacter* Character) const
{
	if (Character == nullptr)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	return (MovementComponent != nullptr)
		&& MovementComponent->IsMovingOnGround()
		&& Character->bIsCrouched
		&& Character->bPressedJump;
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

void APlatformerSoftPlatform::StartIgnoringCharacter(ACharacter* Character, float IgnoreDuration, bool bForceDownwardDrop)
{
	if (Character == nullptr)
	{
		return;
	}

	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->IgnoreActorWhenMoving(this, true);
	}

	Mesh->IgnoreActorWhenMoving(Character, true);
	Character->MoveIgnoreActorAdd(this);
	IgnoredCharactersUntilTime.Add(Character, GetWorld()->GetTimeSeconds() + FMath::Max(IgnoreDuration, 0.01f));

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

void APlatformerSoftPlatform::StopIgnoringCharacter(ACharacter* Character)
{
	if (Character == nullptr)
	{
		return;
	}

	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->IgnoreActorWhenMoving(this, false);
	}

	Mesh->IgnoreActorWhenMoving(Character, false);
	Character->MoveIgnoreActorRemove(this);
	IgnoredCharactersUntilTime.Remove(Character);
}

void APlatformerSoftPlatform::UpdateIgnoredCharacters()
{
	if (UWorld* World = GetWorld())
	{
		const float CurrentTime = World->GetTimeSeconds();
		TArray<TWeakObjectPtr<ACharacter>> CharactersToRestore;
		CharactersToRestore.Reserve(IgnoredCharactersUntilTime.Num());

		for (const TPair<TWeakObjectPtr<ACharacter>, float>& Pair : IgnoredCharactersUntilTime)
		{
			if (!Pair.Key.IsValid() || (Pair.Value <= CurrentTime))
			{
				CharactersToRestore.Add(Pair.Key);
			}
		}

		for (const TWeakObjectPtr<ACharacter>& CharacterPtr : CharactersToRestore)
		{
			if (CharacterPtr.IsValid())
			{
				StopIgnoringCharacter(CharacterPtr.Get());
			}
			else
			{
				IgnoredCharactersUntilTime.Remove(CharacterPtr);
			}
		}
	}
}
