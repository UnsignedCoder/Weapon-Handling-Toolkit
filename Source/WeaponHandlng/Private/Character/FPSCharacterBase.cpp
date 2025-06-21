// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FPSCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "WeaponHandlingModule/Public/Component/WeaponHandlingComponent.h"


// Sets default values
AFPSCharacterBase::AFPSCharacterBase() {
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponHandlingComponent = CreateDefaultSubobject<UWeaponHandlingComponent> ("Weapon Handling Compoennt");

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = true;
}


// Called when the game starts or when spawned
void AFPSCharacterBase::BeginPlay() {
	Super::BeginPlay();
	
}


// Called every frame
void AFPSCharacterBase::Tick( float DeltaTime ) { Super::Tick(DeltaTime); }


// Called to bind functionality to input
void AFPSCharacterBase::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent ) { Super::SetupPlayerInputComponent(PlayerInputComponent); }


void AFPSCharacterBase::FireWeapon() {
	WeaponHandlingComponent->WeaponAttack();
}