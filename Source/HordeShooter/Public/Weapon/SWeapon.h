// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

UCLASS()
class HORDESHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFire();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float WeaponRange;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float CriticalMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class UDamageType> DamageType;

	// Name of the socket that MuzzleEffect will be attached to
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	// Visual Effect that will be spawned at the barrel of the Gun
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class UParticleSystem* MuzzleEffect;

	// Visual Effect that will be spawned at hit location of the bullet
	// If the Surface hit is Default
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class UParticleSystem* DefaultImpactEffect;

	// Visual Effect that will be spawned at hit location of the bullet
	// If the Surface hit is Vulnerable
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class UParticleSystem* VulnerableImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class UCameraShake> CameraShake;

	FTimerHandle TimerHandle_WeaponFire;

	// Holds the last time this weapon was shot
	float LastFiredTime;

	// Fire rate of the weapon. Should be used as Bullets Per Minute.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// Helper variable to determine the amount of time between shots.
	// 60 - ROF = FireDelay
	float FireDelay;

protected:

	// Plays the fire effect at the weapon muzzle
	void PlayFireEffects();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
};
