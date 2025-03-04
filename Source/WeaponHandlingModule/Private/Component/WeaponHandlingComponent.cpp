// Modular weapon component for handling different weapon types and firing behaviors

#include "Component/WeaponHandlingComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Interfaces/DamageInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

/**
 * Constructor that sets safe default values for all weapon properties
 */
UWeaponHandlingComponent::UWeaponHandlingComponent():
    WeaponType(EWeaponType::EWT_Raycast), FiringMode(EFiringMode::EFM_Single), WeaponRange(0),WeaponDamage(0), bShouldPerformWeaponTraceTest(false),
    ShotPattern(EShotPattern::ESP_Single), PallatesPerBullet(1), MinimumSpreadRange(-MaximumSpreadRange), MaximumSpreadRange(60), FireWeaponTraceEndLocation(0, 0, 0) {
    // Enable ticking for continuous fire support
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


/**
 * Initializes the component when gameplay begins
 */
void UWeaponHandlingComponent::BeginPlay() {
    Super::BeginPlay();
    
    // ...
}


/**
 * Updates weapon state every frame
 * 
 * @param DeltaTime - Time elapsed since last update
 * @param TickType - Type of tick being performed
 * @param ThisTickFunction - Pointer to the tick function being executed
 */
void UWeaponHandlingComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}


/**
 * Traces from the center of the screen to find what the player is aiming at
 * 
 * @param ActorsToIgnore - List of actors that should not be hit by the trace
 * @param TraceHitResult - Contains information about what was hit by the trace (output)
 * @param TraceEndLocation - The end point of the trace (output)
 * 
 * @return True if the trace hit something, false otherwise
 */
bool UWeaponHandlingComponent::ScreenTrace( const TArray<AActor*>& ActorsToIgnore, FHitResult& TraceHitResult, FVector& TraceEndLocation ) const {
    // Calculate screen center
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);

    // Use viewport center as aim point
    const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

    FVector TraceStartLocation, TraceStartDirection;

    // Convert screen aim to world coordinates
    if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
        ViewportCenter, TraceStartLocation, TraceStartDirection)) {
        
        // Set up collision filtering
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActors(ActorsToIgnore);
        
        // Apply weapon's shot pattern
        switch(ShotPattern) {
            case EShotPattern::ESP_Single: {
                    // Direct trace for precision weapons
                    TraceEndLocation = TraceStartLocation + ( TraceStartDirection * WeaponRange );
                    break;
                }
            case EShotPattern::ESP_Spread: {
                    // Random spread for shotgun-like weapons
                    const FVector BulletSpread(FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange), 
                                            FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange),
                                            FMath::RandRange(MinimumSpreadRange, MaximumSpreadRange));
                    TraceEndLocation = TraceStartLocation + (( TraceStartDirection * WeaponRange ) + BulletSpread);
                    break;
                }
            case EShotPattern::ESP_Cluster: break;
            case EShotPattern::ESP_MultiShot: break;
        }

        GetWorld()->LineTraceSingleByChannel(TraceHitResult, TraceStartLocation, TraceEndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
        
        // Use impact point for visual alignment
        if (TraceHitResult.bBlockingHit) {
            TraceEndLocation = TraceHitResult.ImpactPoint;
            return true;
        }
    }
    return false;
}


/**
 * Performs a second trace from the weapon barrel to ensure shots don't pass through walls
 * This prevents shooting through obstacles when the camera position clips through geometry
 * 
 * @param ActorsToIgnore - List of actors that should not be hit by the trace
 * @param Mesh - The weapon's skeletal mesh component
 * @param TraceHitResult - Contains information about what was hit by the trace (output)
 * @param TraceEndLocation - The end point of the trace (output)
 * 
 * @return True if the trace hit something, false otherwise
 */
bool UWeaponHandlingComponent::WeaponTrace( const TArray<AActor*>& ActorsToIgnore, const USkeletalMeshComponent* Mesh, FHitResult& TraceHitResult, FVector& TraceEndLocation ) const {
    
    // First trace finds player's intended target
    const bool bCrosshairHit = ScreenTrace(ActorsToIgnore, TraceHitResult, TraceEndLocation);

    if ( bCrosshairHit ) {
        // Use crosshair hit as target
        TraceEndLocation = TraceHitResult.Location;
    }

    // Second trace checks if shot from barrel is possible
    USkeletalMeshSocket const* BarrelSocket = Mesh->GetSocketByName(WeaponBarrelSocket);
    
    // Get barrel position for shot origin
    const FVector TraceStartLocation = BarrelSocket->GetSocketTransform(Mesh).GetLocation();
    const FVector TraceDirection = TraceStartLocation.GetSafeNormal();
    
    // Trace from barrel to target
    TraceEndLocation = TraceStartLocation + (TraceDirection * WeaponRange);
    GetWorld()->LineTraceSingleByChannel(TraceHitResult, TraceStartLocation, TraceEndLocation, ECC_Vehicle);

    if(TraceHitResult.bBlockingHit) {
        // Update end location for effects
        TraceEndLocation = TraceHitResult.Location;
        return true;
    }
    return false;
}


/**
 * Creates visual beam effects for weapon fire (bullet trails)
 * 
 * @param TraceHitResult - Information about what was hit by the weapon
 * @param Mesh - The weapon's skeletal mesh component
 */
