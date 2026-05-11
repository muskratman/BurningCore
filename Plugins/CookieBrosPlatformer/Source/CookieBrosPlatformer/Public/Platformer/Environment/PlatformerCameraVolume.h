#pragma once

#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "GameFramework/Actor.h"
#include "PlatformerCameraVolume.generated.h"

class APlatformerCameraManager;
class APlatformerCharacterBase;
class UBoxComponent;
class UPlatformerPathComponent;
class UPrimitiveComponent;
class UTextRenderComponent;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerCameraVolumeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Rig")
	FVector CameraLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Rig")
	FRotator CameraRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Rig")
	FVector SpringArmLocation = FVector(0.0f, 0.0f, 70.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Rig")
	FRotator SpringArmRotation = FRotator(-7.0f, -90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Rig", meta=(ClampMin="0.0", Units="cm"))
	float SpringArmLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="1.0", Units="cm"))
	float OrthographicWidth = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="m/s"))
	float IdleSpeedThreshold = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float HorizontalOffset = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0"))
	float HorizontalOffsetInterpSpeedStart = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0"))
	float HorizontalOffsetInterpSpeedEnd = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float VerticalOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0"))
	float VerticalOffsetInterpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float DeadZoneWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float DeadZoneHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float BoundBoxWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0", Units="cm"))
	float BoundBoxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Manager", meta=(ClampMin="0.0"))
	float CrouchInterpSpeed = 8.0f;
};

/**
 * Camera tuning volume with separate enter/restore triggers.
 * Uses a level-facing settings struct and applies it as a transient runtime override.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerCameraVolume : public AActor
{
	GENERATED_BODY()

public:
	APlatformerCameraVolume();

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetVolumeSizeNew(const FVector& InSize);

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetVolumeSizeDefault(const FVector& InSize);

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetLocationNew(const FVector& InLocation);

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetLocationDefault(const FVector& InLocation);

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetTargetCameraSettings(const FPlatformerCameraVolumeSettings& InSettings);

	UFUNCTION(BlueprintCallable, Category="Camera Volume")
	void SetCameraBlendSettings(float InBlendTime, float InEaseExponent);

	FORCEINLINE const FVector& GetVolumeSizeNew() const { return VolumeSizeNew; }
	FORCEINLINE const FVector& GetVolumeSizeDefault() const { return VolumeSizeDefault; }
	FORCEINLINE const FVector& GetLocationNew() const { return LocationNew; }
	FORCEINLINE const FVector& GetLocationDefault() const { return LocationDefault; }
	FORCEINLINE const FPlatformerCameraVolumeSettings& GetTargetCameraSettings() const { return TargetCameraSettings; }
	FORCEINLINE float GetBlendTime() const { return BlendTime; }
	FORCEINLINE float GetEaseExponent() const { return EaseExponent; }

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> NewCameraVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DefaultCameraVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UTextRenderComponent> NewCameraVolumeText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UTextRenderComponent> DefaultCameraVolumeText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlatformerPathComponent> VolumePathComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Shape", meta=(ClampMin="1.0", Units="cm"))
	FVector VolumeSizeNew = FVector(400.0f, 1000.0f, 400.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Shape", meta=(MakeEditWidget=true))
	FVector LocationNew = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Shape", meta=(ClampMin="1.0", Units="cm"))
	FVector VolumeSizeDefault = FVector(400.0f, 1000.0f, 400.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Shape", meta=(MakeEditWidget=true))
	FVector LocationDefault = FVector(500.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Settings")
	FPlatformerCameraVolumeSettings TargetCameraSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Transition", meta=(ClampMin="0.0", Units="s"))
	float BlendTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Volume|Transition", meta=(ClampMin="0.01"))
	float EaseExponent = 2.0f;

private:
	UFUNCTION()
	void OnNewCameraVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDefaultCameraVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void RefreshVolumeLayout();
	void StartCameraTransition(AActor* TriggeringActor, const FPlatformerCameraVolumeSettings& TargetSettings);
	APlatformerCameraManager* ResolveCameraManager(AActor* TriggeringActor) const;
	APlatformerCharacterBase* ResolvePlatformerCharacter(AActor* TriggeringActor) const;
	FPlatformerCameraVolumeSettings CaptureCurrentSettings(APlatformerCameraManager* CameraManager, APlatformerCharacterBase* Character) const;
	void ApplyBlendedSettings(float Alpha) const;
	static FDeveloperPlatformerCameraManagerSettings ToCameraManagerSettings(const FPlatformerCameraVolumeSettings& Settings, EPlatformerCameraProjectionMode ProjectionMode);
	static FDeveloperPlatformerCameraSettings ToCameraRigSettings(const FPlatformerCameraVolumeSettings& Settings);
	static FPlatformerCameraVolumeSettings FromRuntimeSettings(
		const FDeveloperPlatformerCameraManagerSettings& CameraManagerSettings,
		const FDeveloperPlatformerCameraSettings& CameraRigSettings);
	static FPlatformerCameraVolumeSettings LerpSettings(const FPlatformerCameraVolumeSettings& From, const FPlatformerCameraVolumeSettings& To, float Alpha);

	UPROPERTY(Transient)
	FPlatformerCameraVolumeSettings DefaultCameraSettings;

	UPROPERTY(Transient)
	FPlatformerCameraVolumeSettings TransitionStartSettings;

	UPROPERTY(Transient)
	FPlatformerCameraVolumeSettings TransitionTargetSettings;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlatformerCameraManager> ActiveCameraManager;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlatformerCharacterBase> ActiveCharacter;

	UPROPERTY(Transient)
	EPlatformerCameraProjectionMode ActiveProjectionMode = EPlatformerCameraProjectionMode::Perspective;

	UPROPERTY(Transient)
	bool bHasCapturedDefaultSettings = false;

	UPROPERTY(Transient)
	bool bTransitionActive = false;

	UPROPERTY(Transient)
	float TransitionElapsedTime = 0.0f;
};
