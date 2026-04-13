#include "Traversal/PlatformerTraversalMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/PlatformerCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerLedgeGrab.h"
#include "Traversal/PlatformerTraversalConfigDataAsset.h"
#include "Traversal/PlatformerTraversalGameplayTags.h"

namespace PlatformerTraversalPrivate
{
	constexpr float VerticalWallNormalThreshold = 0.2f;
	constexpr float FloorStandTolerance = 2.0f;

	bool IsTraversalBlockingSurfaceHit(const FHitResult& Hit)
	{
		const UPrimitiveComponent* HitComponent = Hit.GetComponent();
		return Hit.bBlockingHit
			&& (HitComponent != nullptr)
			&& (HitComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			&& (HitComponent->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block);
	}
}

UPlatformerTraversalMovementComponent::UPlatformerTraversalMovementComponent()
{
}

void UPlatformerTraversalMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CacheCapsuleState();
}

void UPlatformerTraversalMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPendingCapsuleRestore && TraversalState != EPlatformerTraversalState::SlideDash)
	{
		RestoreCapsuleSizeIfPossible();
	}

	UpdateAutomaticTraversal();
}

void UPlatformerTraversalMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	switch (static_cast<EPlatformerTraversalCustomMode>(CustomMovementMode))
	{
	case EPlatformerTraversalCustomMode::LedgeHang:
		PhysLedgeHang(DeltaTime, Iterations);
		return;

	case EPlatformerTraversalCustomMode::LedgeClimb:
		PhysLedgeClimb(DeltaTime, Iterations);
		return;

	case EPlatformerTraversalCustomMode::WallSlide:
		PhysWallSlide(DeltaTime, Iterations);
		return;

	case EPlatformerTraversalCustomMode::SlideDash:
		PhysSlideDash(DeltaTime, Iterations);
		return;

	default:
		break;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UPlatformerTraversalMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Falling)
	{
		FallingStartTime = GetWorldTimeSafe();
	}
	else if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		FallingStartTime = -1.0f;
		if (TraversalState == EPlatformerTraversalState::WallSlide)
		{
			ExitWallSlide(false);
		}
	}

	if (MovementMode != MOVE_Custom && TraversalState != EPlatformerTraversalState::None)
	{
		if (TraversalState == EPlatformerTraversalState::SlideDash)
		{
			FinishSlideDash(true, MovementMode == MOVE_Falling);
		}
		else if (TraversalState == EPlatformerTraversalState::WallSlide)
		{
			ExitWallSlide(MovementMode == MOVE_Falling);
		}
		else
		{
			UpdateTraversalState(EPlatformerTraversalState::None);
		}
	}
}

void UPlatformerTraversalMovementComponent::SetTraversalConfig(UPlatformerTraversalConfigDataAsset* InTraversalConfig)
{
	TraversalConfig = InTraversalConfig;

	if (TraversalConfig)
	{
		bTraversalEnabled = true;
	}
}

void UPlatformerTraversalMovementComponent::SetTraversalEnabled(bool bInTraversalEnabled)
{
	if (bTraversalEnabled == bInTraversalEnabled)
	{
		return;
	}

	bTraversalEnabled = bInTraversalEnabled;
	if (!bTraversalEnabled && TraversalState != EPlatformerTraversalState::None)
	{
		CancelTraversal();
	}
}

void UPlatformerTraversalMovementComponent::SetDefaultLedgeSettings(const FPlatformerLedgeTraversalSettings& InSettings)
{
	DefaultLedgeSettings = InSettings;
}

void UPlatformerTraversalMovementComponent::SetDefaultSlideDashSettings(const FPlatformerSlideDashSettings& InSettings)
{
	DefaultSlideDashSettings = InSettings;
}

void UPlatformerTraversalMovementComponent::SetDefaultWallSettings(const FPlatformerWallTraversalSettings& InSettings)
{
	DefaultWallSettings = InSettings;
}

void UPlatformerTraversalMovementComponent::SetDeveloperTraversalSettingsOverride(
	const FPlatformerLedgeTraversalSettings& InLedgeSettings,
	const FPlatformerSlideDashSettings& InSlideDashSettings,
	const FPlatformerWallTraversalSettings& InWallSettings)
{
	bHasDeveloperTraversalSettingsOverride = true;
	DeveloperLedgeSettingsOverride = InLedgeSettings;
	DeveloperSlideDashSettingsOverride = InSlideDashSettings;
	DeveloperWallSettingsOverride = InWallSettings;
}

void UPlatformerTraversalMovementComponent::ClearDeveloperTraversalSettingsOverride()
{
	bHasDeveloperTraversalSettingsOverride = false;
}

void UPlatformerTraversalMovementComponent::SetTraversalInputVector(FVector2D InTraversalInputVector)
{
	TraversalInputVector = InTraversalInputVector;

	if (TraversalState == EPlatformerTraversalState::SlideDash && ShouldCancelSlideDashFromInput())
	{
		FinishSlideDash(true, false);
	}
}

