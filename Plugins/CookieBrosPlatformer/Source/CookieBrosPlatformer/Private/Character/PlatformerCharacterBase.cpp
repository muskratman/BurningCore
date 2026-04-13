#include "Character/PlatformerCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/SideViewMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/PlatformerDeveloperSettingsSubsystem.h"
#include "Core/PlatformerPlayerControllerBase.h"
#include "Developer/DeveloperJumpTrajectory.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GAS/Abilities/GA_PlatformerJump.h"
#include "GAS/PlatformerGameplayTags.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/PlatformerAbilitySet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/SpringArmComponent.h"
#include "Platformer/Environment/PlatformerLedgeGrab.h"
#include "Platformer/Environment/PlatformerLadder.h"

APlatformerCharacterBase::APlatformerCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USideViewMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DeveloperJumpTrajectoryClass = ADeveloperJumpTrajectory::StaticClass();

	if (AttributeSet)
	{
		AttributeSet->InitMaxHealth(100.0f);
		AttributeSet->InitHealth(100.0f);
		AttributeSet->InitRangeBaseAttackDamage(25.0f);
		AttributeSet->InitRangeChargedAttackDamage(75.0f);
	}

	UpdateHealthWidgetPlacement();
}

void APlatformerCharacterBase::InitializeAbilities(const UPlatformerAbilitySet* AbilitySet)
{
	if (!AbilitySystemComponent || !AbilitySet || !HasAuthority())
	{
		return;
	}

	for (const FPlatformerAbilitySet_Ability& AbilityData : AbilitySet->Abilities)
	{
		if (AbilityData.AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityData.AbilityClass, AbilityData.AbilityLevel, INDEX_NONE, this));
		}
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(this, this);

	for (const FPlatformerAbilitySet_Effect& EffectData : AbilitySet->Effects)
	{
		if (!EffectData.EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(EffectData.EffectClass, EffectData.EffectLevel, ContextHandle);
		if (EffectSpec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
		}
	}
}

void APlatformerCharacterBase::ApplyDeveloperCharacterSettings(const FDeveloperPlatformerCharacterSettings& DeveloperSettings)
{
	ApplyDeveloperCameraSettings(DeveloperSettings.DeveloperCameraSettings);
	ApplyDeveloperCharacterMovementSettings(DeveloperSettings.DeveloperCharacterMovementSettings);
	ApplyDeveloperCombatSettings(DeveloperSettings.DeveloperCombatSettings);
}

void APlatformerCharacterBase::ApplyDeveloperSettingsSnapshot(const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot)
{
	const bool bPreviousShowJumpTrajectoryPreview = bShowJumpTrajectoryPreview;

	ApplyDeveloperCameraSettings(DeveloperSettingsSnapshot.CharacterSettings.DeveloperCameraSettings);
	ApplyDeveloperCharacterMovementSettings(DeveloperSettingsSnapshot.CharacterSettings.DeveloperCharacterMovementSettings);

	if (!DeveloperSettingsSnapshot.bHasSavedJumpHorizontalSpeed)
	{
		ClearDeveloperJumpHorizontalSpeedOverride();
	}

	if (!DeveloperSettingsSnapshot.bHasSavedCrouchCapsuleScale)
	{
		ClearDeveloperCrouchCapsuleScaleOverride();
	}

	if (!DeveloperSettingsSnapshot.bHasSavedJumpTrajectoryPreview)
	{
		SetShowJumpTrajectoryPreview(bPreviousShowJumpTrajectoryPreview);
		RefreshJumpTrajectoryPreview();
	}

	if (DeveloperSettingsSnapshot.bHasSavedCombatSettings)
	{
		ApplyDeveloperCombatSettings(ResolveDeveloperCombatSettingsForApplication(
			DeveloperSettingsSnapshot.CharacterSettings.DeveloperCombatSettings));
	}
	else
	{
		SetHasActiveDeveloperCombatSettings(false);
		ActiveDeveloperCombatSettings = FDeveloperPlatformerCombatSettings();
	}
}

FDeveloperPlatformerCharacterSettings APlatformerCharacterBase::CaptureDeveloperCharacterSettings() const
{
	FDeveloperPlatformerCharacterSettings DeveloperSettings;
	DeveloperSettings.DeveloperCameraSettings = CaptureDeveloperCameraSettings();
	DeveloperSettings.DeveloperCharacterMovementSettings = CaptureDeveloperCharacterMovementSettings();
	DeveloperSettings.DeveloperCombatSettings = CaptureDeveloperCombatSettings();
	return DeveloperSettings;
}

