#include "PlayableDragonCharacter.h"
#include "Character/SideViewMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayableDragonCharacter::APlayableDragonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
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

void APlayableDragonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetCharacterMovement())
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
			if (DashAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(DashAbilityClass, 1, INDEX_NONE, this));
			if (JumpAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(JumpAbilityClass, 1, INDEX_NONE, this));
			if (BaseShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(BaseShotAbilityClass, 1, INDEX_NONE, this));
			if (ChargeShotAbilityClass) ASC->GiveAbility(FGameplayAbilitySpec(ChargeShotAbilityClass, 1, INDEX_NONE, this));
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
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_Move);
		if (JumpAction) {
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_JumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayableDragonCharacter::Input_JumpEnd);
		}
		if (DashAction) EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayableDragonCharacter::Input_Dash);
		
		if (BaseShotAction)
		{
			// Используем Triggered вместо Started, чтобы триггер "Tap" из Blueprint мог корректно отсеивать долгие нажатия
			EnhancedInputComponent->BindAction(BaseShotAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_BaseShot);
		}
		
		if (ChargeShotAction)
		{
			// Triggered сработает для "HoldAndRelease" при отпускании, если время выдержано
			EnhancedInputComponent->BindAction(ChargeShotAction, ETriggerEvent::Triggered, this, &APlayableDragonCharacter::Input_ChargeShotStart);
			// Нам больше не нужны Completed/Canceled для C++, так как тайминг обрабатывает сам Enhanced Input (HoldAndRelease)
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
		float MoveValue = Value.Get<float>();
		
		// In side scrolling, movement is typically along the X axis
		const FVector MoveDir = FVector(1.0f, 0.0f, 0.0f);
		AddMovementInput(MoveDir, MoveValue);

		// If flying, also allow basic vertical movement based on some context, 
		// but since we only have left/right bound to Move, we might need a 2D vector for real flying.
		// For this test, we map simple WASD where Jump/Glide might handle Z, or just rely on the 2D MoveAction.
		if (bIsFlying && Value.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D MoveVector = Value.Get<FVector2D>();
			AddMovementInput(FVector(0.0f, 0.0f, 1.0f), MoveVector.Y);
		}
	}
}

void APlayableDragonCharacter::Input_JumpStart(const FInputActionValue& Value)
{
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
	// Tell GAS that the jump key was released so UGA_Jump can call EndAbility()
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
		if (DashAbilityClass)
		{
			ASC->TryActivateAbilityByClass(DashAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_BaseShot(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (BaseShotAbilityClass)
		{
			ASC->TryActivateAbilityByClass(BaseShotAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_ChargeShotStart(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ChargeShotAbilityClass)
		{
			ASC->TryActivateAbilityByClass(ChargeShotAbilityClass);
		}
	}
}

void APlayableDragonCharacter::Input_ChargeShotEnd(const FInputActionValue& Value)
{
	// Tell GA_DragonChargeShot to release and fire
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ChargeShotAbilityClass)
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(ChargeShotAbilityClass))
			{
				ASC->AbilitySpecInputReleased(*Spec);
			}
		}
	}
}

void APlayableDragonCharacter::Input_FlyToggle(const FInputActionValue& Value)
{
	if (!bIsFlying)
	{
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
	if (GetCharacterMovement()->IsFalling() && !bIsFlying)
	{
		bIsGliding = true;
		GetCharacterMovement()->GravityScale = DefaultGravityScale * 0.1f;
	}
}

void APlayableDragonCharacter::Input_GlideEnd(const FInputActionValue& Value)
{
	bIsGliding = false;
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
}
