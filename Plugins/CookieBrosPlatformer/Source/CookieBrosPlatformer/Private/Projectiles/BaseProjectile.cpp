#include "Projectiles/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Interfaces/Damageable.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnBeginOverlap);
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

void ABaseProjectile::ApplyProjectileMaxDistance(float InProjectileMaxDistance)
{
	ProjectileMaxDistance = FMath::Max(0.0f, InProjectileMaxDistance);
	SetActorTickEnabled(ProjectileMaxDistance > 0.0f);
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ProjectileMaxDistance <= 0.0f)
	{
		return;
	}

	if (FVector::DistSquared(GetActorLocation(), ProjectileSpawnLocation) >= FMath::Square(ProjectileMaxDistance))
	{
		Destroy();
	}
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileSpawnLocation = GetActorLocation();

	if (AActor* InstigatorActor = GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(InstigatorActor, true);
	}

	if (AActor* OwnerActor = GetOwner(); OwnerActor && OwnerActor != GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);
	}

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	SetActorTickEnabled(ProjectileMaxDistance > 0.0f);
	SetLifeSpan(Lifetime);
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShouldIgnoreImpact(OtherActor, OtherComp))
	{
		return;
	}

	HandleImpact(OtherActor, OtherComp, Hit);
}

void ABaseProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (ShouldIgnoreImpact(OtherActor, OtherComp))
	{
		return;
	}

	const bool bIsPawnOverlap = OtherComp->GetCollisionObjectType() == ECC_Pawn;
	const bool bIsDamageableActor = OtherActor && OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass());
	if (!bIsPawnOverlap && !bIsDamageableActor)
	{
		return;
	}

	FHitResult ResolvedHit = SweepResult;
	if (!bFromSweep)
	{
		ResolvedHit.Location = OtherActor ? OtherActor->GetActorLocation() : GetActorLocation();
		ResolvedHit.ImpactPoint = ResolvedHit.Location;
		ResolvedHit.bBlockingHit = false;
	}

	HandleImpact(OtherActor, OtherComp, ResolvedHit);
}

void ABaseProjectile::HandleImpact(AActor* /*OtherActor*/, UPrimitiveComponent* /*OtherComp*/, const FHitResult& /*Hit*/)
{
	Destroy();
}

bool ABaseProjectile::ShouldIgnoreImpact(const AActor* OtherActor, const UPrimitiveComponent* OtherComp) const
{
	return !OtherActor
		|| OtherActor == this
		|| OtherActor == GetInstigator()
		|| OtherActor == GetOwner()
		|| !OtherComp;
}
