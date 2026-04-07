#include "Platformer/Environment/PlatformerFallingPlatform.h"

#include "GameFramework/Character.h"
#include "TimerManager.h"

APlatformerFallingPlatform::APlatformerFallingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerFallingPlatform.PlatformerFallingPlatform")));

	BlockMesh->SetMobility(EComponentMobility::Movable);
	BlockMesh->SetCollisionObjectType(ECC_WorldDynamic);
}

void APlatformerFallingPlatform::BeginPlay()
{
	Super::BeginPlay();
	InitialActorLocation = GetActorLocation();
}

void APlatformerFallingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFalling)
	{
		AddActorWorldOffset(FVector(0.0f, 0.0f, -FallingSpeed * DeltaTime), true);
	}
}

void APlatformerFallingPlatform::HandleCharacterEnteredSurface(ACharacter* Character)
{
	if (bPendingFall || bFalling)
	{
		return;
	}

	bPendingFall = true;
	GetWorldTimerManager().SetTimer(FallTimerHandle, this, &APlatformerFallingPlatform::StartFalling, FallDelay, false);
}

void APlatformerFallingPlatform::StartFalling()
{
	bPendingFall = false;
	bFalling = true;

	if (bRespawnAfterFall && (RespawnDelay > 0.0f))
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APlatformerFallingPlatform::RespawnPlatform, RespawnDelay, false);
	}
}

void APlatformerFallingPlatform::RespawnPlatform()
{
	bPendingFall = false;
	bFalling = false;
	SetActorLocation(InitialActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
}