FPlatformerDeveloperSettingsSnapshot APlatformerCharacterBase::CaptureDeveloperSettingsSnapshot() const
{
	FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
	DeveloperSettingsSnapshot.CharacterSettings = CaptureDeveloperCharacterSettings();
	DeveloperSettingsSnapshot.bHasSavedCombatSettings = HasActiveDeveloperCombatSettings();
	DeveloperSettingsSnapshot.bHasSavedJumpHorizontalSpeed = bHasDeveloperJumpHorizontalSpeedOverride;
	DeveloperSettingsSnapshot.bHasSavedCrouchCapsuleScale = bHasDeveloperCrouchCapsuleScaleOverride;
	DeveloperSettingsSnapshot.bHasSavedJumpTrajectoryPreview = true;
	return DeveloperSettingsSnapshot;
}

FVector APlatformerCharacterBase::GetPlatformerCameraFocusLocation() const
{
	return GetActorLocation();
}

float APlatformerCharacterBase::ResolveDeveloperCrouchCapsuleScale(float DefaultCrouchCapsuleScale) const
{
	return bHasDeveloperCrouchCapsuleScaleOverride
		? DeveloperCrouchCapsuleScaleOverride
		: FMath::Max(DefaultCrouchCapsuleScale, 0.0f);
}

float APlatformerCharacterBase::ResolveDeveloperJumpHorizontalSpeed(float DefaultJumpHorizontalSpeed) const
{
	return bHasDeveloperJumpHorizontalSpeedOverride
		? DeveloperJumpHorizontalSpeedOverride
		: FMath::Max(DefaultJumpHorizontalSpeed, 0.0f);
}

void APlatformerCharacterBase::NotifyLadderAvailable(APlatformerLadder* Ladder)
{
	if (!Ladder)
	{
		return;
	}

	AvailableLadder = Ladder;
}

void APlatformerCharacterBase::NotifyLadderUnavailable(APlatformerLadder* Ladder)
{
	if (!Ladder)
	{
		return;
	}

	if (AvailableLadder == Ladder)
	{
		AvailableLadder = nullptr;
	}

	if (ActiveLadder == Ladder)
	{
		ExitLadder(Ladder);

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}

void APlatformerCharacterBase::NotifyLedgeGrabAvailable(APlatformerLedgeGrab* LedgeGrab)
{
	if (!LedgeGrab)
	{
		return;
	}

	AvailableLedgeGrabs.AddUnique(LedgeGrab);
}

void APlatformerCharacterBase::NotifyLedgeGrabUnavailable(APlatformerLedgeGrab* LedgeGrab)
{
	if (!LedgeGrab)
	{
		return;
	}

	AvailableLedgeGrabs.RemoveAll(
		[LedgeGrab](const TWeakObjectPtr<APlatformerLedgeGrab>& AvailableLedgeGrab)
		{
			return !AvailableLedgeGrab.IsValid() || AvailableLedgeGrab.Get() == LedgeGrab;
		});
}

bool APlatformerCharacterBase::EnterLadder(APlatformerLadder* Ladder)
{
	if (!Ladder)
	{
		return false;
	}

	if (bIsOnLadder && ActiveLadder == Ladder)
	{
		return true;
	}

	if (AvailableLadder != Ladder)
	{
		return false;
	}

	if (bIsOnLadder)
	{
		ExitLadder();
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	SavedPreLadderGravityScale = MovementComponent->GravityScale;
	SavedPreLadderMovementMode = MovementComponent->MovementMode;

	if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
	{
		bHadPreLadderGravityOverride = SideViewMovementComponent->HasExternalGravityScaleOverride();
		PreLadderGravityOverride = SideViewMovementComponent->GetExternalGravityScaleOverride();
		SideViewMovementComponent->SetExternalGravityScaleOverride(Ladder->GetClimbGravityScale());
	}
	else
	{
		bHadPreLadderGravityOverride = false;
		PreLadderGravityOverride = SavedPreLadderGravityScale;
		MovementComponent->GravityScale = Ladder->GetClimbGravityScale();
	}

	if (Ladder->UsesFlyingMovementMode())
	{
		MovementComponent->SetMovementMode(MOVE_Flying);
	}

	ActiveLadder = Ladder;
	bIsOnLadder = true;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(PlatformerGameplayTags::State_Movement_Ladder);
	}

	if (Ladder->ShouldSnapCharacterDepthToLadder())
	{
		FVector CharacterLocation = GetActorLocation();
		CharacterLocation.Y = Ladder->GetActorLocation().Y;
		SetActorLocation(CharacterLocation);
	}

	OnEnteredLadder(Ladder);
	return true;
}

void APlatformerCharacterBase::ExitLadder(APlatformerLadder* Ladder)
{
	if (!bIsOnLadder || !ActiveLadder)
	{
		return;
	}

	if (Ladder && ActiveLadder != Ladder)
	{
		return;
	}

	APlatformerLadder* ExitedLadder = ActiveLadder;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			if (bHadPreLadderGravityOverride)
			{
				SideViewMovementComponent->SetExternalGravityScaleOverride(PreLadderGravityOverride);
			}
			else
			{
				SideViewMovementComponent->ClearExternalGravityScaleOverride();
			}
		}
		else
		{
			MovementComponent->GravityScale = SavedPreLadderGravityScale;
		}

		MovementComponent->SetMovementMode(SavedPreLadderMovementMode);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerGameplayTags::State_Movement_Ladder);
	}

	ActiveLadder = nullptr;
	bIsOnLadder = false;
	bHadPreLadderGravityOverride = false;
	PreLadderGravityOverride = SavedPreLadderGravityScale;

	OnExitedLadder(ExitedLadder);
}

