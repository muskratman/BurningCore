#include "PlayableDragonCharacter.h"

#include "Character/DragonFormComponent.h"
#include "GAS/Abilities/GA_PlatformerDash.h"
#include "GAS/Abilities/GA_PlatformerSlideDash.h"
#include "GAS/Abilities/GA_DragonBaseShot.h"
#include "GAS/Abilities/GA_DragonChargeShot.h"
#include "Character/SideViewMovementComponent.h"
#include "Traversal/PlatformerTraversalComponent.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayableDragonCharacter::APlayableDragonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlatformerTraversalMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	TraversalComponent = CreateDefaultSubobject<UPlatformerTraversalComponent>(TEXT("TraversalComponent"));
	bIsGliding = false;
	bIsFlying = false;
	DefaultGravityScale = 2.0f; // Typical platformer gravity fallback

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
}

USideViewMovementComponent* APlayableDragonCharacter::GetSideViewMovementComponent() const
{
	return Cast<USideViewMovementComponent>(GetCharacterMovement());
}

UDragonFormComponent* APlayableDragonCharacter::GetDragonFormComponent() const
{
	return GetFormComponent();
}

UPlatformerTraversalComponent* APlayableDragonCharacter::GetTraversalComponent() const
{
	return TraversalComponent;
}

UPlatformerTraversalMovementComponent* APlayableDragonCharacter::GetTraversalMovementComponent() const
{
	return Cast<UPlatformerTraversalMovementComponent>(GetCharacterMovement());
}

TSubclassOf<UGameplayAbility> APlayableDragonCharacter::ResolveDashAbilityClass() const
{
	TSubclassOf<UGameplayAbility> ResolvedDashAbilityClass = DashAbilityClass;
	if (!ResolvedDashAbilityClass || ResolvedDashAbilityClass == UGA_PlatformerDash::StaticClass())
	{
		ResolvedDashAbilityClass = UGA_PlatformerSlideDash::StaticClass();
	}

	return ResolvedDashAbilityClass;
}

TSubclassOf<UGameplayAbility> APlayableDragonCharacter::ResolveBaseShotAbilityClass() const
{
	TSubclassOf<UGameplayAbility> ResolvedBaseShotAbilityClass = BaseShotAbilityClass;
	if (!ResolvedBaseShotAbilityClass)
	{
		ResolvedBaseShotAbilityClass = UGA_DragonBaseShot::StaticClass();
	}

	return ResolvedBaseShotAbilityClass;
}

TSubclassOf<UGameplayAbility> APlayableDragonCharacter::ResolveChargeShotAbilityClass() const
{
	TSubclassOf<UGameplayAbility> ResolvedChargeShotAbilityClass = ChargeShotAbilityClass;
	if (!ResolvedChargeShotAbilityClass)
	{
		ResolvedChargeShotAbilityClass = UGA_DragonChargeShot::StaticClass();
	}

	return ResolvedChargeShotAbilityClass;
}

void APlayableDragonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UPlatformerTraversalComponent* CharacterTraversalComponent = GetTraversalComponent())
	{
		if (!CharacterTraversalComponent->GetTraversalConfig() && TraversalConfig)
		{
			CharacterTraversalComponent->SetTraversalConfig(TraversalConfig);
		}

		CharacterTraversalComponent->ApplyTraversalSettings();
	}

	if (!GetTraversalMovementComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayableDragonCharacter requires UPlatformerTraversalMovementComponent. Reload or recompile BP_DragonCharacter so the redirected movement component class is applied."));
	}
	
	if (const USideViewMovementComponent* SideViewMovementComponent = GetSideViewMovementComponent())
	{
		DefaultGravityScale = SideViewMovementComponent->GetBaseGravityScale();
	}
	else if (GetCharacterMovement())
	{
		DefaultGravityScale = GetCharacterMovement()->GravityScale;
	}
}

void APlayableDragonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	
	// Grant temporary abilities for testing abilities
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (HasAuthority())
		{
			const TSubclassOf<UGameplayAbility> ResolvedDashAbilityClass = ResolveDashAbilityClass();
			const TSubclassOf<UGameplayAbility> ResolvedBaseShotAbilityClass = ResolveBaseShotAbilityClass();
			const TSubclassOf<UGameplayAbility> ResolvedChargeShotAbilityClass = ResolveChargeShotAbilityClass();

			if (ResolvedDashAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedDashAbilityClass, 1, INDEX_NONE, this));
			if (JumpAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(JumpAbilityClass, 1, INDEX_NONE, this));
			if (CrouchAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(CrouchAbilityClass, 1, INDEX_NONE, this));
			if (ResolvedBaseShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedBaseShotAbilityClass, 1, INDEX_NONE, this));
			if (ResolvedChargeShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedChargeShotAbilityClass, 1, INDEX_NONE, this));
		}
	}
}

void APlayableDragonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APlayableDragonCharacter::Input_Move);
		}
		if (JumpAction) {
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_JumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_JumpEnd);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APlayableDragonCharacter::Input_JumpEnd);
		}
		if (DashAction) EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_Dash);
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_CrouchStart);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_CrouchEnd);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &APlayableDragonCharacter::Input_CrouchEnd);
		}
		
		if (BaseShotAction && (!bUseUnifiedShotInput || !ChargeShotAction))
		{
			EnhancedInputComponent->BindAction(BaseShotAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_BaseShot);
		}
		
		if (ChargeShotAction)
		{
			EnhancedInputComponent->BindAction(ChargeShotAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_ChargeShotStart);
			EnhancedInputComponent->BindAction(ChargeShotAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_ChargeShotStart);
			EnhancedInputComponent->BindAction(ChargeShotAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_ChargeShotEnd);
			EnhancedInputComponent->BindAction(ChargeShotAction, ETriggerEvent::Canceled, this, &APlayableDragonCharacter::Input_ChargeShotEnd);
		}

		if (FlyAction) EnhancedInputComponent->BindAction(FlyAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_FlyToggle);
		if (GlideAction) {
			EnhancedInputComponent->BindAction(GlideAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_GlideStart);
			EnhancedInputComponent->BindAction(GlideAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_GlideEnd);
		}
	}
}

void APlayableDragonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsOnLadder())
	{
		if (bLadderClimbUpHeld && !bLadderClimbDownHeld)
		{
			AddMovementInput(FVector::UpVector, 1.0f);
		}
		else if (bLadderClimbDownHeld && !bLadderClimbUpHeld)
		{
			AddMovementInput(FVector::DownVector, 1.0f);
		}
		else if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			FVector Velocity = MovementComponent->Velocity;
			Velocity.Z = 0.0f;
			MovementComponent->Velocity = Velocity;
		}
	}

	// Cap falling speed if gliding
	if (bIsGliding && GetCharacterMovement()->Velocity.Z < -200.0f)
	{
		GetCharacterMovement()->Velocity.Z = -200.0f;
	}
}

void APlayableDragonCharacter::ApplyDeveloperCharacterSettings(const FDeveloperPlatformerCharacterSettings& DeveloperSettings)
{
	Super::ApplyDeveloperCharacterSettings(DeveloperSettings);
	ApplyDeveloperChargeShotSettings(DeveloperSettings.DeveloperChargeShotSettings, true);
	ApplyDeveloperTraversalSettings(
		DeveloperSettings.DeveloperLedgeSettings,
		DeveloperSettings.DeveloperSlideDashSettings,
		DeveloperSettings.DeveloperWallSettings,
		true);
}

void APlayableDragonCharacter::ApplyDeveloperSettingsSnapshot(const FPlatformerDeveloperSettingsSnapshot& DeveloperSettingsSnapshot)
{
	Super::ApplyDeveloperSettingsSnapshot(DeveloperSettingsSnapshot);
	ApplyDeveloperChargeShotSettings(
		DeveloperSettingsSnapshot.CharacterSettings.DeveloperChargeShotSettings,
		DeveloperSettingsSnapshot.bHasSavedChargeShotSettings);
	ApplyDeveloperTraversalSettings(
		DeveloperSettingsSnapshot.CharacterSettings.DeveloperLedgeSettings,
		DeveloperSettingsSnapshot.CharacterSettings.DeveloperSlideDashSettings,
		DeveloperSettingsSnapshot.CharacterSettings.DeveloperWallSettings,
		DeveloperSettingsSnapshot.bHasSavedTraversalSettings);
}

