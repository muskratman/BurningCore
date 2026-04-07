#include "Platformer/Environment/PlatformerSlipperyBlock.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

APlatformerSlipperyBlock::APlatformerSlipperyBlock()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerSlipperyBlock.PlatformerSlipperyBlock")));
}

void APlatformerSlipperyBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClearInvalidOverlappingCharacters();

	TArray<TWeakObjectPtr<ACharacter>> CharactersToRestore;
	CharactersToRestore.Reserve(CachedMovementStates.Num());

	for (const TPair<TWeakObjectPtr<ACharacter>, FPlatformerSlipperyMovementState>& Pair : CachedMovementStates)
	{
		ACharacter* Character = Pair.Key.Get();
		if ((Character == nullptr) || !GetOverlappingCharacters().Contains(Character) || !IsCharacterStandingOnSurface(Character))
		{
			CharactersToRestore.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : CharactersToRestore)
	{
		if (CharacterPtr.IsValid())
		{
			RestoreCharacter(CharacterPtr.Get());
		}
	}

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : GetOverlappingCharacters())
	{
		ACharacter* Character = CharacterPtr.Get();
		if ((Character != nullptr) && !CachedMovementStates.Contains(Character) && IsCharacterStandingOnSurface(Character))
		{
			HandleCharacterEnteredSurface(Character);
		}
	}
}

void APlatformerSlipperyBlock::HandleCharacterEnteredSurface(ACharacter* Character)
{
	if ((Character == nullptr) || CachedMovementStates.Contains(Character))
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		FPlatformerSlipperyMovementState SavedState;
		SavedState.GroundFriction = MovementComponent->GroundFriction;
		SavedState.BrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
		SavedState.BrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		CachedMovementStates.Add(Character, SavedState);

		MovementComponent->GroundFriction = GroundFrictionOverride;
		MovementComponent->BrakingFrictionFactor = BrakingFrictionFactorOverride;
		MovementComponent->BrakingDecelerationWalking = BrakingDecelerationWalkingOverride;
	}
}

void APlatformerSlipperyBlock::HandleCharacterLeftSurface(ACharacter* Character)
{
	RestoreCharacter(Character);
}

void APlatformerSlipperyBlock::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (const TPair<TWeakObjectPtr<ACharacter>, FPlatformerSlipperyMovementState>& Pair : CachedMovementStates)
	{
		if (Pair.Key.IsValid())
		{
			RestoreCharacter(Pair.Key.Get());
		}
	}

	CachedMovementStates.Empty();
	Super::EndPlay(EndPlayReason);
}

void APlatformerSlipperyBlock::RestoreCharacter(ACharacter* Character)
{
	if (Character == nullptr)
	{
		return;
	}

	const FPlatformerSlipperyMovementState* SavedState = CachedMovementStates.Find(Character);
	if (SavedState == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->GroundFriction = SavedState->GroundFriction;
		MovementComponent->BrakingFrictionFactor = SavedState->BrakingFrictionFactor;
		MovementComponent->BrakingDecelerationWalking = SavedState->BrakingDecelerationWalking;
	}

	CachedMovementStates.Remove(Character);
}