void APlatformerCharacterBase::GetAvailableLedgeGrabs(TArray<APlatformerLedgeGrab*>& OutLedgeGrabs) const
{
	OutLedgeGrabs.Reset();

	for (const TWeakObjectPtr<APlatformerLedgeGrab>& AvailableLedgeGrab : AvailableLedgeGrabs)
	{
		if (APlatformerLedgeGrab* ResolvedLedgeGrab = AvailableLedgeGrab.Get())
		{
			OutLedgeGrabs.Add(ResolvedLedgeGrab);
		}
	}
}

void APlatformerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		InitializeAbilities(DefaultAbilitySet);
	}

	LoadAndApplyDeveloperSettings();
	EnsureDeveloperJumpTrajectoryActor();
	RefreshJumpTrajectoryPreview();
}

void APlatformerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOnLadder)
	{
		if (!ActiveLadder)
		{
			ExitLadder();
			if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
			{
				MovementComponent->SetMovementMode(MOVE_Falling);
			}
			return;
		}
	}
	else if (AvailableLadder && !IsValid(AvailableLadder))
	{
		AvailableLadder = nullptr;
	}

	AvailableLedgeGrabs.RemoveAll(
		[](const TWeakObjectPtr<APlatformerLedgeGrab>& AvailableLedgeGrab)
		{
			return !AvailableLedgeGrab.IsValid();
		});
}

void APlatformerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyDeveloperJumpTrajectoryActor();
	Super::EndPlay(EndPlayReason);
}

void APlatformerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input binding lives in higher-level player/controller shells.
}

void APlatformerCharacterBase::FellOutOfWorld(const UDamageType& DamageType)
{
	if (AttributeSet && AttributeSet->GetHealth() > 0.0f)
	{
		AttributeSet->SetHealth(0.0f);
		SyncCombatLifeStateFromAttributes();
	}
}

void APlatformerCharacterBase::OnCombatDeath(AActor* DamageInstigatorActor)
{
	Super::OnCombatDeath(DamageInstigatorActor);

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetHiddenInGame(true, true);
		CharacterMesh->SetVisibility(false, true);
	}

	if (UCapsuleComponent* CharacterCapsule = GetCapsuleComponent())
	{
		CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->SetVisibility(false);
	}

	if (APlatformerPlayerControllerBase* PlatformerPlayerController = Cast<APlatformerPlayerControllerBase>(GetController()))
	{
		PlatformerPlayerController->HandleControlledCharacterDeath();
	}
}

void APlatformerCharacterBase::OnCombatRevived()
{
	Super::OnCombatRevived();

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetHiddenInGame(false, true);
		CharacterMesh->SetVisibility(true, true);
	}

	if (UCapsuleComponent* CharacterCapsule = GetCapsuleComponent())
	{
		CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (HealthWidgetComponent)
	{
		RefreshHealthWidget();
	}
}

void APlatformerCharacterBase::OnEnteredLadder(APlatformerLadder* Ladder)
{
}

void APlatformerCharacterBase::OnExitedLadder(APlatformerLadder* Ladder)
{
}

