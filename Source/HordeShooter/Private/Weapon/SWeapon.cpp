// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShake.h"
#include "HordeShooter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


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
		CollisionParams.bReturnPhysicalMaterial = true;

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

			UParticleSystem* SelectedEffect = nullptr;
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());
			
			switch (SurfaceType) {
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = VulnerableImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}

		PlayFireEffects();
	}
}

void ASWeapon::PlayFireEffects() {
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (CameraShake) {
		APawn* Owner = Cast<APawn>(GetOwner());
		if (Owner) {
			APlayerController* PC = Cast<APlayerController>(Owner->GetController());
			if (PC) {
				PC->ClientPlayCameraShake(CameraShake);
			}
		}
	}
	
}