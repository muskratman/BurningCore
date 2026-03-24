#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DragonFormComponent.generated.h"

class UDragonFormDataAsset;

/**
 * UDragonFormComponent
 * Manages the active form and handles form switching logic.
 */
UCLASS(ClassGroup="Dragon", meta=(BlueprintSpawnableComponent))
class BURNINGCORE_API UDragonFormComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDragonFormComponent();

	UFUNCTION(BlueprintCallable, Category="Form")
	void SwitchForm(FGameplayTag NewFormTag);

	UFUNCTION(BlueprintPure, Category="Form")
	const UDragonFormDataAsset* GetActiveFormData() const;

	UFUNCTION(BlueprintPure, Category="Form")
	FGameplayTag GetActiveFormTag() const { return ActiveFormTag; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFormChanged, FGameplayTag, OldForm, FGameplayTag, NewForm);
	
	UPROPERTY(BlueprintAssignable, Category="Form")
	FOnFormChanged OnFormChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Form")
	TMap<FGameplayTag, TObjectPtr<UDragonFormDataAsset>> FormRegistry;

	UPROPERTY(VisibleAnywhere, Category="Form")
	FGameplayTag ActiveFormTag;
};
