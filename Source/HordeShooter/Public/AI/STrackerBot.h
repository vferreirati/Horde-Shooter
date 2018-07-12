// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class HORDESHOOTER_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	class USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	class USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	bool bExploded;

	bool bTimerHasStarted;

	// Next point in navigation path
	FVector NextPathPoint;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	FTimerHandle TimerHandle_SelfDamage;

	FTimerHandle TimerHandle_CheckForBots;

	UPROPERTY(BlueprintReadOnly, Category = "TrackerBot")
	// Current power level of this bot. Increases with the amount of nearby bots
	int32 CurrentPowerLevel;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Gets the next path point to player character
	FVector GetNextPathPoint();

	// Explode the bot dealing radial damage
	void SelfDestruct();

	// Inflicts damage to self.
	UFUNCTION()	
	void DamageSelf();

	// Gets the amount of nearby bots and calculates the power level
	UFUNCTION()
	void CalculatePowerLevel();

	// Function subscribed to HealthComp OnHealthChanged
	UFUNCTION()
	void OnHealthChanged(class USHealthComponent* HealthComponent, float Health, float Damage, 
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
