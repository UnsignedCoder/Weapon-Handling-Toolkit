// Modular weapon component for handling different weapon types and firing behaviors

#include "Component/WeaponHandlingComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Logging.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon/RangedWeapon.h"


/**
 * Creates a weapon handling system with safe defaults.
 * 
 * @note Designed for extension by specific weapon types
 * @warning Requires explicit initialization before use
 * @see InitializeWeaponHandlingComponent()
 */
UWeaponHandlingComponent::UWeaponHandlingComponent(): WeaponMappingContext(nullptr) {
	PrimaryComponentTick.bCanEverTick = true;

	// Cache owner as character for frequent use
	OwningCharacter = Cast<ACharacter>(GetOwner());
}


/**
 * Initiates weapon system activation when gameplay starts.
 * 
 * @note Triggers full component initialization
 * @remark Safe for repeated calls during lifetime
 * @see InitializeWeaponHandlingComponent()
 */
void UWeaponHandlingComponent::BeginPlay() {
	Super::BeginPlay();

	// Double-check owner assignment as BeginPlay may be called before owner is fully set
	if (!OwningCharacter) {
		OwningCharacter = Cast<ACharacter>(GetOwner());
	}
	
	// Early out if we still don't have a valid owner
	if (!OwningCharacter) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No valid character found"));
		return;
	}
	InitializeWeaponHandlingComponent();
}


/**
 * Processes continuous weapon state updates.
 * 
 * @param DeltaTime Frame time increment  
 * @param TickType Type of update being performed
 * @param ThisTickFunction Specific tick being processed
 * 
 * @note Base implementation empty - override for weapon behaviors
 * @remark Typical uses include:
 *         - Cooldown tracking
 *         - Charge mechanics
 *         - State machine updates
 */
void UWeaponHandlingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


/**
 * Completes weapon system setup and enables player control.
 * 
 * Performs:
 * - Controller validation
 * - Input system configuration
 * - Control binding
 * - Initialization event signaling
 * 
 * @note Emits OnWeaponHandlingComponentInitialized upon success
 * @warning Requires:
 *          - Valid player controller hierarchy
 *          - Configured input mapping context
 * @remark Logs detailed warnings for all setup failures
 */
void UWeaponHandlingComponent::InitializeWeaponHandlingComponent() {
	// Get controller through instigator chain rather than direct owner
	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PlayerController) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No valid PlayerController found"));
		return;
	}

	// Need local player for input subsystem access
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No LocalPlayer found"));
		return;
	}

	// Set up enhanced input system if available
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
		Subsystem->AddMappingContext(WeaponMappingContext, 0);
		UE_LOG(LogTemp, Log, TEXT("WeaponHandlingComponent: Added mapping context"));
	}

	// Cache input component for binding
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (EnhancedInputComponent) {
		SetupInputBindings(EnhancedInputComponent);
	} else {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No EnhancedInputComponent found"));
	}

	// Ensure we have a valid socket for weapon attachment
	checkf(!WeaponAttachmentSocket.IsNone(), TEXT("WeaponHandlingComponent: No valid weapon attachment socket found"));
	
	// Notify other components that initialization is complete
	OnWeaponHandlingComponentInitialized.Broadcast(this, GetOwner());

}


/**
 * Establishes control bindings for weapon actions.
 * 
 * @param InputComponent Input system component to configure
 * @note Base implementation binds only primary fire action
 * @remark Override to add:
 *         - Alternate fire modes
 *         - Reload actions  
 *         - Weapon switching
 * @warning Must call parent implementation when overriding
 */
void UWeaponHandlingComponent::SetupInputBindings( UEnhancedInputComponent* InputComponent ) {
	// Bind primary fire to Triggered event (continuous while pressed)
	InputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &UWeaponHandlingComponent::WeaponAttack);
	InputComponent->BindAction(EquipWeaponAction, ETriggerEvent::Started, this, &UWeaponHandlingComponent::EquipWeapon);
	InputComponent->BindAction(UnequipWeaponAction, ETriggerEvent::Started, this, &UWeaponHandlingComponent::UnequipWeapon);
}


/**
 * Executes weapon firing sequence.
 * 
 * @note Coordinates with ActiveWeapon to:
 *        - Process hit detection
 *        - Play visual effects
 *        - Apply damage
 * @remark Handles ownership and instigator setup
 * @warning Requires:
 *          - Valid ActiveWeapon
 *          - Owner must be a Character
 */