bool UPlatformerTraversalMovementComponent::HandleTraversalJumpPressed()
{
	if (!bTraversalEnabled)
	{
		return false;
	}

	if (TraversalState == EPlatformerTraversalState::LedgeHang)
	{
		StartLedgeClimb();
		return true;
	}

	if (TraversalState == EPlatformerTraversalState::WallSlide)
	{
		PerformWallKick();
		return true;
	}

	if (TraversalState == EPlatformerTraversalState::SlideDash)
	{
		FinishSlideDash(true, false);
		return false;
	}

	return false;
}

bool UPlatformerTraversalMovementComponent::HandleTraversalCrouchPressed()
{
	if (!bTraversalEnabled)
	{
		return false;
	}

	if (TraversalState == EPlatformerTraversalState::LedgeHang)
	{
		DropFromLedge();
		return true;
	}

	return false;
}

void UPlatformerTraversalMovementComponent::HandleTraversalJumpReleased()
{
}

void UPlatformerTraversalMovementComponent::HandleTraversalCrouchReleased()
{
}

bool UPlatformerTraversalMovementComponent::StartSlideDash()
{
	if (!bTraversalEnabled || !CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FPlatformerSlideDashSettings& SlideDashSettings = GetSlideDashSettings();
	if (!SlideDashSettings.bEnabled
		|| !IsMovingOnGround()
		|| TraversalState != EPlatformerTraversalState::None
		|| GetWorldTimeSafe() < SlideDashRecoveryEndTime)
	{
		return false;
	}

	const float DirectionSign = GetTraversalDirectionSign();
	if (FMath::IsNearlyZero(DirectionSign))
	{
		return false;
	}

	SlideDashStartTime = GetWorldTimeSafe();
	SlideDashStartLocation = UpdatedComponent->GetComponentLocation();
	SlideDashDirectionSign = DirectionSign;
	Velocity = FVector(SlideDashDirectionSign * SlideDashSettings.DashSpeed, 0.0f, 0.0f);
	ApplySlideDashCapsule();
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::SlideDash));
	UpdateTraversalState(EPlatformerTraversalState::SlideDash);
	return true;
}

void UPlatformerTraversalMovementComponent::CancelTraversal()
{
	switch (TraversalState)
	{
	case EPlatformerTraversalState::LedgeHang:
	case EPlatformerTraversalState::LedgeClimb:
		DropFromLedge();
		break;

	case EPlatformerTraversalState::WallSlide:
		ExitWallSlide(true);
		break;

	case EPlatformerTraversalState::SlideDash:
		FinishSlideDash(true, !IsMovingOnGround());
		break;

	default:
		break;
	}
}

bool UPlatformerTraversalMovementComponent::IsTraversalStateActive() const
{
	return TraversalState != EPlatformerTraversalState::None;
}

bool UPlatformerTraversalMovementComponent::IsInCustomTraversalMode() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode != 0;
}

bool UPlatformerTraversalMovementComponent::IsHangingOnLedge() const
{
	return TraversalState == EPlatformerTraversalState::LedgeHang;
}

bool UPlatformerTraversalMovementComponent::IsWallSliding() const
{
	return TraversalState == EPlatformerTraversalState::WallSlide;
}

bool UPlatformerTraversalMovementComponent::IsSlideDashing() const
{
	return TraversalState == EPlatformerTraversalState::SlideDash;
}

bool UPlatformerTraversalMovementComponent::IsInSlideDashRecovery() const
{
	return GetWorldTimeSafe() < SlideDashRecoveryEndTime;
}

bool UPlatformerTraversalMovementComponent::IsAttackBlockedByTraversal() const
{
	return TraversalState == EPlatformerTraversalState::SlideDash
		|| TraversalState == EPlatformerTraversalState::LedgeHang
		|| TraversalState == EPlatformerTraversalState::LedgeClimb;
}

void UPlatformerTraversalMovementComponent::CacheCapsuleState()
{
	if (const ACharacter* Character = CharacterOwner)
	{
		if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			CachedCapsuleState.UnscaledRadius = CapsuleComponent->GetUnscaledCapsuleRadius();
			CachedCapsuleState.UnscaledHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
			CachedCapsuleState.ShapeScale = CapsuleComponent->GetShapeScale();
			CachedCapsuleState.bValid = true;
		}
	}
}

void UPlatformerTraversalMovementComponent::UpdateAutomaticTraversal()
{
	if (!bTraversalEnabled || !CharacterOwner || !UpdatedComponent || IsInCustomTraversalMode())
	{
		return;
	}

	if (ShouldUseFallingTraversal())
	{
		FVector LedgeHangResult = FVector::ZeroVector;
		FVector LedgeClimbResult = FVector::ZeroVector;
		FVector LedgeNormal = FVector::ZeroVector;
		if (TryFindLedgeGrab(LedgeHangResult, LedgeClimbResult, LedgeNormal))
		{
			EnterLedgeHang(LedgeHangResult, LedgeClimbResult, LedgeNormal);
			return;
		}

		FVector WallNormal = FVector::ZeroVector;
		FVector WallAnchorLocation = FVector::ZeroVector;
		AActor* WallActor = nullptr;
		if (TryFindWallSlide(WallNormal, WallAnchorLocation, WallActor))
		{
			EnterWallSlide(WallNormal, WallAnchorLocation, WallActor);
		}
	}
}

