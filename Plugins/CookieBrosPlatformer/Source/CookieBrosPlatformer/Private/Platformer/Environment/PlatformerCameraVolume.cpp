#include "Platformer/Environment/PlatformerCameraVolume.h"

#include "Character/PlatformerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Platformer/Camera/PlatformerCameraManager.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"

APlatformerCameraVolume::APlatformerCameraVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	NewCameraVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("NewCameraVolume"));
	NewCameraVolume->SetupAttachment(Root);
	NewCameraVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NewCameraVolume->SetCollisionObjectType(ECC_WorldDynamic);
	NewCameraVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	NewCameraVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	NewCameraVolume->SetGenerateOverlapEvents(true);
	NewCameraVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerCameraVolume::OnNewCameraVolumeBeginOverlap);

	DefaultCameraVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DefaultCameraVolume"));
	DefaultCameraVolume->SetupAttachment(Root);
	DefaultCameraVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DefaultCameraVolume->SetCollisionObjectType(ECC_WorldDynamic);
	DefaultCameraVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	DefaultCameraVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DefaultCameraVolume->SetGenerateOverlapEvents(true);
	DefaultCameraVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformerCameraVolume::OnDefaultCameraVolumeBeginOverlap);

	NewCameraVolumeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NewCameraVolumeText"));
	NewCameraVolumeText->SetupAttachment(Root);
	NewCameraVolumeText->SetText(NSLOCTEXT("CookieBrosPlatformer", "NewCameraSettingsText", "New Camera Settings"));
	NewCameraVolumeText->SetHorizontalAlignment(EHTA_Center);
	NewCameraVolumeText->SetVerticalAlignment(EVRTA_TextCenter);
	NewCameraVolumeText->SetWorldSize(48.0f);
	NewCameraVolumeText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewCameraVolumeText->SetGenerateOverlapEvents(false);

	DefaultCameraVolumeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DefaultCameraVolumeText"));
	DefaultCameraVolumeText->SetupAttachment(Root);
	DefaultCameraVolumeText->SetText(NSLOCTEXT("CookieBrosPlatformer", "ResetCameraSettingsText", "Reset Camera Settings"));
	DefaultCameraVolumeText->SetHorizontalAlignment(EHTA_Center);
	DefaultCameraVolumeText->SetVerticalAlignment(EVRTA_TextCenter);
	DefaultCameraVolumeText->SetWorldSize(48.0f);
	DefaultCameraVolumeText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DefaultCameraVolumeText->SetGenerateOverlapEvents(false);

	VolumePathComponent = CreateDefaultSubobject<UPlatformerPathComponent>(TEXT("VolumePath"));
	VolumePathComponent->SetupAttachment(Root);
}

void APlatformerCameraVolume::SetVolumeSizeNew(const FVector& InSize)
{
	VolumeSizeNew = InSize.ComponentMax(FVector(1.0f));
	RefreshVolumeLayout();
}

void APlatformerCameraVolume::SetVolumeSizeDefault(const FVector& InSize)
{
	VolumeSizeDefault = InSize.ComponentMax(FVector(1.0f));
	RefreshVolumeLayout();
}

void APlatformerCameraVolume::SetLocationNew(const FVector& InLocation)
{
	LocationNew = InLocation;
	RefreshVolumeLayout();
}

void APlatformerCameraVolume::SetLocationDefault(const FVector& InLocation)
{
	LocationDefault = InLocation;
	RefreshVolumeLayout();
}

void APlatformerCameraVolume::SetTargetCameraSettings(const FPlatformerCameraVolumeSettings& InSettings)
{
	TargetCameraSettings = InSettings;
}

void APlatformerCameraVolume::SetCameraBlendSettings(float InBlendTime, float InEaseExponent)
{
	BlendTime = FMath::Max(InBlendTime, 0.0f);
	EaseExponent = FMath::Max(InEaseExponent, 0.01f);
}

void APlatformerCameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVolumeLayout();
}

void APlatformerCameraVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlatformerCameraManager* CameraManager = ActiveCameraManager.Get();
	if (!bTransitionActive || !CameraManager)
	{
		bTransitionActive = false;
		SetActorTickEnabled(false);
		return;
	}

	const float ResolvedBlendTime = FMath::Max(BlendTime, 0.0f);
	TransitionElapsedTime += FMath::Max(DeltaTime, 0.0f);
	const float LinearAlpha = ResolvedBlendTime <= KINDA_SMALL_NUMBER
		? 1.0f
		: FMath::Clamp(TransitionElapsedTime / ResolvedBlendTime, 0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, LinearAlpha, FMath::Max(EaseExponent, 0.01f));

	ApplyBlendedSettings(EasedAlpha);

	if (LinearAlpha >= 1.0f)
	{
		bTransitionActive = false;
		SetActorTickEnabled(false);
	}
}

