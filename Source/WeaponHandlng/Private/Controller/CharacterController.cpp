// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponHandlng/Public/Controller/CharacterController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/FPSCharacterBase.h"
#include "GameFramework/Character.h"

/**
 * @brief Called when the controller possesses a pawn.
 * 
 * This function is invoked when the controller takes control of a pawn. It initializes
 * the EnhancedInputComponent, sets up the input mappings, and binds actions to their
 * corresponding handler methods.
 * 
 * @param aPawn The pawn that the controller possesses.
 */
void ACharacterController::OnPossess( APawn* aPawn ) {
	// Call the base class's OnPossess method
	Super::OnPossess(aPawn);
	PlayerCharacter = Cast<AFPSCharacterBase>(aPawn);

	// Get the EnhancedInputComponent from the InputComponent
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(EnhancedInputComponent, TEXT("Enhanced Input component is valid"));

	// Add the MappingContext to the EnhancedInputLocalPlayerSubsystem
	if ( UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()) ) { Subsystem->AddMappingContext(MappingContext, 0); }

	// Bind input actions to their respective methods
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterController::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterController::HandleLookAndAiming);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacterController::HandleJump);
}


/**
 * @brief Called when the controller unpossesses a pawn.
 * 
 * This function is invoked when the controller releases control of a pawn. It calls
 * the base class's OnUnPossess method.
 */
void ACharacterController::OnUnPossess() {
	// Call the base class's OnUnPossess method
	Super::OnUnPossess();
}


/**
 * @brief Called when the Move action is triggered.
 * 
 * This function is invoked when the Move action is triggered. It processes the movement
 * input value to move the character forward/backward and right/left.
 * 
 * @param Value The input value for the Move action.
 */
void ACharacterController::Move( const FInputActionValue& Value ) {
	// Get the movement vector from the input value
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Add movement input in the forward and right directions
	PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorForwardVector(), MovementVector.Y);
	PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorRightVector(), MovementVector.X);
}

/**
 * @brief Called when the Look action is triggered.
 * 
 * This function is invoked when the Look action is triggered. It processes the look
 * input value to rotate the character's view. The sensitivity of the rotation depends
 * on whether the character is aiming or not.
 * 
 * @param Value The input value for the Look action.
 */
void ACharacterController::HandleLookAndAiming( const FInputActionValue& Value ) {
	// Get the look axis value from the input value
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	AddYawInput(LookAxisValue.X );
	AddPitchInput(LookAxisValue.Y);
}

void ACharacterController::HandleJump() {
	PlayerCharacter->Jump();
}
