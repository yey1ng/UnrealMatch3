// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Match3PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MATCH3_API AMatch3PlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AMatch3PlayerController(const FObjectInitializer& ObjectInitializer);

	/** Add points. If points are negative or we force immediate update, the score will display instantly instead of counting up. */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddScore(int32 Points, bool bForceImmediateUpdate = false);

	/** Get the actual score (not the score that is displayed) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScore();

	/** Get the score that is currently displayed (not the actual score) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetDisplayedScore();

	/** Override in BPs to power up bombs. */
	UFUNCTION(BlueprintNativeEvent, Category = "Game")
	int32 CalculateBombPower();
	virtual int32 CalculateBombPower_Implementation();

	/** Current combo power. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	int32 ComboPower;

	/** Maximum combo power for this player, can be changed based on avatar. TODO: Set this from the avatar class. (version 2.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	int32 MaxComboPower;

protected:
	/** Current actual score, though not the display value. */
	UPROPERTY()
	int32 Score;

	/** Score that is displayed on screen (as an integer). */
	UPROPERTY()
	float DisplayedScore;

	/** Rate at which displayed score climbs to reach actual score. Currently does not go faster with bigger scores. */
	UPROPERTY(EditAnywhere)
	float ScoreChangeRate;

	/** Periodic function to manage score updates */
	void TickScoreDisplay();
	FTimerHandle TickScoreDisplayHandle;
};
