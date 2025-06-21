// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseWeapon.h"

#include "Logging.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ABaseWeapon::ABaseWeapon() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("Box Component (Collision Box)");
	SetRootComponent(CollisionBox);

	// Create and configure the weapon's visual representation
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Mesh");
	WeaponMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	
}

// Called every frame
void ABaseWeapon::Tick( float DeltaTime ) {
	Super::Tick(DeltaTime);

}

/**
 * Establishes ownership chain for proper attribution.
 * 
 * @param NewOwner Character instance possessing this weapon
 * @note Critical for correct damage credit assignment
 */
void ABaseWeapon::SetOwningCharacter(ACharacter* NewOwner) {
	OwningCharacter = NewOwner;
}


void ABaseWeapon::Fall() {
	SetOwningCharacter(nullptr);
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	GetWeaponMesh()->DetachFromComponent(DetachmentRules);
	
	if (bShouldUsePhysicsSimulation) {
		GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Block);
		GetWeaponMesh()->SetSimulatePhysics(true);

		FTimerHandle SimulationTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(SimulationTimerHandle, [this](){
			GetWeaponMesh()->SetSimulatePhysics(false);
			GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}, 8.0f, false);
	} else {
		FHitResult GroundTraceHitResult;
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() - FVector(0, 0, 5000),UEngineTypes::ConvertToTraceType(ECC_Visibility),
			true, TArray<AActor*>(), EDrawDebugTrace::None, GroundTraceHitResult, true);

		if (GroundTraceHitResult.bBlockingHit) {
			SetActorLocation(GroundTraceHitResult.ImpactPoint);
		} else {
			UE_LOG(LogWeaponHandlingModule, Warning, TEXT("BaseWeapon::Fall - No ground hit detected, weapon may fall through the world!"));
			bShouldUsePhysicsSimulation = true; // Reset to default behavior
			Fall();
		}
	}
	
}

/**
 * Adds single actor to collision exclusion list.
 * 
 * @param IgnoredActor Specific entity to ignore in weapon traces
 * @note Typically used to prevent hitting weapon owner
 */
void ABaseWeapon::AddActorToIgnore(AActor* IgnoredActor) {
	if (IgnoredActor) {
		ActorsToIgnore.Emplace(IgnoredActor);
	}
}


/**
 * Batch adds multiple actors to collision exclusion.
 * 
 * @param IgnoredActors Array of entities to exclude from traces
 * @note Efficient solution for ignoring related actor groups
 */
void ABaseWeapon::AddActorToIgnore(TArray<AActor*> IgnoredActors) {
	if (IgnoredActors.Num() > 0) {
		for (AActor* Actor : IgnoredActors) {
			ActorsToIgnore.Emplace(Actor);
		}
	}
}

void ABaseWeapon::LaunchAttack( FHitResult& WeaponAttackHitResult, AController* InstigatorController ) {}


