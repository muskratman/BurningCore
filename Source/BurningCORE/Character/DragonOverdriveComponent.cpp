#include "Character/DragonOverdriveComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Attributes/DragonAttributeSet.h"

UDragonOverdriveComponent::UDragonOverdriveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsActive = false;
	ActivationThreshold = 100.0f;
	DrainRate = 10.0f;
}

void UDragonOverdriveComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* Owner = GetOwner())
	{
		CachedASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	}
}

void UDragonOverdriveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsActive && CachedASC)
	{
		if (CachedASC->HasAttributeSetForAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute()))
		{
			float CurrentEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute());
			float NewEnergy = FMath::Max(0.0f, CurrentEnergy - (DrainRate * DeltaTime));
			CachedASC->SetNumericAttributeBase(UDragonAttributeSet::GetOverdriveEnergyAttribute(), NewEnergy);
			
			if (NewEnergy <= 0.0f)
			{
				DeactivateOverdrive();
			}
		}
	}
}

void UDragonOverdriveComponent::AddOverdriveEnergy(float Amount)
{
	if (bIsActive || !CachedASC) return;
	
	if (CachedASC->HasAttributeSetForAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute()))
	{
		float CurrentEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute());
		float MaxEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetMaxOverdriveEnergyAttribute());
		CachedASC->SetNumericAttributeBase(UDragonAttributeSet::GetOverdriveEnergyAttribute(), FMath::Clamp(CurrentEnergy + Amount, 0.0f, MaxEnergy));
	}
}

bool UDragonOverdriveComponent::TryActivateOverdrive()
{
	if (bIsActive || !CachedASC) return false;
	
	if (CachedASC->HasAttributeSetForAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute()))
	{
		float CurrentEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute());
		if (CurrentEnergy >= ActivationThreshold)
		{
			bIsActive = true;
			OnOverdriveStateChanged.Broadcast(true);
			return true;
		}
	}
	
	return false;
}

void UDragonOverdriveComponent::DeactivateOverdrive()
{
	if (!bIsActive) return;
	
	bIsActive = false;
	
	if (CachedASC)
	{
		CachedASC->SetNumericAttributeBase(UDragonAttributeSet::GetOverdriveEnergyAttribute(), 0.0f);
	}
	
	OnOverdriveStateChanged.Broadcast(false);
}

bool UDragonOverdriveComponent::IsOverdriveActive() const
{
	return bIsActive;
}

float UDragonOverdriveComponent::GetOverdrivePercent() const
{
	if (!CachedASC) return 0.0f;
	
	if (!CachedASC->HasAttributeSetForAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute())) return 0.0f;
	
	float CurrentEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetOverdriveEnergyAttribute());
	float MaxEnergy = CachedASC->GetNumericAttribute(UDragonAttributeSet::GetMaxOverdriveEnergyAttribute());
	
	if (MaxEnergy <= 0.0f) return 0.0f;
	
	return FMath::Clamp(CurrentEnergy / MaxEnergy, 0.0f, 1.0f);
}
