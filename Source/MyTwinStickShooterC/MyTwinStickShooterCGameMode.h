// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/TextRenderActor.h"
#include "MyTwinStickShooterCGameMode.generated.h"
//class UWidgetAnimation;

UCLASS(MinimalAPI)
class AMyTwinStickShooterCGameMode : public AGameModeBase
{
	GENERATED_BODY()
private:
	//Game Parameters
	int Score;
	bool bAbleToSpawnEnemy;
	//float TimeSinceSpawnLastEnemy;
	int NumOfEnemySpawned;
	bool bAbleToUpdateLevel;
	bool bBossCleaned;
	bool bEnemyCleaned;
	int EnemyDestroyedInLevel;
	int AllEnemyDestroyed;
	int EnemyNumOfLevel;
	float DeltaTimeOfSpawnEnemy;
	int EnemyEachWave;

protected:
	virtual void BeginPlay() override;
	void SpawnEnemyAtRandomLocation();
	// Get the Score Render Text
	ATextRenderActor* GetLevelTextActor();
	// Update the score render text
	void UpdateScoreText();
	// function to call by timerhandle to spawn benifit package
	void AdvanceTimerForBenifitPackage();

	void SpawnBossAtRandomLocation();
	void SpawnEnemyWave();
	void SpawnBossWithHint();
	void SetHintTextWhenLevelBegin();
	void UpdateLevel();
	void CleanHintText();
	// Load UI Widgets
	void LoadWidgets();
	// called when restart name 
	void CleanEnemyAndBoss();
	void ResetProperty();
	void ShowStartMenu();
	// To Set Widgets remain in the view
	void SetAllWidgets(TArray<UUserWidget*> RemainWidgets);
	// Switch Musics
	void SwitchToStarterMusic();
	void SwitchToBackGroundMusic();
	// make a hint for boss time with animation
	void HintBoss();
	void CleanBossHint();
	class UWidgetAnimation* GetAnimationsByName(UUserWidget* Widget, FString AnimationName);
	FTimerHandle PackageTimerHandle;
	FTimerHandle BossTimerHandle;
	FTimerHandle EnemyTimerHandle;
	FTimerHandle TextTimerHandle;
public:
	AMyTwinStickShooterCGameMode();
	virtual void Tick(float DeltaSeconds) override;

	// TextRender on the ground to show the score and level
	UPROPERTY(Category = UI, VisibleAnywhere, BlueprintReadWrite)
		class ATextRenderActor * LevelTextRender;
	// HealthBarWidget for player
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget* HealthBarWidget;
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UProgressBar* HealthBarProgress;
	// Hint Text Widget, show when level up and boss coming
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget *HintWidget;
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UTextBlock *HintText;
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UWidgetAnimation *HintTextWarningAnimation;
	// UI for GameOver
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget *GameOverWidget;
	// UI for LeaderBoard
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget *LeaderBoardWidget;
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UScrollBox* LeaderBoard;
	// UI for StartMenu
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget* StartMenuWidget;
	// UI for QuickGuider
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
		class UUserWidget* QuickGuideWidget;

	// Audio components
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class UAudioComponent* StarterMusic;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		class UAudioComponent* BackGroundMusic;
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
		float BackGroundVolume;
	// Game Level
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		int Level;
	// Enemy num of the first level
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		int InitEnemyNumOfLevel;
	// the number of enemies that each time we spawn of the first level
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		int InitEnemyEachWave;
	// the period of enemy wave of the first level
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float InitDeltaTimeOfSpawnEnemy;
	// time for player to get ready to face boss
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float WaitTimeOfSpawnBoss;
	// time for player to get ready to next level
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float DeltaTimeBetweenLevel;
	// the center point of random spawn actors
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		FVector CenterPoint;
	// the range vector of random spawn actors
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		FVector RangeVector;
	// the text render object name 
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		FString LevelTextName;
	// the period of spawn benefit package
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float DeltaTimeOfBenefitPackage;
	// the scale of benefit package
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		FVector BenefitPackageScale;
	UFUNCTION()
		void UpdatePropertyWhenLevelUp();
	UFUNCTION()
		int ScoreForEachEnemy();
	UFUNCTION()
		int ScoreForBoss();
	UFUNCTION()
		void GameOver();
	UFUNCTION(BlueprintCallable, Category = "RestartGame")
		void RestartGame();
	UFUNCTION(BlueprintCallable, Category = "RestartGame")
		void SaveScore(const FString& PlayerName);
	UFUNCTION(BlueprintCallable, Category = "RestartGame")
		void ShowLeaderBoard();
	UFUNCTION(BlueprintCallable, Category = "StartGame")
		void BeginGame();
	UFUNCTION(BlueprintCallable, Category = "StartGame")
		void ShowQuickGuide();
	UFUNCTION(BlueprintCallable, Category = "StartGame")
		void ReturnToStartMenu();
	// called when player destroy an enemy;
	void EnemyDestroyedByPlayer();
	// called when player destroy a boss;
	void BossDestroyedByPlayer();
};



