// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/RangedWeapon.h"
#include "Logging.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

/**
 * Constructs weapon with core visual representation.
 * 
 * @note Establishes skeletal mesh as primary visual component
 * @warning Weapon remains non-functional until configured with valid WeaponData
 */
ARangedWeapon::ARangedWeapon() : CurrentBurstShotCount(0), bShouldFireWeapon(true), bShouldBurstShotCooldown(false) {
	PrimaryActorTick.bCanEverTick = true;
}


void ARangedWeapon::BeginPlay() {
	Super::BeginPlay();
}


void ARangedWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}


/**
 * Visualizes projectile trajectory from muzzle to impact point.
 * 
 * @param TraceHitResult Contains impact data including:
 *        - ImpactPoint: World location where projectile hit
 *        - TraceEnd: Final point if no collision occurred
 * @param Mesh Reference weapon mesh for correct muzzle positioning
 * 
 * @note Creates temporary beam effect showing shot path
 * @warning Requires properly configured BeamTrail particle system
 * @remark Handles both hit-and-miss cases appropriately
 */
void ARangedWeapon::SpawnBulletTrail(const FHitResult& TraceHitResult, const USkeletalMeshComponent* Mesh) const {
	// Early out if required assets aren't configured
	if (!WeaponData.BeamTrail || !Mesh) {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("Missing BeamTrail effect and/or Mesh reference"));
		return;
	}

	// Get precise muzzle location from socket
	const FTransform WeaponSocketTransform = Mesh->GetSocketTransform(WeaponBarrelSocket);
	if (!WeaponSocketTransform.IsValid()) {
		UE_LOG(LogWeaponHandlingModule, Error, TEXT("Invalid barrel socket transform"));
		return;
	}

	// Spawn beam effect at muzzle location
	UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponData.BeamTrail, WeaponSocketTransform);

	// Configure beam target based on hit results
	if (Beam) {
		const FVector TargetLocation = TraceHitResult.bBlockingHit ? TraceHitResult.ImpactPoint : TraceHitResult.TraceEnd;
		Beam->SetVectorParameter(FName("Target"), TargetLocation);
	}
}


/**
 * Coordinates core firing sequence including visual feedback.
 * 
 * @param IgnoredActors Entities to exclude from hit detection
 * @param WeaponFireHitResult Output container for hit analysis
 * @param InstigatorController Responsible controller for attribution
 * 
 * @note Always plays muzzle flash when configured
 * @remark Base implementation handles visuals - override for game-specific damage
 */
void ARangedWeapon::ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	// Play muzzle flash effect if configured
	if (WeaponData.MuzzleFlash) {
		UGameplayStatics::SpawnEmitterAttached(WeaponData.MuzzleFlash, GetWeaponMesh(), WeaponBarrelSocket);
	} else {
		UE_LOG(LogWeaponHandlingModule, Warning, TEXT("Missing MuzzleFlash effect"));
	}
	// Visualize bullet trajectory
	SpawnBulletTrail(WeaponFireHitResult, GetWeaponMesh());
}


/**
 * Delegates to configured shot pattern implementation.
 * 
 * @param IgnoredActors Entities excluded from collision checks
 * @param WeaponFireHitResult Output for combat resolution data  
 * @param InstigatorController Controller responsible for this action
 * 
 * @note Plays firing sound regardless of hit success
 * @remark Supports both precision and scatter shot configurations
 */
void ARangedWeapon::ExecuteWeaponFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	// Handle different shot patterns
	switch (WeaponData.ShotPattern) {
		case EShotPattern::ESP_Single:
			ShootWeapon(IgnoredActors, WeaponFireHitResult, InstigatorController);
			break;

		case EShotPattern::ESP_Spread:
			// Fire multiple pellets for spread pattern
			for (uint8 Pellet = 1; Pellet <= WeaponData.PelletsPerBullet; Pellet++) {
				ShootWeapon(IgnoredActors, WeaponFireHitResult, InstigatorController);
			}
			break;
	}

	// Play weapon sound if configured
	if (WeaponData.WeaponFireSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData.WeaponFireSound, 
			GetOwner()->GetActorLocation());
	}
}


/**
 * Resets global firing cooldown state.
 * 
 * @note Called automatically by weapon timers
 * @warning Only affects weapons currently in cooldown
 */
void ARangedWeapon::ResetShouldFireWeapon() {
	if (!bShouldFireWeapon) {
		bShouldFireWeapon = true;
	}
}


