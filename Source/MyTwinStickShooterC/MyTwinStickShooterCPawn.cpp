// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MyTwinStickShooterCPawn.h"
#include "PlayerProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
//#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "MyTwinStickShooterCGameMode.h"
#include "Components/AudioComponent.h"
const FName AMyTwinStickShooterCPawn::MoveForwardBinding("MoveForward");
const FName AMyTwinStickShooterCPawn::MoveRightBinding("MoveRight");
const FName AMyTwinStickShooterCPawn::FireForwardBinding("FireForward");
const FName AMyTwinStickShooterCPawn::FireRightBinding("FireRight");

AMyTwinStickShooterCPawn::AMyTwinStickShooterCPawn()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> OnFireAudio(TEXT("SoundWave'/Game/StarterContent/Audio/Fire01.Fire01'"));
	USoundBase* OnFireSound = OnFireAudio.Object;
	OnFireSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OnFireAudio"));
	OnFireSoundComponent->bAutoActivate = false;;
	if(OnFireSound!=nullptr)
		OnFireSoundComponent->SetSound(OnFireSound);
	OnFireSoundComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	static ConstructorHelpers::FObjectFinder<USoundBase> BoomAudio(TEXT("SoundWave'/Game/StarterContent/Audio/Explosion01.Explosion01'"));
	BoomSound = BoomAudio.Object;

	FireSoundVolume = 1.f;
	OnFireSoundVolume = 1.f;
	BoomSoundVolume = 1.f;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Create the boomup particle system compoent
	BoomParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("BoomParticle"));// BoomParticleAsset;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> BoomParticleAsset(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (BoomParticleAsset.Succeeded())
	{
		BoomParticle = BoomParticleAsset.Object;
	}

	// Create the onfire particle system component
	OnFireParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OnFireParticle"));
	OnFireParticle->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	OnFireParticle->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> OnFireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (OnFireParticleAsset.Succeeded())
	{
		OnFireParticle->SetTemplate(OnFireParticleAsset.Object);
	}
	//Create health bar widget component
	//HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	//HealthBarWidgetComponent->AttachToComponent(CameraBoom,FAttachmentTransformRules::KeepRelativeTransform);
	//UClass* Widget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_HealthBarWidget.BP_HealthBarWidget_C'"));
	//UUserWidget* Bar= CreateWidget<UUserWidget>(GetWorld(), Widget);
	//Bar->AddToViewport();
	//HealthBarWidgetComponent->SetWidgetClass(Widget);

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	// Health
	MaxHealth = 10.f;

	InitFireRate = 1.f;
	FastestFireRate = .3f;
	
	InitProjectileNum = 1;
	MaxProjectileNum = 3;

	SmoothBlendTime = 5;
}

void AMyTwinStickShooterCPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
	PlayerInputComponent->BindAxis(FireRightBinding);
}



void AMyTwinStickShooterCPawn::Tick(float DeltaSeconds)
{

	/*To do when game over*/
	if (bPlayerIsDead) {
		if (ShipMeshComponent->IsVisible()) {
			HidePlayer();//Destroy();
			ChangeCamera();
			((AMyTwinStickShooterCGameMode*)(UGameplayStatics::GetGameMode(GetWorld())))->GameOver();
		}
	}
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
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
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f);

	// Try and fire a shot
	FireShot(FireDirection);
}