void UPlatformerTraversalMovementComponent::UpdateTraversalState(EPlatformerTraversalState NewState)
{
	if (TraversalState == NewState)
	{
		return;
	}

	const EPlatformerTraversalState PreviousState = TraversalState;
	const FGameplayTag PreviousTag = GetStateTagForTraversalState(PreviousState);
	if (PreviousTag.IsValid())
	{
		RemoveLooseTag(PreviousTag);
	}

	RemovePersistentCue();

	TraversalState = NewState;

	const FGameplayTag NewTag = GetStateTagForTraversalState(NewState);
	if (NewTag.IsValid())
	{
		AddLooseTag(NewTag);
	}

	if (TraversalConfig)
	{
		switch (TraversalState)
		{
		case EPlatformerTraversalState::LedgeHang:
			AddPersistentCue(TraversalConfig->LedgeSettings.HangCueTag);
			break;

		case EPlatformerTraversalState::WallSlide:
			AddPersistentCue(TraversalConfig->WallSettings.WallSlideCueTag);
			break;

		case EPlatformerTraversalState::SlideDash:
			AddPersistentCue(TraversalConfig->SlideDashSettings.DashCueTag);
			break;

		default:
			break;
		}
	}

	OnTraversalStateChanged.Broadcast(PreviousState, TraversalState);
}

void UPlatformerTraversalMovementComponent::AddPersistentCue(const FGameplayTag& CueTag)
{
	if (!CueTag.IsValid())
	{
		return;
	}

	if (ActivePersistentCueTag == CueTag)
	{
		return;
	}

	RemovePersistentCue();
	ActivePersistentCueTag = CueTag;

	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			FGameplayCueParameters CueParameters;
			CueParameters.Location = CharacterOwner->GetActorLocation();
			AbilitySystemComponent->AddGameplayCue(CueTag, CueParameters);
		}
	}
}

void UPlatformerTraversalMovementComponent::RemovePersistentCue()
{
	if (!ActivePersistentCueTag.IsValid())
	{
		return;
	}

	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			FGameplayCueParameters CueParameters;
			CueParameters.Location = CharacterOwner->GetActorLocation();
			AbilitySystemComponent->RemoveGameplayCue(ActivePersistentCueTag);
		}
	}

	ActivePersistentCueTag = FGameplayTag();
}

void UPlatformerTraversalMovementComponent::ExecuteCue(const FGameplayTag& CueTag) const
{
	if (!CueTag.IsValid())
	{
		return;
	}

	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			FGameplayCueParameters CueParameters;
			CueParameters.Location = CharacterOwner->GetActorLocation();
			AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
		}
	}
}

void UPlatformerTraversalMovementComponent::AddLooseTag(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			AbilitySystemComponent->AddLooseGameplayTag(Tag);
		}
	}
}

void UPlatformerTraversalMovementComponent::RemoveLooseTag(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(CharacterOwner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(Tag);
		}
	}
}

FGameplayTag UPlatformerTraversalMovementComponent::GetStateTagForTraversalState(EPlatformerTraversalState State) const
{
	switch (State)
	{
	case EPlatformerTraversalState::LedgeHang:
		return PlatformerTraversalGameplayTags::State_Movement_LedgeHang;

	case EPlatformerTraversalState::LedgeClimb:
		return PlatformerTraversalGameplayTags::State_Movement_LedgeClimb;

	case EPlatformerTraversalState::WallSlide:
		return PlatformerTraversalGameplayTags::State_Movement_WallSlide;

	case EPlatformerTraversalState::SlideDash:
		return PlatformerTraversalGameplayTags::State_Movement_SlideDash;

	default:
		break;
	}

	return FGameplayTag::EmptyTag;
}

float UPlatformerTraversalMovementComponent::GetWorldTimeSafe() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

float UPlatformerTraversalMovementComponent::GetTraversalDirectionSign() const
{
	if (!TraversalInputVector.IsNearlyZero())
	{
		return FMath::Sign(TraversalInputVector.X);
	}

	if (!Velocity.IsNearlyZero())
	{
		return FMath::Sign(Velocity.X);
	}

	if (CharacterOwner)
	{
		return FMath::Sign(CharacterOwner->GetActorForwardVector().X);
	}

	return 0.0f;
}

const FPlatformerLedgeTraversalSettings& UPlatformerTraversalMovementComponent::GetLedgeSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return DeveloperLedgeSettingsOverride;
	}

	return TraversalConfig ? TraversalConfig->LedgeSettings : DefaultLedgeSettings;
}

const FPlatformerSlideDashSettings& UPlatformerTraversalMovementComponent::GetSlideDashSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return DeveloperSlideDashSettingsOverride;
	}

	return TraversalConfig ? TraversalConfig->SlideDashSettings : DefaultSlideDashSettings;
}

const FPlatformerWallTraversalSettings& UPlatformerTraversalMovementComponent::GetWallSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return DeveloperWallSettingsOverride;
	}

	return TraversalConfig ? TraversalConfig->WallSettings : DefaultWallSettings;
}

