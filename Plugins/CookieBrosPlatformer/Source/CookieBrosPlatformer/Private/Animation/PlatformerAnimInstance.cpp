#include "Animation/PlatformerAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "Animation/PlatformerAnimDataAsset.h"
#include "Animation/PlatformerAnimGameplayTags.h"
#include "Animation/PlatformerLocomotionAnimSet.h"
#include "Character/PlatformerCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/PlatformerGameplayTags.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

void UPlatformerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<APlatformerCharacterBase>(TryGetPawnOwner());
	if (CachedCharacter)
	{
		CachedASC = CachedCharacter->GetAbilitySystemComponent();
		CachedTraversalMovementComponent = Cast<UPlatformerTraversalMovementComponent>(CachedCharacter->GetCharacterMovement());
		CacheAnimDataFromCharacter();
	}
}

void UPlatformerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter)
	{
		CachedCharacter = Cast<APlatformerCharacterBase>(TryGetPawnOwner());
		if (!CachedCharacter)
		{
			return;
		}

		CachedASC = CachedCharacter->GetAbilitySystemComponent();
		CachedTraversalMovementComponent = Cast<UPlatformerTraversalMovementComponent>(CachedCharacter->GetCharacterMovement());

		if (!bCachedDataInitialized)
		{
			CacheAnimDataFromCharacter();
		}
	}

	if (!CachedASC)
	{
		CachedASC = CachedCharacter->GetAbilitySystemComponent();
	}

	if (!CachedTraversalMovementComponent)
	{
		CachedTraversalMovementComponent = Cast<UPlatformerTraversalMovementComponent>(CachedCharacter->GetCharacterMovement());
	}

	UpdateMovementProperties(DeltaSeconds);
	UpdateLookProperties();
	UpdateGameplayTagProperties();
	UpdateDashStateProperties(DeltaSeconds);
	UpdateLadderStateProperties();
	UpdateLedgeGrabStateProperties(DeltaSeconds);
	UpdateLandingStateProperties(DeltaSeconds);
	UpdateAbilityMontageProperties();
	UpdateDerivedStateProperties();
}

void UPlatformerAnimInstance::UpdateMovementProperties(float DeltaSeconds)
{
	if (!CachedCharacter)
	{
		return;
	}

	const FVector Velocity = CachedCharacter->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	VerticalVelocity = Velocity.Z;

	if (const UCharacterMovementComponent* MovementComponent = CachedCharacter->GetCharacterMovement())
	{
		bIsInAir = MovementComponent->IsFalling();
		bHasAcceleration = MovementComponent->GetCurrentAcceleration().Size2D() > 0.0f;
	}
	else
	{
		bIsInAir = false;
		bHasAcceleration = false;
	}

	bIsCrouching = CachedCharacter->bIsCrouched;
	bIsOnLadder = CachedCharacter->IsOnLadder();
	LadderClimbInput = CachedCharacter->GetLadderClimbInput();
	LadderSpeed = bIsOnLadder ? FMath::Abs(Velocity.Z) : 0.0f;

	if (!FMath::IsNearlyZero(Velocity.X))
	{
		MovementDirectionX = FMath::Sign(Velocity.X);
	}
}

void UPlatformerAnimInstance::UpdateGameplayTagProperties()
{
	if (!CachedASC)
	{
		bIsDashing = CachedTraversalMovementComponent && CachedTraversalMovementComponent->IsDashing();
		bIsWallSliding = false;
		bIsWallJumping = false;
		bIsLedgeHanging = CachedTraversalMovementComponent && CachedTraversalMovementComponent->IsHangingOnLedge();
		bIsLedgeClimbing = CachedTraversalMovementComponent && CachedTraversalMovementComponent->IsClimbingLedge();
		bIsCharging = false;
		bIsChargePartial = false;
		bIsChargeFull = false;
		bIsDead = false;
		return;
	}

	// Traversal states
	bIsDashing = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Movement_Dash);
	bIsWallSliding = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Movement_WallSlide);
	bIsWallJumping = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Movement_WallJump);
	bIsLedgeHanging = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Movement_LedgeHang);
	bIsLedgeClimbing = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Movement_LedgeClimb);
	if (CachedTraversalMovementComponent)
	{
		bIsDashing = bIsDashing || CachedTraversalMovementComponent->IsDashing();
		bIsLedgeHanging = bIsLedgeHanging || CachedTraversalMovementComponent->IsHangingOnLedge();
		bIsLedgeClimbing = bIsLedgeClimbing || CachedTraversalMovementComponent->IsClimbingLedge();
	}

	// Combat states
	bIsCharging = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Combat_Charging);
	bIsChargePartial = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargePartial);
	bIsChargeFull = CachedASC->HasMatchingGameplayTag(PlatformerTraversalGameplayTags::State_Combat_ChargeFull);
	bIsDead = CachedASC->HasMatchingGameplayTag(PlatformerGameplayTags::State_Combat_Dead);
}

