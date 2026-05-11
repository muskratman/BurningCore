#include "Platformer/Environment/PlatformerDropThroughPlatformComponent.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"

UPlatformerDropThroughPlatformComponent::
    UPlatformerDropThroughPlatformComponent() {
  PrimaryComponentTick.bCanEverTick = true;
}

void UPlatformerDropThroughPlatformComponent::InitializeDropThroughPlatform(
    UPrimitiveComponent *InBlockingComponent,
    USceneComponent *InTopCheckLayoutRoot, UBoxComponent *InTopCheckBox,
    USceneComponent *InBottomCheckLayoutRoot, UBoxComponent *InBottomCheckBox) {
  BlockingComponent = InBlockingComponent;
  TopCheckLayoutRoot = InTopCheckLayoutRoot;
  TopCheckBox = InTopCheckBox;
  BottomCheckLayoutRoot = InBottomCheckLayoutRoot;
  BottomCheckBox = InBottomCheckBox;

  ConfigureCheckBox(TopCheckBox);
  ConfigureCheckBox(BottomCheckBox);

  if (TopCheckBox) {
    TopCheckBox->OnComponentBeginOverlap.RemoveDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnTopCheckBeginOverlap);
    TopCheckBox->OnComponentEndOverlap.RemoveDynamic(
        this, &UPlatformerDropThroughPlatformComponent::OnTopCheckEndOverlap);
    TopCheckBox->OnComponentBeginOverlap.AddDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnTopCheckBeginOverlap);
    TopCheckBox->OnComponentEndOverlap.AddDynamic(
        this, &UPlatformerDropThroughPlatformComponent::OnTopCheckEndOverlap);
  }

  if (BottomCheckBox) {
    BottomCheckBox->OnComponentBeginOverlap.RemoveDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnBottomCheckBeginOverlap);
    BottomCheckBox->OnComponentEndOverlap.RemoveDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnBottomCheckEndOverlap);
    BottomCheckBox->OnComponentBeginOverlap.AddDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnBottomCheckBeginOverlap);
    BottomCheckBox->OnComponentEndOverlap.AddDynamic(
        this,
        &UPlatformerDropThroughPlatformComponent::OnBottomCheckEndOverlap);
  }

  UpdateCheckBoxCollisionState();
}

void UPlatformerDropThroughPlatformComponent::RefreshDropThroughPlatformLayout(
    const FVector &InPlatformSize) {
  const FVector ResolvedPlatformSize =
      InPlatformSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
  const FVector TopCheckExtent(
      ResolvedPlatformSize.X * 0.5f, ResolvedPlatformSize.Y * 0.5f,
      FMath::Max(TopCheckHeight * 0.5f, 1.0f));
  const FVector BottomCheckExtent(
      ResolvedPlatformSize.X * 0.5f, ResolvedPlatformSize.Y * 0.5f,
      FMath::Max(BottomCheckHeight * 0.5f, 1.0f));

  if (TopCheckBox) {
    TopCheckBox->SetBoxExtent(TopCheckExtent);
  }

  if (TopCheckLayoutRoot) {
    PlatformerEnvironment::ApplyRelativeTransform(
        TopCheckLayoutRoot,
        FVector(0.0f, 0.0f, ResolvedPlatformSize.Z + TopCheckExtent.Z),
        FRotator::ZeroRotator, FVector::OneVector, TopCheckTransformOffset);
  }

  if (BottomCheckBox) {
    BottomCheckBox->SetBoxExtent(BottomCheckExtent);
  }

  if (BottomCheckLayoutRoot) {
    PlatformerEnvironment::ApplyRelativeTransform(
        BottomCheckLayoutRoot, FVector(0.0f, 0.0f, -BottomCheckExtent.Z),
        FRotator::ZeroRotator, FVector::OneVector,
        BottomCheckTransformOffset);
  }

  UpdateCheckBoxCollisionState();
}

void UPlatformerDropThroughPlatformComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  ClearInvalidCharacterSet(CharactersAbovePlatform);
  ClearInvalidCharacterSet(CharactersBelowPlatform);
}

void UPlatformerDropThroughPlatformComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  RestoreTrackedCharacterCollision();
  Super::EndPlay(EndPlayReason);
}

bool UPlatformerDropThroughPlatformComponent::RequestCharacterDropThrough(
    ACharacter *Character) {
  if (!bDropThroughEnabled || !Character) {
    return false;
  }

  if (!CharactersAbovePlatform.Contains(Character)) {
    return false;
  }

  StartIgnoringCharacter(Character, true);
  return true;
}

void UPlatformerDropThroughPlatformComponent::RestoreCharacterCollision(
    ACharacter *Character) {
  if (!Character) {
    return;
  }

  CharactersAbovePlatform.Remove(Character);
  CharactersBelowPlatform.Remove(Character);
  SetCharacterIgnoreComponentWhenMoving(Character, false);
}

