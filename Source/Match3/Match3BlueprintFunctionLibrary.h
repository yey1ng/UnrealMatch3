// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match3BlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MATCH3_API UMatch3BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:

	/** Get a list of all player controllers, then pick the first local one we find. */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(UObject* WorldContextObject);

	/** Get the online account ID (as an encoded hex string) associated with the provided player controller's player state. Returns a blank string on failure. */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay")
	static FString GetOnlineAccountID(APlayerController* PlayerController);

	/** Function to identify whether or not game is currently being played. */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static bool IsGameActive(UObject* WorldContextObject);

	/** Function to identify whether or not game is currently being played. */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static void PauseGameTimer(UObject* WorldContextObject, bool bPause);
};
