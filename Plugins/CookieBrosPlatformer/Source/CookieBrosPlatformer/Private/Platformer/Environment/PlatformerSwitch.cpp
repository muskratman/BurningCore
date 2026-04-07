#include "Platformer/Environment/PlatformerSwitch.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlatformerSwitch::APlatformerSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerSwitch.PlatformerSwitch")));

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SwitchMeshLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SwitchMeshLayoutRoot"));
	SwitchMeshLayoutRoot->SetupAttachment(Root);

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	SwitchMesh->SetupAttachment(SwitchMeshLayoutRoot);
	SwitchMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SwitchMesh->SetCollisionObjectType(ECC_WorldDynamic);
	SwitchMesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		SwitchMesh->SetStaticMesh(CubeMesh.Object);
	}

	TriggerVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TriggerVolumeLayoutRoot"));
	TriggerVolumeLayoutRoot->SetupAttachment(Root);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(TriggerVolumeLayoutRoot);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerSwitch::OnTriggerVolumeBeginOverlap);
}

void APlatformerSwitch::SetTriggerExtent(const FVector& InTriggerExtent)
{
	TriggerExtent = InTriggerExtent.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerSwitch::BeginPlay()
{
	Super::BeginPlay();
	SetSwitchState(bStartsOn, nullptr);
}

void APlatformerSwitch::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TriggerVolume->SetBoxExtent(TriggerExtent.ComponentMax(FVector(1.0f, 1.0f, 1.0f)));

	PlatformerEnvironment::ApplyRelativeTransform(
		SwitchMeshLayoutRoot,
		FVector(0.0f, 0.0f, 50.0f),
		FRotator::ZeroRotator,
		FVector(0.8f, 0.3f, 1.0f),
		SwitchMeshTransformOffset);

	PlatformerEnvironment::ApplyRelativeTransform(
		TriggerVolumeLayoutRoot,
		FVector(0.0f, 0.0f, TriggerExtent.Z),
		FRotator::ZeroRotator,
		FVector::OneVector,
		TriggerVolumeTransformOffset);
}

void APlatformerSwitch::Interaction(AActor* Interactor)
{
	ActivateSwitch(Interactor);
}

void APlatformerSwitch::ActivateSwitch(AActor* InstigatorActor)
{
	const double CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if ((CurrentTime - LastActivationTime) < ActivationCooldown)
	{
		return;
	}

	LastActivationTime = CurrentTime;

	if (bOneShot && bIsOn)
	{
		return;
	}

	const bool bNewIsOn = bToggleOnEveryActivation ? !bIsOn : true;
	SetSwitchState(bNewIsOn, InstigatorActor);
}

void APlatformerSwitch::SetSwitchState(bool bNewIsOn, AActor* InstigatorActor)
{
	if (bIsOn == bNewIsOn)
	{
		return;
	}

	bIsOn = bNewIsOn;

	for (AActor* TargetActor : ToggleTargets)
	{
		ApplyToggleTargetState(TargetActor, bIsOn);
	}

	if (bIsOn)
	{
		for (AActor* TargetActor : InteractionTargets)
		{
			if (IPlatformerInteractable* Interactable = Cast<IPlatformerInteractable>(TargetActor))
			{
				Interactable->Interaction(InstigatorActor != nullptr ? InstigatorActor : this);
			}
		}
	}
}

void APlatformerSwitch::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor == nullptr) || (OtherActor == this))
	{
		return;
	}

	if (bOnlyCharactersCanActivate && !Cast<ACharacter>(OtherActor))
	{
		return;
	}

	ActivateSwitch(OtherActor);
}

void APlatformerSwitch::ApplyToggleTargetState(AActor* TargetActor, bool bEnabled) const
{
	if (TargetActor == nullptr)
	{
		return;
	}

	TargetActor->SetActorHiddenInGame(!bEnabled);
	TargetActor->SetActorEnableCollision(bEnabled);
	TargetActor->SetActorTickEnabled(bEnabled);
}
