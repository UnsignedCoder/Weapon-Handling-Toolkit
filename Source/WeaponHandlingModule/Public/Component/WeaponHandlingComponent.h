// WeaponHandlingComponent provides a flexible and modular way to handle different weapon behaviors
// in a game environment. It abstracts weapon firing mechanics to support both immediate (raycast) 
// and physical (projectile) weapon systems while maintaining consistency in the firing interface.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponHandlingComponent.generated.h"

/**
 * Defines the core firing mechanism of the weapon to support different gameplay styles
 * and performance requirements. Raycast weapons are better for fast-paced, precise combat
 * while projectile weapons add physicality and tactical depth.
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_Raycast     UMETA(DisplayName = "Raycast"),     // Instant-hit scanning for precise weapons
    EWT_Projectile  UMETA(DisplayName = "Projectile")   // Physical projectiles for realistic ballistics
};

/**
 * Controls how the weapon responds to player input, allowing for different weapon archetypes
 * and playstyles. This affects both the feel of the weapon and its tactical usage.
 */
UENUM(BlueprintType) 
enum class EFiringMode : uint8
{
    EFM_Single      UMETA(DisplayName = "Single"),      // Precision-focused, controlled firing
    EFM_Burst       UMETA(DisplayName = "Burst"),       // Balance between control and volume of fire
    EFM_Automatic   UMETA(DisplayName = "Automatic")    // Sustained fire for suppression and close quarters
};

/**
 * Defines how multiple shots are distributed when fired. This affects weapon effectiveness
 * at different ranges and situations, creating distinct weapon behaviors without needing
 * separate weapon classes.
 */
UENUM(BlueprintType)
enum class EShotPattern : uint8
{
    ESP_Single        UMETA(DisplayName = "Single"),      // Direct, precise shot for rifles and pistols
    ESP_Spread        UMETA(DisplayName = "Spread"),      // Cone-shaped spread for shotguns and similar
    ESP_Cluster       UMETA(DisplayName = "Cluster", Hidden),    // Reserved for future clustered shot patterns
    ESP_MultiShot     UMETA(DisplayName = "Multishot", Hidden)  // Reserved for future multi-projectile weapons
};

/**
 * WeaponHandlingComponent handles all aspects of weapon firing mechanics, including
 * hit detection, shot patterns, and firing modes. It's designed to be modular and reusable
 * across different weapon types while maintaining consistent behavior and networking support.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONHANDLINGMODULE_API UWeaponHandlingComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UWeaponHandlingComponent();

    /**
     * Primary weapon firing function that handles all weapon types and configurations.
     * Centralizes weapon logic to ensure consistent behavior across different weapon types
     * and simplifies the implementation of new weapons.
     * 
     * @param ActorsToIgnore Actors to exclude from hit detection (typically the shooter and their team)
     * @param Mesh The weapon mesh used for barrel position and direction
     * @param WeaponFIreHitResult Contains hit information when the weapon successfully hits a target
     * @param WeaponRangeEndLocation The end point of the weapon's range, used for effects and debugging
     * @param InstigatorController
     */
    UFUNCTION(BlueprintCallable)
    void FireWeapon( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController );

protected:
    virtual void BeginPlay() override;

    /**
     * Performs the initial trace from screen center to determine the player's intended target.
     * This method supports both hip-fire and aimed shooting while maintaining consistent
     * accuracy and feel across different camera perspectives.
     * 
     * Returns false if no collision is detected within weapon range.
     */
    bool ScreenTrace(const TArray<AActor*>& ActorsToIgnore, FHitResult& TraceHitResult, FVector& TraceEndLocation) const;
    

    /**
     * Handles the actual weapon discharge logic, separating it from the public interface
     * to allow for future expansion (like weapon states, ammo checks, etc.) without
     * modifying the core firing logic.
     */
    void ExecuteWeaponFire( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) const;

    /**
     * Performs a secondary trace from the weapon barrel to prevent shooting through walls
     * when using third-person or over-the-shoulder cameras. This ensures realistic weapon
     * behavior regardless of camera position while maintaining good game feel.
     */
    bool WeaponTrace(const TArray<AActor*>& ActorsToIgnore, const USkeletalMeshComponent* Mesh, FHitResult& TraceHitResult, FVector& TraceEndLocation) const;
    
