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
#include "Net/UnrealNetwork.h"


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

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	BulletSpread = 2.f;
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

	// If who called this function isn't the server
	if (Role < ROLE_Authority) {
		// We ASK the server to run this code
		ServerFire();
	}

	AActor* Owner = GetOwner();
	if (Owner) {
		FVector EyesLocation;
		FRotator EyesRotation;
		Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);

		FHitResult OutHit;
		FVector StartLocation = EyesLocation;
		FVector ShotDirection = EyesRotation.Vector();
		
		float HalfRad = FMath::DegreesToRadians(BulletSpread);

		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

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

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		// Blocking hit!
		if (bSuccess) {			
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());
			EndLocation = OutHit.ImpactPoint;

			float DamageToApply = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE) {
				DamageToApply *= CriticalMultiplier;
			}

			UGameplayStatics::ApplyPointDamage(OutHit.GetActor(), DamageToApply, ShotDirection, OutHit, Owner->GetInstigatorController(), Owner, DamageType);

			PlayImpactEffect(SurfaceType, OutHit.ImpactPoint);
		}

		// TODO: Find a way to remove this
		if (Role == ROLE_Authority) {
			HitScanTrace.TraceTo = EndLocation;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		PlayFireEffects();
		LastFiredTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::ServerFire_Implementation() {
	Fire();
}

bool ASWeapon::ServerFire_Validate() {
	return true;
}

void ASWeapon::OnRep_HitScanTrace() {
	// Play cosmetic FX
	PlayFireEffects();
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
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
	
	if (FireSound) {
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MeshComp->GetSocketLocation(MuzzleSocketName));
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint) {
	
	UParticleSystem* SelectedEffect = nullptr;

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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}