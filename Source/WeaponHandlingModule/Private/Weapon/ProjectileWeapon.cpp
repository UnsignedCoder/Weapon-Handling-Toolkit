﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"


// Sets default values
AProjectileWeapon::AProjectileWeapon() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProjectileWeapon::BeginPlay() {
	Super::BeginPlay();
	
}

void AProjectileWeapon::ShootWeapon( const TArray<AActor*>& IgnoredActors, FHitResult& WeaponFireHitResult, AController* InstigatorController ) {
	Super::ShootWeapon(IgnoredActors, WeaponFireHitResult, InstigatorController);
	
}

// Called every frame
void AProjectileWeapon::Tick( float DeltaTime ) {
	Super::Tick(DeltaTime);

}