void AMyTwinStickShooterCPawn::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire && !bPlayerIsDead)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{

			SpawnProjectileWave(FireDirection);
			bCanFire = false;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AMyTwinStickShooterCPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void AMyTwinStickShooterCPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void AMyTwinStickShooterCPawn::BeginPlay()
{
	Super::BeginPlay();
	ResetPlayer();
	
}

TArray<FVector> AMyTwinStickShooterCPawn::ProjectileOffset(const FVector& FireDirection,const float DistanceBetweenProjectiles = 10.f)
{
	float FireForwardValue = FireDirection.X;
	float FireRightValue = FireDirection.Y;
	float DistanceFromCenter;
	TArray<FVector> Offset;
	int Num = ProjectileNum;
	if (ProjectileNum % 2 == 1) {
		Offset.Add(FVector(0.f));
		Num -= 1;
		DistanceFromCenter = DistanceBetweenProjectiles;
	}
	else {
		DistanceFromCenter = DistanceBetweenProjectiles/2;
	}
	while (Num > 0) {
		Num -= 2;
		Offset.Add(FVector(DistanceFromCenter*FireRightValue, -DistanceFromCenter * FireForwardValue, 0.f).GetClampedToMaxSize(DistanceFromCenter));
		Offset.Add(FVector(-DistanceFromCenter * FireRightValue, DistanceFromCenter * FireForwardValue, 0.f).GetClampedToMaxSize(DistanceFromCenter));
		DistanceFromCenter += DistanceBetweenProjectiles;
	}
	return Offset;
}

void AMyTwinStickShooterCPawn::SpawnProjectileWave(const FVector & FireDirection)
{
	const FRotator FireRotation = FireDirection.Rotation();
	// Spawn projectile at an offset from this pawn
	const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		TArray<FVector> Offsets = ProjectileOffset(FireDirection,100.f);
		for (int i = 0; i < Offsets.Num(); i++) {
			FVector offset = Offsets[i];
			APlayerProjectile* PlayerProjectile = World->SpawnActor<APlayerProjectile>(SpawnLocation + offset, FireRotation);
		}
	}
}

void AMyTwinStickShooterCPawn::ChangeCamera()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!(ShipMeshComponent->IsVisible())) {
		if (PlayerController->GetViewTarget() != FullViewCamera && FullViewCamera != NULL)
			PlayerController->SetViewTargetWithBlend(FullViewCamera, SmoothBlendTime);
	}
	else {
		if (PlayerController->GetViewTarget() != this)
			PlayerController->SetViewTargetWithBlend(this, SmoothBlendTime);
	}
}


void AMyTwinStickShooterCPawn::UpdateCondition()
{
	//HealthBarProgress->SetPercent(Health / MaxHealth);
	((AMyTwinStickShooterCGameMode*)(GetWorld()->GetAuthGameMode()))->HealthBarProgress->SetPercent(Health / MaxHealth);
	if (Health == 0 && !bPlayerIsDead) {
		bPlayerIsDead = true;
		// turn off fire and turn on explosion
		OnFireParticle->DeactivateSystem();
		OnFireSoundComponent->FadeOut(1.f, 0.f);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BoomSound, GetActorLocation(), FRotator::ZeroRotator, BoomSoundVolume);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomParticle, FTransform(GetActorRotation(), GetActorLocation(), GetActorScale()), true);
		
	}
	else if (!bPlayerInBadCondition && Health <= 0.3f*MaxHealth) {
		bPlayerInBadCondition = true;
		OnFireParticle->ActivateSystem();
		// try and play the sound if specified
		if(!(OnFireSoundComponent->IsPlaying()))
		OnFireSoundComponent->FadeIn(1.f,OnFireSoundVolume);
	}
	else if (bPlayerInBadCondition && Health > 0.3f*MaxHealth) {
		bPlayerInBadCondition = false;
		OnFireParticle->DeactivateSystem();
		OnFireSoundComponent->FadeOut(1.f, 0.f);
	}
}

void AMyTwinStickShooterCPawn::HidePlayer()
{
	ShipMeshComponent->SetVisibility(false,true);
	//ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AMyTwinStickShooterCPawn::ResetPlayer()
{
	ShipMeshComponent->SetVisibility(true, true);

	if(GetWorld())
		ChangeCamera();

	// Weapon
	bCanFire = true;
	// Dead Flag
	bPlayerIsDead = false;
	// Bad condition flag
	bPlayerInBadCondition = false;
	Health = MaxHealth;
	FireRate = InitFireRate;
	ProjectileNum = InitProjectileNum;
}

void AMyTwinStickShooterCPawn::ReduceHealth(float HowMuchReduce)
{
	Health = FMath::Clamp(Health - HowMuchReduce, 0.f, MaxHealth);
	UpdateCondition();
}

void AMyTwinStickShooterCPawn::GetBenefit(float HealthBenifit)
{
	if (Health < MaxHealth) {
		Health = FMath::Clamp<float>(Health + HealthBenifit, 0.f, MaxHealth);
		UpdateCondition();
	}
	else if(FireRate>FastestFireRate){
		FireRate = FMath::Clamp<float>(FireRate/2, FastestFireRate, InitFireRate);
	}
	else if (ProjectileNum < MaxProjectileNum) {
		ProjectileNum = FMath::Clamp<int>(ProjectileNum + 1, InitProjectileNum, MaxProjectileNum);
	}
}

