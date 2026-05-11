#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerDropThroughPlatformComponent.generated.h"

class ACharacter;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

/**
 * Reusable one-way platform collision helper for block-like platform actors.
 *
 * A character standing in the top check volume can request drop-through.
 * A character entering from the lower check volume temporarily ignores the
 * blocking component, allowing jump-through behavior from below.
 */
UCLASS(ClassGroup = (Platformer), meta = (BlueprintSpawnableComponent))
class COOKIEBROSPLATFORMER_API UPlatformerDropThroughPlatformComponent
    : public UActorComponent {
  GENERATED_BODY()

public:
  UPlatformerDropThroughPlatformComponent();

  virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction)
      override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void InitializeDropThroughPlatform(
      UPrimitiveComponent *InBlockingComponent,
      USceneComponent *InTopCheckLayoutRoot, UBoxComponent *InTopCheckBox,
      USceneComponent *InBottomCheckLayoutRoot,
      UBoxComponent *InBottomCheckBox);

  void RefreshDropThroughPlatformLayout(const FVector &InPlatformSize);

  UFUNCTION(BlueprintCallable, Category = "Drop Through Platform|Collision")
  bool RequestCharacterDropThrough(ACharacter *Character);

  UFUNCTION(BlueprintCallable, Category = "Drop Through Platform|Collision")
  void RestoreCharacterCollision(ACharacter *Character);

  UFUNCTION(BlueprintCallable, Category = "Drop Through Platform|Collision")
  void SetDropThroughEnabled(bool bInDropThroughEnabled);

  FORCEINLINE bool IsDropThroughEnabled() const {
    return bDropThroughEnabled;
  }

protected:
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Collision")
  bool bDropThroughEnabled = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Collision",
            meta = (ClampMin = "1.0", Units = "cm"))
  float TopCheckHeight = 24.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Collision",
            meta = (ClampMin = "1.0", Units = "cm"))
  float BottomCheckHeight = 24.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Collision",
            meta = (ClampMin = "0.0", Units = "cm/s"))
  float DropThroughDownwardSpeed = 200.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Components")
  FPlatformerComponentTransformOffset TopCheckTransformOffset;

  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Drop Through Platform|Components")
  FPlatformerComponentTransformOffset BottomCheckTransformOffset;

private:
  UPROPERTY(Transient)
  TObjectPtr<UPrimitiveComponent> BlockingComponent;

  UPROPERTY(Transient)
  TObjectPtr<USceneComponent> TopCheckLayoutRoot;

  UPROPERTY(Transient)
  TObjectPtr<UBoxComponent> TopCheckBox;

  UPROPERTY(Transient)
  TObjectPtr<USceneComponent> BottomCheckLayoutRoot;

  UPROPERTY(Transient)
  TObjectPtr<UBoxComponent> BottomCheckBox;

  TSet<TWeakObjectPtr<ACharacter>> CharactersAbovePlatform;
  TSet<TWeakObjectPtr<ACharacter>> CharactersBelowPlatform;

  UFUNCTION()
  void OnTopCheckBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                              AActor *OtherActor,
                              UPrimitiveComponent *OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep,
                              const FHitResult &SweepResult);

  UFUNCTION()
  void OnTopCheckEndOverlap(UPrimitiveComponent *OverlappedComponent,
                            AActor *OtherActor, UPrimitiveComponent *OtherComp,
                            int32 OtherBodyIndex);

  UFUNCTION()
  void OnBottomCheckBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                                 AActor *OtherActor,
                                 UPrimitiveComponent *OtherComp,
                                 int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult &SweepResult);

  UFUNCTION()
  void OnBottomCheckEndOverlap(UPrimitiveComponent *OverlappedComponent,
                               AActor *OtherActor,
                               UPrimitiveComponent *OtherComp,
                               int32 OtherBodyIndex);

  void ConfigureCheckBox(UBoxComponent *CheckBox) const;
  void UpdateCheckBoxCollisionState() const;
  void ClearInvalidCharacterSet(TSet<TWeakObjectPtr<ACharacter>> &CharacterSet);
  void RestoreTrackedCharacterCollision();
  void StartIgnoringCharacter(ACharacter *Character, bool bForceDownwardDrop);
  void SetCharacterIgnoreComponentWhenMoving(ACharacter *Character,
                                             bool bShouldIgnore);
};