void UPlatformerAnimInstance::UpdateDerivedStateProperties()
{
	const bool bHasGroundMovement = GroundSpeed > MovementStateSpeedThreshold;
	const bool bIsDashAnimationStateActive = bIsDashing || bShouldDashEnd;
	const bool bIsLadderAnimationStateActive = bIsOnLadder || bShouldLadderEnd;
	const bool bIsLedgeGrabAnimationStateActive =
		bIsLedgeHanging || bIsLedgeClimbing || bShouldLedgeGrabStart || bShouldLedgeGrabLoop || bShouldLedgeGrabEnd;
	const bool bCanUseGroundLocomotion =
		!bIsInAir
		&& !bShouldLand
		&& !bIsLadderAnimationStateActive
		&& !bIsLedgeGrabAnimationStateActive
		&& !bIsDead
		&& !bIsDashAnimationStateActive;

	bShouldIdle = bCanUseGroundLocomotion && !bIsCrouching && !bHasGroundMovement;
	bShouldMove = bCanUseGroundLocomotion && !bIsCrouching && bHasGroundMovement;
	bShouldCrouchIdle = bCanUseGroundLocomotion && bIsCrouching && !bHasGroundMovement;
	bShouldCrouchMove = bCanUseGroundLocomotion && bIsCrouching && bHasGroundMovement;
	bShouldJump = bIsInAir && VerticalVelocity > 0.0f && !bIsLadderAnimationStateActive && !bIsLedgeGrabAnimationStateActive && !bIsDead;
	bShouldFall = bIsInAir && VerticalVelocity <= 0.0f && !bIsLadderAnimationStateActive && !bIsLedgeGrabAnimationStateActive && !bIsDead;
	bShouldWallSlide = bIsWallSliding && !bIsDead;
	bShouldWallJump = bIsWallJumping && !bIsDead;
}

void UPlatformerAnimInstance::UpdateDashStateProperties(float DeltaSeconds)
{
	DashElapsedTime = 0.0f;
	DashExpectedDuration = 0.0f;
	DashTimeAlpha = 0.0f;
	DashDistanceAlpha = 0.0f;
	DashProgressAlpha = 0.0f;

	if (bIsDashing && CachedTraversalMovementComponent)
	{
		DashElapsedTime = CachedTraversalMovementComponent->GetDashElapsedTime();
		DashExpectedDuration = CachedTraversalMovementComponent->GetDashDuration();
		DashTimeAlpha = CachedTraversalMovementComponent->GetDashNormalizedTime();
		DashDistanceAlpha = CachedTraversalMovementComponent->GetDashDistanceAlpha();
		DashProgressAlpha = CachedTraversalMovementComponent->GetDashProgressAlpha();
		DashEndStateTimeRemaining = 0.0f;
	}
	else if (bWasDashingLastFrame)
	{
		DashEndStateTimeRemaining = FMath::Max(DashEndStateDuration, 0.0f);
		DashTimeAlpha = 1.0f;
		DashDistanceAlpha = 1.0f;
		DashProgressAlpha = 1.0f;
	}
	else if (DashEndStateTimeRemaining > 0.0f)
	{
		DashEndStateTimeRemaining = FMath::Max(DashEndStateTimeRemaining - DeltaSeconds, 0.0f);
		DashTimeAlpha = 1.0f;
		DashDistanceAlpha = 1.0f;
		DashProgressAlpha = 1.0f;
	}

	bShouldDashStart = bIsDashing
		&& !bIsDead
		&& DashProgressAlpha < DashStartProgressThreshold;
	bShouldDashLoop = bIsDashing
		&& !bIsDead
		&& !bShouldDashStart;
	bShouldDashEnd = !bIsDead && DashEndStateTimeRemaining > 0.0f;

	if (bIsDead)
	{
		DashEndStateTimeRemaining = 0.0f;
		bShouldDashStart = false;
		bShouldDashLoop = false;
		bShouldDashEnd = false;
	}

	bWasDashingLastFrame = bIsDashing;
}

