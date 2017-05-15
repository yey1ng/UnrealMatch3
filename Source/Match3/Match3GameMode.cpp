// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Match3.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Match3GameMode.h"
#include "Match3PlayerController.h"
#include "Match3GameInstance.h"
#include "Match3SaveGame.h"


AMatch3GameMode::AMatch3GameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass =  APawn::StaticClass();
	PlayerControllerClass = AMatch3PlayerController::StaticClass();
	TileMoveSpeed = 50.0f;
	TimeRemaining = 5.0f;
	FinalPlace = 0;
}

void AMatch3GameMode::BeginPlay()
{
	Super::BeginPlay();
	bGameWillBeWon = false;
	ChangeMenuWidget(StartingWidgetClass);
	GetWorldTimerManager().SetTimer(GameOverTimer, this, &AMatch3GameMode::GameOver, TimeRemaining, false);

	// Get our current save data from the game instance.
	UMatch3GameInstance* GameInstance = Cast<UMatch3GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance)
	{
		// If we didn't already have save data, put our defaults into the main array. We'll save it later, if anything noteworthy happens.
		if (!GameInstance->FindSaveDataForLevel(this, SaveGameData))
		{
			GameInstance->UpdateSave(this, SaveGameData);
		}
	}
}

void AMatch3GameMode::GameRestart()
{
	ChangeMenuWidget(nullptr);
	FName LevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, LevelName);
}

void AMatch3GameMode::GameOver()
{
	GetWorldTimerManager().ClearTimer(GameOverTimer);

	if (bGameWillBeWon)
	{
		UMatch3GameInstance* GameInstance = Cast<UMatch3GameInstance>(UGameplayStatics::GetGameInstance(this));
		// Check for top score
		if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
		{
			SaveGameData.TopScore = FMath::Max(PC->GetScore(), SaveGameData.TopScore);
		}
		// Save regardless of whether or not we got a high score, because we save things like number of games played.
		GameInstance->UpdateSave(this, SaveGameData);
		GameInstance->SaveGame();
	}
	ChangeMenuWidget(bGameWillBeWon ? VictoryWidgetClass : DefeatWidgetClass);
	GameWasWon(bGameWillBeWon);
}

bool AMatch3GameMode::IsGameActive() const
{
	// Game is active whenever time hasn't run out or the timer is paused.
	FTimerManager& WorldTimerManager = GetWorldTimerManager();
	return (WorldTimerManager.IsTimerActive(GameOverTimer) || WorldTimerManager.IsTimerPaused(GameOverTimer));
}

void AMatch3GameMode::PauseGameTimer(bool bPause)
{
	if (bPause)
	{
		GetWorldTimerManager().PauseTimer(GameOverTimer);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(GameOverTimer);
	}
}

FString AMatch3GameMode::GetRemainingTimeAsString()
{
	int32 OutInt = FMath::CeilToInt(GetWorldTimerManager().GetTimerRemaining(GameOverTimer));
	return FString::Printf(TEXT("%03i"), FMath::Max(0, OutInt));
}


bool AMatch3GameMode::GetTimerPaused()
{
	return GetWorldTimerManager().IsTimerPaused(GameOverTimer);
}

void AMatch3GameMode::AddScore(int32 Points)
{
	if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		int32 OldScore = PC->GetScore();
		PC->AddScore(Points);
		int32 NewScore = PC->GetScore();
		if (NewScore >= SaveGameData.BronzeScore)
		{
			bGameWillBeWon  = true;
		}

		// Check for medals
		if (NewScore > SaveGameData.GoldScore)
		{
			FinalPlace = 1;
			AwardPlace(1, Points);
		}
		else if (NewScore > SaveGameData.SilverScore)
		{
			FinalPlace = 2;
			AwardPlace(2, Points);
		}
		else if (NewScore > SaveGameData.BronzeScore)
		{
			FinalPlace = 3;
			AwardPlace(3, Points);
		}
		else
		{
			FinalPlace = 0;
			AwardPlace(0, Points);
		}
		
		for (const FMatch3Reward& Reward : Rewards)
		{
			check(Reward.ScoreInterval > 0);
			// Integer division to decide if we've crossed a bonus threshold
			int32 ScoreAwardCount = (NewScore / Reward.ScoreInterval) - (OldScore / Reward.ScoreInterval);
			if (ScoreAwardCount > 0)
			{
				float StartingTimeValue = GetWorldTimerManager().GetTimerRemaining(GameOverTimer);
				if (StartingTimeValue >= 0.0f)
				{
					GetWorldTimerManager().SetTimer(GameOverTimer, this, &AMatch3GameMode::GameOver, StartingTimeValue + (ScoreAwardCount * Reward.TimeAwarded), false);
					AwardBonus();
				}
			}
		}
	}
}

void AMatch3GameMode::UpdateScoresFromLeaderBoard(int32 GoldScore, int32 SilverScore, int32 BronzeScore)
{
	UMatch3GameInstance* GameInstance = Cast<UMatch3GameInstance>(UGameplayStatics::GetGameInstance(this));
	
	SaveGameData.BronzeScore = BronzeScore;
	SaveGameData.SilverScore = SilverScore;
	SaveGameData.GoldScore = GoldScore;

	GameInstance->SaveGame();
}

void AMatch3GameMode::SetComboPower(int32 NewComboPower)
{
	if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		PC->ComboPower = NewComboPower;
	}
}

int32 AMatch3GameMode::GetComboPower()
{
	if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		return PC->ComboPower;
	}
	return 0;
}

int32 AMatch3GameMode::GetMaxComboPower()
{
	if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		return PC->MaxComboPower;
	}
	return 0;
}

int32 AMatch3GameMode::CalculateBombPower_Implementation()
{
	if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		return PC->CalculateBombPower();
	}
	return 0;
}

void AMatch3GameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}
	if (NewWidgetClass)
	{
		if (AMatch3PlayerController* PC = Cast<AMatch3PlayerController>(UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
		{
			CurrentWidget = CreateWidget<UUserWidget>(PC, NewWidgetClass);
			if (CurrentWidget)
			{
				CurrentWidget->AddToViewport();
			}
		}
	}
}
