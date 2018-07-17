// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MyTwinStickShooterCGameMode.h"
#include "MyTwinStickShooterCPawn.h"
#include "EnemyActor.h"
#include "EngineUtils.h"
#include "Components/TextRenderComponent.h"
#include "Components/ProgressBar.h"
#include "UserWidget.h"
#include "BenefitPackageActor.h"
#include "EnemyBossActor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Button.h"
#include "MyTwinStickShooterCProjectile.h"
#include "LeaderBoardSaveGame.h"
#include "ScrollBox.h"
#include "Components/AudioComponent.h"
#include "ConstructorHelpers.h"
#include "WidgetAnimation.h"
#include "Engine.h"
int AEnemyBossActor::BossLevel;
AMyTwinStickShooterCGameMode::AMyTwinStickShooterCGameMode()
{
	// no need to update every frame
	PrimaryActorTick.bCanEverTick = false;
	// set default pawn class to our character class
	DefaultPawnClass = AMyTwinStickShooterCPawn::StaticClass();
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GAMEMODE"));
	// Game Play Parameters
	Level = 0;
	InitEnemyNumOfLevel = 5;
	InitDeltaTimeOfSpawnEnemy = 5;
	DeltaTimeOfBenefitPackage = 10;
	Score = 0;
	DeltaTimeBetweenLevel = 5;
	BenefitPackageScale = FVector(5.f);
	LevelTextName = FString(TEXT("LevelText"));
	WaitTimeOfSpawnBoss = 3.f;
	InitEnemyEachWave = 3;

	CenterPoint = FVector(0.f, 0.f, 210.f);
	RangeVector = FVector(1000.f, 1000.f, 0.f);

	static ConstructorHelpers::FObjectFinder<USoundBase> StarterAudioSource(TEXT("SoundWave'/Game/StarterContent/Audio/Starter_Music01.Starter_Music01'"));
	USoundBase* StarterAudio = StarterAudioSource.Object;
	
	StarterMusic = CreateDefaultSubobject<UAudioComponent>(TEXT("StarterAudio"));
	StarterMusic->bAutoActivate = false;
	if (StarterAudio != nullptr)
		StarterMusic->SetSound(StarterAudio);
	static ConstructorHelpers::FObjectFinder<USoundBase> BackGroundSource(TEXT("SoundWave'/Game/TwinStick/Audio/TheDawn.TheDawn'"));
	USoundBase* BackGroundAudio = BackGroundSource.Object;
	BackGroundMusic = CreateDefaultSubobject<UAudioComponent>(TEXT("BackGroundMusic"));
	BackGroundMusic->bAutoActivate = false;
	if (BackGroundAudio != nullptr)
		BackGroundMusic->SetSound(BackGroundAudio);
	BackGroundVolume = .5f;
}

void AMyTwinStickShooterCGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//{
		//if (bAbleToSpawnEnemy) {
		//	if (TimeSinceSpawnLastEnemy > InitDeltaTimeOfSpawnEnemy) {
		//		SpawnEnemyAtRandomLocation();
		//		NumOfEnemySpawned += 1;
		//		TimeSinceSpawnLastEnemy = 0.f;
		//		if (NumOfEnemySpawned >= InitEnemyNumOfLevel)
		//			bAbleToSpawnEnemy = false;
		//	}
		//	else {
		//		TimeSinceSpawnLastEnemy += DeltaSeconds;
		//	}
		//}
	//}
	//{
	//	if (bAbleToUpdateLevel)
	//		UpdateLevel();
	//}
}

void AMyTwinStickShooterCGameMode::UpdateLevel()
{
	Level += 1;
	UpdatePropertyWhenLevelUp();
	UpdateScoreText();
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &AMyTwinStickShooterCGameMode::SpawnEnemyWave, InitDeltaTimeOfSpawnEnemy, true);
	SetHintTextWhenLevelBegin();
}

