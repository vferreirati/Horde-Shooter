// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Get the movement component, go to the NavAgent and set that this class can Crouch.
	// Otherwise BeginCrouch and EndCrouch WON'T WORK!
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	ZoomedFOV = 60;
	ZoomInterpSpeed = 20.f;
	WeaponSocket = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;
	SpawnDefaultWeapon();
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind movement inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	// Bind look inputs
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	// Bind crouch inputs
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	// Bind jump inputs
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::BeginJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASCharacter::EndJump);

	// Bind zoom inputs
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	// Bind fire inputs
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::Fire);
}

void ASCharacter::MoveForward(float Value) {
	if (Value != 0.f) {
		AddMovementInput(GetActorForwardVector() * Value);
	}
}

void ASCharacter::MoveRight(float Value) {
	if (Value != 0.f) {
		AddMovementInput(GetActorRightVector() * Value);
	}
}

void ASCharacter::BeginCrouch() {
	Crouch();
}

void ASCharacter::EndCrouch() {
	UnCrouch();
}

void ASCharacter::BeginJump() {
	Jump();
}

void ASCharacter::EndJump() {
	StopJumping();
}

FVector ASCharacter::GetPawnViewLocation() const {
	
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::BeginZoom() {
	bWantsToZoom = true;
}

void ASCharacter::EndZoom() {
	bWantsToZoom = false;
}

void ASCharacter::Fire() {
	if (CurrentWeapon) {
		CurrentWeapon->Fire();
	}
}

void ASCharacter::SpawnDefaultWeapon() {
	if (!DefaultWeapon) {
		UE_LOG(LogTemp, Error, TEXT("SCHARACTER DOESN'T HAVE A DEFAULT WEAPON CLASS"))
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentWeapon) {
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);

	} else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn character default weapon"))
	}
}