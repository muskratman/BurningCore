#include "Character/DragonFormComponent.h"
#include "Data/DragonFormDataAsset.h"
#include "BurningCORE.h"

UDragonFormComponent::UDragonFormComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default starting tags
	ActiveFormTag = FGameplayTag::RequestGameplayTag(FName("Form.Base"));
}

void UDragonFormComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Validate base form exists
	if (!FormRegistry.Contains(ActiveFormTag))
	{
		UE_LOG(LogDragon, Warning, TEXT("FormRegistry does not contain active form tag %s"), *ActiveFormTag.ToString());
	}
}

void UDragonFormComponent::SwitchForm(FGameplayTag NewFormTag)
{
	if (NewFormTag == ActiveFormTag) return;
	
	if (FormRegistry.Contains(NewFormTag))
	{
		const FGameplayTag OldForm = ActiveFormTag;
		ActiveFormTag = NewFormTag;
		
		OnFormChanged.Broadcast(OldForm, ActiveFormTag);
		UE_LOG(LogDragon, Log, TEXT("Switched form to %s"), *ActiveFormTag.ToString());
	}
}

const UDragonFormDataAsset* UDragonFormComponent::GetActiveFormData() const
{
	if (const TObjectPtr<UDragonFormDataAsset>* Data = FormRegistry.Find(ActiveFormTag))
	{
		return *Data;
	}
	return nullptr;
}
