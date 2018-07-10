// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);

	ExplosionDamage = 80;
	ExplosionRadius = 200;
	ExplosionImpulse = 1000;
	ExplosionUpwardsImpulse = 500;
	bExploded = false;	
}

void ASExplosiveBarrel::BeginPlay() {
	Super::BeginPlay();

	// Set the values of the RadialForceComponent on BeginPlay
	// In case those values were changed on blueprint
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->ImpulseStrength = ExplosionImpulse;
	RadialForceComp->bImpulseVelChange = true;	// Velocity change should ALWAYS be true.
}

// USHealthComponent* HealthComponent, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float Damage, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {

	if (bExploded) {
		return;
	}

	if (Health == 0) {
		// ded
		bExploded = true;

		// Play Explosion effect
		if (ExplosionEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		}
		
		// Launch Barrel Upwards
		MeshComp->AddImpulse(FVector(0.f, 0.f, ExplosionUpwardsImpulse), NAME_None, true);

		// Change MeshComp Material
		if (ExplodedMaterial) {
			MeshComp->SetMaterial(0, ExplodedMaterial);
		}

		// Apply radial force
		RadialForceComp->FireImpulse();

		// Apply radial damage
		TArray<AActor*> IgnoredActors;
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageTypeClass, IgnoredActors);

		UE_LOG(LogTemp, Warning, TEXT("The barrel exploded causing %s damage, with a explosion radius of %s and applying a radial force of %s!"), *FString::SanitizeFloat(ExplosionDamage), *FString::SanitizeFloat(ExplosionRadius), *FString::SanitizeFloat(ExplosionImpulse))
	}
}