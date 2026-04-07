#include "Projectiles/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	CollisionComp->SetSimulatePhysics(false);
	CollisionComp->SetEnableGravity(false);

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName("NoCollision");

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // Standard projectiles lack gravity
}

void ABaseProjectile::ApplyDeveloperProjectileSpeed(float DeveloperProjectileSpeed)
{
	Speed = FMath::Max(0.0f, DeveloperProjectileSpeed);

	if (!ProjectileMovement)
	{
		return;
	}

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	const FVector CurrentDirection = ProjectileMovement->Velocity.IsNearlyZero()
		? GetActorForwardVector()
		: ProjectileMovement->Velocity.GetSafeNormal();
	ProjectileMovement->Velocity = CurrentDirection * Speed;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	SetLifeSpan(Lifetime);
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator() || !OtherComp)
	{
		return;
	}

	HandleImpact(OtherActor, OtherComp, Hit);
}

void ABaseProjectile::HandleImpact(AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, const FHitResult& /*Hit*/)
{
	Destroy();
}
