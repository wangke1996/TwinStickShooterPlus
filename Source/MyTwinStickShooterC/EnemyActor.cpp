// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyActor.h"
#include "MyTwinStickShooterCProjectile.h"
#include "MyTwinStickShooterCPawn.h"
#include "Engine/World.h"
#include "Public/TimerManager.h"
#include "Kismet/GameplayStatics.h "
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
FVector AEnemyActor::GetMoveDirection()
{
	//AMyTwinStickShooterCPawn* PlayerCharacter = UGameplayStatics::get;
	APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();
	//if The player has been damaged
	if (Player == nullptr)
		return FVector(0.f);
	else {
		const FVector PlayerLocation = Player->GetActorLocation();
		FVector MoveDirection = PlayerLocation - AEnemyActor::GetActorLocation();
		MoveDirection.Z = 0.f;
		return MoveDirection.GetClampedToMaxSize(1.f);
	}
}

FVector AEnemyActor::GetFireDirection()
{
	return GetMoveDirection();
}

// Sets default values
AEnemyActor::AEnemyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/EnemyUFO.EnemyUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);

	// Create the boomup particle system compoent
	BoomParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("BoomParticle"));// BoomParticleAsset;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> BoomParticleAsset(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (BoomParticleAsset.Succeeded())
	{
		BoomParticle = BoomParticleAsset.Object;
	}
	// the Boom sound to play when destroyed
	static ConstructorHelpers::FObjectFinder<USoundBase> BoomAudio(TEXT("SoundWave'/Game/StarterContent/Audio/Explosion02.Explosion02'"));
	BoomSound = BoomAudio.Object;

	BoomSoundVolume = 0.5f;

	// Movement
	MoveSpeed = 100.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 1.f;
	bCanFire = true;
	bIsDead = false;
}

// Called when the game starts or when spawned
void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemyActor::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (!FireDirection.IsNearlyZero())
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector PorjectileLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<AMyTwinStickShooterCProjectile>(PorjectileLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AEnemyActor::ShotTimerExpired, FireRate);
			
			bCanFire = false;
		}
	}
}

void AEnemyActor::ShotTimerExpired()
{
	bCanFire = true;
}

// Called every frame
void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsDead) {
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BoomSound, GetActorLocation(),FRotator::ZeroRotator,BoomSoundVolume);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomParticle, FTransform(GetActorRotation(), GetActorLocation(), GetActorScale()), true);
		Destroy();
	}
	// Find movement direction
	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = GetMoveDirection().GetClampedToMaxSize(1.0f);
	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaTime;

	// If non-zero size, move this actor
	if (!Movement.IsNearlyZero())
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}

	// Create fire direction vector
	const FVector FireDirection = GetFireDirection();

	// Try and fire a shot
	FireShot(FireDirection);
}

void AEnemyActor::GotShot()
{
	bIsDead = true;
}

