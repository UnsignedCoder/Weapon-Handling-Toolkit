// Modular weapon component for handling different weapon types and firing behaviors

#include "Component/WeaponHandlingComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Interfaces/DamageInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

/**
 * Sets up default values for the weapon, such as type, firing mode, and range.
 * Ensures the weapon starts in a safe and predictable state.
 */
UWeaponHandlingComponent::UWeaponHandlingComponent():
	WeaponType(EWeaponType::EWT_Raycast), FiringMode(EFiringMode::EFM_Single), WeaponRange(0), WeaponDamage(0), bShouldPerformWeaponTraceTest(false),
	ShotPattern(EShotPattern::ESP_Single), PallatesPerBullet(1), MinimumSpreadRange(-MaximumSpreadRange), MaximumSpreadRange(60), FireWeaponTraceEndLocation(0, 0, 0), bShouldFireWeapon(true), bShouldBurstShotCooldown(false), WeaponFireRate(12),
	MaxBurstShotCount(0), BurstShotCooldown(0), CurrentBurstShotCount(0) {
	
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UWeaponHandlingComponent::BeginPlay() {
	Super::BeginPlay();

	// ...
}


void UWeaponHandlingComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


/**
 * Determines where the player is aiming by tracing from the center of the screen.
 * Used to find the target location for raycast-based weapons.
 * 
 * @param ActorsToIgnore - Actors that should not be hit by the trace (e.g., the player)
 * @param TraceHitResult - Stores information about what the trace hits
 * @param TraceEndLocation - Stores the end point of the trace
 * 
 * @return True if the trace hits something, false otherwise
 */
bool UWeaponHandlingComponent::ScreenTrace( const TArray<AActor*>& ActorsToIgnore, FHitResult& TraceHitResult, FVector& TraceEndLocation ) const {
	// Find the center of the screen to determine where the player is aiming
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	// Use the center of the screen as the aim point
	const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

	FVector TraceStartLocation, TraceStartDirection;

	// Convert the screen position to a world-space direction for tracing
	if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		ViewportCenter, TraceStartLocation, TraceStartDirection)) {

		// Ignore specified actors to avoid hitting unintended targets
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActors(ActorsToIgnore);

		// Adjust the trace based on the weapon's shot pattern
		switch (ShotPattern) {
			case EShotPattern::ESP_Single: {
				// Direct trace for precise aiming
				TraceEndLocation = TraceStartLocation + (TraceStartDirection * WeaponRange);
				break;
			}
			case EShotPattern::ESP_Spread: {
				// Add random spread to simulate shotgun-like behavior
				const FVector BulletSpread(FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange),
					FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange),
					FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange));
				TraceEndLocation = TraceStartLocation + ((TraceStartDirection * WeaponRange) + BulletSpread);
				break;
			}
			case EShotPattern::ESP_Cluster: break; // Reserved for future cluster-based firing
			case EShotPattern::ESP_MultiShot: break; // Reserved for future multi-shot firing
		}

		// Perform the trace to find the target
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, TraceStartLocation, TraceEndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);

		// If the trace hits something, use the impact point as the target
		if (TraceHitResult.bBlockingHit) {
			TraceEndLocation = TraceHitResult.ImpactPoint;
			return true;
		}
	}
	return false;
}


/**
 * Ensures shots originate from the weapon barrel and don't pass through walls.
 * Prevents unrealistic shooting through obstacles.
 * 
 * @param ActorsToIgnore - Actors that should not be hit by the trace
 * @param Mesh - The weapon's skeletal mesh, used to find the barrel position
 * @param TraceHitResult - Stores information about what the trace hits
 * @param TraceEndLocation - Stores the end point of the trace
 * 
 * @return True if the trace hits something, false otherwise
 */
