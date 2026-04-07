#pragma once

#include "CoreMinimal.h"
#include "PlatformerComponentTransformOverride.generated.h"

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerComponentTransformOffset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Transform")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Transform")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Transform")
	FVector RelativeScale3D = FVector::OneVector;
};
