// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyTwinStickShooterCProjectile.h"
#include "PlayerProjectile.generated.h"

/**
 * 
 */
UCLASS()
class MYTWINSTICKSHOOTERC_API APlayerProjectile : public AMyTwinStickShooterCProjectile
{
	GENERATED_BODY()

public: 
	APlayerProjectile();
	
	/** Function to handle the projectile hitting something */
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
protected:
	virtual void BeginPlay() override;
};
