// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "RayCastWeapon.generated.h"

/**
 * Implements hit-scan weapon behavior using precise raycasting mechanics.
 * 
 * Features:
 * - Instant hit detection with sub-frame accuracy
 * - Dual-trace system (screen-space and weapon-barrel precise)
 * - Realistic bullet simulation from muzzle position
 * - Configurable ballistic properties
 *
 * @note Inherits core weapon functionality from AWeapon
 * @warning Requires:
 *          - ECC_Visibility channel setup
 *          - Properly configured weapon socket
 * @see AWeapon for base weapon functionality
 */
UCLASS()
class WEAPONHANDLINGMODULE_API ARayCastWeapon : public ARangedWeapon
{
	GENERATED_BODY()

public:
	/**
	 * Constructs raycast weapon with ballistic defaults.
	 * 
	 * @note Initializes hit-scan specific parameters
	 * @remark Sets up tick behavior for frame-accurate traces
	 */
	ARayCastWeapon();

protected:
	/**
	 * Validates weapon systems when gameplay begins.
	 * 
	 * @note Performs critical component checks
	 * @warning Verifies:
	 *          - Weapon mesh existence
	 *          - Barrel socket configuration
	 *          - Valid collision profiles
	 */
	virtual void BeginPlay() override;

public:
	/**
	 * Processes continuous weapon simulation updates.
	 * 
	 * @param DeltaTime Frame time increment
	 * @note Handles:
	 *       - Cooldown timers
	 *       - Visual effect updates
	 *       - State transitions
	 */
	virtual void Tick(float DeltaTime) override;

protected:
	/**
	 * Coordinates complete firing sequence for raycast weapons.
	 * 
	 * @param IgnoredActors Entities excluded from hit detection
	 * @param WeaponFireHitResult Output container for hit data
	 * @param InstigatorController Responsible controller reference
	 * 
	 * @note Execution flow:
	 *       1. Determines trace method (screen/weapon)
	 *       2. Performs collision detection
	 *       3. Processes hit/miss results
	 * @see WeaponTrace()
	 * @see ScreenTrace()
	 */
	virtual void ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) override;

	/**
	 * Performs viewport-centered targeting trace.
	 * 
	 * @param ScreenTraceHitResult Output container for trace results
	 * @param IgnoredActors Entities excluded from detection
	 * @return True if collision occurred along view axis
	 * 
	 * @note Uses camera perspective for initial targeting
	 * @remark Typical first-pass for weapon tracing
	 * @warning Requires active player controller
	 */
	bool ScreenTrace(FHitResult& ScreenTraceHitResult, const TArray<AActor*>& IgnoredActors) const;

	/**
	 * Executes precise barrel-to-target ballistic trace.
	 * 
	 * @param WeaponTraceHitResult Output container for trace results
	 * @param IgnoredActors Entities excluded from detection
	 * @return True if collision occurred along bullet path
	 * 
	 * @note Uses weapon socket as origin for realistic trajectories
	 * @remark Typically follows screen trace for precision adjustment
	 * @warning Requires properly named barrel socket
	 */
	bool WeaponTrace(FHitResult& WeaponTraceHitResult, const TArray<AActor*>& IgnoredActors) const;
};