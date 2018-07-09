// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Default weapon range -> 100 meters
	WeaponRange = 10000.f;
	MuzzleSocketName = "MuzzleSocket";
}

void ASWeapon::Fire() {

	AActor* Owner = GetOwner();
	if (Owner) {
		FVector EyesLocation;
		FRotator EyesRotation;
		Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);

		FHitResult OutHit;
		FVector StartLocation = EyesLocation;
		FVector ShotDirection = EyesRotation.Vector();
		FVector EndLocation = EyesLocation + (ShotDirection * WeaponRange);
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Owner);
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.bTraceComplex = true;

		bool bSuccess = GetWorld()->LineTraceSingleByChannel(
			OutHit,
			StartLocation,
			EndLocation,
			ECC_Visibility,
			CollisionParams
		);

		// Blocking hit!
		if (bSuccess) {
			// TODO: Move damage logic to another function
			AActor* HitActor = OutHit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20, ShotDirection, OutHit,	Owner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}

		PlayFireEffects();
	}
}

void ASWeapon::PlayFireEffects() {
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}