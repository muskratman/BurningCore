// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/PlatformerPlayerControllerBase.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SVirtualJoystick.h"

void APlatformerPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!ShouldUseTouchControls() || !IsLocalPlayerController() || !MobileControlsWidgetClass)
	{
		return;
	}

	MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
	if (!MobileControlsWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not spawn mobile controls widget."));
		return;
	}

	MobileControlsWidget->AddToPlayerScreen(0);
}

void APlatformerPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				if (CurrentContext)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}

			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					if (CurrentContext)
					{
						Subsystem->AddMappingContext(CurrentContext, 0);
					}
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (TogglePauseAction)
		{
			EnhancedInputComponent->BindAction(TogglePauseAction, ETriggerEvent::Started, this, &APlatformerPlayerControllerBase::OnTogglePause);
		}
	}
}

void APlatformerPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		InPawn->OnDestroyed.AddUniqueDynamic(this, &APlatformerPlayerControllerBase::OnPawnDestroyed);
	}
}

void APlatformerPlayerControllerBase::OnPawnDestroyed(AActor* DestroyedActor)
{
	FTransform SpawnTransform;
	if (!TryGetRespawnTransform(SpawnTransform))
	{
		return;
	}

	if (APawn* RespawnedPawn = SpawnRespawnPawn(SpawnTransform))
	{
		Possess(RespawnedPawn);
	}
}

bool APlatformerPlayerControllerBase::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

bool APlatformerPlayerControllerBase::TryGetRespawnTransform(FTransform& OutSpawnTransform) const
{
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() == 0 || !ActorList[0])
	{
		return false;
	}

	OutSpawnTransform = ActorList[0]->GetActorTransform();
	return true;
}

APawn* APlatformerPlayerControllerBase::SpawnRespawnPawn(const FTransform& SpawnTransform)
{
	if (!RespawnPawnClass || !GetWorld())
	{
		return nullptr;
	}

	return GetWorld()->SpawnActor<APawn>(RespawnPawnClass, SpawnTransform);
}

void APlatformerPlayerControllerBase::HandlePauseRequested()
{
}

void APlatformerPlayerControllerBase::OnTogglePause(const FInputActionValue& Value)
{
	HandlePauseRequested();
}
