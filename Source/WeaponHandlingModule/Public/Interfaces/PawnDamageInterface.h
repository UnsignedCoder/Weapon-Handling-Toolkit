// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnDamageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPawnDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WEAPONHANDLINGMODULE_API IPawnDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Applies damage to this actor with various parameters to control damage behavior.
	 *
	 * @param DamageAmount           The base amount of damage to apply.
	 * @param InstigatorController   The controller responsible for causing the damage (can be null).
	 * @param DamageCauser           The actor that directly caused the damage (can be null).
	 * @param DamageHitResult        The hit result from the trace/test that caused this damage.
	 * @param DamageCue              Optional sound to play when damage is applied. Defaults to nullptr.
	 * @param DefaultWeaponImpactFX               Optional particle effect to spawn at damage location. Defaults to nullptr.
	 *
	 * This is a pure virtual function that must be implemented by any class that inherits this interface.
	 * Implementing classes should define how damage is calculated, applied, and what visual/audio feedback
	 * is provided. The function should handle damage application logic, spawn any specified effects at the
	 * hit location, and may trigger additional gameplay events like hit reactions or death sequences based
	 * on the resulting health state.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Damage")
	void ApplyDamage( float DamageAmount, AController* InstigatorController, AActor* DamageCauser, FHitResult& DamageHitResult, UParticleSystem* DefaultWeaponImpactFX = nullptr );
};