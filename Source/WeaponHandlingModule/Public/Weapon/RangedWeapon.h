// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "RangedWeapon.generated.h"

class UBoxComponent;
/**
 * Defines tactical firing behaviors that determine weapon rhythm and control requirements.
 * Each mode represents a distinct combat philosophy with unique tradeoffs.
 */
UENUM(BlueprintType)
enum class EFiringMode : uint8 {
	/**
	 * Precision-oriented single discharges
	 * @note Ideal for marksmanship and ammo conservation
	 * @remark Requires player to re-trigger for each shot
	 */
	EFM_Single UMETA(DisplayName = "Single"),

	/**
	 * Controlled burst sequences
	 * @note Balances accuracy with short-range effectiveness
	 * @warning Automatically enforces burst cooldown period
	 */
	EFM_Burst UMETA(DisplayName = "Burst"),

	/**
	 * Sustained automatic fire
	 * @note Maximizes close-quarters firepower
	 * @warning Requires careful recoil management
	 */
	EFM_Automatic UMETA(DisplayName = "Automatic")
};

/**
 * Governs projectile distribution patterns for varied engagement styles.
 * Enables distinct weapon personalities through configuration rather than inheritance.
 */
UENUM(BlueprintType)
enum class EShotPattern : uint8 {
	/**
	 * Pinpoint accurate single projectile
	 * @note Standard for precision weapons
	 * @see EFM_Single for natural pairing
	 */
	ESP_Single UMETA(DisplayName = "Single"),

	/**
	 * Scatter-based multi-projectile cone
	 * @note Creates area denial effect
	 * @remark Effective at close ranges
	 */
	ESP_Spread UMETA(DisplayName = "Spread"),
};

/**
 * Complete weapon configuration package.
 * Serves as data-driven blueprint for weapon behavior and capabilities.
 *
 * @note Designed for easy balancing via editor
 * @see AWeapon for runtime implementation
 */
USTRUCT(BlueprintType)
struct FWeaponData {
	GENERATED_BODY()

	// Initialize with sensible defaults
	FWeaponData() : FiringMode(EFiringMode::EFM_Single), WeaponFireRate(0.2f), MaxBurstShotCount(3), BurstShotCooldown(0.5f), WeaponRange(10000.0f), WeaponDamage(10.0f),
	                bShouldPerformWeaponTraceTest(false), ShotPattern(EShotPattern::ESP_Single), PelletsPerBullet(1), MinimumSpreadRange(-150), MaximumSpreadRange(-MinimumSpreadRange),
	                MuzzleFlash(nullptr), BeamTrail(nullptr), ImpactParticle(nullptr), WeaponFireSound(nullptr), CurrentAmmoCount(500), MaxAmmoCount(500), CurrentClipCount(50), MaxClipCount(50) {}

	// ------------------------------
	// Firing Mechanics
	// ------------------------------

	/** Core tactical behavior defining trigger response */
	UPROPERTY(EditAnywhere, Category = "Weapon | Firing Mechanics")
	EFiringMode FiringMode;

	/** Minimum delay between shots (seconds) */
	UPROPERTY(EditAnywhere, Category = "Weapon | Firing Mechanics")
	float WeaponFireRate;

	/** Shots per burst sequence */
	UPROPERTY(EditAnywhere, Category = "Weapon | Firing Mechanics", meta=(EditCondition = "FiringMode == EFiringMode::EFM_Burst"))
	uint8 MaxBurstShotCount;

	/** Recovery period between bursts (seconds) */
	UPROPERTY(EditAnywhere, Category = "Weapon | Firing Mechanics", meta=(EditCondition = "FiringMode == EFiringMode::EFM_Burst"))
	float BurstShotCooldown;

	// ------------------------------
	// Combat Parameters
	// ------------------------------

	/** Maximum effective engagement range (cm) */
	UPROPERTY(EditAnywhere, Category = "Weapon | Combat Parameters")
	float WeaponRange;

	/** Base damage per successful hit */
	UPROPERTY(EditAnywhere, Category = "Weapon | Combat Parameters")
	float WeaponDamage;

	/** Enables barrel-origin hit detection */
	UPROPERTY(EditAnywhere, Category = "Weapon | Combat Parameters")
	bool bShouldPerformWeaponTraceTest;

	// ------------------------------
	// Shot Characteristics
	// ------------------------------

	/** Projectile distribution strategy */
	UPROPERTY(EditAnywhere, Category = "Weapon | Shot Characteristics")
	EShotPattern ShotPattern;

