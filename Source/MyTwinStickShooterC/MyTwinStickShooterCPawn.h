// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyTwinStickShooterCPawn.generated.h"

UCLASS(Blueprintable)
class AMyTwinStickShooterCPawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;


public:
	AMyTwinStickShooterCPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;
	

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class USoundBase* FireSound;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float FireSoundVolume;
	/** Sound to play when ship on fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class USoundBase* BoomSound;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float BoomSoundVolume;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class UAudioComponent * OnFireSoundComponent;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float OnFireSoundVolume;
	/* The health bar Widget */
	//UPROPERTY(EditAnywhere, Category = HealthBarWidgetComponent)
	//	class UWidgetComponent* HealthBarWidgetComponent;
	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface



	/* Fire a shot in the specified direction */
	void FireShot(FVector FireDirection);

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

protected:
	virtual void BeginPlay() override;
	// calculate the offsets of projectiles, notice that there could be at most 3 projectiles shot out when player firing
	TArray<FVector> ProjectileOffset(const FVector& FireDirection, const float DistanceBetweenProjectiles);
	// spawn projectiles when polayer firing
	void SpawnProjectileWave(const FVector & FireDirection);
	// change camera to full view or local view when player dead or restart the game
	void ChangeCamera();

private:

	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/* A Number indicate the health condition*/
	float Health;
	/* Peroid of player fire */
	float FireRate;
	/* how many projectiles spawed each time player firing */
	int ProjectileNum;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	/*Update condition when health reduced*/
	void UpdateCondition();

	/*HidePlayer when player dead*/
	void HidePlayer();
	/*If the player is dead*/
	bool bPlayerIsDead;
	/*If the player is in low health condition*/
	bool bPlayerInBadCondition;


public:
	/*Max Health Number*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MaxHealth;
	/*Function to reduce health when hitted*/
	void ReduceHealth(float HowMuchReduce);
	/*Visible Player and change camera to local view when restart game*/
	void ResetPlayer();
	/*ParticleSystem that played when player dead*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		UParticleSystem * BoomParticle;
	/*Camera used after player dead*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		AActor * FullViewCamera;
	/* smooth time for camera change*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float SmoothBlendTime;

	/* sup and inf of FireRate */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float InitFireRate;
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float FastestFireRate;
	/* sup and inf of ProjectileNum */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		int InitProjectileNum;
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		int MaxProjectileNum;
	/*ParticleSystem that played when player is nearly dead*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		UParticleSystemComponent * OnFireParticle;
	/* Function to call when the Player eat benifit package */
	UFUNCTION()
		void GetBenefit(float HealthBenefit);
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};

