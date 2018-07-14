// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"

ASGameMode::ASGameMode() {
	TimeBetweenWaves = 2.f;

	// Sets can tick
	PrimaryActorTick.bCanEverTick = true;

	// Sets the tick to every 1 second
	PrimaryActorTick.TickInterval = 1.f;
}

void ASGameMode::StartPlay() {
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave() {
	++WaveCount;
	NumberOfBotsToSpawn = 2 * WaveCount;

	// Sets timer to spawn bots
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);
}

void ASGameMode::EndWave() {
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

}

void ASGameMode::SpawnBotTimerElapsed() {
	SpawnNewBot();

	--NumberOfBotsToSpawn;

	if (NumberOfBotsToSpawn <= 0) {
		EndWave();
	}
}

void ASGameMode::PrepareForNextWave() {
	
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::CheckWaveState() {

	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	// Exit if we're still spawning bots OR the delay has already started
	if (NumberOfBotsToSpawn > 0 || bIsPreparingForWave) {
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* TestPawn = It->Get();

		// if the pawn is null or controlled by the player, skip
		if (!TestPawn || TestPawn->IsPlayerControlled()) {
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0) {
			bIsAnyBotAlive = true; 
			break;
		}
	}

	if (!bIsAnyBotAlive) {
		PrepareForNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive() {
	bool bAnyPlayerAlive = false;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn()) {
			
			APawn* PlayerPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(PlayerPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			
			if (ensure(HealthComp) && HealthComp > 0) {
				bAnyPlayerAlive = true;			// There's at least 1 player alive
				break;
			}
		}
	}

	// if all players are dead, end game
	if (!bAnyPlayerAlive) {
		GameOver();
	}
}

void ASGameMode::GameOver() {
	EndWave();

	// TODO: Finish the match, present game over to players

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died!"));
}