	/** Projectiles emitted per trigger pull */
	UPROPERTY(EditAnywhere, Category = "Weapon | Shot Characteristics", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
	int PelletsPerBullet;

	/** Minimum spread angle (degrees) */
	UPROPERTY(EditAnywhere, Category = "Weapon | Shot Characteristics", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
	float MinimumSpreadRange;

	/** Maximum spread angle (degrees) */
	UPROPERTY(EditAnywhere, Category = "Weapon | Shot Characteristics", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
	float MaximumSpreadRange;

	// ------------------------------
	// Visual Feedback
	// ------------------------------

	/** Muzzle discharge effect */
	UPROPERTY(EditAnywhere, Category = "Weapon | Visual Feedback")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	/** Projectile trail effect */
	UPROPERTY(EditAnywhere, Category = "Weapon | Visual Feedback")
	TObjectPtr<UParticleSystem> BeamTrail;

	/** Surface impact effect */
	UPROPERTY(EditAnywhere, Category = "Weapon | Visual Feedback")
	TObjectPtr<UParticleSystem> ImpactParticle;

	// ------------------------------
	// Audio Feedback
	// ------------------------------

	/** Discharge sound signature */
	UPROPERTY(EditAnywhere, Category = "Weapon | Audio Feedback")
	TObjectPtr<USoundBase> WeaponFireSound;

	// ------------------------------
	// Ammunition
	// ------------------------------

	/** Current available ammunition */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentAmmoCount;

	/** Maximum ammunition capacity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxAmmoCount;

	/** Current rounds in active clip */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentClipCount;

	/** Maximum clip capacity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxClipCount;
};

/**
 * Configurable weapon base class implementing core combat behaviors.
 * Uses data-driven design to support diverse weapon types through composition.
 *
 * @note Designed for extension via Blueprints
 * @see FWeaponData for configuration options
 */
UCLASS(Abstract)
class WEAPONHANDLINGMODULE_API ARangedWeapon : public ABaseWeapon {
	GENERATED_BODY()

public:
	ARangedWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	/**
	 * Primary combat interface triggering all firing behaviors
	 * @param WeaponFireHitResult - Output for hit detection data
	 * @param InstigatorController - Responsible controller for attribution
	 * 
	 * @note Routes to appropriate firing mode implementation
	 * @remark Handles all visual/audio feedback coordination
	 */
	virtual void LaunchAttack( FHitResult& WeaponFireHitResult, AController* InstigatorController ) override;




	/** 
	 * Resets single-shot readiness state
	 * 
	 * @note Called automatically by fire rate timer
	 * @warning Only affects single-fire mode weapons
	 */
	void ResetShouldFireSingleShot();


protected:
	/**
	 * Coordinates complete firing sequence with visual/audio feedback
	 * @param IgnoredActors - Entities excluded from collision
	 * @param WeaponFireHitResult - Output for hit analysis
	 * @param InstigatorController - Responsible controller
	 * 
	 * @note Base implementation handles:
	 *       - Muzzle flash effects
	 *       - Bullet trail visualization
	 * @remark Override to implement custom hit detection
	 */
	virtual void ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController );

	/**
	 * Routes to appropriate firing pattern implementation
	 * @param IgnoredActors - Entities excluded from collision
	 * @param WeaponFireHitResult - Output for hit analysis
	 * @param InstigatorController - Responsible controller
	 * 
	 * @note Handles:
	 *       - Single vs spread shot patterns
	 *       - Weapon firing sound
	 * @see WeaponData.ShotPattern for configuration
	 */
	void ExecuteWeaponFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController );

	/**
	 * Manages burst fire sequence with automatic cooldown
	 * @param IgnoredActors - Entities excluded from collision
	 * @param WeaponFireHitResult - Output for hit analysis
	 * @param InstigatorController - Responsible controller
	 * 
	 * @note Tracks burst progress internally
	 * @remark Enforces cooldown between burst sequences
	 */
	void ExecuteBurstFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController );

	/**
	 * Single-shot precision firing
	 * @param IgnoredActors - Entities excluded from collision
	 * @param WeaponFireHitResult - Output for hit analysis
	 * @param InstigatorController - Responsible controller
	 * 
	 * @note Enforces mandatory trigger reset between shots
	 * @remark Most basic firing implementation
	 */
	void ExecuteSingleFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController );

	/**
	 * Sustained automatic fire
	 * @param IgnoredActors - Entities excluded from collision
	 * @param WeaponFireHitResult - Output for hit analysis
	 * @param InstigatorController - Responsible controller
	 * 
	 * @note Manages rate-of-fire internally between shots
	 * @warning Can rapidly consume ammunition
	 */
	void ExecuteAutomaticFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController );

	/**
	 * Visualizes projectile path between muzzle and impact point
	 * @param TraceHitResult - Validated impact data for visualization
	 * @param Mesh - Weapon visual reference for correct positioning
	 * 
	 * @note Requires configured BeamTrail particle system
	 * @remark Handles both hit and miss visualization cases
	 */
	void SpawnBulletTrail(const FHitResult& TraceHitResult, const USkeletalMeshComponent* Mesh) const;

	/** 
	 * Resets all firing cooldown states to allow new shots
	 * 
	 * @note Called automatically by weapon timers
	 * @warning Only affects weapons currently in cooldown
	 */
	void ResetShouldFireWeapon();

	/** 
	 * Concludes burst sequence recovery period
	 * 
	 * @note Managed internally by burst fire logic
	 * @warning Duration controlled by WeaponData.BurstShotCooldown
	 */
	void ResetBurstShotCooldown();

public:


protected:
	/** Projectile emission origin point */
	UPROPERTY(EditAnywhere, Category = "Weapon | Configuration")
	FName WeaponBarrelSocket;

private:
	// ------------------------------
	// Internal State
	// ------------------------------

	/** Debug visualization endpoint */
	UPROPERTY()
	FVector FireWeaponTraceEndLocation;

	/** Current burst progress counter */
	UPROPERTY()
	uint8 CurrentBurstShotCount;

	/** Global fire readiness flag */
	UPROPERTY()
	bool bShouldFireWeapon;

	/** Burst cooldown state flag */
	UPROPERTY()
	bool bShouldBurstShotCooldown;

	/** Rate-of-fire timer control */
	UPROPERTY()
	FTimerHandle WeaponFireTimer;

	/** Burst recovery timer control */
	UPROPERTY()
	FTimerHandle WeaponBurstCooldownTimer;

protected:
	/** Complete behavior configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon | Configuration", meta = (AllowPrivateAccess = "true"))
	FWeaponData WeaponData;

	// Implementation Notes:
	// 1. Firing flow: WeaponAttack -> [ModeHandler] -> ExecuteWeaponFire -> ShootWeapon
	// 2. Visual effects are managed in ShootWeapon
	// 3. All timing operations use the weapon's configured rates
	// 4. State flags prevent illegal firing sequences
	// 5. Ignored actors list prevents self-collisions
};