bool UPlatformerTraversalMovementComponent::ShouldUseFallingTraversal() const
{
	if (!bTraversalEnabled)
	{
		return false;
	}

	const FPlatformerLedgeTraversalSettings& LedgeSettings = GetLedgeSettings();
	if (!IsFalling() || TraversalState != EPlatformerTraversalState::None)
	{
		return false;
	}

	if (Velocity.Z > 0.0f)
	{
		const float TimeSinceEnteredFalling = FallingStartTime >= 0.0f ? GetWorldTimeSafe() - FallingStartTime : BIG_NUMBER;
		return TimeSinceEnteredFalling <= LedgeSettings.ForgivenessWindow;
	}

	return true;
}

bool UPlatformerTraversalMovementComponent::ShouldCancelSlideDashFromInput() const
{
	if (TraversalState != EPlatformerTraversalState::SlideDash)
	{
		return false;
	}

	const float HorizontalInput = TraversalInputVector.X;
	if (FMath::Abs(HorizontalInput) <= 0.1f)
	{
		return false;
	}

	return HorizontalInput * SlideDashDirectionSign < 0.0f;
}

bool UPlatformerTraversalMovementComponent::RestoreCapsuleSizeIfPossible()
{
	if (!bDashCapsuleApplied || !CachedCapsuleState.bValid || !CharacterOwner)
	{
		bPendingCapsuleRestore = false;
		return true;
	}

	UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return false;
	}

	const float CurrentUnscaledHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
	const float HeightDelta = (CachedCapsuleState.UnscaledHalfHeight - CurrentUnscaledHalfHeight) * CachedCapsuleState.ShapeScale;
	const FVector TargetLocation = UpdatedComponent->GetComponentLocation() + FVector(0.0f, 0.0f, HeightDelta);
	if (!CanUseFullCapsuleAt(TargetLocation))
	{
		bPendingCapsuleRestore = true;
		return false;
	}

	CapsuleComponent->SetCapsuleSize(CachedCapsuleState.UnscaledRadius, CachedCapsuleState.UnscaledHalfHeight, true);
	UpdatedComponent->SetWorldLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	bDashCapsuleApplied = false;
	bPendingCapsuleRestore = false;
	return true;
}

void UPlatformerTraversalMovementComponent::ApplySlideDashCapsule()
{
	if (bDashCapsuleApplied || !CharacterOwner)
	{
		return;
	}

	const FPlatformerSlideDashSettings& SlideDashSettings = GetSlideDashSettings();
	CacheCapsuleState();
	if (!CachedCapsuleState.bValid)
	{
		return;
	}

	UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return;
	}

	float SlideDashCapsuleScale = FMath::Max(SlideDashSettings.DashHitboxScale, 0.0f);
	float StandingCapsuleHalfHeight = CachedCapsuleState.UnscaledHalfHeight;
	if (const APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(CharacterOwner))
	{
		SlideDashCapsuleScale = PlatformerCharacter->ResolveDeveloperCrouchCapsuleScale(SlideDashCapsuleScale);

		if (const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>())
		{
			if (const UCapsuleComponent* DefaultCapsuleComponent = DefaultCharacter->GetCapsuleComponent())
			{
				StandingCapsuleHalfHeight = DefaultCapsuleComponent->GetUnscaledCapsuleHalfHeight();
			}
		}
	}

	const float NewUnscaledHalfHeight = FMath::Max(
		CachedCapsuleState.UnscaledRadius,
		StandingCapsuleHalfHeight * SlideDashCapsuleScale);
	const float HeightDelta = (CachedCapsuleState.UnscaledHalfHeight - NewUnscaledHalfHeight) * CachedCapsuleState.ShapeScale;

	CapsuleComponent->SetCapsuleSize(CachedCapsuleState.UnscaledRadius, NewUnscaledHalfHeight, true);
	UpdatedComponent->SetWorldLocation(
		UpdatedComponent->GetComponentLocation() - FVector(0.0f, 0.0f, HeightDelta),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	bDashCapsuleApplied = true;
	bPendingCapsuleRestore = true;
}

bool UPlatformerTraversalMovementComponent::CanUseFullCapsuleAt(const FVector& TestLocation) const
{
	if (!CachedCapsuleState.bValid || !CharacterOwner || !GetWorld())
	{
		return false;
	}

	const UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TraversalRestoreCapsule), false, CharacterOwner);
	return !GetWorld()->OverlapBlockingTestByProfile(
		TestLocation,
		FQuat::Identity,
		CapsuleComponent->GetCollisionProfileName(),
		FCollisionShape::MakeCapsule(
			CachedCapsuleState.UnscaledRadius * CachedCapsuleState.ShapeScale,
			CachedCapsuleState.UnscaledHalfHeight * CachedCapsuleState.ShapeScale),
		QueryParams);
}

bool UPlatformerTraversalMovementComponent::HasWalkableFloorBelow(const FVector& TestLocation) const
{
	if (!GetWorld() || !CharacterOwner)
	{
		return false;
	}

	const UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return false;
	}

	FFindFloorResult FloorResult;
	FindFloor(TestLocation, FloorResult, false);
	return FloorResult.IsWalkableFloor();
}