bool UWeaponHandlingComponent::WeaponTrace( const TArray<AActor*>& ActorsToIgnore, const USkeletalMeshComponent* Mesh, FHitResult& TraceHitResult, FVector& TraceEndLocation ) const {

	// First, find where the player is aiming
	const bool bCrosshairHit = ScreenTrace(ActorsToIgnore, TraceHitResult, TraceEndLocation);

	if (bCrosshairHit) {
		// Use the crosshair hit as the target location
		TraceEndLocation = TraceHitResult.Location;
	}

	// Find the barrel socket to determine where the shot originates
	const USkeletalMeshSocket* BarrelSocket = Mesh->GetSocketByName(WeaponBarrelSocket);

	// Get the barrel's position and direction for the trace
	const FVector TraceStartLocation = BarrelSocket->GetSocketTransform(Mesh).GetLocation();
	const FVector TraceDirection = TraceStartLocation.GetSafeNormal();

	// Trace from the barrel to the target to ensure the shot is valid
	TraceEndLocation = TraceStartLocation + (TraceDirection * WeaponRange);
	GetWorld()->LineTraceSingleByChannel(TraceHitResult, TraceStartLocation, TraceEndLocation, ECC_Vehicle);

	if (TraceHitResult.bBlockingHit) {
		// Update the end location to the impact point for effects
		TraceEndLocation = TraceHitResult.Location;
		return true;
	}
	return false;
}


/**
 * Creates a visual effect for the bullet trail.
 * Helps players see where their shots are going.
 * 
 * @param TraceHitResult - Information about where the shot hit
 * @param Mesh - The weapon's skeletal mesh, used to find the barrel position
 */
void UWeaponHandlingComponent::SpawnBulletTrail( const FHitResult& TraceHitResult, const USkeletalMeshComponent* Mesh ) const {
	if (BeamTrail) {
		// Spawn the bullet trail effect at the barrel socket
		const FTransform WeaponSocketTransform = Mesh->GetSocketTransform(WeaponBarrelSocket);
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamTrail, WeaponSocketTransform);
		Beam->SetVectorParameter(TEXT("Target"), TraceHitResult.ImpactPoint);
	}
}


/**
 * Applies damage to the target hit by the weapon.
 * Checks if the target can take damage and applies the appropriate amount.
 * 
 * @param WeaponFIreHitResult - Information about what was hit
 * @param InstigatorController - The controller responsible for the damage
 */
void UWeaponHandlingComponent::DamageActor( FHitResult& WeaponFIreHitResult, AController* InstigatorController ) const {
	// Check if the hit actor can take damage
	if (Cast<IDamageInterface>(WeaponFIreHitResult.GetActor())) {
		// Apply damage to the target
		IDamageInterface::Execute_ApplyDamage(GetWorld(), WeaponDamage, InstigatorController, GetOwner(), WeaponFIreHitResult, ImpactParticle);
	}
}


/**
 * Handles the core logic of firing the weapon.
 * Manages different weapon types, firing modes, and visual effects.
 * 
 * @param ActorsToIgnore - Actors that should not be hit by the weapon
 * @param Mesh - The weapon's skeletal mesh, used for animations and effects
 * @param WeaponFIreHitResult - Stores information about what was hit
 * @param WeaponRangeEndLocation - Stores the end point of the weapon's range
 * @param InstigatorController - The controller responsible for firing the weapon
 */
