// Copyright Epic Games, Inc. All Rights Reserved.

#include "CookieBrosPlatformer.h"
#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "FCookieBrosPlatformerModule"

namespace CookieBrosPlatformer::Redirects
{
	static const TArray<FCoreRedirect>& Get()
	{
		static const TArray<FCoreRedirect> Redirects = {
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.Damageable"), TEXT("/Script/CookieBrosPlatformer.Damageable") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.Interactable"), TEXT("/Script/CookieBrosPlatformer.Interactable") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.SideViewMovementComponent"), TEXT("/Script/CookieBrosPlatformer.SideViewMovementComponent") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.GA_Crouch"), TEXT("/Script/CookieBrosPlatformer.GA_Crouch") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.GA_Jump"), TEXT("/Script/CookieBrosPlatformer.GA_Jump") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.GA_Dash"), TEXT("/Script/CookieBrosPlatformer.GA_Dash") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.BaseProjectile"), TEXT("/Script/CookieBrosPlatformer.BaseProjectile") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.CombatProjectile"), TEXT("/Script/CookieBrosPlatformer.CombatProjectile") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.EnemyProjectile"), TEXT("/Script/CookieBrosPlatformer.EnemyProjectile") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerCameraManager"), TEXT("/Script/CookieBrosPlatformer.PlatformerCameraManager") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerPickup"), TEXT("/Script/CookieBrosPlatformer.PlatformerPickup") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerHazardProjectile"), TEXT("/Script/CookieBrosPlatformer.PlatformerHazardProjectile") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerSpikes"), TEXT("/Script/CookieBrosPlatformer.PlatformerSpikes") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerWallTurret"), TEXT("/Script/CookieBrosPlatformer.PlatformerWallTurret") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerInteractable"), TEXT("/Script/CookieBrosPlatformer.PlatformerInteractable") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerPointToPointMover"), TEXT("/Script/CookieBrosPlatformer.PlatformerMovingPlatform") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerMovingPlatform"), TEXT("/Script/CookieBrosPlatformer.PlatformerMovingPlatform") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerGravityVolume"), TEXT("/Script/CookieBrosPlatformer.PlatformerGravityVolume") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerJumpPad"), TEXT("/Script/CookieBrosPlatformer.PlatformerJumpPad") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerLadder"), TEXT("/Script/CookieBrosPlatformer.PlatformerLadder") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerPatrolPlatform"), TEXT("/Script/CookieBrosPlatformer.PlatformerMovingPlatform") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerRamp"), TEXT("/Script/CookieBrosPlatformer.PlatformerRamp") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerSoftPlatform"), TEXT("/Script/CookieBrosPlatformer.PlatformerSoftPlatform") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerTeleporter"), TEXT("/Script/CookieBrosPlatformer.PlatformerTeleporter") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerTriggeredLift"), TEXT("/Script/CookieBrosPlatformer.PlatformerTriggeredLift") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/BurningCORE.PlatformerVanishingBlock"), TEXT("/Script/CookieBrosPlatformer.PlatformerVanishingBlock") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/CookieBrosPlatformer.PlatformerPointToPointMover"), TEXT("/Script/CookieBrosPlatformer.PlatformerMovingPlatform") },
			{ ECoreRedirectFlags::Type_Class, TEXT("/Script/CookieBrosPlatformer.PlatformerPatrolPlatform"), TEXT("/Script/CookieBrosPlatformer.PlatformerMovingPlatform") }
		};

		return Redirects;
	}
}

void FCookieBrosPlatformerModule::StartupModule()
{
	FCoreRedirects::AddRedirectList(CookieBrosPlatformer::Redirects::Get(), TEXT("CookieBrosPlatformer"));
}

void FCookieBrosPlatformerModule::ShutdownModule()
{
	FCoreRedirects::RemoveRedirectList(CookieBrosPlatformer::Redirects::Get(), TEXT("CookieBrosPlatformer"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCookieBrosPlatformerModule, CookieBrosPlatformer)