void UPlatformerAnimInstance::UpdateLadderStateProperties()
{
	if (!CachedCharacter || bIsDead)
	{
		bShouldLadderStart = false;
		bShouldLadderLoop = false;
		bShouldLadderEnd = false;
		return;
	}

	bShouldLadderStart = CachedCharacter->ShouldPlayLadderStartAnimation();
	bShouldLadderLoop = CachedCharacter->ShouldPlayLadderLoopAnimation();
	bShouldLadderEnd = CachedCharacter->ShouldPlayLadderEndAnimation();
}

void UPlatformerAnimInstance::UpdateLedgeGrabStateProperties(float DeltaSeconds)
{
	const bool bIsLedgeHangActive =
		bIsLedgeHanging
		|| (CachedTraversalMovementComponent && CachedTraversalMovementComponent->IsHangingOnLedge());
	const bool bIsLedgeClimbActive =
		bIsLedgeClimbing
		|| (CachedTraversalMovementComponent && CachedTraversalMovementComponent->IsClimbingLedge());
	bIsLedgeHanging = bIsLedgeHangActive;
	bIsLedgeClimbing = bIsLedgeClimbActive;

	LedgeGrabEndElapsedTime = 0.0f;
	LedgeGrabEndExpectedDuration = 0.0f;
	LedgeGrabEndTimeAlpha = 0.0f;

	if (bIsDead)
	{
		LedgeGrabStartStateTimeRemaining = 0.0f;
		LedgeGrabEndStateTimeRemaining = 0.0f;
		bShouldLedgeGrabStart = false;
		bShouldLedgeGrabLoop = false;
		bShouldLedgeGrabEnd = false;
		bIsLedgeHanging = false;
		bIsLedgeClimbing = false;
		bWasLedgeHangingLastFrame = false;
		bWasLedgeClimbingLastFrame = false;
		PreviousFrameVerticalVelocity = VerticalVelocity;
		return;
	}

	const bool bEnteredLedgeHang =
		bIsLedgeHangActive
		&& !bWasLedgeHangingLastFrame
		&& !bWasLedgeClimbingLastFrame;
	if (bEnteredLedgeHang)
	{
		LedgeGrabStartStateTimeRemaining =
			PreviousFrameVerticalVelocity > 0.0f
				? FMath::Max(LedgeGrabStartStateDuration, 0.0f)
				: 0.0f;
	}
	else if (!bIsLedgeHangActive)
	{
		LedgeGrabStartStateTimeRemaining = 0.0f;
	}
	else
	{
		LedgeGrabStartStateTimeRemaining =
			FMath::Max(LedgeGrabStartStateTimeRemaining - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	}

	if (bIsLedgeClimbActive && CachedTraversalMovementComponent)
	{
		LedgeGrabEndStateTimeRemaining = 0.0f;
		LedgeGrabEndElapsedTime = CachedTraversalMovementComponent->GetLedgeClimbElapsedTime();
		LedgeGrabEndExpectedDuration = CachedTraversalMovementComponent->GetLedgeClimbDuration();
		LedgeGrabEndTimeAlpha = CachedTraversalMovementComponent->GetLedgeClimbNormalizedTime();
	}
	else if (bWasLedgeClimbingLastFrame && !bIsInAir)
	{
		LedgeGrabEndStateTimeRemaining = FMath::Max(LedgeGrabEndStateDuration, 0.0f);
		LedgeGrabEndTimeAlpha = 1.0f;
	}
	else if (LedgeGrabEndStateTimeRemaining > 0.0f)
	{
		if (bIsInAir || bIsLedgeHangActive)
		{
			LedgeGrabEndStateTimeRemaining = 0.0f;
		}
		else
		{
			LedgeGrabEndStateTimeRemaining =
				FMath::Max(LedgeGrabEndStateTimeRemaining - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
			LedgeGrabEndTimeAlpha = 1.0f;
		}
	}

	bShouldLedgeGrabStart = bIsLedgeHangActive && LedgeGrabStartStateTimeRemaining > 0.0f;
	bShouldLedgeGrabLoop = bIsLedgeHangActive && !bShouldLedgeGrabStart;
	bShouldLedgeGrabEnd = bIsLedgeClimbActive || LedgeGrabEndStateTimeRemaining > 0.0f;

	bWasLedgeHangingLastFrame = bIsLedgeHangActive;
	bWasLedgeClimbingLastFrame = bIsLedgeClimbActive;
	PreviousFrameVerticalVelocity = VerticalVelocity;
}

void UPlatformerAnimInstance::UpdateLandingStateProperties(float DeltaSeconds)
{
	// Land state only makes sense when the LocomotionAnimSet provides a
	// LandSequence. Null sequence = feature disabled; skip to avoid
	// suppressing idle/run unnecessarily.
	const bool bHasLandSequence = LocomotionAnimSet && LocomotionAnimSet->LandSequence;

	if (!bHasLandSequence || bIsDead)
	{
		LandStateTimeRemaining = 0.0f;
		bShouldLand = false;
		bWasInAirLastFrame = bIsInAir;
		return;
	}

	// Detect air→ground transition. Suppress when ladder or ledge-climb end
	// states are active — they own the exit animation for those cases.
	const bool bLadderOrLedgeExit = bShouldLadderEnd || bShouldLedgeGrabEnd || bIsOnLadder;
	const bool bJustLanded = bWasInAirLastFrame && !bIsInAir && !bLadderOrLedgeExit;

	if (bJustLanded)
	{
		LandStateTimeRemaining = FMath::Max(LandStateDuration, 0.0f);
	}
	else if (bIsInAir || bLadderOrLedgeExit)
	{
		// Abort any active land state if we went back to air or a traversal
		// system took over (e.g. double-jump during landing recovery).
		LandStateTimeRemaining = 0.0f;
	}
	else
	{
		LandStateTimeRemaining = FMath::Max(LandStateTimeRemaining - DeltaSeconds, 0.0f);
	}

	bShouldLand = LandStateTimeRemaining > 0.0f;
	bWasInAirLastFrame = bIsInAir;
}

void UPlatformerAnimInstance::UpdateLookProperties()
{
	if (!CachedCharacter)
	{
		LookYaw = 0.0f;
		LookPitch = 0.0f;
		return;
	}

	const FRotator ControlRotation = CachedCharacter->GetControlRotation();
	const FRotator ActorRotation = CachedCharacter->GetActorRotation();
	const FRotator Delta = (ControlRotation - ActorRotation).GetNormalized();

	LookYaw = FMath::ClampAngle(Delta.Yaw, -90.0f, 90.0f);
	LookPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
}

void UPlatformerAnimInstance::UpdateAbilityMontageProperties()
{
	bIsMeleeAttacking = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_MeleeHit);
	bIsRangedAttacking = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_RangedShot);
	bIsMeleeCharging = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_MeleeChargeLoop);
	bIsRangedCharging = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_RangedChargeLoop);
	bIsHitReacting = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_HitReaction);
	bIsDeathAnimating = IsAbilityMontagePlaying(PlatformerAnimGameplayTags::Anim_Combat_Death);
	bIsPlayingAbilityMontage = IsAnyAbilityMontagePlaying();
}

