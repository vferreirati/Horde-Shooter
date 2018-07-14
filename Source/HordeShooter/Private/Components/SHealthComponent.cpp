// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;
	
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// If the owner is the server
	if (GetOwnerRole() == ROLE_Authority) {

		// Subscribe ourselves to Owner OnTakeAnyDamage thingy
		AActor* Owner = GetOwner();
		if (Owner) {
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeAnyDamage);
		}
	}

	

	Health = DefaultHealth;
}

void USHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser) {

	// lmao
	if (Damage <= 0) {
		return;
	}

	// Update and clamp Health
	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);
	
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void USHealthComponent::OnRep_Health(float OldHealth) {

	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::Heal(float HealAmount) {
	if (HealAmount <= 0 || Health <= 0.f) {
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float USHealthComponent::GetHealth() const {
	return Health;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}