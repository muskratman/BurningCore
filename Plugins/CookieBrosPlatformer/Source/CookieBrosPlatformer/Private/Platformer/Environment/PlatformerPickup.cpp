// Copyright Epic Games, Inc. All Rights Reserved.

#include "Platformer/Environment/PlatformerPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/PlatformerPickupSink.h"
#include "UObject/ConstructorHelpers.h"

APlatformerPickup::APlatformerPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerPickup.PlatformerPickup")));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(RootComponent);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		PickupMesh->SetStaticMesh(SphereMesh.Object);
	}

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Sphere->SetupAttachment(RootComponent);
	Sphere->SetSphereRadius(100.0f);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OnActorBeginOverlap.AddDynamic(this, &APlatformerPickup::BeginOverlap);
}

void APlatformerPickup::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	ACharacter* OverlappedCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlappedCharacter || !OverlappedCharacter->IsPlayerControlled())
	{
		return;
	}

	AGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;
	if (!IsValid(GameMode) || !GameMode->GetClass()->ImplementsInterface(UPlatformerPickupSink::StaticClass()))
	{
		return;
	}

	IPlatformerPickupSink::Execute_ProcessPlatformerPickup(GameMode, this, OverlappedCharacter);
	SetActorEnableCollision(false);
	BP_OnPickedUp();
}
