// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Default weapon range -> 100 meters
	WeaponRange = 10000.f;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::Fire() {

	AActor* Owner = GetOwner();
	if (Owner) {
		FVector EyesLocation;	// Used as StartLocation
		FRotator EyesRotation;	// Garbage, not used by this function.
		Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);

		FHitResult OutHit;
		FVector StartLocation = EyesLocation;
		FVector EndLocation = EyesLocation + (EyesRotation.Vector() * WeaponRange);
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Owner);
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.bTraceComplex = true;

		// Trace the World pawn eyes to crosshair location
		bool bSuccess = GetWorld()->LineTraceSingleByChannel(
			OutHit,
			StartLocation,
			EndLocation,
			ECC_Visibility,
			CollisionParams
		);

		// Blocking hit!
		if (bSuccess) {
			// TODO: Apply damage to hit Actor
		}

		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.f, 0, 1.f);
	}
}