#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"
#include "Character/DragonOverdriveComponent.h"
#include "GAS/Attributes/DragonAttributeSet.h"

ADragonCharacter::ADragonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDragonAttributeSet>(APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()))
{
	FormComponent = CreateDefaultSubobject<UDragonFormComponent>(TEXT("FormComponent"));
	OverdriveComponent = CreateDefaultSubobject<UDragonOverdriveComponent>(TEXT("OverdriveComponent"));
}

UDragonAttributeSet* ADragonCharacter::GetDragonAttributeSet() const
{
	return Cast<UDragonAttributeSet>(AttributeSet);
}