void UWeaponHandlingComponent::WeaponAttack() {
	// Delegate actual firing logic to the weapon itself
	if (!ActiveWeapon) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No active weapon found"));
	}

	if (!OwningCharacter) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No valid character found"));
		return;
	}

	if (FireWeaponMontage) {
		OwningCharacter->GetMesh()->GetAnimInstance()->Montage_Play(FireWeaponMontage);
	}
	
	GEngine->AddOnScreenDebugMessage(11, 5.f, FColor::Red, TEXT("WeaponAttack called"));

	FHitResult WeaponFireHitResult;
	const ACharacter* InstigatorCharacter = OwningCharacter;

	// Pass controller rather than character for damage attribution
	ActiveWeapon->LaunchAttack(WeaponFireHitResult, InstigatorCharacter->GetController());
}


/**
 * Prepares a weapon for use by the character.
 * 
 * @param NewWeapon The weapon instance to initialize
 * @note Performs:
 *       - Ownership assignment
 *       - Mesh attachment
 *       - Collision setup
 * @warning Invalidates if:
 *          - Weapon is null
 *          - Owner character missing
 */
void UWeaponHandlingComponent::InitializeWeapon(ABaseWeapon* NewWeapon) {
	ActiveWeapon = NewWeapon;

	if (!ActiveWeapon) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No valid weapon found"));
		return;
	}

	if (OwningCharacter) {
		// Set ownership chain
		ActiveWeapon->SetOwningCharacter(OwningCharacter);
		ActiveWeapon->SetActorEnableCollision(false);
		
		// Attach weapon mesh to character's mesh at specified socket
		if (ActiveWeapon->GetWeaponMesh()) {
			ActiveWeapon->GetWeaponMesh()->AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachmentSocket);
		}
		
		// Configure collision ignores to prevent self-hits
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Emplace(GetOwner());
		ActorsToIgnore.Emplace(NewWeapon);

		ActiveWeapon->AddActorToIgnore(ActorsToIgnore);
	}
}


/**
 * Releases control of the specified weapon.
 * 
 * @note Performs:
 *       - Physical detachment
 *       - Reference clearing
 * @remark Maintains world position during detachment
 */
void UWeaponHandlingComponent::DropWeapon() {
	// Detach while keeping world position for natural dropping
	ActiveWeapon->Fall();
	
	ActiveWeapon = nullptr;
}


/**
 * Attempts to equip a nearby weapon.
 * 
 * @return True if weapon was successfully equipped
 * @note Searches for weapons within interaction range
 * @remark Handles:
 *         - First-time equips
 *         - Weapon swapping
 *         - Inventory management
 * @warning Sphere overlap parameters hardcoded to 100 units
 */
void UWeaponHandlingComponent::EquipWeapon() {
	// Set up ignore list for overlap check
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Emplace(GetOwner());
	if (ActiveWeapon) {
		ActorsToIgnore.Emplace(ActiveWeapon);
	}
	
	TArray<AActor*> ActorsInRange;
	
	// Find all weapons within 100 units
	UKismetSystemLibrary::SphereOverlapActors( GetWorld(),GetOwner()->GetActorLocation(),WeaponDetectionRange,TArray<TEnumAsByte<EObjectTypeQuery>>(), ABaseWeapon::StaticClass(), ActorsToIgnore,ActorsInRange );
	UKismetSystemLibrary::DrawDebugSphere( GetWorld(), GetOwner()->GetActorLocation(), WeaponDetectionRange, 12, FLinearColor::Red, 0.5f, 0.5f );

	if (ActorsInRange.Num() <= 0 && !ActiveWeapon) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("WeaponHandlingComponent: No weapons found in range"));
		return;
	}
	// Filter to just weapon actors
	TArray<ABaseWeapon*> WeaponsInRange;
	for (AActor* Actor : ActorsInRange) {
		if (ABaseWeapon* Weapon = Cast<ABaseWeapon>(Actor)) {
			WeaponsInRange.Emplace(Weapon);
		}
	}
	
	// Handle first equip case 
	if (!ActiveWeapon && WeaponsInRange.Num() > 0) {
		ActiveWeapon = WeaponsInRange[0];
		WeaponsInRange.Empty();
		
		if (ActiveWeapon) {
			InitializeWeapon(ActiveWeapon);
			return;
		}
	}

	// Handle weapon swap case
	if (ActiveWeapon && WeaponsInRange.Num() > 0) {
		DropWeapon();
		ActiveWeapon = WeaponsInRange[0];
		WeaponsInRange.Empty();
		
		InitializeWeapon(ActiveWeapon);
		return;
	}

	if (ActiveWeapon && WeaponsInRange.Num() <= 0) {
		// No weapons found, drop current weapon
		DropWeapon();
	}
}

void UWeaponHandlingComponent::UnequipWeapon() {
	DropWeapon();
}