void APlatformerCameraVolume::OnNewCameraVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) && OtherActor->IsA<APlatformerCharacterBase>())
	{
		StartCameraTransition(OtherActor, TargetCameraSettings);
	}
}

void APlatformerCameraVolume::OnDefaultCameraVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) && OtherActor->IsA<APlatformerCharacterBase>() && bHasCapturedDefaultSettings)
	{
		StartCameraTransition(OtherActor, DefaultCameraSettings);
	}
}

void APlatformerCameraVolume::RefreshVolumeLayout()
{
	if (NewCameraVolume)
	{
		const FVector ResolvedVolumeSizeNew = VolumeSizeNew.ComponentMax(FVector(1.0f));
		NewCameraVolume->SetBoxExtent(ResolvedVolumeSizeNew * 0.5f);
		NewCameraVolume->SetRelativeLocation(LocationNew);

		if (NewCameraVolumeText)
		{
			NewCameraVolumeText->SetRelativeLocation(LocationNew + FVector(0.0f, 0.0f, ResolvedVolumeSizeNew.Z * 0.5f + 48.0f));
		}
	}

	if (DefaultCameraVolume)
	{
		const FVector ResolvedVolumeSizeDefault = VolumeSizeDefault.ComponentMax(FVector(1.0f));
		DefaultCameraVolume->SetBoxExtent(ResolvedVolumeSizeDefault * 0.5f);
		DefaultCameraVolume->SetRelativeLocation(LocationDefault);

		if (DefaultCameraVolumeText)
		{
			DefaultCameraVolumeText->SetRelativeLocation(LocationDefault + FVector(0.0f, 0.0f, ResolvedVolumeSizeDefault.Z * 0.5f + 48.0f));
		}
	}

	if (VolumePathComponent)
	{
		TArray<FPlatformerPathPoint> PathPoints;
		PathPoints.Add(FPlatformerPathPoint(LocationNew));
		PathPoints.Add(FPlatformerPathPoint(LocationDefault));
		VolumePathComponent->SetPathPoints(PathPoints);
	}
}

void APlatformerCameraVolume::StartCameraTransition(AActor* TriggeringActor, const FPlatformerCameraVolumeSettings& TargetSettings)
{
	APlatformerCameraManager* CameraManager = ResolveCameraManager(TriggeringActor);
	if (!CameraManager)
	{
		return;
	}

	APlatformerCharacterBase* Character = ResolvePlatformerCharacter(TriggeringActor);
	const FPlatformerCameraVolumeSettings CurrentSettings = CaptureCurrentSettings(CameraManager, Character);
	if (!bHasCapturedDefaultSettings)
	{
		DefaultCameraSettings = CurrentSettings;
		bHasCapturedDefaultSettings = true;
	}

	ActiveCameraManager = CameraManager;
	ActiveCharacter = Character;
	ActiveProjectionMode = CameraManager->CaptureDeveloperCameraManagerSettings().DeveloperCameraProjectionMode;
	TransitionStartSettings = CurrentSettings;
	TransitionTargetSettings = TargetSettings;
	TransitionElapsedTime = 0.0f;
	bTransitionActive = true;
	SetActorTickEnabled(true);
}

APlatformerCameraManager* APlatformerCameraVolume::ResolveCameraManager(AActor* TriggeringActor) const
{
	APlayerController* PlayerController = Cast<APlayerController>(TriggeringActor ? TriggeringActor->GetInstigatorController() : nullptr);
	if (!PlayerController)
	{
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	}

	return PlayerController ? Cast<APlatformerCameraManager>(PlayerController->PlayerCameraManager) : nullptr;
}

APlatformerCharacterBase* APlatformerCameraVolume::ResolvePlatformerCharacter(AActor* TriggeringActor) const
{
	return Cast<APlatformerCharacterBase>(TriggeringActor);
}

