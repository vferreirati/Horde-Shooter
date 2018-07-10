// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Subscribe ourselves to Owner OnTakeAnyDamage thingy
	AActor* Owner = GetOwner();
	if (Owner) {
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeAnyDamage);
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser) {

	// lmao
	if (Damage <= 0) {
		return;
	}

	// Update our health clamped.
	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);
	
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

