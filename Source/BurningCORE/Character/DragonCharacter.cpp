#include "Character/DragonCharacter.h"
#include "Character/SideViewMovementComponent.h"
#include "Character/DragonFormComponent.h"
#include "Character/DragonOverdriveComponent.h"
#include "GAS/Attributes/DragonAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

ADragonCharacter::ADragonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USideViewMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Note: USideViewMovementComponent is created by ObjectInitializer override
	
	// Create Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Look from side
	CameraBoom->bDoCollisionTest = false; // Fixed camera doesn't need collision
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create Gameplay Components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true); // Always good practice for GAS, even in singleplayer
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UDragonAttributeSet>(TEXT("AttributeSet"));
	
	FormComponent = CreateDefaultSubobject<UDragonFormComponent>(TEXT("FormComponent"));
	OverdriveComponent = CreateDefaultSubobject<UDragonOverdriveComponent>(TEXT("OverdriveComponent"));
}

UAbilitySystemComponent* ADragonCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADragonCharacter::ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult)
{
	if (!AbilitySystemComponent || !DamageSpec.IsValid()) return;
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
}

bool ADragonCharacter::IsAlive() const
{
	return AttributeSet && AttributeSet->GetHealth() > 0.0f;
}

void ADragonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAbilities(DefaultAbilitySet);
	}
}

void ADragonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Note: Enhanced input bindings are done in PlayerController or specific input components
	// but the ability activation hooks will sit here bridging actions to GAS tags/specs.
}

void ADragonCharacter::InitializeAbilities(const UDragonAbilitySet* AbilitySet)
{
	// Usually iterates over AbilitySet and grants abilities
}