void APlatformerCharacterBase::ApplyDeveloperCameraSettings(const FDeveloperPlatformerCameraSettings& DeveloperCameraSettings)
{
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = DeveloperCameraSettings.DeveloperSpringArmArmLength;
		CameraBoom->SetRelativeLocation(DeveloperCameraSettings.DeveloperSpringArmLocation);
		CameraBoom->SetRelativeRotation(FRotator(
			DeveloperCameraSettings.DeveloperSpringArmRotation.X,
			DeveloperCameraSettings.DeveloperSpringArmRotation.Y,
			DeveloperCameraSettings.DeveloperSpringArmRotation.Z));
	}

	if (FollowCamera)
	{
		FollowCamera->FieldOfView = DeveloperCameraSettings.DeveloperCameraFOV;
		FollowCamera->SetRelativeLocation(FVector(
			DeveloperCameraSettings.DeveloperCameraLocation.Y,
			DeveloperCameraSettings.DeveloperCameraLocation.X,
			DeveloperCameraSettings.DeveloperCameraLocation.Z));
		FollowCamera->SetRelativeRotation(FRotator(
			DeveloperCameraSettings.DeveloperCameraRotation.X,
			DeveloperCameraSettings.DeveloperCameraRotation.Y,
			DeveloperCameraSettings.DeveloperCameraRotation.Z));
	}
}

FDeveloperPlatformerCameraSettings APlatformerCharacterBase::CaptureDeveloperCameraSettings() const
{
	FDeveloperPlatformerCameraSettings DeveloperCameraSettings;

	if (CameraBoom)
	{
		DeveloperCameraSettings.DeveloperSpringArmArmLength = CameraBoom->TargetArmLength;
		DeveloperCameraSettings.DeveloperSpringArmLocation = CameraBoom->GetRelativeLocation();

		const FRotator SpringArmRotation = CameraBoom->GetRelativeRotation();
		DeveloperCameraSettings.DeveloperSpringArmRotation = FVector(
			SpringArmRotation.Pitch,
			SpringArmRotation.Yaw,
			SpringArmRotation.Roll);
	}

	if (FollowCamera)
	{
		DeveloperCameraSettings.DeveloperCameraFOV = FollowCamera->FieldOfView;
		const FVector CameraLocation = FollowCamera->GetRelativeLocation();
		DeveloperCameraSettings.DeveloperCameraLocation = FVector(
			CameraLocation.Y,
			CameraLocation.X,
			CameraLocation.Z);

		const FRotator CameraRotation = FollowCamera->GetRelativeRotation();
		DeveloperCameraSettings.DeveloperCameraRotation = FVector(
			CameraRotation.Pitch,
			CameraRotation.Yaw,
			CameraRotation.Roll);
	}

	return DeveloperCameraSettings;
}

void APlatformerCharacterBase::ApplyDeveloperCharacterMovementSettings(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed);
		MovementComponent->MaxFlySpeed = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed);
		MovementComponent->MaxAcceleration = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration);
		MovementComponent->BrakingDecelerationWalking = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking);
		MovementComponent->JumpZVelocity = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity);
		MovementComponent->Mass = FMath::Max(KINDA_SMALL_NUMBER, DeveloperCharacterMovementSettings.DeveloperMovementMass);
		MovementComponent->BrakingFrictionFactor = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor);
		MovementComponent->GroundFriction = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction);
		MovementComponent->AirControl = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementAirControl);

		if (USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			SideViewMovementComponent->JumpApexGravityMultiplier = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier);
			SideViewMovementComponent->SetBaseGravityScale(DeveloperCharacterMovementSettings.DeveloperMovementGravityScale);
		}
		else
		{
			MovementComponent->GravityScale = FMath::Max(0.0f, DeveloperCharacterMovementSettings.DeveloperMovementGravityScale);
		}
	}

	SetDeveloperCrouchCapsuleScaleOverride(DeveloperCharacterMovementSettings.DeveloperMovementCrouchCapsuleScale);
	SetDeveloperJumpHorizontalSpeedOverride(DeveloperCharacterMovementSettings.DeveloperMovementJumpHorizontalSpeed);
	SetShowJumpTrajectoryPreview(DeveloperCharacterMovementSettings.DeveloperMovementShowJumpTrajectory);
	RefreshJumpTrajectoryPreview();
}

