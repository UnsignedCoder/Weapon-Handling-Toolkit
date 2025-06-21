// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponHandlingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UWeaponHandlingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WEAPONHANDLINGMODULE_API IWeaponHandlingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void FireWeapon( FHitResult& WeaponFIreHitResult, FVector& WeaponRangeEndLocation, AController* InstigatorController ) = 0;
};