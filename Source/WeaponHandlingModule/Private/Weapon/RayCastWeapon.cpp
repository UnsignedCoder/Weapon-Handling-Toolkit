// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/RayCastWeapon.h"

#include "Logging.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

/**
 * Constructs a raycast weapon with default values.
 * 
 * @note Enables ticking by default for weapon behaviors
 * @remark Inherits base weapon functionality
 */
ARayCastWeapon::ARayCastWeapon() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

/**
 * Handles weapon initialization when gameplay begins.
 * 
 * @note Currently uses base weapon setup
 * @remark Extend for raycast-specific initialization
 */
void ARayCastWeapon::BeginPlay() {
	Super::BeginPlay();
}

/**
 * Processes continuous weapon updates.
 * 
 * @param DeltaTime Frame time increment
 * @note Base implementation empty - override for raycast-specific behaviors
 */
void ARayCastWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

/**
 * Determines weapon firing method and executes appropriate trace.
 * 
 * @param IgnoredActors Entities excluded from hit detection
 * @param WeaponFireHitResult Output container for hit data
 * @param InstigatorController Responsible controller reference
 * 
 * @note Uses WeaponData configuration to choose between trace methods
 * @see WeaponTrace()
 * @see ScreenTrace()
 */
void ARayCastWeapon::ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	Super::ShootWeapon(IgnoredActors, WeaponFireHitResult, InstigatorController);

	if (WeaponData.bShouldPerformWeaponTraceTest) {
		WeaponTrace(WeaponFireHitResult, IgnoredActors);
	} else {
		// Perform screen trace if not using weapon trace
		ScreenTrace(WeaponFireHitResult, IgnoredActors);
	}
}

/**
 * Performs precise weapon barrel-to-impact-point trace for accurate bullet simulation.
 * 
 * @param WeaponTraceHitResult Output container for trace results
 * @param IgnoredActors Entities excluded from detection
 * @return True if collision occurred between barrel and extended impact point
 * 
 * @note Two-stage process:
 *       1. Initial screen trace to find approximate target
 *       2. Precise weapon trace from barrel to extended impact point
 * @remark Uses weapon socket location for realistic bullet origin
 * @warning Requires valid barrel socket on weapon mesh
 */
bool ARayCastWeapon::WeaponTrace(FHitResult& WeaponTraceHitResult, const TArray<AActor*>& IgnoredActors) const {
	if (!ScreenTrace(WeaponTraceHitResult, IgnoredActors)) {
		return false;
	}
	
	const USkeletalMeshSocket* BarrelSocket = GetWeaponMesh()->GetSocketByName(WeaponBarrelSocket);

	if (!BarrelSocket) {
		UE_LOG(LogWeaponHandlingModule, Error, TEXT("Invalid barrel socket"));
		return false;
	} 

	// Calculate bullet path from barrel to extended impact point
	FVector BarrelLocation = BarrelSocket->GetSocketLocation(GetWeaponMesh());
	const FVector Direction = (WeaponTraceHitResult.ImpactPoint - BarrelLocation).GetSafeNormal();
	WeaponTraceHitResult.TraceStart = BarrelLocation;
	WeaponTraceHitResult.TraceEnd = WeaponTraceHitResult.ImpactPoint + Direction * WeaponData.WeaponRange;

	// Configure collision parameters
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActors(IgnoredActors);
	
	// Perform precise weapon trace
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHitResult, BarrelLocation, WeaponTraceHitResult.TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	return WeaponTraceHitResult.bBlockingHit;
}

/**
 * Performs screen-space to world hit detection from view center.
 * 
 * @param ScreenTraceHitResult Output container for trace results
 * @param IgnoredActors Entities excluded from detection
 * @return True if collision occurred along view direction
 * 
 * @note Uses player's screen center as aim point
 * @remark Calculation steps:
 *         1. Gets viewport center
 *         2. Deprojects to world space
 *         3. Traces along view direction
 * @warning Requires:
 *          - Valid owning character
 *          - Player controller
 *          - Game viewport
 */
bool ARayCastWeapon::ScreenTrace(FHitResult& ScreenTraceHitResult, const TArray<AActor*>& IgnoredActors) const {
	// Get viewport dimensions
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	// Calculate screen center for aiming
	const FVector2D ScreenCenter = ViewportSize / 2;

	// Convert screen position to world space vectors
	FVector WorldDirection;
	FVector WorldLocation;
	UGameplayStatics::DeprojectScreenToWorld(Cast<APlayerController>(GetOwningCharacter()->GetController()), ScreenCenter,WorldLocation,WorldDirection);

	// Calculate trace start and end points
	const FVector TraceStart = WorldLocation;
	const FVector TraceEnd = WorldLocation + (WorldDirection * WeaponData.WeaponRange);

	// Configure collision parameters
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActors(IgnoredActors);
	
	// Perform the line trace
	GetWorld()->LineTraceSingleByChannel(ScreenTraceHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	// Return hit status
	return ScreenTraceHitResult.bBlockingHit;
}