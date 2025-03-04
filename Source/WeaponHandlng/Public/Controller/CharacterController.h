// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "CharacterController.generated.h"

class AFPSCharacterBase;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class WEAPONHANDLNG_API ACharacterController : public APlayerController {
	GENERATED_BODY()

protected:
	/**
 * @brief Called when the controller possesses a pawn.
 * 
 * This function is called when the controller takes control of a pawn. It sets up the
 * EnhancedInputComponent, binds input actions to their handler methods, and applies
 * the input mapping context.
 * 
 * @param aPawn The pawn that the controller possesses.
 */
	virtual void OnPossess(APawn* aPawn) override;

	/**
	 * @brief Called when the controller unpossesses a pawn.
	 * 
	 * This function is called when the controller releases control of a pawn. It performs
	 * necessary clean-up by calling the base class's OnUnPossess method.
	 */
	virtual void OnUnPossess() override;

public:
	/**
	 * @brief Handles the Move action.
	 * 
	 * This function is called when the Move action is triggered. It processes the movement
	 * input value to move the character forward/backward and right/left.
	 * 
	 * @param Value The input value for the Move action.
	 */
	void Move(const FInputActionValue& Value);

	/**
	 * @brief Handles the Look action.
	 * 
	 * This function is called when the Look action is triggered. It processes the look input
	 * value to rotate the character's view. The sensitivity of the rotation depends on whether
	 * the character is aiming or not.
	 * 
	 * @param Value The input value for the Look action.
	 */
	void HandleLookAndAiming(const FInputActionValue& Value);

	void HandleWeaponFire();

private:
	/**
	 * @brief The enhanced input component for the controller.
	 * 
	 * This component handles input actions and is used for binding actions to their handlers.
	 */
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY()
	TObjectPtr<AFPSCharacterBase> PlayerCharacter;
	/**
	 * @brief The input mapping context for the Belica character.
	 * 
	 * This context defines the input mappings for the character's actions and is applied to
	 * the EnhancedInputLocalPlayerSubsystem.
	 */
	UPROPERTY(EditAnywhere, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> MappingContext;

	/**
	 * @brief The Move action.
	 * 
	 * This action represents the movement input for the character.
	 */
	UPROPERTY(EditAnywhere, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/**
	 * @brief The Look action.
	 * 
	 * This action represents the look/aim input for the character.
	 */
	UPROPERTY(EditAnywhere, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireWeaponAction;
	
};