bool UPlatformerTraversalMovementComponent::TryFindLedgeGrab(
	FVector& OutHangLocation,
	FVector& OutClimbTargetLocation,
	FVector& OutWallNormal) const
{
	if (!bTraversalEnabled || !CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FPlatformerLedgeTraversalSettings& LedgeSettings = GetLedgeSettings();
	if (!LedgeSettings.bEnabled || GetWorldTimeSafe() < LedgeReleaseLockoutEndTime)
	{
		return false;
	}

	if (TryFindTriggeredLedgeGrab(OutHangLocation, OutClimbTargetLocation, OutWallNormal))
	{
		return true;
	}

	return LedgeSettings.bUseLegacyWorldGeometryDetection
		&& TryFindLegacyWorldLedgeGrab(OutHangLocation, OutClimbTargetLocation, OutWallNormal);
}

bool UPlatformerTraversalMovementComponent::TryFindTriggeredLedgeGrab(
	FVector& OutHangLocation,
	FVector& OutClimbTargetLocation,
	FVector& OutWallNormal) const
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const APlatformerCharacterBase* PlatformerCharacter = Cast<APlatformerCharacterBase>(CharacterOwner);
	if (!PlatformerCharacter)
	{
		return false;
	}

	const float DirectionSign = GetTraversalDirectionSign();
	if (FMath::IsNearlyZero(DirectionSign))
	{
		return false;
	}

	const FPlatformerLedgeTraversalSettings& LedgeSettings = GetLedgeSettings();
	TArray<APlatformerLedgeGrab*> CandidateLedgeGrabs;
	PlatformerCharacter->GetAvailableLedgeGrabs(CandidateLedgeGrabs);
	if (CandidateLedgeGrabs.IsEmpty())
	{
		return false;
	}

	const FVector CharacterLocation = UpdatedComponent->GetComponentLocation();
	bool bFoundCandidate = false;
	float BestCandidateDistanceSq = TNumericLimits<float>::Max();

	for (APlatformerLedgeGrab* CandidateLedgeGrab : CandidateLedgeGrabs)
	{
		if (!IsValid(CandidateLedgeGrab))
		{
			continue;
		}

		FVector CandidateHangLocation = FVector::ZeroVector;
		FVector CandidateClimbTargetLocation = FVector::ZeroVector;
		FVector CandidateWallNormal = FVector::ZeroVector;
		if (!CandidateLedgeGrab->TryBuildTraversalTargets(
			*CharacterOwner,
			DirectionSign,
			LockedDepthY,
			LedgeSettings,
			CandidateHangLocation,
			CandidateClimbTargetLocation,
			CandidateWallNormal))
		{
			continue;
		}

		if (!CanUseFullCapsuleAt(CandidateClimbTargetLocation) || !HasWalkableFloorBelow(CandidateClimbTargetLocation))
		{
			continue;
		}

		const FVector CandidateDelta = CandidateHangLocation - CharacterLocation;
		const float CandidateDistanceSq = FMath::Square(CandidateDelta.X) + FMath::Square(CandidateDelta.Z);
		if (!bFoundCandidate || CandidateDistanceSq < BestCandidateDistanceSq)
		{
			bFoundCandidate = true;
			BestCandidateDistanceSq = CandidateDistanceSq;
			OutHangLocation = CandidateHangLocation;
			OutClimbTargetLocation = CandidateClimbTargetLocation;
			OutWallNormal = CandidateWallNormal;
		}
	}

	return bFoundCandidate;
}