void UWeaponHandlingComponent::SpawnBulletTrail( const FHitResult& TraceHitResult, const USkeletalMeshComponent* Mesh ) const {
    if (BeamTrail) {
        const FTransform WeaponSocketTransform = Mesh->GetSocketTransform(WeaponBarrelSocket);
        UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation( GetWorld(), BeamTrail, WeaponSocketTransform );
        Beam->SetVectorParameter(TEXT("Target"), TraceHitResult.ImpactPoint);
    }
}


/**
 * Applies damage to an actor hit by a weapon.
 * 
 * This function checks if the hit actor implements the IDamageInterface, and if so,
 * executes the ApplyDamage function on that interface to inflict damage.
 * 
 * @param WeaponFIreHitResult The hit result containing information about what was hit
 * @param InstigatorController The controller responsible for causing the damage
 */
void UWeaponHandlingComponent::DamageActor(FHitResult& WeaponFIreHitResult, AController* InstigatorController) const {
    // Check if the hit actor implements the damage interface
    if (Cast<IDamageInterface>(WeaponFIreHitResult.GetActor())) {
        // Execute the ApplyDamage function on the interface, passing relevant damage parameters
        IDamageInterface::Execute_ApplyDamage(GetWorld(), WeaponDamage, InstigatorController, GetOwner(), WeaponFIreHitResult, ImpactParticle);
    }
}


/**
 * Core weapon firing logic that handles different weapon types and shot patterns
 * This function manages the visual and gameplay effects of firing
 * 
 * @param ActorsToIgnore - List of actors that should not be hit by weapon fire
 * @param Mesh - The weapon's skeletal mesh component
 * @param WeaponFIreHitResult - Information about what was hit by the weapon (output)
 * @param WeaponRangeEndLocation - The end point of the weapon's range (output)
 * @param InstigatorController - The controller responsible for firing the weapon
 */
void UWeaponHandlingComponent::ExecuteWeaponFire( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) const {
    if (MuzzleFlash) { UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, WeaponBarrelSocket); }
    if (WeaponFireMontage) { Mesh->GetAnimInstance()->Montage_Play(WeaponFireMontage); }

    switch(WeaponType) {
        case EWeaponType::EWT_Raycast: switch(ShotPattern) {
                case EShotPattern::ESP_Single:
                    // Use barrel trace for physical accuracy or screen trace for responsiveness
                    if( bShouldPerformWeaponTraceTest ) {
                        WeaponTrace(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation );
                        UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), WeaponRangeEndLocation, FLinearColor::Blue, 10, 5);

                        SpawnBulletTrail(WeaponFIreHitResult, Mesh);
                        
                        DamageActor(WeaponFIreHitResult, InstigatorController);
                    } else {
                        ScreenTrace(ActorsToIgnore, WeaponFIreHitResult, WeaponRangeEndLocation);
                        UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), WeaponRangeEndLocation, FLinearColor::Blue, 10, 5);

                        DamageActor(WeaponFIreHitResult, InstigatorController);
                    }
                    break;

                case EShotPattern::ESP_Spread:
                    // Fire multiple pellets for shotgun-like weapons
                    if( bShouldPerformWeaponTraceTest ) {
                        for ( int Pallate = 1; Pallate <= PallatesPerBullet; Pallate++) {
                            WeaponTrace(ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation );
                            UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), WeaponRangeEndLocation, FLinearColor::Blue, 10, 5);

                            SpawnBulletTrail(WeaponFIreHitResult, Mesh);

                            DamageActor(WeaponFIreHitResult, InstigatorController);
                        }
                        
                    } else {
                        for ( int Pallate = 1; Pallate <= PallatesPerBullet; Pallate++) {
                            ScreenTrace(ActorsToIgnore, WeaponFIreHitResult, WeaponRangeEndLocation);
                            UKismetSystemLibrary::DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), WeaponRangeEndLocation, FLinearColor::Blue, 10, 5);

                            SpawnBulletTrail(WeaponFIreHitResult, Mesh);

                            DamageActor(WeaponFIreHitResult, InstigatorController);
                        }
                    }
                    break;

                // Future weapon patterns
                case EShotPattern::ESP_Cluster: break;
                case EShotPattern::ESP_MultiShot: break;
            }
            break;

        // Reserved for physical projectiles
        case EWeaponType::EWT_Projectile: break;
    }
    if (WeaponFireSound) { UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, GetOwner()->GetActorLocation()); }
}


/**
 * Main public function to activate the weapon and fire
 * This is the entry point that other game systems should call
 * 
 * @param ActorsToIgnore - List of actors that should not be hit by weapon fire
 * @param Mesh - The weapon's skeletal mesh component
 * @param WeaponFIreHitResult - Information about what was hit by the weapon (output)
 * @param WeaponRangeEndLocation - The end point of the weapon's range (output)
 * @param InstigatorController - The controller responsible for firing the weapon
 */
void UWeaponHandlingComponent::FireWeapon( const TArray<AActor*>& ActorsToIgnore, USkeletalMeshComponent* Mesh, FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) {
    ExecuteWeaponFire( ActorsToIgnore, Mesh, WeaponFIreHitResult, WeaponRangeEndLocation, InstigatorController);
}