#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerSurfaceBlockBase.h"
#include "PlatformerConveyor.generated.h"

class UMaterialInstanceDynamic;

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerConveyor : public APlatformerSurfaceBlockBase
{
	GENERATED_BODY()

public:
	APlatformerConveyor();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void RefreshBlockLayout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Conveyor", meta=(Units="cm/s"))
	FVector LocalConveyorVelocity = FVector(250.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Conveyor")
	bool bAffectCharactersInAir = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicConveyorMaterial;

	float GetMaterialDirectionValue() const;
	void RefreshConveyorMaterialDirection();
};
