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
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"


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

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200.f);

	// SphereComp optimization code
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetCollisionObjectType(ECC_Pawn);	// Set ourselves as Pawn
	SphereComp->SetupAttachment(MeshComp);

	bUseVelocityChange = true;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;
	bExploded = false;
	ExplosionRadius = 350.f;
	ExplosionDamage = 40.f;
	SelfDamageInterval = 0.25;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority) {
		// Find initial move to
		NextPathPoint = GetNextPathPoint();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_CheckForBots, this, &ASTrackerBot::CalculatePowerLevel, 1.f, true);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded) {
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
	
}

FVector ASTrackerBot::GetNextPathPoint() {

	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* TestPawn = It->Get();

		if (!TestPawn || USHealthComponent::IsFriendly(TestPawn, this)) {
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0) {

			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if (Distance < NearestTargetDistance) {

				NearestTargetDistance = Distance;
				BestTarget = TestPawn;
			}

		}
	}

	if (BestTarget) {
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1) {

			// Return the point in the world of the next path
			return NavPath->PathPoints[1];
		}
	}

	return GetActorLocation();
}

void ASTrackerBot::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float Damage, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) {

	// Explode when ded
	if (Health <= 0) {
		SelfDestruct();
	}

	if (!MatInst) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	
	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

}

void ASTrackerBot::SelfDestruct() {

	if (bExploded) {
		return;
	}

	bExploded = true;

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetSimulatePhysics(false);

	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	if (Role == ROLE_Authority) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float DamageToApply = ExplosionDamage;

		if (CurrentPowerLevel > 0) {
			++CurrentPowerLevel;
			DamageToApply *= CurrentPowerLevel;
		}

		// Apply radial damage
		UGameplayStatics::ApplyRadialDamage(this, DamageToApply, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		SetLifeSpan(2.f);
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor) {
	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

	Super::NotifyActorBeginOverlap(OtherActor);

	if (bTimerHasStarted || bExploded) {
		return;
	}
	
	// If we overlapped with a player
	if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this)) {
		bTimerHasStarted = true;

		// Play the Trackerbot warning
		if (SelfDestructSound) {
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}

		if (Role == ROLE_Authority) {
			// Set timer to inflict damage to self until explosion
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.f);
			
		}
	}
}

void ASTrackerBot::DamageSelf() {
	UGameplayStatics::ApplyDamage(this, 20.f, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::CalculatePowerLevel() {

	// Gets the Amount of Bots overlapping us
	TArray<AActor*> OverlappedActors;
	TSubclassOf<ASTrackerBot> OwnClass = this->GetClass();
	SphereComp->GetOverlappingActors(OverlappedActors, OwnClass);
	CurrentPowerLevel = OverlappedActors.Num();

	// Gets the total of bots in the world
	TArray<AActor*> TotalBots;
	UGameplayStatics::GetAllActorsOfClass(this, OwnClass, TotalBots);
	int32 MaxPowerLevel = TotalBots.Num();	

	if (!MatInst) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst) {
		float Alpha = CurrentPowerLevel / (float)MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

void ASTrackerBot::RefreshPath() {
	NextPathPoint = GetNextPathPoint();
}