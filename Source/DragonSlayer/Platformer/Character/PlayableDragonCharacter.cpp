#include "PlayableDragonCharacter.h"

#include "Character/DragonFormComponent.h"
#include "GAS/Abilities/GA_PlatformerDash.h"
#include "GAS/Abilities/GA_DragonBaseShot.h"
#include "GAS/Abilities/GA_DragonChargeShot.h"
#include "GAS/Abilities/GA_HitReaction.h"
#include "Character/SideViewMovementComponent.h"
#include "Traversal/PlatformerTraversalComponent.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Platformer/Environment/PlatformerDropThroughPlatformComponent.h"
#include "Platformer/Environment/PlatformerSoftPlatform.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float CrouchInputThreshold = -0.5f;
	const TCHAR* DefaultLookActionPath = TEXT("/Game/Blueprints/Input/Actions/IA_Look.IA_Look");

	float ReadVerticalInputValue(const FInputActionValue& Value)
	{
		if (Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			return Value.Get<FVector2D>().Y;
		}

		if (Value.GetValueType() == EInputActionValueType::Axis1D)
		{
			return Value.Get<float>();
		}

		if (Value.GetValueType() == EInputActionValueType::Boolean)
		{
			return Value.Get<bool>() ? 1.0f : 0.0f;
		}

		return 0.0f;
	}
}

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
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(DefaultLookActionPath);
	if (LookActionAsset.Succeeded())
	{
		LookAction = LookActionAsset.Object;
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
	if (!ResolvedDashAbilityClass)
	{
		ResolvedDashAbilityClass = UGA_PlatformerDash::StaticClass();
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

TSubclassOf<UGameplayAbility> APlayableDragonCharacter::ResolveHitReactionAbilityClass() const
{
	TSubclassOf<UGameplayAbility> ResolvedHitReactionAbilityClass = HitReactionAbilityClass;
	if (!ResolvedHitReactionAbilityClass)
	{
		ResolvedHitReactionAbilityClass = UGA_HitReaction::StaticClass();
	}

	return ResolvedHitReactionAbilityClass;
}

void APlayableDragonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

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
			const TSubclassOf<UGameplayAbility> ResolvedHitReactionAbilityClass = ResolveHitReactionAbilityClass();

			if (ResolvedDashAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedDashAbilityClass, 1, INDEX_NONE, this));
			if (JumpAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(JumpAbilityClass, 1, INDEX_NONE, this));
			if (CrouchAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(CrouchAbilityClass, 1, INDEX_NONE, this));
			if (ResolvedBaseShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedBaseShotAbilityClass, 1, INDEX_NONE, this));
			if (ResolvedChargeShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedChargeShotAbilityClass, 1, INDEX_NONE, this));
			if (ResolvedHitReactionAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ResolvedHitReactionAbilityClass, 1, INDEX_NONE, this));
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
		if (UInputAction* ResolvedLookAction = ResolveLookAction())
		{
			EnhancedInputComponent->BindAction(ResolvedLookAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_Look);
			EnhancedInputComponent->BindAction(ResolvedLookAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_Look);
			EnhancedInputComponent->BindAction(ResolvedLookAction, ETriggerEvent::Canceled, this, &APlayableDragonCharacter::Input_Look);
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
		DeveloperSettings.DeveloperDashSettings,
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
		DeveloperSettingsSnapshot.CharacterSettings.DeveloperDashSettings,
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
		DeveloperSettings.DeveloperDashSettings = CharacterTraversalComponent->GetResolvedDashSettings();
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
	const FPlatformerDashSettings& DeveloperDashSettings,
	const FPlatformerWallTraversalSettings& DeveloperWallSettings,
	bool bHasSavedTraversalSettings)
{
	if (UPlatformerTraversalComponent* CharacterTraversalComponent = GetTraversalComponent())
	{
		if (bHasSavedTraversalSettings)
		{
			CharacterTraversalComponent->SetDeveloperTraversalSettingsOverride(
				DeveloperLedgeSettings,
				DeveloperDashSettings,
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

		if (IsLadderTopFinishActive())
		{
			if (Value.GetValueType() == EInputActionValueType::Axis2D)
			{
				HandleLadderClimbInput(MoveVector.Y, LadderLookInputThreshold);
			}
			return;
		}

		const bool bHas2DMoveInput = Value.GetValueType() == EInputActionValueType::Axis2D;
		const bool bExitedLadderHorizontally =
			bHas2DMoveInput && HandleLadderHorizontalExitInput(MoveVector.X, LadderLookInputThreshold);
		if (!bExitedLadderHorizontally && bHas2DMoveInput && HandleLadderClimbInput(MoveVector.Y, LadderLookInputThreshold))
		{
			return;
		}
		if (IsOnLadder())
		{
			SetLadderClimbInput(0.0f);
			return;
		}

		if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
		{
			TraversalMovementComponent->SetTraversalInputVector(MoveVector);

			if (TraversalMovementComponent->IsInCustomTraversalMode())
			{
				return;
			}
		}

		const FVector MoveDir = FVector(1.0f, 0.0f, 0.0f);
		AddMovementInput(MoveDir, MoveValue);

		if (GetCharacterMovement()->IsFlying() && bHas2DMoveInput)
		{
			AddMovementInput(FVector(0.0f, 0.0f, 1.0f), MoveVector.Y);
		}
		else if (bHas2DMoveInput)
		{
			if (MoveVector.Y < CrouchInputThreshold)
			{
				if (!bMoveCrouchHeld)
				{
					bMoveCrouchHeld = true;
					BeginCrouchRequest(Value);
				}
			}
			else if (bMoveCrouchHeld && MoveVector.IsNearlyZero())
			{
				bMoveCrouchHeld = false;
				if (!bCrouchActionHeld)
				{
					EndCrouchRequest(Value);
				}
			}
		}
	}
}

void APlayableDragonCharacter::Input_Look(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const float VerticalLookInput = ReadVerticalInputValue(Value);
	HandleLadderClimbInput(VerticalLookInput, LadderLookInputThreshold);
}

UInputAction* APlayableDragonCharacter::ResolveLookAction()
{
	if (!LookAction)
	{
		LookAction = LoadObject<UInputAction>(nullptr, DefaultLookActionPath);
	}

	return LookAction;
}

void APlayableDragonCharacter::Input_JumpStart(const FInputActionValue& Value)
{
	if (IsLadderTopFinishActive())
	{
		return;
	}

	if (IsOnLadder())
	{
		PerformLadderJump();
		return;
	}

	bool bCanceledDashForJump = false;

	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		const bool bWasDashing = TraversalMovementComponent->IsDashing();
		if (TraversalMovementComponent->HandleTraversalJumpPressed())
		{
			return;
		}

		bCanceledDashForJump = bWasDashing && !TraversalMovementComponent->IsDashing();
	}

	bool bIsHoldingDown = bIsCrouched || (GetTraversalMovementComponent() && GetTraversalMovementComponent()->GetTraversalInputVector().Y < CrouchInputThreshold);
	if (bIsHoldingDown)
	{
		if (UPrimitiveComponent* MovementBase = GetCharacterMovement()->GetMovementBase())
		{
			AActor* MovementBaseOwner = MovementBase->GetOwner();
			if (MovementBaseOwner)
			{
				if (UPlatformerDropThroughPlatformComponent* DropThroughPlatform =
					MovementBaseOwner->FindComponentByClass<UPlatformerDropThroughPlatformComponent>())
				{
					if (DropThroughPlatform->RequestCharacterDropThrough(this))
					{
						return;
					}
				}
			}

			if (APlatformerSoftPlatform* SoftPlatform = Cast<APlatformerSoftPlatform>(MovementBaseOwner))
			{
				if (SoftPlatform->RequestCharacterDropThrough(this))
				{
					return;
				}
			}
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
	if (IsLadderTopFinishActive())
	{
		return;
	}

	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		TraversalMovementComponent->HandleTraversalJumpReleased();
		if (TraversalMovementComponent->IsInCustomTraversalMode())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	if (IsLadderTopFinishActive())
	{
		return;
	}

	bCrouchActionHeld = true;
	BeginCrouchRequest(Value);
}

void APlayableDragonCharacter::Input_CrouchEnd(const FInputActionValue& Value)
{
	bCrouchActionHeld = false;
	if (IsLadderTopFinishActive())
	{
		return;
	}

	if (!bMoveCrouchHeld)
	{
		EndCrouchRequest(Value);
	}
}

void APlayableDragonCharacter::BeginCrouchRequest(const FInputActionValue& Value)
{
	if (IsLadderTopFinishActive())
	{
		return;
	}

	if (IsOnLadder())
	{
		PerformLadderCrouch();
		return;
	}

	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (TraversalMovementComponent->HandleTraversalCrouchPressed())
		{
			return;
		}
	}

	if (const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (!MovementComponent->IsMovingOnGround())
		{
			return;
		}
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

void APlayableDragonCharacter::EndCrouchRequest(const FInputActionValue& Value)
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		TraversalMovementComponent->HandleTraversalCrouchReleased();
		if (TraversalMovementComponent->IsInCustomTraversalMode())
		{
			return;
		}
	}

	if (IsOnLadder())
	{
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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	if (IsLadderTopFinishActive())
	{
		return;
	}

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
	SetLadderClimbInput(0.0f);
}

void APlayableDragonCharacter::OnExitedLadder(APlatformerLadder* Ladder)
{
	SetLadderClimbInput(0.0f);
}