FDeveloperPlatformerCharacterMovementSettings APlatformerCharacterBase::CaptureDeveloperCharacterMovementSettings() const
{
	FDeveloperPlatformerCharacterMovementSettings DeveloperCharacterMovementSettings;

	if (const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		DeveloperCharacterMovementSettings.DeveloperMovementMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		DeveloperCharacterMovementSettings.DeveloperMovementMaxFlySpeed = MovementComponent->MaxFlySpeed;
		DeveloperCharacterMovementSettings.DeveloperMovementMaxAcceleration = MovementComponent->MaxAcceleration;
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		DeveloperCharacterMovementSettings.DeveloperMovementJumpZVelocity = MovementComponent->JumpZVelocity;
		DeveloperCharacterMovementSettings.DeveloperMovementMass = MovementComponent->Mass;
		DeveloperCharacterMovementSettings.DeveloperMovementBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
		DeveloperCharacterMovementSettings.DeveloperMovementGroundFriction = MovementComponent->GroundFriction;
		DeveloperCharacterMovementSettings.DeveloperMovementAirControl = MovementComponent->AirControl;

		if (const USideViewMovementComponent* SideViewMovementComponent = Cast<USideViewMovementComponent>(MovementComponent))
		{
			DeveloperCharacterMovementSettings.DeveloperMovementJumpApexGravityMultiplier = SideViewMovementComponent->JumpApexGravityMultiplier;
			DeveloperCharacterMovementSettings.DeveloperMovementGravityScale = SideViewMovementComponent->GetBaseGravityScale();
		}
		else
		{
			DeveloperCharacterMovementSettings.DeveloperMovementGravityScale = MovementComponent->GravityScale;
		}
	}

	DeveloperCharacterMovementSettings.DeveloperMovementCrouchCapsuleScale = CaptureDeveloperCrouchCapsuleScale();
	DeveloperCharacterMovementSettings.DeveloperMovementJumpHorizontalSpeed = CaptureDeveloperJumpHorizontalSpeed();
	DeveloperCharacterMovementSettings.DeveloperMovementShowJumpTrajectory = bShowJumpTrajectoryPreview;

	return DeveloperCharacterMovementSettings;
}

void APlatformerCharacterBase::SetDeveloperCrouchCapsuleScaleOverride(float InCrouchCapsuleScale)
{
	bHasDeveloperCrouchCapsuleScaleOverride = true;
	DeveloperCrouchCapsuleScaleOverride = FMath::Max(InCrouchCapsuleScale, 0.0f);
	ApplyResolvedCrouchCapsuleScale();
}

void APlatformerCharacterBase::ClearDeveloperCrouchCapsuleScaleOverride()
{
	bHasDeveloperCrouchCapsuleScaleOverride = false;
	DeveloperCrouchCapsuleScaleOverride = 0.0f;
	ApplyResolvedCrouchCapsuleScale();
}

void APlatformerCharacterBase::SetDeveloperJumpHorizontalSpeedOverride(float InJumpHorizontalSpeed)
{
	bHasDeveloperJumpHorizontalSpeedOverride = true;
	DeveloperJumpHorizontalSpeedOverride = FMath::Max(InJumpHorizontalSpeed, 0.0f);
}

void APlatformerCharacterBase::ClearDeveloperJumpHorizontalSpeedOverride()
{
	bHasDeveloperJumpHorizontalSpeedOverride = false;
	DeveloperJumpHorizontalSpeedOverride = 0.0f;
}

float APlatformerCharacterBase::CaptureDeveloperCrouchCapsuleScale() const
{
	if (bHasDeveloperCrouchCapsuleScaleOverride)
	{
		return DeveloperCrouchCapsuleScaleOverride;
	}

	return ResolveDefaultCrouchCapsuleScale();
}

void APlatformerCharacterBase::SetShowJumpTrajectoryPreview(bool bInShowJumpTrajectoryPreview)
{
	bShowJumpTrajectoryPreview = bInShowJumpTrajectoryPreview;

	if (ADeveloperJumpTrajectory* JumpTrajectoryActor = EnsureDeveloperJumpTrajectoryActor())
	{
		JumpTrajectoryActor->SetShowJumpTrajectoryPreview(bShowJumpTrajectoryPreview);
	}
}

