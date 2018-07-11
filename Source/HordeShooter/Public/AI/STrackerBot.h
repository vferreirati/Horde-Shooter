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

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	// Next point in navigation path
	FVector NextPathPoint;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Gets the next path point to player character
	FVector GetNextPathPoint();

	// Function subscribed to HealthComp OnHealthChanged
	UFUNCTION()
	void OnHealthChanged(class USHealthComponent* HealthComponent, float Health, float Damage, 
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