void UPlatformerDropThroughPlatformComponent::SetDropThroughEnabled(
    bool bInDropThroughEnabled) {
  if (bDropThroughEnabled && !bInDropThroughEnabled) {
    RestoreTrackedCharacterCollision();
  }

  bDropThroughEnabled = bInDropThroughEnabled;
  UpdateCheckBoxCollisionState();
}

void UPlatformerDropThroughPlatformComponent::ConfigureCheckBox(
    UBoxComponent *CheckBox) const {
  if (!CheckBox) {
    return;
  }

  CheckBox->SetCollisionObjectType(ECC_WorldDynamic);
  CheckBox->SetCollisionResponseToAllChannels(ECR_Ignore);
  CheckBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  CheckBox->SetCanEverAffectNavigation(false);
}

void UPlatformerDropThroughPlatformComponent::UpdateCheckBoxCollisionState()
    const {
  const ECollisionEnabled::Type CollisionEnabled =
      bDropThroughEnabled ? ECollisionEnabled::QueryOnly
                          : ECollisionEnabled::NoCollision;

  if (TopCheckBox) {
    TopCheckBox->SetCollisionEnabled(CollisionEnabled);
    TopCheckBox->SetGenerateOverlapEvents(bDropThroughEnabled);
  }

  if (BottomCheckBox) {
    BottomCheckBox->SetCollisionEnabled(CollisionEnabled);
    BottomCheckBox->SetGenerateOverlapEvents(bDropThroughEnabled);
  }
}

void UPlatformerDropThroughPlatformComponent::OnTopCheckBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (!bDropThroughEnabled) {
    return;
  }

  if (ACharacter *Character = Cast<ACharacter>(OtherActor)) {
    CharactersAbovePlatform.Add(Character);
  }
}

void UPlatformerDropThroughPlatformComponent::OnTopCheckEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  if (ACharacter *Character = Cast<ACharacter>(OtherActor)) {
    CharactersAbovePlatform.Remove(Character);
    if (!CharactersBelowPlatform.Contains(Character)) {
      SetCharacterIgnoreComponentWhenMoving(Character, false);
    }
  }
}

void UPlatformerDropThroughPlatformComponent::OnBottomCheckBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (!bDropThroughEnabled) {
    return;
  }

  if (ACharacter *Character = Cast<ACharacter>(OtherActor)) {
    CharactersBelowPlatform.Add(Character);
    SetCharacterIgnoreComponentWhenMoving(Character, true);
  }
}

void UPlatformerDropThroughPlatformComponent::OnBottomCheckEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  if (ACharacter *Character = Cast<ACharacter>(OtherActor)) {
    CharactersBelowPlatform.Remove(Character);
    if (!CharactersAbovePlatform.Contains(Character)) {
      SetCharacterIgnoreComponentWhenMoving(Character, false);
    }
  }
}

void UPlatformerDropThroughPlatformComponent::ClearInvalidCharacterSet(
    TSet<TWeakObjectPtr<ACharacter>> &CharacterSet) {
  for (auto It = CharacterSet.CreateIterator(); It; ++It) {
    if (!It->IsValid()) {
      It.RemoveCurrent();
    }
  }
}

void UPlatformerDropThroughPlatformComponent::RestoreTrackedCharacterCollision() {
  TSet<TWeakObjectPtr<ACharacter>> TrackedCharacters = CharactersAbovePlatform;
  TrackedCharacters.Append(CharactersBelowPlatform);

  for (const TWeakObjectPtr<ACharacter> &TrackedCharacter : TrackedCharacters) {
    if (ACharacter *Character = TrackedCharacter.Get()) {
      SetCharacterIgnoreComponentWhenMoving(Character, false);
    }
  }

  CharactersAbovePlatform.Empty();
  CharactersBelowPlatform.Empty();
}

void UPlatformerDropThroughPlatformComponent::StartIgnoringCharacter(
    ACharacter *Character, bool bForceDownwardDrop) {
  if (!Character) {
    return;
  }

  SetCharacterIgnoreComponentWhenMoving(Character, true);

  if (bForceDownwardDrop) {
    if (UCharacterMovementComponent *MovementComponent =
            Character->GetCharacterMovement()) {
      FVector Velocity = MovementComponent->Velocity;
      Velocity.Z = FMath::Min(Velocity.Z, -DropThroughDownwardSpeed);
      MovementComponent->Velocity = Velocity;
      MovementComponent->SetMovementMode(MOVE_Falling);
    }
  }
}

void UPlatformerDropThroughPlatformComponent::
    SetCharacterIgnoreComponentWhenMoving(ACharacter *Character,
                                          bool bShouldIgnore) {
  if (!Character || !BlockingComponent) {
    return;
  }

  if (UPrimitiveComponent *RootPrimitiveComponent =
          Cast<UPrimitiveComponent>(Character->GetRootComponent())) {
    RootPrimitiveComponent->IgnoreComponentWhenMoving(BlockingComponent,
                                                      bShouldIgnore);
  }
}
