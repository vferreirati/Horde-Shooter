// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

UCLASS()
class HORDESHOOTER_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UDecalComponent* DecalComp;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	TSubclassOf<class ASPowerupActor> PowerupClass;

	// Amount of seconds that takes for a new powerup to respawn
	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CooldownDuration;

	class ASPowerupActor* PowerupInstance;

	FTimerHandle TimerHandle_RespawnTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Respawn the powerup of this pickup
	void Respawn();
};
