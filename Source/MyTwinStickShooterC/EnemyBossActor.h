// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyActor.h"
#include "EnemyBossActor.generated.h"

/**
 * 
 */
UCLASS()
class MYTWINSTICKSHOOTERC_API AEnemyBossActor : public AEnemyActor
{
	GENERATED_BODY()
private:
	void UpdateCondition();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/* Properties of the boss, changed with level */
	float Health;
	float MaxHealth;
	float ProjectileSpeed;
	int ProjectileNum;
	FVector BasicGunOffset;
	TArray<FRotator> ProjectileRotation(const FRotator& FireRotation, const float SpanAngleBetweenProjectiles);
	TArray<FVector> ProjectileOffset(const FVector& FireDirection, const float DistanceBetweenProjectiles);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	/* Set properties of the boss related to BossLevel */
	UFUNCTION()
		void SetBossProperty();
	/* sup of ProjectileSpeed */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MaxProjectileSpeed;
	/* Sup of MoveSpeed */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MaxMoveSpeed;	
	/*ParticleSystem that played when boss is nearly dead*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		UParticleSystemComponent * OnFireParticle;	
	/* Health bar for Boss */
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UProgressBar* HealthBarProgress;
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget* HealthBarWidget;
	/* Audio to play when OnFirePartile is active */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class UAudioComponent * OnFireSoundComponent;
	/* The volume of OnFireSound */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float OnFireSoundVolume;
	AEnemyBossActor();
	/* Boss Fire a wave of projectiles */
	virtual void FireShot(FVector FireDirection) override;
	/* Called when boss got a shot*/
	virtual void GotShot() override;
	/* Called when boss is dead */
	void RemoveSelf();
	/* Method to Set BossLevel */
	static void InitLevel(int Level);
	/* Level of the Boss */
	static int BossLevel;
};
