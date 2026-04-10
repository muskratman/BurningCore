#include "Data/DragonFormDataAsset.h"

FPlatformerChargeShotTuning UDragonFormDataAsset::GetResolvedChargeShotTuning() const
{
	FPlatformerChargeShotTuning ResolvedTuning = ChargeShotTuning;
	ResolvedTuning.FullChargeTime = ResolvedTuning.FullChargeTime > 0.0f
		? ResolvedTuning.FullChargeTime
		: FMath::Max(ChargeTime, 0.0f);

	if (ResolvedTuning.PartialChargeTime <= 0.0f)
	{
		ResolvedTuning.PartialChargeTime = ResolvedTuning.FullChargeTime * 0.5f;
	}

	ResolvedTuning.PartialChargeTime = FMath::Clamp(
		ResolvedTuning.PartialChargeTime,
		0.0f,
		ResolvedTuning.FullChargeTime);
	ResolvedTuning.PartialDamageMultiplier = FMath::Max(ResolvedTuning.PartialDamageMultiplier, 1.0f);
	ResolvedTuning.PartialProjectileSpeedMultiplier = FMath::Max(ResolvedTuning.PartialProjectileSpeedMultiplier, 0.0f);
	ResolvedTuning.FullProjectileSpeedMultiplier = FMath::Max(ResolvedTuning.FullProjectileSpeedMultiplier, 0.0f);
	return ResolvedTuning;
}
