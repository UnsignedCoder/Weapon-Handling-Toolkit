// WeaponHandlingComponent provides a flexible and modular way to handle different weapon behaviors
// in a game environment. It abstracts weapon firing mechanics to support both immediate (raycast) 
// and physical (projectile) weapon systems while maintaining consistency in the firing interface.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ActorComponent.h"
#include "WeaponHandlingComponent.generated.h"

class UInputMappingContext;
class ABaseWeapon;

/**
 * Signals completion of weapon system initialization.
 * 
 * @param WeaponHandlingComponent The fully configured component instance
 * @param ComponentOwner The actor possessing this weapon system
 * 
 * @note Triggered after successful input system configuration
 * @warning Not fired if controller or input systems fail to initialize
 * @see InitializeWeaponHandlingComponent()
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponHandlingComponentInitialized,
	UWeaponHandlingComponent*, WeaponHandlingComponent,
	AActor*, ComponentOwner);

/**
 * Centralized weapon management system for combat actors.
 *
 * Provides:
 * - Unified firing interface for diverse weapon types
 * - Input-driven action system
 * - Extensible behavior framework
 * - Network-compatible architecture
 *
 * @note Designed for inheritance - override key methods for specialized weapons
 * @warning Requires proper input configuration before use
 * @see ABaseWeapon for implementable weapon types
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class WEAPONHANDLINGMODULE_API UWeaponHandlingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponHandlingComponent();

	/**
	* Executes weapon firing sequence.
	* 
	* @note Delegates actual firing logic to ActiveWeapon
	* @remark Handles instigator and ownership setup
	* @see ABaseWeapon::FireWeapon()
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
	void WeaponAttack();
	
	/**
	 * Notification system for initialization completion.
	 * 
	 * @note Binds automatically to Blueprint event graphs
	 * @remark Fires immediately if component already initialized
	 * @see InitializeWeaponHandlingComponent()
	 */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponHandlingComponentInitialized OnWeaponHandlingComponentInitialized;

protected:
	/**
	 * Activates weapon system when gameplay begins.
	 * 
	 * @note Automatically triggers full initialization
	 * @see InitializeWeaponHandlingComponent()
	 */
	virtual void BeginPlay() override;

	/**
	 * Establishes control bindings for weapon actions.
	 * 
	 * @param EnhancedInputComponent Input system to configure
	 * @note Override to implement custom control schemes
	 * @warning Base version only binds FireWeaponAction
	 */
	virtual void SetupInputBindings(UEnhancedInputComponent* EnhancedInputComponent);

	/**
	 * Completes weapon system activation sequence.
	 * 
	 * Handles:
	 * - Control scheme validation
	 * - Input pipeline setup
	 * - Event system preparation
	 * 
	 * @note Broadcasts OnWeaponHandlingComponentInitialized when ready
	 * @warning Requires valid player controller hierarchy
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Initialization")
	virtual void InitializeWeaponHandlingComponent();

	
	/**
	 * Prepares a weapon for use by configuring ownership and attachment.
	 * 
	 * @param NewWeapon Weapon instance to initialize
	 * @note Performs:
	 *       - Character ownership assignment
	 *       - Mesh attachment to socket
	 *       - Collision ignore setup
	 * @warning Requires valid OwningCharacter
	 */
	void InitializeWeapon(ABaseWeapon* NewWeapon);

	/**
	 * Releases control of a weapon while maintaining its world position.
	 * 
	 * @note Preserves weapon's transform for natural dropping behavior
	 * @see EquipWeapon() for the inverse operation
	 */
	void DropWeapon();
	
	/**
	 * Attempts to equip a weapon from nearby environment or inventory.
	 * 
	 * @return True if weapon was successfully equipped
	 * @note Handles both initial equips and weapon swapping
	 * @remark Uses sphere overlap with 100 unit radius by default
	 */
	virtual void EquipWeapon();

	virtual void UnequipWeapon();

public:
	/**
	 * Processes continuous weapon state updates.
	 * 
	 * @param DeltaTime Frame time increment
	 * @param TickType Update type being processed
	 * @param ThisTickFunction Tick metadata
	 * 
	 * @note Override for weapon-specific frame logic
	 * @remark Typical uses include cooldowns and state machines
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Cached reference to owning character for frequent access
	UPROPERTY()
	TObjectPtr<ACharacter> OwningCharacter = nullptr;

	/**
	 * Available weapon inventory.
	 * 
	 * @note Managed through internal weapon switching logic
	 * @see ActiveWeapon for currently equipped weapon
	 */
	UPROPERTY()
	TArray<ABaseWeapon*> Weapons;

	// Cached input component reference for binding management
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	// Socket name on character mesh where weapons attach
	UPROPERTY(EditAnywhere, Category = "Weapon|Configuration", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName WeaponAttachmentSocket;

	UPROPERTY(EditAnywhere, Category = "Weapon|Configuration", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float WeaponDetectionRange = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon|Configuration", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	
	/**
	 * Control scheme definition for weapon actions.
	 * 
	 * @note Must contain all weapon-related input actions
	 * @warning Not setting this prevents input binding
	 * @see FireWeaponAction for required minimum configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Weapon|Input")
	TObjectPtr<UInputMappingContext> WeaponMappingContext;

	/**
	 * Currently equipped weapon instance.
	 * 
	 * @note Managed internally during weapon switching
	 * @see Weapons inventory for available options
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = true) )
	TObjectPtr<ABaseWeapon> ActiveWeapon;

	/**
	 * Primary fire trigger action.
	 * 
	 * @note Base implementation requires this minimum binding
	 * @remark Extend with additional actions in child classes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Input", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Input", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EquipWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Input", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UnequipWeaponAction;

	
	// Implementation Notes:
	// - All weapon actions flow through WeaponAttack() for consistent behavior
	// - Input binding happens during InitializeWeaponHandlingComponent()
	// - Weapon state changes should go through Initialize/DropWeapon methods
	// - The component maintains minimal weapon state - most logic lives in ARangedWeapon
};