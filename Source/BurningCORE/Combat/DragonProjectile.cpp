#include "Combat/DragonProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/Damageable.h"
#include "GameplayEffect.h"

ADragonProjectile::ADragonProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->OnComponentHit.AddDynamic(this, &ADragonProjectile::OnHit);
	RootComponent = CollisionSphere;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// TrailVFX is assigned in BP
	
	InitialLifeSpan = Lifetime;
}

void ADragonProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
}

void ADragonProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
	{
		ApplyDamageAndStatus(OtherActor);

		if (!bPiercing)
		{
			Destroy(); // Or return to pool
		}
	}
}

void ADragonProjectile::ApplyDamageAndStatus(AActor* Target)
{
	if (IDamageable* DamageableTarget = Cast<IDamageable>(Target))
	{
		// Make up a dummy hit result if one wasn't provided directly
		FHitResult DummyHit;
		DummyHit.Location = Target->GetActorLocation();
		
		if (DamageEffectSpec.IsValid())
		{
			DamageableTarget->ApplyDamage(DamageEffectSpec, DummyHit);
		}
		
		if (StatusEffectSpec.IsValid())
		{
			DamageableTarget->ApplyDamage(StatusEffectSpec, DummyHit); // Status is applied through same pipeline
		}
	}
}
