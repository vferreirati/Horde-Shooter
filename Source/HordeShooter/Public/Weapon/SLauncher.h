// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/SWeapon.h"
#include "SLauncher.generated.h"

/**
 * 
 */
UCLASS()
class HORDESHOOTER_API ASLauncher : public ASWeapon
{
	GENERATED_BODY()
	
	
protected:
	// Class of the projectile that will be spawned by this weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AActor> ProjectileClass;
	
protected:
	// Spawns a projectile in the world.
	// See ProjectileClass.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire() override;
};