int AMyTwinStickShooterCGameMode::ScoreForEachEnemy()
{
	return Level;
}

int AMyTwinStickShooterCGameMode::ScoreForBoss()
{
	return 10*Level;
}

void AMyTwinStickShooterCGameMode::GameOver()
{
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(GameOverWidget);
	SetAllWidgets(RemainWidgets);

	SwitchToStarterMusic();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//Mode.SetWidgetToFocus(GameOverWidget);
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = true;
	UTextBlock * ScoreText = Cast<UTextBlock>(GameOverWidget->GetWidgetFromName(TEXT("ScoreText")));
	UTextBlock * LevelText = Cast<UTextBlock>(GameOverWidget->GetWidgetFromName(TEXT("LevelText")));
	UTextBlock * EnemyDestroyedText = Cast<UTextBlock>(GameOverWidget->GetWidgetFromName(TEXT("EnemyDestroyedText")));
	ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %04d"), Score)));
	LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level Passed: %02d"), Level-1)));
	EnemyDestroyedText->SetText(FText::FromString(FString::Printf(TEXT("Enemy Destroyed: %04d"),AllEnemyDestroyed)));
}

void AMyTwinStickShooterCGameMode::RestartGame()
{
	CleanEnemyAndBoss();

	TArray<UUserWidget *> RemainWidgets;
	RemainWidgets.Add(HealthBarWidget);
	RemainWidgets.Add(HintWidget);
	SetAllWidgets(RemainWidgets);

	SwitchToBackGroundMusic();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = false;
	AMyTwinStickShooterCPawn* PlayerActor =(AMyTwinStickShooterCPawn*) PlayerController->GetPawn();
	PlayerActor->ResetPlayer();

	BeginGame();
}

void AMyTwinStickShooterCGameMode::ShowQuickGuide()
{
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(QuickGuideWidget);
	SetAllWidgets(RemainWidgets);

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->bShowMouseCursor = true;
	QuickGuideWidget->SetUserFocus(PlayerController);
}

void AMyTwinStickShooterCGameMode::ReturnToStartMenu()
{
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(StartMenuWidget);
	SetAllWidgets(RemainWidgets);

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->bShowMouseCursor = true;
	StartMenuWidget->SetUserFocus(PlayerController);
	
}

void AMyTwinStickShooterCGameMode::EnemyDestroyedByPlayer()
{
	AllEnemyDestroyed++;
	Score += ScoreForEachEnemy();
	EnemyDestroyedInLevel += 1;
	if (EnemyDestroyedInLevel == EnemyNumOfLevel) {
		bEnemyCleaned = true;
		if (!bBossCleaned)
			SpawnBossWithHint();
	}
	UpdateScoreText();
}

void AMyTwinStickShooterCGameMode::BossDestroyedByPlayer()
{
	bBossCleaned = true;
	Score += ScoreForBoss();
	UpdateLevel();
}


void AMyTwinStickShooterCGameMode::BeginPlay()
{
	Super::BeginPlay();
	LoadWidgets();
	LevelTextRender = GetLevelTextActor();
	GetWorldTimerManager().SetTimer(PackageTimerHandle, this, &AMyTwinStickShooterCGameMode::AdvanceTimerForBenifitPackage, DeltaTimeOfBenefitPackage, true);
	ShowStartMenu();


}

void AMyTwinStickShooterCGameMode::SpawnEnemyAtRandomLocation()
{
	FVector RandomPosition = FMath::VRand() * RangeVector + CenterPoint;
	GetWorld()->SpawnActor<AEnemyActor>(AEnemyActor::StaticClass(),RandomPosition, FRotator::ZeroRotator);
}

ATextRenderActor* AMyTwinStickShooterCGameMode::GetLevelTextActor()
{
	for (TActorIterator<ATextRenderActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->GetName().Equals(LevelTextName)) {
			ATextRenderActor *TextRenderActor = *ActorItr;
			return TextRenderActor;
		}
	}
	return nullptr;
}

