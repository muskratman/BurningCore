#include "Platformer/Environment/PlatformerConveyor.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName ConveyorDirectionParameterName(TEXT("Direction"));
	constexpr float ConveyorLeftToRightMaterialDirection = 0.75f;
	constexpr float ConveyorRightToLeftMaterialDirection = 0.25f;
}

APlatformerConveyor::APlatformerConveyor()
{
	PrimaryActorTick.bCanEverTick = true;
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerConveyor.PlatformerConveyor")));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ConveyorMaterial(TEXT("/CookieBrosPlatformer/Materials/MI_Conveyor.MI_Conveyor"));
	if (ConveyorMaterial.Succeeded())
	{
		BlockMesh->SetMaterial(0, ConveyorMaterial.Object);
	}
}

void APlatformerConveyor::BeginPlay()
{
	Super::BeginPlay();
	RefreshConveyorMaterialDirection();
}

void APlatformerConveyor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshConveyorMaterialDirection();
}

void APlatformerConveyor::RefreshBlockLayout()
{
	const FVector ResolvedBlockSize = BlockSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));

	PlatformerEnvironment::ApplyRelativeTransform(
		BlockMeshLayoutRoot,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		ResolvedBlockSize / 100.0f,
		BlockMeshTransformOffset);
}

float APlatformerConveyor::GetMaterialDirectionValue() const
{
	return LocalConveyorVelocity.X < 0.0f
		? ConveyorRightToLeftMaterialDirection
		: ConveyorLeftToRightMaterialDirection;
}

void APlatformerConveyor::RefreshConveyorMaterialDirection()
{
	if (BlockMesh == nullptr)
	{
		return;
	}

	if ((DynamicConveyorMaterial == nullptr) || (BlockMesh->GetMaterial(0) != DynamicConveyorMaterial))
	{
		DynamicConveyorMaterial = BlockMesh->CreateDynamicMaterialInstance(0);
	}

	if (DynamicConveyorMaterial != nullptr)
	{
		DynamicConveyorMaterial->SetScalarParameterValue(ConveyorDirectionParameterName, GetMaterialDirectionValue());
	}
}

void APlatformerConveyor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClearInvalidOverlappingCharacters();

	const float ConveyorSpeedX = LocalConveyorVelocity.X;
	if (FMath::IsNearlyZero(ConveyorSpeedX))
	{
		return;
	}

	const FVector WorldDelta(ConveyorSpeedX * DeltaTime, 0.0f, 0.0f);

	for (const TWeakObjectPtr<ACharacter>& CharacterPtr : GetOverlappingCharacters())
	{
		ACharacter* Character = CharacterPtr.Get();
		if (Character == nullptr)
		{
			continue;
		}

		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			const bool bStandingOnThisConveyor = IsCharacterStandingOnSurface(Character);
			if (MovementComponent->IsMovingOnGround() && !bStandingOnThisConveyor)
			{
				continue;
			}

			if (!bAffectCharactersInAir && !MovementComponent->IsMovingOnGround())
			{
				continue;
			}
		}

		Character->AddActorWorldOffset(WorldDelta, true);
	}
}