bool UPlatformerTraversalMovementComponent::TryFindLegacyWorldLedgeGrab(
	FVector& OutHangLocation,
	FVector& OutClimbTargetLocation,
	FVector& OutWallNormal) const
{
	if (!bTraversalEnabled || !CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FPlatformerLedgeTraversalSettings& LedgeSettings = GetLedgeSettings();
	if (!LedgeSettings.bEnabled || GetWorldTimeSafe() < LedgeReleaseLockoutEndTime)
	{
		return false;
	}

	const float DirectionSign = GetTraversalDirectionSign();
	if (FMath::IsNearlyZero(DirectionSign))
	{
		return false;
	}

	const UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (!CapsuleComponent)
	{
		return false;
	}

	const float CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	const FVector CharacterLocation = UpdatedComponent->GetComponentLocation();
	const FVector ForwardVector = FVector(DirectionSign, 0.0f, 0.0f);

	const FVector WallProbeStart = CharacterLocation + FVector(0.0f, 0.0f, LedgeSettings.MinHangHeight);
	const FVector WallProbeEnd = WallProbeStart + ForwardVector * LedgeSettings.DetectionDistance;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TraversalLedgeWallProbe), false, CharacterOwner);
	TArray<FHitResult> WallHits;
	const bool bHitAnyWall = GetWorld()->SweepMultiByObjectType(
		WallHits,
		WallProbeStart,
		WallProbeEnd,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(LedgeSettings.ForwardProbeRadius),
		QueryParams);

	FHitResult WallHit;
	bool bFoundValidWallHit = false;
	if (bHitAnyWall)
	{
		for (const FHitResult& CandidateWallHit : WallHits)
		{
			if (!PlatformerTraversalPrivate::IsTraversalBlockingSurfaceHit(CandidateWallHit))
			{
				continue;
			}

			if (FMath::Abs(CandidateWallHit.Normal.Z) > PlatformerTraversalPrivate::VerticalWallNormalThreshold)
			{
				continue;
			}

			WallHit = CandidateWallHit;
			bFoundValidWallHit = true;
			break;
		}
	}

	if (!bFoundValidWallHit)
	{
		return false;
	}

	const FVector TopTraceStart =
		WallHit.ImpactPoint
		+ ForwardVector * (CapsuleRadius + LedgeSettings.TopSurfaceProbeForwardOffset)
		+ FVector(0.0f, 0.0f, LedgeSettings.MaxReachHeight);
	const FVector TopTraceEnd = TopTraceStart - FVector(0.0f, 0.0f, LedgeSettings.MaxReachHeight + CapsuleHalfHeight + 48.0f);

	TArray<FHitResult> TopHits;
	const bool bHitAnyTop = GetWorld()->LineTraceMultiByObjectType(
		TopHits,
		TopTraceStart,
		TopTraceEnd,
		ObjectQueryParams,
		QueryParams);

	FHitResult TopHit;
	bool bFoundValidTopHit = false;
	if (bHitAnyTop)
	{
		for (const FHitResult& CandidateTopHit : TopHits)
		{
			if (!PlatformerTraversalPrivate::IsTraversalBlockingSurfaceHit(CandidateTopHit))
			{
				continue;
			}

			if (CandidateTopHit.Normal.Z < GetWalkableFloorZ())
			{
				continue;
			}

			TopHit = CandidateTopHit;
			bFoundValidTopHit = true;
			break;
		}
	}

	if (!bFoundValidTopHit)
	{
		return false;
	}

	const float FeetZ = CharacterLocation.Z - CapsuleHalfHeight;
	const float LedgeHeight = TopHit.Location.Z - FeetZ;
	if (LedgeHeight < LedgeSettings.MinHangHeight || LedgeHeight > LedgeSettings.MaxReachHeight)
	{
		return false;
	}

	OutWallNormal = FVector(FMath::Sign(WallHit.Normal.X), 0.0f, 0.0f);
	OutHangLocation = FVector(
		WallHit.ImpactPoint.X + OutWallNormal.X * (CapsuleRadius - LedgeSettings.HangForwardOffset),
		LockedDepthY,
		TopHit.Location.Z - CapsuleHalfHeight + LedgeSettings.HangVerticalOffset);

	OutClimbTargetLocation = FVector(
		TopHit.Location.X,
		LockedDepthY,
		TopHit.Location.Z + CapsuleHalfHeight + PlatformerTraversalPrivate::FloorStandTolerance);

	return CanUseFullCapsuleAt(OutClimbTargetLocation) && HasWalkableFloorBelow(OutClimbTargetLocation);
}

bool UPlatformerTraversalMovementComponent::TryFindWallSlide(
	FVector& OutWallNormal,
	FVector& OutAnchorLocation,
	AActor*& OutWallActor) const
{
	if (!bTraversalEnabled || !CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FPlatformerWallTraversalSettings& WallSettings = GetWallSettings();
	if (!WallSettings.bEnabled || Velocity.Z > -WallSettings.MinFallSpeedForSlide)
	{
		return false;
	}

	float DirectionSign = 0.0f;
	if (TraversalState == EPlatformerTraversalState::WallSlide && !FMath::IsNearlyZero(WallSlideNormal.X))
	{
		// While we are already attached to a wall, keep probing that same side so the player can
		// press away from the wall and still execute a wall kick on the next jump press.
		DirectionSign = -FMath::Sign(WallSlideNormal.X);
	}
	else
	{
		DirectionSign = GetTraversalDirectionSign();
	}

	if (FMath::IsNearlyZero(DirectionSign))
	{
		return false;
	}

	const FVector ProbeStart = UpdatedComponent->GetComponentLocation() + FVector(0.0f, 0.0f, WallSettings.ProbeHeightOffset);
	const FVector ProbeEnd = ProbeStart + FVector(DirectionSign * WallSettings.ProbeDistance, 0.0f, 0.0f);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TraversalWallProbe), false, CharacterOwner);
	TArray<FHitResult> WallHits;
	const bool bHitAnyWall = GetWorld()->SweepMultiByObjectType(
		WallHits,
		ProbeStart,
		ProbeEnd,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(12.0f),
		QueryParams);

	FHitResult WallHit;
	bool bFoundValidWallHit = false;
	if (bHitAnyWall)
	{
		for (const FHitResult& CandidateWallHit : WallHits)
		{
			if (!PlatformerTraversalPrivate::IsTraversalBlockingSurfaceHit(CandidateWallHit))
			{
				continue;
			}

			if (FMath::Abs(CandidateWallHit.Normal.Z) > PlatformerTraversalPrivate::VerticalWallNormalThreshold)
			{
				continue;
			}

			WallHit = CandidateWallHit;
			bFoundValidWallHit = true;
			break;
		}
	}

	if (!bFoundValidWallHit)
	{
		return false;
	}

	OutWallNormal = FVector(FMath::Sign(WallHit.Normal.X), 0.0f, 0.0f);
	OutAnchorLocation = WallHit.ImpactPoint;
	OutWallActor = WallHit.GetActor();

	if (OutWallActor == LastWallKickActor.Get()
		&& GetWorldTimeSafe() < SameWallReattachEndTime
		&& FVector::DotProduct(LastWallKickNormal, OutWallNormal) > 0.8f)
	{
		return false;
	}

	return true;
}

