#include "Platformer/Environment/PlatformerDestructibleBlock.h"

#include "TimerManager.h"

APlatformerDestructibleBlock::APlatformerDestructibleBlock()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerDestructibleBlock.PlatformerDestructibleBlock")));
}

void APlatformerDestructibleBlock::BeginPlay()
{
	Super::BeginPlay();
	CurrentHitPoints = FMath::Max(MaxHitPoints, 1.0f);
}

void APlatformerDestructibleBlock::ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult)
{
	ApplyPointDamage(FMath::Max(DamagePerHit, 1.0f));
}

bool APlatformerDestructibleBlock::IsAlive() const
{
	return CurrentHitPoints > 0.0f;
}

float APlatformerDestructibleBlock::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ResolvedDamage = FMath::Max(DamageAmount, DamagePerHit);
	ApplyPointDamage(ResolvedDamage);
	return ResolvedDamage;
}

void APlatformerDestructibleBlock::ApplyPointDamage(float DamageAmount)
{
	if (!IsAlive())
	{
		return;
	}

	CurrentHitPoints = FMath::Max(0.0f, CurrentHitPoints - FMath::Max(DamageAmount, 1.0f));
	if (CurrentHitPoints <= 0.0f)
	{
		HandleBroken();
	}
}

void APlatformerDestructibleBlock::HandleBroken()
{
	BlockMesh->SetHiddenInGame(true);
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (bDestroyActorOnBreak)
	{
		Destroy();
		return;
	}

	if (bRespawnAfterBreak && (RespawnDelay > 0.0f))
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APlatformerDestructibleBlock::RespawnBlock, RespawnDelay, false);
	}
}

void APlatformerDestructibleBlock::RespawnBlock()
{
	CurrentHitPoints = FMath::Max(MaxHitPoints, 1.0f);
	BlockMesh->SetHiddenInGame(false);
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