FPlatformerCameraVolumeSettings APlatformerCameraVolume::CaptureCurrentSettings(
	APlatformerCameraManager* CameraManager,
	APlatformerCharacterBase* Character) const
{
	const FDeveloperPlatformerCameraManagerSettings CameraManagerSettings = CameraManager
		? CameraManager->CaptureDeveloperCameraManagerSettings()
		: FDeveloperPlatformerCameraManagerSettings();
	const FDeveloperPlatformerCameraSettings CameraRigSettings = Character
		? Character->CaptureCameraRigSettings()
		: FDeveloperPlatformerCameraSettings();

	return FromRuntimeSettings(CameraManagerSettings, CameraRigSettings);
}

void APlatformerCameraVolume::ApplyBlendedSettings(float Alpha) const
{
	const FPlatformerCameraVolumeSettings BlendedSettings = LerpSettings(TransitionStartSettings, TransitionTargetSettings, Alpha);
	if (APlatformerCameraManager* CameraManager = ActiveCameraManager.Get())
	{
		CameraManager->ApplyDeveloperCameraManagerSettings(ToCameraManagerSettings(BlendedSettings, ActiveProjectionMode));
	}

	if (APlatformerCharacterBase* Character = ActiveCharacter.Get())
	{
		Character->ApplyCameraRigSettings(ToCameraRigSettings(BlendedSettings));
	}
}

FDeveloperPlatformerCameraManagerSettings APlatformerCameraVolume::ToCameraManagerSettings(
	const FPlatformerCameraVolumeSettings& Settings,
	EPlatformerCameraProjectionMode ProjectionMode)
{
	FDeveloperPlatformerCameraManagerSettings Result;
	Result.DeveloperCameraProjectionMode = ProjectionMode;
	Result.DeveloperCameraManagerOrthographicWidth = Settings.OrthographicWidth;
	Result.DeveloperCameraManagerIdleSpeedThreshold = Settings.IdleSpeedThreshold;
	Result.DeveloperCameraManagerHorizontalOffset = Settings.HorizontalOffset;
	Result.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart = Settings.HorizontalOffsetInterpSpeedStart;
	Result.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd = Settings.HorizontalOffsetInterpSpeedEnd;
	Result.DeveloperCameraManagerVerticalOffset = Settings.VerticalOffset;
	Result.DeveloperCameraManagerVerticalOffsetInterpSpeed = Settings.VerticalOffsetInterpSpeed;
	Result.DeveloperCameraManagerDeadZoneWidth = Settings.DeadZoneWidth;
	Result.DeveloperCameraManagerDeadZoneHeight = Settings.DeadZoneHeight;
	Result.DeveloperCameraManagerBoundBoxWidth = Settings.BoundBoxWidth;
	Result.DeveloperCameraManagerBoundBoxHeight = Settings.BoundBoxHeight;
	Result.DeveloperCameraManagerCrouchInterpSpeed = Settings.CrouchInterpSpeed;
	return Result;
}

FDeveloperPlatformerCameraSettings APlatformerCameraVolume::ToCameraRigSettings(const FPlatformerCameraVolumeSettings& Settings)
{
	FDeveloperPlatformerCameraSettings Result;
	Result.DeveloperSpringArmArmLength = Settings.SpringArmLength;
	Result.DeveloperSpringArmLocation = Settings.SpringArmLocation;
	Result.DeveloperSpringArmRotation = FVector(Settings.SpringArmRotation.Pitch, Settings.SpringArmRotation.Yaw, Settings.SpringArmRotation.Roll);
	Result.DeveloperCameraLocation = Settings.CameraLocation;
	Result.DeveloperCameraRotation = FVector(Settings.CameraRotation.Pitch, Settings.CameraRotation.Yaw, Settings.CameraRotation.Roll);
	return Result;
}