FDeveloperPlatformerCharacterSettings APlayableDragonCharacter::CaptureDeveloperCharacterSettings() const
{
	FDeveloperPlatformerCharacterSettings DeveloperSettings = Super::CaptureDeveloperCharacterSettings();

	if (const UDragonFormComponent* DragonFormComponent = GetDragonFormComponent())
	{
		FPlatformerChargeShotTuning ChargeShotTuning;
		if (DragonFormComponent->TryGetResolvedChargeShotTuning(ChargeShotTuning))
		{
			DeveloperSettings.DeveloperChargeShotSettings = ChargeShotTuning;
		}
	}

	if (const UPlatformerTraversalComponent* CharacterTraversalComponent = GetTraversalComponent())
	{
		DeveloperSettings.DeveloperLedgeSettings = CharacterTraversalComponent->GetResolvedLedgeSettings();
		DeveloperSettings.DeveloperSlideDashSettings = CharacterTraversalComponent->GetResolvedSlideDashSettings();
		DeveloperSettings.DeveloperWallSettings = CharacterTraversalComponent->GetResolvedWallSettings();
	}

	return DeveloperSettings;
}

FPlatformerDeveloperSettingsSnapshot APlayableDragonCharacter::CaptureDeveloperSettingsSnapshot() const
{
	FPlatformerDeveloperSettingsSnapshot DeveloperSettingsSnapshot = Super::CaptureDeveloperSettingsSnapshot();
	DeveloperSettingsSnapshot.CharacterSettings = CaptureDeveloperCharacterSettings();
	DeveloperSettingsSnapshot.bHasSavedChargeShotSettings = true;
	DeveloperSettingsSnapshot.bHasSavedTraversalSettings = true;
	return DeveloperSettingsSnapshot;
}

void APlayableDragonCharacter::ApplyDeveloperCharacterMovementSettings(const FDeveloperPlatformerCharacterMovementSettings& DeveloperCharacterMovementSettings)
{
	Super::ApplyDeveloperCharacterMovementSettings(DeveloperCharacterMovementSettings);

	if (const USideViewMovementComponent* SideViewMovementComponent = GetSideViewMovementComponent())
	{
		DefaultGravityScale = SideViewMovementComponent->GetBaseGravityScale();
	}
	else if (const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		DefaultGravityScale = MovementComponent->GravityScale;
	}

	PreGlideGravityOverride = DefaultGravityScale;
}

void APlayableDragonCharacter::ApplyDeveloperChargeShotSettings(
	const FPlatformerChargeShotTuning& DeveloperChargeShotSettings,
	bool bHasSavedChargeShotSettings)
{
	if (UDragonFormComponent* DragonFormComponent = GetDragonFormComponent())
	{
		if (bHasSavedChargeShotSettings)
		{
			DragonFormComponent->SetDeveloperChargeShotTuningOverride(DeveloperChargeShotSettings);
		}
		else
		{
			DragonFormComponent->ClearDeveloperChargeShotTuningOverride();
		}
	}
}

void APlayableDragonCharacter::ApplyDeveloperTraversalSettings(
	const FPlatformerLedgeTraversalSettings& DeveloperLedgeSettings,
	const FPlatformerSlideDashSettings& DeveloperSlideDashSettings,
	const FPlatformerWallTraversalSettings& DeveloperWallSettings,
	bool bHasSavedTraversalSettings)
{
	if (UPlatformerTraversalComponent* CharacterTraversalComponent = GetTraversalComponent())
	{
		if (bHasSavedTraversalSettings)
		{
			CharacterTraversalComponent->SetDeveloperTraversalSettingsOverride(
				DeveloperLedgeSettings,
				DeveloperSlideDashSettings,
				DeveloperWallSettings);
		}
		else
		{
			CharacterTraversalComponent->ClearDeveloperTraversalSettingsOverride();
		}
	}
}

void APlayableDragonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	// Reset states on land
	if (bIsGliding)
	{
		Input_GlideEnd(FInputActionValue());
	}
	if (bIsFlying)
	{
		Input_FlyToggle(FInputActionValue());
	}
}

void APlayableDragonCharacter::Input_Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		float MoveValue = 0.0f;
		FVector2D MoveVector = FVector2D::ZeroVector;

		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			MoveVector = Value.Get<FVector2D>();
			MoveValue = MoveVector.X;
		}
		else
		{
			MoveValue = Value.Get<float>();
			MoveVector.X = MoveValue;
		}

		if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
		{
			TraversalMovementComponent->SetTraversalInputVector(MoveVector);

			if (TraversalMovementComponent->IsInCustomTraversalMode())
			{
				return;
			}
		}

		if (IsOnLadder())
		{
			if (!FMath::IsNearlyZero(MoveValue))
			{
				AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MoveValue);
			}

			return;
		}
		
		// In side scrolling, movement is typically along the X axis
		const FVector MoveDir = FVector(1.0f, 0.0f, 0.0f);
		AddMovementInput(MoveDir, MoveValue);

		// If flying, also allow basic vertical movement based on some context, 
		// but since we only have left/right bound to Move, we might need a 2D vector for real flying.
		// For this test, we map simple WASD where Jump/Glide might handle Z, or just rely on the 2D MoveAction.
		if (GetCharacterMovement()->IsFlying() && Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			AddMovementInput(FVector(0.0f, 0.0f, 1.0f), MoveVector.Y);
		}
	}
}

void APlayableDragonCharacter::Input_JumpStart(const FInputActionValue& Value)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (TraversalMovementComponent->HandleTraversalJumpPressed())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
		bLadderClimbUpHeld = true;
		return;
	}

	if (APlatformerLadder* CandidateLadder = GetAvailableLadder())
	{
		if (EnterLadder(CandidateLadder))
		{
			bLadderClimbUpHeld = true;
			return;
		}
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		// Try to activate Jump Ability if granted
		if (JumpAbilityClass)
		{
			ASC->TryActivateAbilityByClass(JumpAbilityClass);
		}
		else
		{
			// Fallback to native jump
			Jump();
		}
	}
}

void APlayableDragonCharacter::Input_JumpEnd(const FInputActionValue& Value)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		TraversalMovementComponent->HandleTraversalJumpReleased();
		if (TraversalMovementComponent->IsInCustomTraversalMode())
		{
			bLadderClimbUpHeld = false;
			return;
		}
	}

	if (IsOnLadder() || bLadderClimbUpHeld)
	{
		bLadderClimbUpHeld = false;
		return;
	}

	// Tell GAS that the jump key was released so UGA_PlatformerJump can call EndAbility()
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (JumpAbilityClass)
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(JumpAbilityClass))
			{
				ASC->AbilitySpecInputReleased(*Spec);
			}
		}
	}

	StopJumping();
}

void APlayableDragonCharacter::Input_Dash(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (const TSubclassOf<UGameplayAbility> ResolvedDashAbilityClass = ResolveDashAbilityClass())
		{
			ASC->TryActivateAbilityByClass(ResolvedDashAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_CrouchStart(const FInputActionValue& Value)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (TraversalMovementComponent->HandleTraversalCrouchPressed())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
		bLadderClimbDownHeld = true;
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (CrouchAbilityClass)
		{
			ASC->TryActivateAbilityByClass(CrouchAbilityClass);
			return;
		}
	}

	Crouch();
}

void APlayableDragonCharacter::Input_CrouchEnd(const FInputActionValue& Value)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		TraversalMovementComponent->HandleTraversalCrouchReleased();
		if (TraversalMovementComponent->IsInCustomTraversalMode())
		{
			bLadderClimbDownHeld = false;
			return;
		}
	}

	if (IsOnLadder() || bLadderClimbDownHeld)
	{
		bLadderClimbDownHeld = false;
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (CrouchAbilityClass)
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(CrouchAbilityClass))
			{
				ASC->AbilitySpecInputReleased(*Spec);
			}
			return;
		}
	}

	UnCrouch();
}

