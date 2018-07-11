// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "HordeShooter.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "Gameframework/Character.h"
#include "SHealthComponent.h"


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
		UE_LOG(LogTemp, Warning, TEXT("Force added"))
	} else {
		NextPathPoint = GetNextPathPoint();
		UE_LOG(LogTemp, Warning, TEXT("Called next point"))
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

	// TODO: Pulse the material on hit
	if (!MatInst) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	
	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())
}