float APlatformerCharacterBase::ResolveDefaultCrouchCapsuleScale() const
{
	const ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
	const UCharacterMovementComponent* DefaultMovementComponent = DefaultCharacter
		? DefaultCharacter->GetCharacterMovement()
		: GetCharacterMovement();
	const float StandingCapsuleHalfHeight = ResolveStandingCapsuleHalfHeight();
	if (!DefaultMovementComponent || StandingCapsuleHalfHeight <= UE_KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	return FMath::Max(DefaultMovementComponent->GetCrouchedHalfHeight() / StandingCapsuleHalfHeight, 0.0f);
}

float APlatformerCharacterBase::ResolveStandingCapsuleHalfHeight() const
{
	const ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
	const UCapsuleComponent* DefaultCapsuleComponent = DefaultCharacter
		? DefaultCharacter->GetCapsuleComponent()
		: GetCapsuleComponent();
	return DefaultCapsuleComponent
		? FMath::Max(DefaultCapsuleComponent->GetUnscaledCapsuleHalfHeight(), 0.0f)
		: 0.0f;
}

void APlatformerCharacterBase::ApplyResolvedCrouchCapsuleScale()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	UCapsuleComponent* CapsuleComponent = GetCapsuleComponent();
	if (!MovementComponent || !CapsuleComponent)
	{
		return;
	}

	const float StandingCapsuleHalfHeight = ResolveStandingCapsuleHalfHeight();
	if (StandingCapsuleHalfHeight <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float ResolvedCrouchCapsuleScale = ResolveDeveloperCrouchCapsuleScale(ResolveDefaultCrouchCapsuleScale());
	const float ResolvedCrouchedHalfHeight = FMath::Max(
		CapsuleComponent->GetUnscaledCapsuleRadius(),
		StandingCapsuleHalfHeight * ResolvedCrouchCapsuleScale);
	MovementComponent->SetCrouchedHalfHeight(ResolvedCrouchedHalfHeight);

	if (bIsCrouched)
	{
		MovementComponent->Crouch(false);
	}
}

float APlatformerCharacterBase::CaptureDeveloperJumpHorizontalSpeed() const
{
	if (bHasDeveloperJumpHorizontalSpeedOverride)
	{
		return DeveloperJumpHorizontalSpeedOverride;
	}

	if (const UGA_PlatformerJump* JumpAbility = FindGrantedJumpAbility())
	{
		return JumpAbility->GetJumpHorizontalSpeed();
	}

	return 0.0f;
}

const UGA_PlatformerJump* APlatformerCharacterBase::FindGrantedJumpAbility() const
{
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = AbilitySystemComponent->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities)
	{
		if (const UGA_PlatformerJump* JumpAbility = Cast<UGA_PlatformerJump>(AbilitySpec.Ability))
		{
			return JumpAbility;
		}
	}

	return nullptr;
}

void APlatformerCharacterBase::RefreshJumpTrajectoryPreview()
{
	if (ADeveloperJumpTrajectory* JumpTrajectoryActor = EnsureDeveloperJumpTrajectoryActor())
	{
		JumpTrajectoryActor->SetShowJumpTrajectoryPreview(bShowJumpTrajectoryPreview);
		JumpTrajectoryActor->RefreshTrajectoryPreview();
	}
}

ADeveloperJumpTrajectory* APlatformerCharacterBase::SpawnJumpTrajectorySnapshotActor()
{
	ADeveloperJumpTrajectory* JumpTrajectoryActor = EnsureDeveloperJumpTrajectoryActor();
	if (!JumpTrajectoryActor)
	{
		return nullptr;
	}

	JumpTrajectoryActor->SetShowJumpTrajectoryPreview(bShowJumpTrajectoryPreview);
	JumpTrajectoryActor->RefreshTrajectoryPreview();
	return JumpTrajectoryActor->SpawnSnapshotCopy();
}