void APlayableDragonCharacter::Input_BaseShot(const FInputActionValue& Value)
{
	if (bUseUnifiedShotInput && ChargeShotAction)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (const TSubclassOf<UGameplayAbility> ResolvedBaseShotAbilityClass = ResolveBaseShotAbilityClass())
		{
			ASC->TryActivateAbilityByClass(ResolvedBaseShotAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_ChargeShotStart(const FInputActionValue& Value)
{
	if (bChargeShotInputHeld)
	{
		return;
	}

	bChargeShotInputHeld = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (const TSubclassOf<UGameplayAbility> ResolvedChargeShotAbilityClass = ResolveChargeShotAbilityClass())
		{
			ASC->TryActivateAbilityByClass(ResolvedChargeShotAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_ChargeShotEnd(const FInputActionValue& Value)
{
	if (!bChargeShotInputHeld)
	{
		return;
	}

	bChargeShotInputHeld = false;

	// Tell GA_DragonChargeShot to release and fire
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (const TSubclassOf<UGameplayAbility> ResolvedChargeShotAbilityClass = ResolveChargeShotAbilityClass())
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(ResolvedChargeShotAbilityClass))
			{
				ASC->AbilitySpecInputReleased(*Spec);
			}
		}
	}
}

void APlayableDragonCharacter::Input_FlyToggle(const FInputActionValue& Value)
{
	if (const UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (TraversalMovementComponent->IsTraversalStateActive())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
		return;
	}

	if (!bIsFlying)
	{
		if (bIsGliding)
		{
			Input_GlideEnd(FInputActionValue());
		}

		bIsFlying = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
	else
	{
		bIsFlying = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void APlayableDragonCharacter::Input_GlideStart(const FInputActionValue& Value)
{
	if (const UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (TraversalMovementComponent->IsTraversalStateActive())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
		return;
	}

	if (GetCharacterMovement()->IsFalling() && !bIsFlying && !bIsGliding)
	{
		bIsGliding = true;

		if (USideViewMovementComponent* SideViewMovementComponent = GetSideViewMovementComponent())
		{
			bHadPreGlideGravityOverride = SideViewMovementComponent->HasExternalGravityScaleOverride();
			PreGlideGravityOverride = SideViewMovementComponent->GetExternalGravityScaleOverride();
			SideViewMovementComponent->SetExternalGravityScaleOverride(DefaultGravityScale * 0.1f);
		}
		else if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->GravityScale = DefaultGravityScale * 0.1f;
		}
	}
}

void APlayableDragonCharacter::Input_GlideEnd(const FInputActionValue& Value)
{
	if (!bIsGliding)
	{
		return;
	}

	bIsGliding = false;

	if (USideViewMovementComponent* SideViewMovementComponent = GetSideViewMovementComponent())
	{
		if (bHadPreGlideGravityOverride)
		{
			SideViewMovementComponent->SetExternalGravityScaleOverride(PreGlideGravityOverride);
		}
		else
		{
			SideViewMovementComponent->ClearExternalGravityScaleOverride();
		}
	}
	else if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GravityScale = DefaultGravityScale;
	}

	bHadPreGlideGravityOverride = false;
	PreGlideGravityOverride = DefaultGravityScale;
}

void APlayableDragonCharacter::OnEnteredLadder(APlatformerLadder* Ladder)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		TraversalMovementComponent->CancelTraversal();
	}

	if (bIsGliding)
	{
		Input_GlideEnd(FInputActionValue());
	}

	// Ladder climb uses its own movement state and should not inherit the manual fly toggle flag.
	bIsFlying = false;
	bLadderClimbDownHeld = false;
}

void APlayableDragonCharacter::OnExitedLadder(APlatformerLadder* Ladder)
{
	bLadderClimbUpHeld = false;
	bLadderClimbDownHeld = false;
}
