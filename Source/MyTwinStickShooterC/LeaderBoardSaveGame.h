// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LeaderBoardSaveGame.generated.h"
/**
 * 
 */
UCLASS()
class MYTWINSTICKSHOOTERC_API ULeaderBoardSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	ULeaderBoardSaveGame();
	void AddNewLeader(FString Name,uint16 Score);
	//UPROPERTY(VisibleAnywhere, Category = Basic)
	//TArray<ULeaderPlayer> LeaderList;
	/* Name of Score Leaders */
	UPROPERTY(SaveGame)
		TArray<FString> NameList;
	/* High Scores */
	UPROPERTY(SaveGame)
		TArray<uint16> ScoreList;
	/* Number of top scores shows in leaderboard */
	UPROPERTY(EditAnyWhere, Category = Basic)
		uint16 NumOfLeader;
};