public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SpawnBulletTrail( const FHitResult& TraceHitResult, const USkeletalMeshComponent* Mesh ) const;
    
    /**
     * Applies damage to an actor hit by a weapon.
     * 
     * @param WeaponFIreHitResult Reference to the hit result containing information about what was hit
     * @param InstigatorController Pointer to the controller responsible for causing the damage
     * 
     * @note This function only applies damage if the hit actor implements the IDamageInterface
     * @see IDamageInterface::ApplyDamage
     */
    void DamageActor(FHitResult& WeaponFIreHitResult, AController* InstigatorController) const;

private:
    /**
     * Core Weapon Properties
     */
    /** The type of weapon (e.g., raycast, projectile, melee) */
    UPROPERTY(EditAnywhere, Category = "Weapon | Core")
    EWeaponType WeaponType;

    /** Socket name on the weapon mesh where projectiles or traces originate from */
    UPROPERTY(EditAnywhere, Category = "Weapon | Core")
    FName WeaponBarrelSocket;

    /**
     * Firing Configuration
     */
    /** Determines how the weapon fires (single shot, burst, automatic) - only for raycast weapons */
    UPROPERTY(EditAnywhere, Category = "Weapon | Firing", meta=(EditCondition = "WeaponType == EWeaponType::EWT_Raycast"))
    EFiringMode FiringMode;

    /** Maximum distance in units that the weapon's raycast can travel */
    UPROPERTY(EditAnywhere, Category = "Weapon | Firing", meta=(EditCondition = "WeaponType == EWeaponType::EWT_Raycast"))
    float WeaponRange;

    /** Base damage value inflicted by each successful hit */
    UPROPERTY(EditAnywhere, Category = "Weapon | Firing", meta=(EditCondition = "WeaponType == EWeaponType::EWT_Raycast"))
    float WeaponDamage;

    /** Whether to perform additional trace tests for weapon firing (for advanced hit detection) */
    UPROPERTY(EditAnywhere, Category = "Weapon | Firing", meta=(EditCondition = "WeaponType == EWeaponType::EWT_Raycast"))
    bool bShouldPerformWeaponTraceTest;

    /**
     * Shot Pattern Configuration
     */
    /** Pattern for weapon shots (straight line, spread, custom) - only for raycast weapons */
    UPROPERTY(EditAnywhere, Category = "Weapon | ShotPattern", meta=(EditCondition = "WeaponType == EWeaponType::EWT_Raycast"))
    EShotPattern ShotPattern;

    /** Number of pellets/projectiles fired per bullet for spread pattern weapons */
    UPROPERTY(EditAnywhere, Category = "Weapon | ShotPattern", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
    int PallatesPerBullet;

    /** Minimum angle/distance for the spread pattern (tighter spread) */
    UPROPERTY(EditAnywhere, Category = "Weapon | ShotPattern", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
    float MinimumSpreadRange;

    /** Maximum angle/distance for the spread pattern (wider spread) */
    UPROPERTY(EditAnywhere, Category = "Weapon | ShotPattern", meta=(EditCondition = "ShotPattern == EShotPattern::ESP_Spread"))
    float MaximumSpreadRange;

    /**
     * Visual Effects
     */
    /** Particle system to spawn at the muzzle when the weapon fires */
    UPROPERTY(EditAnywhere, Category = "Weapon | Effects")
    TObjectPtr<UParticleSystem> MuzzleFlash;

    /** Particle effect for bullet trail/tracer visualization */
    UPROPERTY(EditAnywhere, Category = "Weapon | Effects")
    TObjectPtr<UParticleSystem> BeamTrail;

    /** Particle effect spawned at the impact point of the weapon's trace/projectile */
    UPROPERTY(EditAnywhere, Category = "Weapon | Effects")
    TObjectPtr<UParticleSystem> ImpactParticle;

    /**
     * Audio Effects
     */
    /** Sound to play when the weapon is fired */
    UPROPERTY(EditAnywhere, Category = "Weapon | Audio")
    TObjectPtr<USoundBase> WeaponFireSound;

    /**
     * Animation
     */
    /** Animation montage to play on the character when the weapon is fired */
    UPROPERTY(EditAnywhere, Category = "Weapon | Animation")
    TObjectPtr<UAnimMontage> WeaponFireMontage;

    UPROPERTY()
    FVector FireWeaponTraceEndLocation;
};