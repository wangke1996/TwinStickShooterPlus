// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyActor.generated.h"


UCLASS()
class MYTWINSTICKSHOOTERC_API AEnemyActor : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AEnemyActor();

protected:
	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/* Flag to dead*/
	bool bIsDead;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	FVector GetMoveDirection();
	FVector GetFireDirection();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Fire a shot in the specified direction */
	virtual void FireShot(FVector FireDirection);

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	/* Sound to play when destroyed */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class USoundBase* BoomSound;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float BoomSoundVolume;
	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		FVector GunOffset;

	/*Explosion Animation*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		UParticleSystem * BoomParticle;

	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MoveSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent *ShipMeshComponent;

	virtual void GotShot();
};
