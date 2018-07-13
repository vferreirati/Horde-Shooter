// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "TimerManager.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.f;
	NumberOfTicks = 0.f;
}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASPowerupActor::OnTickPowerup() {
	++TicksProcessed;

	OnPowerupTicked();

	if (TicksProcessed >= NumberOfTicks) {
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup() {
	
	OnActivated();

	if (PowerupInterval > 0.f) {
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	} else {
		OnTickPowerup();
	}
}