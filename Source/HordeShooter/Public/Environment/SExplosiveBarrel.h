// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

UCLASS()
class HORDESHOOTER_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class URadialForceComponent* RadialForceComp;

	// Material to be applied on Explosion
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	class UMaterialInterface* ExplodedMaterial;

	// Effect that will be played on Explosion
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionDamage;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionImpulse;

	// Force to be applied upwards of the Barrel
	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionUpwardsImpulse;

	bool bExploded;

protected:
	// Function subscribed to HealthComponent OnHealthChanged
	UFUNCTION()
	void OnHealthChanged(class USHealthComponent* HealthComponent, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Explode this barrel!!!
	void Explode();
	
};