float UPlatformerAnimInstance::PlayAbilityMontage(FGameplayTag AnimTag, float PlayRate)
{
	UAnimMontage* Montage = ResolveAbilityMontage(AnimTag);
	if (!Montage)
	{
		return 0.0f;
	}

	// Use the entry's default play rate if caller uses default
	if (AnimData)
	{
		if (const FPlatformerAbilityAnimEntry* Entry = AnimData->FindAnimEntry(AnimTag))
		{
			if (FMath::IsNearlyEqual(PlayRate, 1.0f))
			{
				PlayRate = Entry->DefaultPlayRate;
			}
		}
	}

	return Montage_Play(Montage, PlayRate);
}

void UPlatformerAnimInstance::StopAbilityMontage(FGameplayTag AnimTag, float BlendOutTime)
{
	UAnimMontage* Montage = ResolveAbilityMontage(AnimTag);
	if (Montage)
	{
		Montage_Stop(BlendOutTime, Montage);
	}
}

UAnimMontage* UPlatformerAnimInstance::ResolveAbilityMontage(const FGameplayTag& AnimTag) const
{
	if (!AnimData)
	{
		return nullptr;
	}

	return AnimData->FindMontage(AnimTag);
}

bool UPlatformerAnimInstance::IsAbilityMontagePlaying(const FGameplayTag& AnimTag) const
{
	UAnimMontage* Montage = ResolveAbilityMontage(AnimTag);
	return Montage && Montage_IsPlaying(Montage);
}

