// Fill out your copyright notice in the Description page of Project Settings.

#include "BenefitPackageActor.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyTwinStickShooterCPawn.h"
#include "Engine.h"
#include "MyTwinStickShooterCProjectile.h"
// Sets default values
ABenefitPackageActor::ABenefitPackageActor()
{
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/TwinStick/Meshes/BenefitPackage.BenefitPackage"));

	static ConstructorHelpers::FObjectFinder<USoundBase> BenifitSoundSource(TEXT("SoundWave'/Game/TwinStick/Audio/GetBenefit.GetBenefit'"));
	BenefitSound = BenifitSoundSource.Object;
	BenefitSoundVolume = 1.f;

	// Create mesh component for the projectile sphere
	PackageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PackageMesh"));
	PackageMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	PackageMesh->SetupAttachment(RootComponent);
	PackageMesh->BodyInstance.SetCollisionProfileName("BenefitPackage");
	PackageMesh->OnComponentHit.AddDynamic(this, &ABenefitPackageActor::OnHit);		// set up a notification for when this component hits something
	PackageMesh->SetCollisionResponseToChannel(AMyTwinStickShooterCProjectile::CollisionChannel, ECollisionResponse::ECR_Ignore);
	RootComponent = PackageMesh;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	PackageMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PackageMovement0"));
	PackageMovement->UpdatedComponent = PackageMesh;
	PackageMovement->InitialSpeed = 500.f;
	PackageMovement->MaxSpeed = 1000.f;
	PackageMovement->bRotationFollowsVelocity = true;
	PackageMovement->bShouldBounce = true;
	PackageMovement->Bounciness = 1.f;
	PackageMovement->ProjectileGravityScale = 0.f; // No gravity

													  // Die after 60 seconds by default
	InitialLifeSpan = 0.0f;

	HealthBenift = 3.f;
}

// Called when the game starts or when spawned
void ABenefitPackageActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABenefitPackageActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PackageMovement->Velocity.Z = 0.f;
}

void ABenefitPackageActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, Hit.GetActor()->GetName());

	if (OtherActor != NULL && OtherActor->GetClass()->GetName().Equals("MyTwinStickShooterCPawn")) {
		((AMyTwinStickShooterCPawn *)OtherActor)->GetBenefit(HealthBenift);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BenefitSound, GetActorLocation(), FRotator::ZeroRotator, BenefitSoundVolume);
		Destroy();
	}
	else if (OtherActor != NULL && OtherActor->GetClass()->GetName().Contains("Projectile"))
		return;

}
