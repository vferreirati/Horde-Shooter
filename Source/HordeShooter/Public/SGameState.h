// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8 {
	WaitingToStart,

	// Wave was complete, waiting the delay between waves
	WaveComplete,

	// Wave is playing and bots are being spawned
	WaveInProgress,

	// Wave is playing but no bots are being spawned
	WaitingToComplete,

	// All players are dead
	GameOver
};

/**
 * 
 */
UCLASS()
class HORDESHOOTER_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	
public:
	void SetWaveState(EWaveState NewState);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WaveState, BlueprintReadOnly, Category = "GameState")
	EWaveState WaveState;

protected:
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);
};