ADeveloperJumpTrajectory* APlatformerCharacterBase::EnsureDeveloperJumpTrajectoryActor()
{
	if (IsValid(DeveloperJumpTrajectoryActor))
	{
		return DeveloperJumpTrajectoryActor;
	}

	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return nullptr;
	}

	if (!GetCapsuleComponent())
	{
		return nullptr;
	}

	UClass* JumpTrajectoryClass = DeveloperJumpTrajectoryClass.Get();
	if (!JumpTrajectoryClass)
	{
		JumpTrajectoryClass = ADeveloperJumpTrajectory::StaticClass();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	DeveloperJumpTrajectoryActor = World->SpawnActor<ADeveloperJumpTrajectory>(
		JumpTrajectoryClass,
		GetCapsuleComponent()->GetComponentTransform(),
		SpawnParameters);
	if (!DeveloperJumpTrajectoryActor)
	{
		return nullptr;
	}

	DeveloperJumpTrajectoryActor->AttachToComponent(
		GetCapsuleComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	DeveloperJumpTrajectoryActor->SetActorRelativeLocation(FVector::ZeroVector);
	DeveloperJumpTrajectoryActor->SetActorRelativeRotation(FRotator::ZeroRotator);
	DeveloperJumpTrajectoryActor->InitializeAttachedPreview(this);
	DeveloperJumpTrajectoryActor->SetShowJumpTrajectoryPreview(bShowJumpTrajectoryPreview);

	return DeveloperJumpTrajectoryActor;
}

void APlatformerCharacterBase::DestroyDeveloperJumpTrajectoryActor()
{
	if (!IsValid(DeveloperJumpTrajectoryActor))
	{
		return;
	}

	DeveloperJumpTrajectoryActor->Destroy();
	DeveloperJumpTrajectoryActor = nullptr;
}

void APlatformerCharacterBase::ApplyDeveloperCombatSettings(const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings)
{
	ActiveDeveloperCombatSettings = DeveloperCombatSettings;
	SetHasActiveDeveloperCombatSettings(true);

	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	const FGameplayAttribute DeveloperMaxHealthAttribute = UPlatformerCharacterAttributeSet::GetMaxHealthAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMaxHealthAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperMaxHealthAttribute,
			FMath::Max(DeveloperCombatSettings.DeveloperCombatMaxHealth, 1.0f));
	}

	const FGameplayAttribute DeveloperHealthAttribute = UPlatformerCharacterAttributeSet::GetHealthAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperHealthAttribute))
	{
		const float ResolvedDeveloperMaxHealth = AbilitySystemComponent->GetNumericAttribute(DeveloperMaxHealthAttribute);
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperHealthAttribute,
			FMath::Clamp(DeveloperCombatSettings.DeveloperCombatCurrentHealth, 0.0f, ResolvedDeveloperMaxHealth));
	}

	const FGameplayAttribute DeveloperMeleeAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetMeleeAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMeleeAttackDamageAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperMeleeAttackDamageAttribute,
			FMath::Max(0.0f, DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage));
	}

	const FGameplayAttribute DeveloperMeleeAttackDelayAttribute = UPlatformerCharacterAttributeSet::GetMeleeAttackDelayAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMeleeAttackDelayAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperMeleeAttackDelayAttribute,
			FMath::Max(0.0f, DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay));
	}

	const FGameplayAttribute DeveloperRangeBaseAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetRangeBaseAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeBaseAttackDamageAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperRangeBaseAttackDamageAttribute,
			FMath::Max(0.0f, DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage));
	}

	const FGameplayAttribute DeveloperRangeChargedAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetRangeChargedAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeChargedAttackDamageAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperRangeChargedAttackDamageAttribute,
			FMath::Max(0.0f, DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage));
	}

	const FGameplayAttribute DeveloperRangeAttackDelayAttribute = UPlatformerCharacterAttributeSet::GetRangeAttackDelayAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeAttackDelayAttribute))
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			DeveloperRangeAttackDelayAttribute,
			FMath::Max(0.0f, DeveloperCombatSettings.DeveloperCombatRangeAttackDelay));
	}

	RefreshHealthWidget();
	SyncCombatLifeStateFromAttributes();
}

