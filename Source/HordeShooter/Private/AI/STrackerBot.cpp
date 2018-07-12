// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "HordeShooter.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "Gameframework/Character.h"
#include "SHealthComponent.h"
#include "DrawDebugHelpers.h"


// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::OnHealthChanged);

	bUseVelocityChange = true;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;
	bExploded = false;
	ExplosionRadius = 200.f;
	ExplosionDamage = 40.f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	// Find initial move to
	NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

	if (DistanceToTarget > RequiredDistanceToTarget) {
		// Keep moving towards next target
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();

		ForceDirection *= MovementForce;

		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
	} else {
		NextPathPoint = GetNextPathPoint();
	}
}

FVector ASTrackerBot::GetNextPathPoint() {

	// TODO: Improve this for multiplayer
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerCharacter);
	
	if (NavPath->PathPoints.Num() > 1) {

		// Return the point in the world of the next path
		return NavPath->PathPoints[1];
	}

	return GetActorLocation();
}

void ASTrackerBot::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float Damage, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {

	// Explode when ded

	if (!MatInst) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	
	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

	if (Health <= 0) {
		SelfDestruct();
	}
}

void ASTrackerBot::SelfDestruct() {

	if (bExploded) {
		return;
	}

	bExploded = true;

	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	// Apply radial damage
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 5.f, 0, 2.f);
	Destroy();
}