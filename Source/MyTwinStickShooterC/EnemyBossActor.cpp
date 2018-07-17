// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBossActor.h"
#include "Kismet/GameplayStatics.h "
#include "MyTwinStickShooterCProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyTwinStickShooterCGameMode.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/ProgressBar.h"
#include "UserWidget.h"
#include "ConstructorHelpers.h"
#include "Components/AudioComponent.h"
TArray<FRotator> AEnemyBossActor::ProjectileRotation(const FRotator& FireRotation, const float SpanAngleBetweenProjectiles)
{
	TArray<FRotator> FireRotations;
	FireRotations.Add(FireRotation);
	int Num = ProjectileNum - 1;
	float DeltaAngel = SpanAngleBetweenProjectiles * 2.f / Num;
	float AngelFromCenter = 0.f;
	while (Num > 0) {
		AngelFromCenter += DeltaAngel;
		FRotator Rotation = FireRotation;
		Rotation.Yaw = FireRotation.Yaw - AngelFromCenter;
		FireRotations.Add(FRotator(Rotation));
		Rotation.Yaw = FireRotation.Yaw + AngelFromCenter;
		FireRotations.Add(FRotator(Rotation));
		Num -= 2;
	}

	return FireRotations;
}
TArray<FVector> AEnemyBossActor::ProjectileOffset(const FVector& FireDirection, const float DistanceBetweenProjectiles = 100.f)
{
	float FireForwardValue = FireDirection.X;
	float FireRightValue = FireDirection.Y;
	float DistanceFromCenter;
	TArray<FVector> Offset;
	Offset.Add(FVector(0.f));
	int Num = ProjectileNum-1;
	DistanceFromCenter = DistanceBetweenProjectiles;
	while (Num > 0) {
		Num -= 2;
		Offset.Add(FVector(DistanceFromCenter*FireRightValue, -DistanceFromCenter * FireForwardValue, 0.f).GetClampedToMaxSize(DistanceFromCenter));
		Offset.Add(FVector(-DistanceFromCenter * FireRightValue, DistanceFromCenter * FireForwardValue, 0.f).GetClampedToMaxSize(DistanceFromCenter));
		DistanceFromCenter += DistanceBetweenProjectiles;
	}
	return Offset;
}
void AEnemyBossActor::SetBossProperty()
{
	MaxHealth = BossLevel * 10;
	int Scale = BossLevel + 2;
	SetActorScale3D(FVector(Scale));
	GunOffset = BasicGunOffset * Scale;
	ProjectileNum = 1 + BossLevel*2;
	ProjectileSpeed = FMath::Clamp<float>(500.f+100*BossLevel, 500.f, MaxProjectileSpeed);
	FireRate = 2.f / BossLevel;
	MoveSpeed = FMath::Clamp<float>(100.f*BossLevel, 100.f, MaxMoveSpeed);
}

AEnemyBossActor::AEnemyBossActor()
{
	BasicGunOffset = GunOffset;
	PrimaryActorTick.bCanEverTick = true;
	MaxMoveSpeed = 500.f;
	MaxProjectileSpeed =1500.f;
	SetBossProperty();
	Health = MaxHealth;
	// Create the onfire particle system component
	OnFireParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OnFireParticle"));
	OnFireParticle->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	OnFireParticle->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> OnFireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (OnFireParticleAsset.Succeeded())
	{
		OnFireParticle->SetTemplate(OnFireParticleAsset.Object);
	}



	static ConstructorHelpers::FObjectFinder<USoundBase> OnFireAudio(TEXT("SoundWave'/Game/StarterContent/Audio/Fire01.Fire01'"));
	USoundBase* OnFireSound = OnFireAudio.Object;
	OnFireSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OnFireAudio"));
	OnFireSoundComponent->bAutoActivate = false;
	if (OnFireSound != nullptr)
		OnFireSoundComponent->SetSound(OnFireSound);
	OnFireSoundVolume = 1.f;
	BoomSoundVolume = 1.f;
	OnFireSoundComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

}
void AEnemyBossActor::FireShot(FVector FireDirection)
{
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (!FireDirection.IsNearlyZero())
		{

			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
			float ZOffset = PlayerLocation.Z - GetActorLocation().Z;//Make Sure that the projectile is at the same height with player;
			const FVector GunLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset) + FVector(0.f, 0.f, ZOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{

				TArray<FRotator> Rotations = ProjectileRotation(FireRotation, 45.f);
				TArray<FVector> Offsets = ProjectileOffset(FireDirection, 100.f);
				
				for (int i = 0; i < ProjectileNum; i++) {
					const FRotator Rotation = Rotations[i];
					const FVector Offset = Offsets[i];

					AMyTwinStickShooterCProjectile* BossProjectile = World->SpawnActor<AMyTwinStickShooterCProjectile>(GunLocation+Offset, Rotation);
					BossProjectile->GetProjectileMovement()->SetVelocityInLocalSpace(FVector(ProjectileSpeed, 0.f, 0.f));
					
				}
				//system("pause");
			}
			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AEnemyBossActor::ShotTimerExpired, FireRate);

			bCanFire = false;
		}
	}
}
void AEnemyBossActor::GotShot()
{
	Health--;
	UpdateCondition();
}
void AEnemyBossActor::RemoveSelf()
{
	HealthBarWidget->RemoveFromParent();
	Destroy();
}
void AEnemyBossActor::InitLevel(int Level)
{
	BossLevel = Level;
}
// Called when the game starts or when spawned
void AEnemyBossActor::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	UClass* Widget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_BossHealthBarWidget.BP_BossHealthBarWidget_C'"));
	HealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), Widget);
	HealthBarWidget->AddToViewport();
	HealthBarProgress = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(TEXT("HealthBarProgress")));
	HealthBarProgress->SetPercent(1.f);
}
void AEnemyBossActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
void AEnemyBossActor::UpdateCondition()
{
	HealthBarProgress->SetPercent(Health / MaxHealth);
	if (Health <= 0) {
		// turn off fire and turn on explosion
		OnFireParticle->DeactivateSystem();
		OnFireSoundComponent->FadeOut(0.f, 0.f);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BoomSound, GetActorLocation(), FRotator::ZeroRotator, BoomSoundVolume);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomParticle, FTransform(GetActorRotation(), GetActorLocation(), GetActorScale()), true);
		((AMyTwinStickShooterCGameMode*)(UGameplayStatics::GetGameMode(GetWorld())))->BossDestroyedByPlayer();
		RemoveSelf();
	}
	else if (Health <= 0.3f*MaxHealth) {
		OnFireParticle->ActivateSystem();
		if(!(OnFireSoundComponent->IsPlaying()))
			OnFireSoundComponent->FadeIn(1.f, OnFireSoundVolume);
	}
}