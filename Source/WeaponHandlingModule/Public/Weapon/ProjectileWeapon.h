// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class WEAPONHANDLINGMODULE_API AProjectileWeapon : public ARangedWeapon {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) override;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};