bool UPlatformerAnimInstance::IsAnyAbilityMontagePlaying() const
{
	if (!AnimData)
	{
		return false;
	}

	for (const FPlatformerAbilityAnimEntry& Entry : AnimData->AbilityAnimations)
	{
		if (Entry.Montage && Montage_IsPlaying(Entry.Montage.Get()))
		{
			return true;
		}
	}

	return false;
}

void UPlatformerAnimInstance::CacheAnimDataFromCharacter()
{
	bCachedDataInitialized = true;

	if (!CachedCharacter)
	{
		return;
	}

	if (!AnimData)
	{
		AnimData = CachedCharacter->GetAnimDataAsset();
	}

	if (!LocomotionAnimSet)
	{
		LocomotionAnimSet = CachedCharacter->GetLocomotionAnimSet();
	}

	if (LocomotionAnimSet)
	{
		CachedLocomotionBS         = LocomotionAnimSet->LocomotionBlendSpace;
		CachedCrouchBS             = LocomotionAnimSet->CrouchBlendSpace;
		CachedJumpStartSequence    = LocomotionAnimSet->JumpStartSequence;
		CachedFallLoopSequence     = LocomotionAnimSet->FallLoopSequence;
		CachedLandSequence         = LocomotionAnimSet->LandSequence;
		CachedDashStartSequence    = LocomotionAnimSet->DashStartSequence;
		CachedDashLoopSequence     = LocomotionAnimSet->DashLoopSequence;
		CachedDashEndSequence      = LocomotionAnimSet->DashEndSequence;
		CachedWallSlideSequence    = LocomotionAnimSet->WallSlideSequence;
		CachedWallJumpSequence     = LocomotionAnimSet->WallJumpSequence;
		CachedLadderStartSequence  = LocomotionAnimSet->LadderStartSequence;
		CachedLadderLoopBS         = LocomotionAnimSet->LadderLoopBlendSpace;
		CachedLadderEndSequence    = LocomotionAnimSet->LadderEndSequence;
		CachedLedgeGrabStartSequence = LocomotionAnimSet->LedgeGrabStartSequence;
		CachedLedgeGrabLoopSequence  = LocomotionAnimSet->LedgeGrabLoopSequence;
		CachedLedgeClimbSequence   = LocomotionAnimSet->LedgeClimbSequence;
		CachedLookAimOffset        = LocomotionAnimSet->LookAimOffset;

		bHasDash     = CachedDashStartSequence    != nullptr;
		bHasWall     = CachedWallSlideSequence     != nullptr;
		bHasLadder   = CachedLadderStartSequence   != nullptr;
		bHasLedgeGrab = CachedLedgeGrabStartSequence != nullptr;
		bHasLand     = CachedLandSequence          != nullptr;
		bHasLook     = CachedLookAimOffset         != nullptr;
	}
}