void UWeaponHandlingComponent::ExecuteWeaponFire( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) const {
	// Play muzzle flash and firing animation if available
	if (MuzzleFlash) { UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, WeaponBarrelSocket); }
	if (WeaponFireMontage) { Mesh->GetAnimInstance()->Montage_Play(WeaponFireMontage); }

	switch (WeaponType) {
		case EWeaponType::EWT_Raycast: 
			switch (ShotPattern) {
				case EShotPattern::ESP_Single:
					// Fire a single shot, either from the barrel or screen center
					if (bShouldPerformWeaponTraceTest) {
						WeaponTrace(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation);
						SpawnBulletTrail(WeaponFIreHitResult, Mesh);
						DamageActor(WeaponFIreHitResult, InstigatorController);

						UKismetSystemLibrary::DrawDebugLine(this, Mesh->GetSocketLocation(WeaponBarrelSocket), WeaponRangeEndLocation, FLinearColor::Red, 6, 2);
					} else {
						ScreenTrace(ActorsToIgnore, WeaponFIreHitResult, WeaponRangeEndLocation);
						DamageActor(WeaponFIreHitResult, InstigatorController);
						UKismetSystemLibrary::DrawDebugLine(this, Mesh->GetSocketLocation(WeaponBarrelSocket), WeaponRangeEndLocation, FLinearColor::Red, 6, 2);
					}
					break;

				case EShotPattern::ESP_Spread:
					// Fire multiple pellets for a spread effect
					for (int Pallate = 1; Pallate <= PallatesPerBullet; Pallate++) {
						if (bShouldPerformWeaponTraceTest) {
							WeaponTrace(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation);
						} else {
							ScreenTrace(ActorsToIgnore, WeaponFIreHitResult, WeaponRangeEndLocation);
						}
						SpawnBulletTrail(WeaponFIreHitResult, Mesh);
						DamageActor(WeaponFIreHitResult, InstigatorController);
						UKismetSystemLibrary::DrawDebugLine(this, Mesh->GetSocketLocation(WeaponBarrelSocket), WeaponRangeEndLocation, FLinearColor::Red, 6, 2);
					}
					break;

				case EShotPattern::ESP_Cluster: break; // Reserved for future cluster-based firing
				case EShotPattern::ESP_MultiShot: break; // Reserved for future multi-shot firing
			}
			break;

		case EWeaponType::EWT_Projectile: break; // Reserved for projectile-based weapons
	}

	// Play the weapon firing sound if available
	if (WeaponFireSound) { UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, GetOwner()->GetActorLocation()); }
}

/**
 * Resets the weapon's firing state to allow another shot.
 * Used for single-shot and burst-fire weapons.
 */
void UWeaponHandlingComponent::ResetShouldFireWeapon() { 
	if (!bShouldFireWeapon) { 
		bShouldFireWeapon = true; 
	} 
}

/**
 * Resets the burst fire cooldown to allow another burst.
 * Ensures burst fire behaves as intended.
 */
void UWeaponHandlingComponent::ResetBurstShotCooldown() { 
	if (bShouldBurstShotCooldown) { 
		bShouldBurstShotCooldown = false; 
	} 
}

/**
 * Main function to fire the weapon based on its firing mode.
 * Handles single-shot, burst, and automatic firing.
 * 
 * @param ActorsToIgnore - Actors that should not be hit by the weapon
 * @param Mesh - The weapon's skeletal mesh, used for animations and effects
 * @param WeaponFIreHitResult - Stores information about what was hit
 * @param WeaponRangeEndLocation - Stores the end point of the weapon's range
 * @param InstigatorController - The controller responsible for firing the weapon
 */
void UWeaponHandlingComponent::FireWeapon( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) {
	switch (FiringMode) {
		case EFiringMode::EFM_Single: 
			// Fire a single shot
			break;
		case EFiringMode::EFM_Burst:
			// Fire a burst of shots with a cooldown between bursts
			if (bShouldFireWeapon && !bShouldBurstShotCooldown) {
				ExecuteWeaponFire(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation, InstigatorController);
				bShouldFireWeapon = false;
				CurrentBurstShotCount++;
				if (CurrentBurstShotCount >= MaxBurstShotCount) {
					CurrentBurstShotCount = 0;
					bShouldBurstShotCooldown = true;
					GetWorld()->GetTimerManager().SetTimer(WeaponBurstCooldownTImer, this, &UWeaponHandlingComponent::ResetBurstShotCooldown, BurstShotCooldown);
				}
				GetWorld()->GetTimerManager().SetTimer(WeaponFireTimer, this, &UWeaponHandlingComponent::ResetShouldFireWeapon, WeaponFireRate, false);
			}
			break;
		case EFiringMode::EFM_Automatic:
			// Fire continuously as long as the trigger is held
			if (bShouldFireWeapon) {
				ExecuteWeaponFire(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation, InstigatorController);
				bShouldFireWeapon = false;
				GetWorld()->GetTimerManager().SetTimer(WeaponFireTimer, this, &UWeaponHandlingComponent::ResetShouldFireWeapon, WeaponFireRate, false);
			}
			break;
	}
}