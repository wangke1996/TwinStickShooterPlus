// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerProjectile.h"
#include "ConstructorHelpers.h"
#include "EnemyActor.h"
#include "EnemyBossActor.h"
#include "MyTwinStickShooterCGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
//#include "Engine.h"
APlayerProjectile::APlayerProjectile() {
	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlayerProjectileMeshAsset(TEXT("/Game/TwinStick/Meshes/PlayerProjectile.PlayerProjectile"));

	// change mesh component for the projectile sphere
	ProjectileMesh->SetStaticMesh(PlayerProjectileMeshAsset.Object);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("PlayerProjectile");
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMesh->OnComponentHit.RemoveAll(this);
	ProjectileMesh->OnComponentHit.AddDynamic(this, &APlayerProjectile::OnHit);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::FormatAsNumber(Speed));
}

void APlayerProjectile::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if (OtherActor != NULL && OtherActor->GetClass()->GetName().Equals("EnemyBossActor")) {
		((AEnemyBossActor *)OtherActor)->GotShot();
	}
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 20.0f, GetActorLocation());
	}
	if (OtherActor != NULL && OtherActor->GetClass()->GetName().Equals("EnemyActor")) {
		((AEnemyActor *)OtherActor)->GotShot();
		((AMyTwinStickShooterCGameMode*)(GetWorld()->GetAuthGameMode()))->EnemyDestroyedByPlayer();
	}
	Destroy();
}

void APlayerProjectile::BeginPlay()
{
	Super::BeginPlay();
	//ProjectileMesh->OnComponentHit.AddDynamic(this, &APlayerProjectile::OnHit);		// set up a notification for when this component hits something
}
