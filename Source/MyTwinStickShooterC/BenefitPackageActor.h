// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BenefitPackageActor.generated.h"

UCLASS()
class MYTWINSTICKSHOOTERC_API ABenefitPackageActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABenefitPackageActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FString PackageType;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* PackageMesh;
	/** Package movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* PackageMovement;
	/* Health Benifit each time*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GamePlay, meta = (AllowPrivateAccess = "true"))
		float HealthBenift;

	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		USoundBase* BenefitSound;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float BenefitSoundVolume;
	const static ECollisionChannel CollisionChannel = ECC_GameTraceChannel2;
};
