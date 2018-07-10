// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "HordeShooter.h"
#include "Components/CapsuleComponent.h"
#include "SHealthComponent.h"


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

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	// Get the movement component, go to the NavAgent and set that this class can Crouch.
	// Otherwise BeginCrouch and EndCrouch WON'T WORK!
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Set the Capsule Component to IGNORE the Weapon channel so the weapon trace hits the mesh and gets the correct Physical Material
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 60;
	ZoomInterpSpeed = 20.f;
	WeaponSocket = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;
	bIsDead = false;
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
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

void ASCharacter::StartFire() {
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire() {
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
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

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float Damage,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {

	if (Health == 0.f && !bIsDead) {
		// Ded
		bIsDead = true;

		// Stop movement
		GetMovementComponent()->StopMovementImmediately();

		// If he's holding a weapon, make sure it stopped firing
		if (CurrentWeapon) {
			CurrentWeapon->StopFire();
		}

		// Disable collision of his CapsuleComp
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Detach from controller
		DetachFromControllerPendingDestroy();

		// Timer to get removed from map
		SetLifeSpan(5.f);
	}
}