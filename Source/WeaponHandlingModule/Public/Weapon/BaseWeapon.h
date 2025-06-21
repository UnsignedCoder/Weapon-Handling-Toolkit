// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

class UBoxComponent;

UCLASS()
class WEAPONHANDLINGMODULE_API ABaseWeapon : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

protected:

public:
/** 
* Establishes ownership chain for damage attribution
* @param NewOwner - Character possessing this weapon
* 
* @note Required for proper damage credit assignment
* @warning Must be called before weapon can be used
*/
	void SetOwningCharacter(ACharacter* NewOwner);

	void Fall();

	/** 
	 * Adds single actor to collision exclusion list 
	 * @param IgnoredActor - Entity to exclude from hit detection
	 * 
	 * @note Typically used to prevent hitting weapon owner
	 * @see AddActorToIgnore(TArray<AActor*>) for batch operations
	*/
	void AddActorToIgnore(AActor* IgnoredActor);

	/**
	 * Batch adds actors to collision exclusion list
	 * @param IgnoredActors - Array of entities to exclude from hit detection
	 * 
	 * @note More efficient than individual additions
	 * @see AddActorToIgnore(AActor*) for single additions
	 */
	void AddActorToIgnore(TArray<AActor*> IgnoredActors);

	virtual void LaunchAttack( FHitResult& WeaponAttackHitResult, AController* InstigatorController );

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ACharacter> OwningCharacter;

	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;
	
	/** Visual representation component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "Weapon | Configuration", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bShouldUsePhysicsSimulation = true;

	/** Collision ignore list */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon | Configuration", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> ActorsToIgnore;
public:
	
	/** 
	 * Gets current owning character reference
	 * @return Character instance that possesses this weapon
	 */
	FORCEINLINE TObjectPtr<ACharacter> GetOwningCharacter() const { return OwningCharacter; }
	
	/** 
	 * Retrieves weapon visual representation 
	 * @return Skeletal mesh component used for weapon visualization
	 */
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE  TArray<AActor*> GetActorsToIgnore() const { return ActorsToIgnore; }
};