FPlatformerCameraVolumeSettings APlatformerCameraVolume::FromRuntimeSettings(
	const FDeveloperPlatformerCameraManagerSettings& CameraManagerSettings,
	const FDeveloperPlatformerCameraSettings& CameraRigSettings)
{
	FPlatformerCameraVolumeSettings Result;
	Result.CameraLocation = CameraRigSettings.DeveloperCameraLocation;
	Result.CameraRotation = FRotator(
		CameraRigSettings.DeveloperCameraRotation.X,
		CameraRigSettings.DeveloperCameraRotation.Y,
		CameraRigSettings.DeveloperCameraRotation.Z);
	Result.SpringArmLocation = CameraRigSettings.DeveloperSpringArmLocation;
	Result.SpringArmRotation = FRotator(
		CameraRigSettings.DeveloperSpringArmRotation.X,
		CameraRigSettings.DeveloperSpringArmRotation.Y,
		CameraRigSettings.DeveloperSpringArmRotation.Z);
	Result.SpringArmLength = CameraRigSettings.DeveloperSpringArmArmLength;
	Result.OrthographicWidth = CameraManagerSettings.DeveloperCameraManagerOrthographicWidth;
	Result.IdleSpeedThreshold = CameraManagerSettings.DeveloperCameraManagerIdleSpeedThreshold;
	Result.HorizontalOffset = CameraManagerSettings.DeveloperCameraManagerHorizontalOffset;
	Result.HorizontalOffsetInterpSpeedStart = CameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedStart;
	Result.HorizontalOffsetInterpSpeedEnd = CameraManagerSettings.DeveloperCameraManagerHorizontalOffsetInterpSpeedEnd;
	Result.VerticalOffset = CameraManagerSettings.DeveloperCameraManagerVerticalOffset;
	Result.VerticalOffsetInterpSpeed = CameraManagerSettings.DeveloperCameraManagerVerticalOffsetInterpSpeed;
	Result.DeadZoneWidth = CameraManagerSettings.DeveloperCameraManagerDeadZoneWidth;
	Result.DeadZoneHeight = CameraManagerSettings.DeveloperCameraManagerDeadZoneHeight;
	Result.BoundBoxWidth = CameraManagerSettings.DeveloperCameraManagerBoundBoxWidth;
	Result.BoundBoxHeight = CameraManagerSettings.DeveloperCameraManagerBoundBoxHeight;
	Result.CrouchInterpSpeed = CameraManagerSettings.DeveloperCameraManagerCrouchInterpSpeed;
	return Result;
}

FPlatformerCameraVolumeSettings APlatformerCameraVolume::LerpSettings(
	const FPlatformerCameraVolumeSettings& From,
	const FPlatformerCameraVolumeSettings& To,
	float Alpha)
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FPlatformerCameraVolumeSettings Result;
	Result.CameraLocation = FMath::Lerp(From.CameraLocation, To.CameraLocation, ClampedAlpha);
	Result.CameraRotation = FMath::Lerp(From.CameraRotation, To.CameraRotation, ClampedAlpha);
	Result.SpringArmLocation = FMath::Lerp(From.SpringArmLocation, To.SpringArmLocation, ClampedAlpha);
	Result.SpringArmRotation = FMath::Lerp(From.SpringArmRotation, To.SpringArmRotation, ClampedAlpha);
	Result.SpringArmLength = FMath::Lerp(From.SpringArmLength, To.SpringArmLength, ClampedAlpha);
	Result.OrthographicWidth = FMath::Lerp(From.OrthographicWidth, To.OrthographicWidth, ClampedAlpha);
	Result.IdleSpeedThreshold = FMath::Lerp(From.IdleSpeedThreshold, To.IdleSpeedThreshold, ClampedAlpha);
	Result.HorizontalOffset = FMath::Lerp(From.HorizontalOffset, To.HorizontalOffset, ClampedAlpha);
	Result.HorizontalOffsetInterpSpeedStart = FMath::Lerp(From.HorizontalOffsetInterpSpeedStart, To.HorizontalOffsetInterpSpeedStart, ClampedAlpha);
	Result.HorizontalOffsetInterpSpeedEnd = FMath::Lerp(From.HorizontalOffsetInterpSpeedEnd, To.HorizontalOffsetInterpSpeedEnd, ClampedAlpha);
	Result.VerticalOffset = FMath::Lerp(From.VerticalOffset, To.VerticalOffset, ClampedAlpha);
	Result.VerticalOffsetInterpSpeed = FMath::Lerp(From.VerticalOffsetInterpSpeed, To.VerticalOffsetInterpSpeed, ClampedAlpha);
	Result.DeadZoneWidth = FMath::Lerp(From.DeadZoneWidth, To.DeadZoneWidth, ClampedAlpha);
	Result.DeadZoneHeight = FMath::Lerp(From.DeadZoneHeight, To.DeadZoneHeight, ClampedAlpha);
	Result.BoundBoxWidth = FMath::Lerp(From.BoundBoxWidth, To.BoundBoxWidth, ClampedAlpha);
	Result.BoundBoxHeight = FMath::Lerp(From.BoundBoxHeight, To.BoundBoxHeight, ClampedAlpha);
	Result.CrouchInterpSpeed = FMath::Lerp(From.CrouchInterpSpeed, To.CrouchInterpSpeed, ClampedAlpha);
	return Result;
}
