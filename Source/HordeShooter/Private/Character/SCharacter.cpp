// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Components/InputComponent.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind movement inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
}

void ASCharacter::MoveForward(float Value) {
	if (Value != 0.f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASCharacter::MoveRight(float Value) {
	if (Value != 0.f) {
		AddMovementInput(GetActorRightVector(), Value);
	}
}

