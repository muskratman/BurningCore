#pragma once

#include "CoreMinimal.h"
#include "Platformer/Character/PlatformerInteractable.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "GameFramework/Actor.h"
#include "PlatformerSwitch.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerSwitch : public AActor, public IPlatformerInteractable
{
	GENERATED_BODY()

public:
	APlatformerSwitch();

	virtual void Interaction(AActor* Interactor) override;
	void SetTriggerExtent(const FVector& InTriggerExtent);

	FORCEINLINE const FVector& GetTriggerExtent() const { return TriggerExtent; }

	UFUNCTION(BlueprintCallable, Category="Switch")
	void ActivateSwitch(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Switch")
	void SetSwitchState(bool bNewIsOn, AActor* InstigatorActor = nullptr);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SwitchMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> SwitchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TriggerVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Shape")
	FVector TriggerExtent = FVector(64.0f, 64.0f, 64.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Components")
	FPlatformerComponentTransformOffset SwitchMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Components")
	FPlatformerComponentTransformOffset TriggerVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Behaviour")
	bool bStartsOn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Behaviour")
	bool bToggleOnEveryActivation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Behaviour")
	bool bOneShot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Behaviour")
	bool bOnlyCharactersCanActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float ActivationCooldown = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Identity")
	FString SwitchId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Targets")
	TArray<TObjectPtr<AActor>> InteractionTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Switch|Targets")
	TArray<TObjectPtr<AActor>> ToggleTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch|State")
	bool bIsOn = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	double LastActivationTime = -DBL_MAX;

	UFUNCTION()
	void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyToggleTargetState(AActor* TargetActor, bool bEnabled) const;
};
