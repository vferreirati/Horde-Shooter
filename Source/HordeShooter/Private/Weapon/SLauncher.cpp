// Fill out your copyright notice in the Description page of Project Settings.

#include "SLauncher.h"
#include "Engine/World.h"

void ASLauncher::Fire() {

	if (!ProjectileClass) {
		UE_LOG(LogTemp, Error, TEXT("PROJECTILE CLASS NOT SET"))
		return;
	}
	AActor* Owner = GetOwner();
	if (Owner) {
		FVector TrashLocation;
		FRotator SpawnRotation;

		Owner->GetActorEyesViewPoint(TrashLocation, SpawnRotation);

		FVector SpawnLocation = MeshComp->GetSocketLocation(MuzzleSocketName);		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

}