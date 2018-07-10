// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// Event called when health changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComponent, float, Health, float, Damage, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class HORDESHOOTER_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;
	

protected:	
	// Called when the game starts
	virtual void BeginPlay() override;

	// Function used on subscription of Owner->OnTakeAnyDamage()
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
