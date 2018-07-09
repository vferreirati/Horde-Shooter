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
#include "TimerManager.h"


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Default weapon range -> 100 meters
	WeaponRange = 10000.f;
	MuzzleSocketName = "MuzzleSocket";
	BaseDamage = 15;
	CriticalMultiplier = 2;
	RateOfFire = 600;
}

void ASWeapon::BeginPlay() {
	Super::BeginPlay();

	FireDelay = 60 / RateOfFire;
}

void ASWeapon::StartFire() {
	float FirstShotDelay = LastFiredTime + FireDelay - GetWorld()->TimeSeconds;
	FirstShotDelay = FMath::Clamp(FirstShotDelay, 0.f, FireDelay);
	GetWorldTimerManager().SetTimer(TimerHandle_WeaponFire, this, &ASWeapon::Fire, FireDelay, true, FirstShotDelay);
}

void ASWeapon::StopFire() {
	GetWorldTimerManager().ClearTimer(TimerHandle_WeaponFire);
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
			COLLISION_WEAPON,
			CollisionParams
		);

		// Blocking hit!
		if (bSuccess) {			

			UParticleSystem* SelectedEffect = nullptr;
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());

			float DamageToApply = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE) {
				DamageToApply *= CriticalMultiplier;
			}
			
			switch (SurfaceType) {
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = VulnerableImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			UGameplayStatics::ApplyPointDamage(OutHit.GetActor(), DamageToApply, ShotDirection, OutHit, Owner->GetInstigatorController(), this, DamageType);

			if (SelectedEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}

		PlayFireEffects();
		LastFiredTime = GetWorld()->TimeSeconds;
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