void UPlatformerTraversalMovementComponent::EnterLedgeHang(
	const FVector& HangLocation,
	const FVector& ClimbTargetLocation,
	const FVector& WallNormal)
{
	LedgeHangLocation = HangLocation;
	LedgeClimbTargetLocation = ClimbTargetLocation;
	LedgeWallNormal = WallNormal;
	Velocity = FVector::ZeroVector;
	StopMovementImmediately();
	UpdatedComponent->SetWorldLocation(LedgeHangLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::LedgeHang));
	UpdateTraversalState(EPlatformerTraversalState::LedgeHang);
}

void UPlatformerTraversalMovementComponent::StartLedgeClimb()
{
	if (TraversalState != EPlatformerTraversalState::LedgeHang)
	{
		return;
	}

	const float ClimbDistance = FVector::Dist(LedgeHangLocation, LedgeClimbTargetLocation);
	const float ClimbSpeed = GetLedgeSettings().ClimbSpeed;
	LedgeClimbStartTime = GetWorldTimeSafe();
	LedgeClimbDuration = ClimbSpeed > 0.0f ? FMath::Max(ClimbDistance / ClimbSpeed, 0.05f) : 0.05f;
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::LedgeClimb));
	UpdateTraversalState(EPlatformerTraversalState::LedgeClimb);

	if (TraversalConfig)
	{
		ExecuteCue(TraversalConfig->LedgeSettings.ClimbCueTag);
	}
}

void UPlatformerTraversalMovementComponent::DropFromLedge()
{
	if (TraversalState != EPlatformerTraversalState::LedgeHang && TraversalState != EPlatformerTraversalState::LedgeClimb)
	{
		return;
	}

	LedgeReleaseLockoutEndTime = GetWorldTimeSafe() + GetLedgeSettings().RegrabCooldown;
	UpdateTraversalState(EPlatformerTraversalState::None);
	SetMovementMode(MOVE_Falling);
}

void UPlatformerTraversalMovementComponent::PhysLedgeHang(float DeltaTime, int32 Iterations)
{
	Velocity = FVector::ZeroVector;
	UpdatedComponent->SetWorldLocation(LedgeHangLocation, false, nullptr, ETeleportType::TeleportPhysics);

	if (TraversalInputVector.Y > 0.5f)
	{
		StartLedgeClimb();
		return;
	}

	if (TraversalInputVector.Y < -0.5f)
	{
		DropFromLedge();
	}
}

void UPlatformerTraversalMovementComponent::PhysLedgeClimb(float DeltaTime, int32 Iterations)
{
	const float ElapsedTime = GetWorldTimeSafe() - LedgeClimbStartTime;
	const float Alpha = LedgeClimbDuration > 0.0f ? FMath::Clamp(ElapsedTime / LedgeClimbDuration, 0.0f, 1.0f) : 1.0f;
	const FVector TargetLocation = FMath::Lerp(LedgeHangLocation, LedgeClimbTargetLocation, Alpha);
	UpdatedComponent->SetWorldLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	Velocity = FVector::ZeroVector;

	if (Alpha >= 1.0f)
	{
		UpdateTraversalState(EPlatformerTraversalState::None);
		SetMovementMode(MOVE_Walking);
	}
}

void UPlatformerTraversalMovementComponent::EnterWallSlide(
	const FVector& WallNormal,
	const FVector& AnchorLocation,
	AActor* WallActor)
{
	WallSlideNormal = WallNormal;
	WallSlideAnchorLocation = AnchorLocation;
	WallSlideActor = WallActor;
	WallSlideStartTime = GetWorldTimeSafe();
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EPlatformerTraversalCustomMode::WallSlide));
	UpdateTraversalState(EPlatformerTraversalState::WallSlide);
}

void UPlatformerTraversalMovementComponent::ExitWallSlide(bool bSetToFallingMode)
{
	WallSlideActor = nullptr;
	WallSlideNormal = FVector::ZeroVector;
	WallSlideAnchorLocation = FVector::ZeroVector;
	UpdateTraversalState(EPlatformerTraversalState::None);

	if (bSetToFallingMode)
	{
		SetMovementMode(MOVE_Falling);
	}
}