FDeveloperPlatformerCombatSettings APlatformerCharacterBase::CaptureDeveloperCombatSettings() const
{
	FDeveloperPlatformerCombatSettings DeveloperCombatSettings = ActiveDeveloperCombatSettings;

	if (!AbilitySystemComponent || !AttributeSet)
	{
		return DeveloperCombatSettings;
	}

	const FGameplayAttribute DeveloperMaxHealthAttribute = UPlatformerCharacterAttributeSet::GetMaxHealthAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMaxHealthAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatMaxHealth = AbilitySystemComponent->GetNumericAttribute(DeveloperMaxHealthAttribute);
	}
	else
	{
		DeveloperCombatSettings.DeveloperCombatMaxHealth = AttributeSet->GetMaxHealth();
	}

	const FGameplayAttribute DeveloperHealthAttribute = UPlatformerCharacterAttributeSet::GetHealthAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperHealthAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatCurrentHealth = AbilitySystemComponent->GetNumericAttribute(DeveloperHealthAttribute);
	}
	else
	{
		DeveloperCombatSettings.DeveloperCombatCurrentHealth = AttributeSet->GetHealth();
	}

	const FGameplayAttribute DeveloperMeleeAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetMeleeAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMeleeAttackDamageAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatMeleeAttackDamage = AbilitySystemComponent->GetNumericAttribute(DeveloperMeleeAttackDamageAttribute);
	}

	const FGameplayAttribute DeveloperMeleeAttackDelayAttribute = UPlatformerCharacterAttributeSet::GetMeleeAttackDelayAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperMeleeAttackDelayAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatMeleeAttackDelay = AbilitySystemComponent->GetNumericAttribute(DeveloperMeleeAttackDelayAttribute);
	}

	const FGameplayAttribute DeveloperRangeBaseAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetRangeBaseAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeBaseAttackDamageAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage = AbilitySystemComponent->GetNumericAttribute(DeveloperRangeBaseAttackDamageAttribute);
	}

	const FGameplayAttribute DeveloperRangeChargedAttackDamageAttribute = UPlatformerCharacterAttributeSet::GetRangeChargedAttackDamageAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeChargedAttackDamageAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage = AbilitySystemComponent->GetNumericAttribute(DeveloperRangeChargedAttackDamageAttribute);
	}

	const FGameplayAttribute DeveloperRangeAttackDelayAttribute = UPlatformerCharacterAttributeSet::GetRangeAttackDelayAttribute();
	if (AbilitySystemComponent->HasAttributeSetForAttribute(DeveloperRangeAttackDelayAttribute))
	{
		DeveloperCombatSettings.DeveloperCombatRangeAttackDelay = AbilitySystemComponent->GetNumericAttribute(DeveloperRangeAttackDelayAttribute);
	}

	return DeveloperCombatSettings;
}

FDeveloperPlatformerCombatSettings APlatformerCharacterBase::ResolveDeveloperCombatSettingsForApplication(
	const FDeveloperPlatformerCombatSettings& DeveloperCombatSettings) const
{
	FDeveloperPlatformerCombatSettings ResolvedDeveloperCombatSettings = DeveloperCombatSettings;
	const bool bUsesLegacyCombatDefaults =
		FMath::IsNearlyEqual(ResolvedDeveloperCombatSettings.DeveloperCombatMaxHealth, 10.0f)
		&& FMath::IsNearlyEqual(ResolvedDeveloperCombatSettings.DeveloperCombatCurrentHealth, 10.0f)
		&& FMath::IsNearlyEqual(ResolvedDeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage, 1.0f)
		&& FMath::IsNearlyEqual(ResolvedDeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage, 1.0f);

	if (bUsesLegacyCombatDefaults)
	{
		ResolvedDeveloperCombatSettings.DeveloperCombatMaxHealth = 100.0f;
		ResolvedDeveloperCombatSettings.DeveloperCombatCurrentHealth = 100.0f;
		ResolvedDeveloperCombatSettings.DeveloperCombatRangeBaseAttackDamage = 25.0f;
		ResolvedDeveloperCombatSettings.DeveloperCombatRangeChargedAttackDamage = 75.0f;
	}

	return ResolvedDeveloperCombatSettings;
}

void APlatformerCharacterBase::LoadAndApplyDeveloperSettings()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPlatformerDeveloperSettingsSubsystem* DeveloperSettingsSubsystem =
			GameInstance->GetSubsystem<UPlatformerDeveloperSettingsSubsystem>())
		{
			FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot;
			if (DeveloperSettingsSubsystem->TryLoadCurrentSnapshot(DeveloperSettingsSnapshot))
			{
				ApplyDeveloperSettingsSnapshot(DeveloperSettingsSnapshot);
				return;
			}
		}
	}

	SetHasActiveDeveloperCombatSettings(false);
	ActiveDeveloperCombatSettings = FDeveloperPlatformerCombatSettings();
}

void APlatformerCharacterBase::SetHasActiveDeveloperCombatSettings(bool bInHasActiveDeveloperCombatSettings)
{
	bHasActiveDeveloperCombatSettings = bInHasActiveDeveloperCombatSettings;
}

bool APlatformerCharacterBase::HasActiveDeveloperCombatSettings() const
{
	return bHasActiveDeveloperCombatSettings;
}

const FDeveloperPlatformerCombatSettings& APlatformerCharacterBase::GetActiveDeveloperCombatSettings() const
{
	return ActiveDeveloperCombatSettings;
}

float APlatformerCharacterBase::GetHealthWidgetVerticalPadding() const
{
	return PlatformerHealthWidgetVerticalPadding;
}
