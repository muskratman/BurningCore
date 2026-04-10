#include "Platformer/Environment/PlatformerClosingDoor.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"

APlatformerClosingDoor::APlatformerClosingDoor()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerBlock.PlatformerBlock")));

	TriggerVolumeLayoutRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TriggerVolumeLayoutRoot"));
	TriggerVolumeLayoutRoot->SetupAttachment(Root);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(TriggerVolumeLayoutRoot);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetCanEverAffectNavigation(false);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerClosingDoor::OnTriggerBeginOverlap);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformerClosingDoor::OnTriggerEndOverlap);

	PlatformSize = FVector(100.0f, 100.0f, 100.0f);
	PointBBaseRelativeLocation = FVector(0.0f, 0.0f, -300.0f);
	MoveSpeed = 600.0f;
	PointADelay = 0.0f;
	PointBDelay = 0.0f;
	bAutoStart = false;
	bIsRepeatable = false;
}

void APlatformerClosingDoor::SetTriggerSize(const FVector& InTriggerSize)
{
	TriggerSize = InTriggerSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
}

void APlatformerClosingDoor::BeginPlay()
{
	Super::BeginPlay();

	OverlappingCharacters.Empty();
	bHasBeenTriggered = bStartAtPointB;
	SetTriggerEnabled(!bHasBeenTriggered && !IsAtPointB());
}

void APlatformerClosingDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TriggerVolume->SetBoxExtent(TriggerSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f)) * 0.5f);
	PlatformerEnvironment::ApplyRelativeTransform(
		TriggerVolumeLayoutRoot,
		TriggerBaseRelativeLocation,
		FRotator::ZeroRotator,
		FVector::OneVector,
		TriggerVolumeTransformOffset);
}

void APlatformerClosingDoor::Interaction(AActor* Interactor)
{
	TryTriggerDoor();
}

void APlatformerClosingDoor::ResetInteraction()
{
	Super::ResetInteraction();

	ClearInvalidOverlappingCharacters();

	if (IsAtPointA() && !IsMoving())
	{
		bHasBeenTriggered = false;
		SetTriggerEnabled(true);
		return;
	}

	SetTriggerEnabled(false);
}

void APlatformerClosingDoor::HandleReachedPointB()
{
	Super::HandleReachedPointB();

	OverlappingCharacters.Empty();
	SetTriggerEnabled(false);
}

void APlatformerClosingDoor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlatformerCharacterBase* TriggeringCharacter = nullptr;
	if (!IsValidTriggeringCharacter(OtherActor, TriggeringCharacter))
	{
		return;
	}

	OverlappingCharacters.Add(TriggeringCharacter);

	if (!bTriggerOnEndOverlap)
	{
		TryTriggerDoor();
	}
}

void APlatformerClosingDoor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlatformerCharacterBase* TriggeringCharacter = nullptr;
	if (!IsValidTriggeringCharacter(OtherActor, TriggeringCharacter))
	{
		return;
	}

	OverlappingCharacters.Remove(TriggeringCharacter);
	ClearInvalidOverlappingCharacters();

	if (bTriggerOnEndOverlap && OverlappingCharacters.Num() == 0)
	{
		TryTriggerDoor();
	}
}

bool APlatformerClosingDoor::IsValidTriggeringCharacter(AActor* OtherActor, APlatformerCharacterBase*& OutCharacter) const
{
	OutCharacter = Cast<APlatformerCharacterBase>(OtherActor);
	if (OutCharacter == nullptr)
	{
		return false;
	}

	if (bOnlyPlayerControlledCharacters && !OutCharacter->IsPlayerControlled())
	{
		OutCharacter = nullptr;
		return false;
	}

	return true;
}

void APlatformerClosingDoor::TryTriggerDoor()
{
	if (bHasBeenTriggered || IsMoving() || IsAtPointB())
	{
		return;
	}

	bHasBeenTriggered = true;
	OverlappingCharacters.Empty();
	SetTriggerEnabled(false);
	StartMovingToPointB();
}

void APlatformerClosingDoor::SetTriggerEnabled(bool bEnabled)
{
	if (TriggerVolume == nullptr)
	{
		return;
	}

	TriggerVolume->SetGenerateOverlapEvents(bEnabled);
	TriggerVolume->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void APlatformerClosingDoor::ClearInvalidOverlappingCharacters()
{
	for (auto It = OverlappingCharacters.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}