void AMyTwinStickShooterCGameMode::UpdateScoreText()
{
	if (LevelTextRender)
		LevelTextRender->GetTextRender()->SetText(FText::FromString(FString::Printf(TEXT("LEVEL %02d\nSCORE %d"), Level,Score)));
}

void AMyTwinStickShooterCGameMode::AdvanceTimerForBenifitPackage()
{
	TActorIterator<ABenefitPackageActor> ActorItr(GetWorld());
	if (ActorItr)
		return;
	FVector RandomPosition = FMath::VRand() * RangeVector + CenterPoint;
	ABenefitPackageActor* BenefitPackage=GetWorld()->SpawnActor<ABenefitPackageActor>(ABenefitPackageActor::StaticClass(), RandomPosition, FRotator(0.f,FMath::FRandRange(-180.f,180.f),0.f));
	BenefitPackage->SetActorScale3D(BenefitPackageScale);
}
void AMyTwinStickShooterCGameMode::SpawnBossAtRandomLocation()
{
	FVector RandomLocation = FMath::VRand() * RangeVector + CenterPoint;
	FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, RandomLocation);
	AEnemyBossActor::BossLevel = Level;
	FActorSpawnParameters BossSpawnParameter;
	BossSpawnParameter.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AEnemyBossActor>(AEnemyBossActor::StaticClass(), RandomLocation, FRotator::ZeroRotator);
}
void AMyTwinStickShooterCGameMode::SpawnEnemyWave()
{
	if (bAbleToSpawnEnemy) {
		for(int i =0;i<EnemyEachWave;i++)
			SpawnEnemyAtRandomLocation();
		NumOfEnemySpawned += EnemyEachWave;
		if (NumOfEnemySpawned >= EnemyNumOfLevel)
			bAbleToSpawnEnemy = false;
	}
}
void AMyTwinStickShooterCGameMode::SpawnBossWithHint()
{
	TActorIterator<AEnemyBossActor> ActorItr(GetWorld());
	if (ActorItr)
		return;
	GetWorldTimerManager().SetTimer(BossTimerHandle, this, &AMyTwinStickShooterCGameMode::SpawnBossAtRandomLocation, WaitTimeOfSpawnBoss, false);
	HintBoss();
	//GetWorldTimerManager().SetTimer(TextTimerHandle, this, &AMyTwinStickShooterCGameMode::HintBoss, 0.8f, true);
	FTimerHandle CleanTimerHandle;
	GetWorldTimerManager().SetTimer(CleanTimerHandle, this, &AMyTwinStickShooterCGameMode::CleanBossHint, FMath::Clamp<float>(WaitTimeOfSpawnBoss-1.f,1.f,WaitTimeOfSpawnBoss), false);
}
void AMyTwinStickShooterCGameMode::SetHintTextWhenLevelBegin()
{
	HintText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d\nEnemy: %d"), Level, EnemyNumOfLevel)));
	GetWorldTimerManager().SetTimer(TextTimerHandle, this, &AMyTwinStickShooterCGameMode::CleanHintText, FMath::Clamp<float>(InitDeltaTimeOfSpawnEnemy - 1.f, 1.f, InitDeltaTimeOfSpawnEnemy), false);
}
void AMyTwinStickShooterCGameMode::UpdatePropertyWhenLevelUp()
{
	EnemyNumOfLevel = (int)(EnemyNumOfLevel * 1.5f);
	EnemyEachWave = int(EnemyEachWave*1.5f);
	InitDeltaTimeOfSpawnEnemy *= 0.9f;
	bAbleToSpawnEnemy = true;
	bAbleToUpdateLevel = false;
	bEnemyCleaned = false;
	bBossCleaned = false;
	//TimeSinceSpawnLastEnemy = InitDeltaTimeOfSpawnEnemy-DeltaTimeBetweenLevel;
	NumOfEnemySpawned = 0;
	EnemyDestroyedInLevel = 0;
}
void AMyTwinStickShooterCGameMode::CleanHintText()
{
	HintText->SetText(FText::FromString(""));
}
void AMyTwinStickShooterCGameMode::LoadWidgets()
{
	/* Load StartMenu Widget*/
	UClass* SMWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_StartMenu.BP_StartMenu_C'"));
	StartMenuWidget = CreateWidget<UUserWidget>(GetWorld(), SMWidget);
	StartMenuWidget->bIsFocusable = true;
	//StartMenuWidget->AddToViewport();

	/* Load QuickGuide Widget */
	UClass* QGWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_QuickGuide.BP_QuickGuide_C'"));
	QuickGuideWidget = CreateWidget<UUserWidget>(GetWorld(), QGWidget);
	QuickGuideWidget->bIsFocusable = true;

	/* Load HealthBarWidget */
	UClass* HBWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_HealthBarWidget.BP_HealthBarWidget_C'"));
	HealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), HBWidget);
	//HealthBarWidget->AddToViewport();
	HealthBarProgress = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(TEXT("HealthBarProgress")));
	HealthBarWidget->bIsFocusable = false;

	/* Load Hint Widget */
	UClass* HWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_HintWidget.BP_HintWidget_C'"));
	HintWidget = CreateWidget<UUserWidget>(GetWorld(), HWidget);
	//HintWidget->AddToViewport();
	HintText = Cast<UTextBlock>(HintWidget->GetWidgetFromName(TEXT("HintText")));
	CleanHintText();
	HintTextWarningAnimation = GetAnimationsByName(HintWidget, FString(TEXT("Warning")));
	HintWidget->bIsFocusable = false;

	/* Load GameOver Widget */
	UClass* GOverWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_GameOverWidget.BP_GameOverWidget_C'"));
	GameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GOverWidget);
	GameOverWidget->bIsFocusable = true;

	/* Load LeaderBoard Widget */
	UClass* LBWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_LeaderBoardWidget.BP_LeaderBoardWidget_C'"));
	LeaderBoardWidget = CreateWidget<UUserWidget>(GetWorld(), LBWidget);
	LeaderBoard = Cast<UScrollBox>(LeaderBoardWidget->GetWidgetFromName(TEXT("LedaerBoardScrollBox")));
	LeaderBoardWidget->bIsFocusable = true;

}
void AMyTwinStickShooterCGameMode::CleanEnemyAndBoss()
{
	for (TActorIterator<AEnemyBossActor> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
		//ActorItr->HealthBarWidget->RemoveFromViewport();
		ActorItr->RemoveSelf();
	}
	for (TActorIterator<AEnemyActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		ActorItr->Destroy();
	for (TActorIterator<AMyTwinStickShooterCProjectile> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
		ActorItr->Destroy();
	}
	GetWorldTimerManager().ClearTimer(BossTimerHandle);
	GetWorldTimerManager().ClearTimer(EnemyTimerHandle);
}
//void AMyTwinStickShooterCGameMode::ResetWidgets()
//{
//	if(!(HealthBarWidget->IsInViewport()))
//		HealthBarWidget->AddToViewport();
//	if(GameOverWidget->IsInViewport())
//		GameOverWidget->RemoveFromViewport();
//	CleanHintText();
//	if(LeaderBoardWidget->IsInViewport())
//		LeaderBoardWidget->RemoveFromViewport();
//	UpdateScoreText();
//}
void AMyTwinStickShooterCGameMode::ResetProperty()
{
	Score = 0;
	Level = 0;
	AllEnemyDestroyed = 0;
	
}
void AMyTwinStickShooterCGameMode::BeginGame()
{
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(HealthBarWidget);
	RemainWidgets.Add(HintWidget);
	SetAllWidgets(RemainWidgets);

	SwitchToBackGroundMusic();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
	PlayerController->SetInputMode(Mode);
	PlayerController->bShowMouseCursor = false;
	EnemyNumOfLevel = InitEnemyNumOfLevel;
	DeltaTimeOfSpawnEnemy = InitDeltaTimeOfSpawnEnemy;
	Score = 0;
	AllEnemyDestroyed = 0;
	Level = 0;
	EnemyEachWave = InitEnemyEachWave;
	UpdateScoreText();
	HealthBarProgress->SetPercent(1.f);
	UpdateLevel();
}
void AMyTwinStickShooterCGameMode::ShowStartMenu()
{
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(StartMenuWidget);
	SetAllWidgets(RemainWidgets);
	
	SwitchToStarterMusic();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->bShowMouseCursor = true;
	StartMenuWidget->SetUserFocus(PlayerController);
	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//Mode.SetWidgetToFocus(GameOverWidget);
	PlayerController->SetInputMode(Mode);
}
void AMyTwinStickShooterCGameMode::SetAllWidgets(TArray<UUserWidget*> RemainWidgets)
{
	TArray<UUserWidget*> AllWidgets;
	AllWidgets.Add(HealthBarWidget);
	AllWidgets.Add(LeaderBoardWidget);
	AllWidgets.Add(HintWidget);
	AllWidgets.Add(GameOverWidget);
	AllWidgets.Add(StartMenuWidget);
	AllWidgets.Add(QuickGuideWidget);
	UUserWidget *Widget;
	for (int i = 0; i < AllWidgets.Num(); i++) {
		Widget = AllWidgets[i];
		if (RemainWidgets.Contains(Widget)) {
			if (!(Widget->IsInViewport()))
				Widget->AddToViewport();
		}
		else {
			if (Widget->IsInViewport())
				Widget->RemoveFromViewport();
		}
	}
}
void AMyTwinStickShooterCGameMode::SwitchToStarterMusic()
{
	StarterMusic->FadeIn(3.f, BackGroundVolume);
	if(BackGroundMusic->IsPlaying())
		BackGroundMusic->FadeOut(0.f, 0.f);
}
void AMyTwinStickShooterCGameMode::SwitchToBackGroundMusic()
{
	BackGroundMusic->FadeIn(3.f, BackGroundVolume);
	if (StarterMusic->IsPlaying())
		StarterMusic->FadeOut(0.f, 0.f);
}
void AMyTwinStickShooterCGameMode::HintBoss()
{
	HintText->SetText(FText::FromString("Test Boss Time !!"));
	if (HintTextWarningAnimation == nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "Null PTR");
	HintWidget->PlayAnimation(HintTextWarningAnimation, 0.f, 3);
	//FTimerHandle CleanTimerHandle;
	//GetWorldTimerManager().SetTimer(CleanTimerHandle, this, &AMyTwinStickShooterCGameMode::CleanHintText, 0.5, false);
}
void AMyTwinStickShooterCGameMode::CleanBossHint()
{
	CleanHintText();
	//GetWorldTimerManager().ClearTimer(TextTimerHandle);
	//UWidgetAnimation* WarningAnimation = Cast<UWidgetAnimation>(HintWidget->GetWidgetFromName(TEXT("Warning")));
	//if(HintWidget->IsPlayingAnimation())
	//	HintWidget->StopAnimation(WarningAnimation);
}
void AMyTwinStickShooterCGameMode::SaveScore(const FString& PlayerName)
{
	if (LeaderBoardWidget->IsInViewport())
		return;
	//ULeaderBoardSaveGame* LeaderBoardSaveGame = Cast<ULeaderBoardSaveGame>(UGameplayStatics::CreateSaveGameObject(ULeaderBoardSaveGame::StaticClass()));
	ULeaderBoardSaveGame* LeaderBoardSaveGame = Cast<ULeaderBoardSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LeaderBoard"),0));
	if (!LeaderBoardSaveGame) {
		LeaderBoardSaveGame = Cast<ULeaderBoardSaveGame>(UGameplayStatics::CreateSaveGameObject(ULeaderBoardSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(LeaderBoardSaveGame, TEXT("LeaderBoard"),0);
	}
	LeaderBoardSaveGame->AddNewLeader(PlayerName, Score);
	UGameplayStatics::SaveGameToSlot(LeaderBoardSaveGame, TEXT("LeaderBoard"), 0);

}
void AMyTwinStickShooterCGameMode::ShowLeaderBoard()
{
	if (LeaderBoardWidget->IsInViewport())
		return;

	LeaderBoard->ClearChildren();
	ULeaderBoardSaveGame* LeaderBoardSaveGame = Cast<ULeaderBoardSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LeaderBoard"), 0));
	if (!LeaderBoardSaveGame) {
		LeaderBoardSaveGame = Cast<ULeaderBoardSaveGame>(UGameplayStatics::CreateSaveGameObject(ULeaderBoardSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(LeaderBoardSaveGame, TEXT("LeaderBoard"),0);
	}
	UClass* BP_PlayerScoreWidget = LoadClass<UUserWidget>(NULL, TEXT("WidgetBlueprint'/Game/UI/BP_PlayerScore.BP_PlayerScore_C'"));
	TArray<FString> LeaderNameList = LeaderBoardSaveGame->NameList;
	TArray<uint16> LeaderScoreList = LeaderBoardSaveGame->ScoreList;
	for (int i = 0; i < LeaderNameList.Num(); i++) {
		FString Name = LeaderNameList[i];
		uint16 Score = LeaderScoreList[i];
		UUserWidget* PlayerScoreWdiget = CreateWidget<UUserWidget>(GetWorld(), BP_PlayerScoreWidget);
		UTextBlock* NameText = Cast<UTextBlock>(PlayerScoreWdiget->GetWidgetFromName(TEXT("PlayerNameText")));
		UTextBlock* ScoreText = Cast<UTextBlock>(PlayerScoreWdiget->GetWidgetFromName(TEXT("PlayerScoreText")));
		NameText->SetText(FText::FromString(Name));
		ScoreText->SetText(FText::AsNumber(Score));
		LeaderBoard->AddChild(PlayerScoreWdiget);
	}
	
	TArray<UUserWidget*> RemainWidgets;
	RemainWidgets.Add(LeaderBoardWidget);
	SetAllWidgets(RemainWidgets);

	GameOverWidget->SetUserFocus(GetWorld()->GetFirstPlayerController());
}
//void AMyTwinStickShooterCGameMode::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//	if (HealthBarWidget) {
//		HealthBarProgress = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(TEXT("HealthBarProgress")));
//		if (HealthBarProgress)
//			HealthBarProgress->SetPercent(1.f);
//	}
//}

UWidgetAnimation* AMyTwinStickShooterCGameMode::GetAnimationsByName(UUserWidget* Widget,FString AnimationName)
{
	UProperty* Prop = Widget->GetClass()->PropertyLink;
	// Run through all properties of this class to find any widget animations
	while (Prop != nullptr)
	{
		if (Prop->GetName().Equals(AnimationName)) {
			// Only interested in object properties
			if (Prop->GetClass() == UObjectProperty::StaticClass())
			{
				UObjectProperty* ObjectProp = Cast<UObjectProperty>(Prop);

				// Only want the properties that are widget animations
				if (ObjectProp->PropertyClass == UWidgetAnimation::StaticClass())
				{
					UObject* object = ObjectProp->GetObjectPropertyValue_InContainer(Widget);

					UWidgetAnimation* WidgetAnim = Cast<UWidgetAnimation>(object);

					if (WidgetAnim != nullptr)
					{
							return WidgetAnim;
					}
				}
			}
		}

		Prop = Prop->PropertyLinkNext;
	}
	return nullptr;
}