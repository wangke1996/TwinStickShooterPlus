// Fill out your copyright notice in the Description page of Project Settings.

#include "LeaderBoardSaveGame.h"
#include "Kismet/GameplayStatics.h"
ULeaderBoardSaveGame::ULeaderBoardSaveGame() {
	NumOfLeader = 10;
}
void ULeaderBoardSaveGame::AddNewLeader(FString Name,uint16 Score)
{
	if (ScoreList.Num() >= NumOfLeader && ScoreList[NumOfLeader - 1] >= Score)
		return;
	int index = 0;
	while (index < ScoreList.Num() && Score < ScoreList[index])
		index++;
	ScoreList.Insert(Score, index);
	NameList.Insert(Name, index);
	if (ScoreList.Num() > NumOfLeader)
	{
		ScoreList.SetNum(NumOfLeader);
		NameList.SetNum(NumOfLeader);
	}
}