/**
 * Completes burst sequence recovery period.
 * 
 * @note Managed internally by burst fire logic
 * @warning Duration controlled by WeaponData.BurstShotCooldown
 */
void ARangedWeapon::ResetBurstShotCooldown() {
	if (bShouldBurstShotCooldown) {
		bShouldBurstShotCooldown = false;
	}
}


/**
 * Re-enables single-shot weapons after firing.
 * 
 * @note Only affects weapons in single-fire mode
 * @remark Timer-controlled - not for direct calls
 */
void ARangedWeapon::ResetShouldFireSingleShot() {
	if (!bShouldFireWeapon && WeaponData.FiringMode == EFiringMode::EFM_Single) {
		bShouldFireWeapon = true;
	}
}


/**
 * Manages burst fire sequence with automatic pacing.
 * 
 * @param IgnoredActors Entities excluded from collision
 * @param WeaponFireHitResult Output for combat results  
 * @param InstigatorController Responsible controller
 * 
 * @note Tracks shots within current burst automatically
 * @remark Enforces cooldown between burst sequences
 */
void ARangedWeapon::ExecuteBurstFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	if (bShouldFireWeapon && !bShouldBurstShotCooldown) {
		// Fire single shot in burst sequence
		ExecuteWeaponFire(IgnoredActors, WeaponFireHitResult, InstigatorController);
		bShouldFireWeapon = false;
		CurrentBurstShotCount++;

		// Check if burst sequence is complete
		if (CurrentBurstShotCount >= WeaponData.MaxBurstShotCount) {
			CurrentBurstShotCount = 0;
			bShouldBurstShotCooldown = true;
			// Start burst cooldown timer
			GetWorld()->GetTimerManager().SetTimer(WeaponBurstCooldownTimer, this, &ARangedWeapon::ResetBurstShotCooldown, WeaponData.BurstShotCooldown);
		}

		// Set timer for next shot in burst
		GetWorld()->GetTimerManager().SetTimer(WeaponFireTimer, this, &ARangedWeapon::ResetShouldFireWeapon, WeaponData.WeaponFireRate, false);
	}
}


/**
 * Handles precision single-shot firing mechanics.
 * 
 * @param IgnoredActors Entities excluded from collision
 * @param WeaponFireHitResult Output for combat results  
 * @param InstigatorController Responsible controller
 * 
 * @note Requires explicit trigger for each shot
 * @remark Cooldown controlled by WeaponFireRate
 */
void ARangedWeapon::ExecuteSingleFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	if (bShouldFireWeapon) {
		ExecuteWeaponFire(IgnoredActors, WeaponFireHitResult, InstigatorController);
		bShouldFireWeapon = false;
	}
}


/**
 * Manages sustained automatic fire behavior.
 * 
 * @param IgnoredActors Entities excluded from collision
 * @param WeaponFireHitResult Output for combat results
 * @param InstigatorController Responsible controller
 * 
 * @note Fires continuously while trigger is active
 * @warning Can rapidly consume ammunition reserves
 */
void ARangedWeapon::ExecuteAutomaticFire( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	if (bShouldFireWeapon) {
		ExecuteWeaponFire(IgnoredActors, WeaponFireHitResult, InstigatorController);
		bShouldFireWeapon = false;
		// Set timer for next automatic shot
		GetWorld()->GetTimerManager().SetTimer(WeaponFireTimer,this, &ARangedWeapon::ResetShouldFireWeapon, WeaponData.WeaponFireRate, false);
	}
}


/**
 * Primary firing interface routing to mode-specific implementations.
 * 
 * @param WeaponFireHitResult Output container for combat results
 * @param InstigatorController Controller responsible for this action
 * 
 * @note Supports all configured firing modes
 * @warning Uses internal ActorsToIgnore list for collision
 */
void ARangedWeapon::LaunchAttack( FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	// Route to appropriate firing mode implementation
	switch (WeaponData.FiringMode) {
		case EFiringMode::EFM_Single: 
			ExecuteSingleFire(GetActorsToIgnore(), WeaponFireHitResult, InstigatorController);
			break;
		case EFiringMode::EFM_Burst: 
			ExecuteBurstFire(GetActorsToIgnore(), WeaponFireHitResult, InstigatorController);
			break;
		case EFiringMode::EFM_Automatic: 
			ExecuteAutomaticFire(GetActorsToIgnore(), WeaponFireHitResult, InstigatorController);
			break;
	}
}


