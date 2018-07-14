// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HORDESHOOTER_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	// Timer that deals with bot spawning logic
	FTimerHandle TimerHandle_BotSpawner;

	// Handles the time between waves
	FTimerHandle TimerHandle_NextWaveStart;

	// Bots to spawn in current wave.
	int32 NumberOfBotsToSpawn;

	// Current wave number
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "Gamemode")
	float TimeBetweenWaves;

protected:

	// Spawns a bot in the level
	// Implemented in blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "Gamemode")
	void SpawnNewBot();

	// Verifies if there's a bot slot in the current wave
	// Spawns only if true.
	void SpawnBotTimerElapsed();
	
	// Start spawning bots
	void StartWave();

	// Stop spawning bots
	void EndWave();

	// Sets timer for next wave 
	void PrepareForNextWave();

	// Checks if there's any bot alive. 
	// If all bots are dead AND BotsToSpawn == 0, ends the current wave
	void CheckWaveState();

	//Checks if there's any player alive
	// If all players are dead, ends the game.
	void CheckAnyPlayerAlive();

	// Ends the game
	void GameOver();
};