void UPlatformerTraversalMovementComponent::PhysWallSlide(float DeltaTime, int32 Iterations)
{
	if (!bTraversalEnabled || !CharacterOwner)
	{
		ExitWallSlide(true);
		return;
	}

	const FPlatformerWallTraversalSettings& WallSettings = GetWallSettings();
	if (WallSettings.ClingTime > 0.0f && (GetWorldTimeSafe() - WallSlideStartTime) >= WallSettings.ClingTime)
	{
		ExitWallSlide(true);
		return;
	}

	FVector NewWallNormal = FVector::ZeroVector;
	FVector NewAnchorLocation = FVector::ZeroVector;
	AActor* NewWallActor = nullptr;
	if (!TryFindWallSlide(NewWallNormal, NewAnchorLocation, NewWallActor))
	{
		ExitWallSlide(true);
		return;
	}

	WallSlideNormal = NewWallNormal;
	WallSlideAnchorLocation = NewAnchorLocation;
	WallSlideActor = NewWallActor;

	const UCapsuleComponent* CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	const float CapsuleRadius = CapsuleComponent ? CapsuleComponent->GetScaledCapsuleRadius() : 0.0f;
	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FVector SlideLocation(
		WallSlideAnchorLocation.X + WallSlideNormal.X * (CapsuleRadius + 2.0f),
		LockedDepthY,
		CurrentLocation.Z);
	UpdatedComponent->SetWorldLocation(SlideLocation, false, nullptr, ETeleportType::TeleportPhysics);

	Velocity = FVector(0.0f, 0.0f, -WallSettings.SlideSpeed);
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector(0.0f, 0.0f, Velocity.Z * DeltaTime), UpdatedComponent->GetComponentQuat(), true, Hit);

	FFindFloorResult FloorResult;
	FindFloor(UpdatedComponent->GetComponentLocation(), FloorResult, false);
	if (FloorResult.IsWalkableFloor())
	{
		UpdateTraversalState(EPlatformerTraversalState::None);
		SetMovementMode(MOVE_Walking);
	}
}

void UPlatformerTraversalMovementComponent::PerformWallKick()
{
	if (!bTraversalEnabled)
	{
		return;
	}

	const FPlatformerWallTraversalSettings& WallSettings = GetWallSettings();
	const float JumpAngleRadians = FMath::DegreesToRadians(WallSettings.WallJumpAngleDegrees);
	const FVector JumpDirection =
		(WallSlideNormal * FMath::Cos(JumpAngleRadians) + FVector::UpVector * FMath::Sin(JumpAngleRadians)).GetSafeNormal();

	LastWallKickActor = WallSlideActor;
	LastWallKickNormal = WallSlideNormal;
	SameWallReattachEndTime = GetWorldTimeSafe() + WallSettings.SameWallReattachCooldown;
	ExitWallSlide(false);
	SetMovementMode(MOVE_Falling);
	Velocity = JumpDirection * WallSettings.WallJumpForce;
}

void UPlatformerTraversalMovementComponent::FinishSlideDash(bool bInterrupted, bool bForceFallingMode)
{
	SlideDashRecoveryEndTime = GetWorldTimeSafe() + GetSlideDashSettings().DashRecovery;
	Velocity = FVector(0.0f, 0.0f, bForceFallingMode ? Velocity.Z : 0.0f);
	UpdateTraversalState(EPlatformerTraversalState::None);
	RestoreCapsuleSizeIfPossible();

	if (bForceFallingMode)
	{
		SetMovementMode(MOVE_Falling);
	}
	else
	{
		SetMovementMode(MOVE_Walking);
	}
}

void UPlatformerTraversalMovementComponent::PhysSlideDash(float DeltaTime, int32 Iterations)
{
	if (!bTraversalEnabled)
	{
		FinishSlideDash(true, !IsMovingOnGround());
		return;
	}

	const FPlatformerSlideDashSettings& SlideDashSettings = GetSlideDashSettings();
	FFindFloorResult FloorResult;
	FindFloor(UpdatedComponent->GetComponentLocation(), FloorResult, false);
	if (!FloorResult.IsWalkableFloor())
	{
		FinishSlideDash(true, true);
		return;
	}

	const float ElapsedTime = GetWorldTimeSafe() - SlideDashStartTime;
	const float TravelDistance = FMath::Abs(UpdatedComponent->GetComponentLocation().X - SlideDashStartLocation.X);
	if (ElapsedTime >= SlideDashSettings.DashDuration || TravelDistance >= SlideDashSettings.DashDistance)
	{
		FinishSlideDash(false, false);
		return;
	}

	const float RemainingDistance = FMath::Max(SlideDashSettings.DashDistance - TravelDistance, 0.0f);
	const float FrameDistance = FMath::Min(SlideDashSettings.DashSpeed * DeltaTime, RemainingDistance);
	if (FrameDistance <= KINDA_SMALL_NUMBER)
	{
		FinishSlideDash(false, false);
		return;
	}

	FVector DashDelta(SlideDashDirectionSign * FrameDistance, 0.0f, 0.0f);
	if (FloorResult.HitResult.IsValidBlockingHit())
	{
		DashDelta = FVector::VectorPlaneProject(DashDelta, FloorResult.HitResult.ImpactNormal);
		DashDelta.Y = 0.0f;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(DashDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		const bool bBlockedByWall = FMath::Abs(Hit.Normal.X) > PlatformerTraversalPrivate::VerticalWallNormalThreshold;
		if (bBlockedByWall)
		{
			FinishSlideDash(false, false);
			return;
		}
	}

	Velocity = FVector(SlideDashDirectionSign * SlideDashSettings.DashSpeed, 0.0f, 0.0f);

	FindFloor(UpdatedComponent->GetComponentLocation(), FloorResult, false);
	if (!FloorResult.IsWalkableFloor())
	{
		FinishSlideDash(true, true);
	}
}
