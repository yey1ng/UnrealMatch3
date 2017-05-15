// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"
#include "Match3GameMode.h"
#include "Match3GameInstance.generated.h"


/**
 * 
 */
UCLASS()
class MATCH3_API UMatch3GameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UMatch3GameInstance();

	// Called by the game at startup.
	void Init() override;

	// Called by the game as it shuts down.
	void Shutdown() override;

	// This function is used to initialize or change our save game slot. Data is transferred when creating a new, non-default slot.
	void InitSaveGameSlot();

	/** Load the current saved game, if it exists. */
	bool FindSaveDataForLevel(UObject* WorldContextObject, FMatch3LevelSaveData& OutSaveData);

	/** Save our game. All save game data is included. */
	UFUNCTION(BlueprintCallable, Category = "Saved Game")
	void SaveGame();

	/** Look up a custom int32 variable in our saved game. FieldName is not case-sensitive. */
	UFUNCTION(BlueprintCallable, Category = "Saved Game")
	bool LoadCustomInt(FString FieldName, int32& Value);

	/** Store a custom int32 variable in our saved game. FieldName is not case-sensitive. */
	UFUNCTION(BlueprintCallable, Category = "Saved Game")
	void SaveCustomInt(FString FieldName, int32 Value);

	/** Remove a custom int32 variable from our saved game. FieldName is not case-sensitive. */
	UFUNCTION(BlueprintCallable, Category = "Saved Game")
	void ClearCustomInt(FString FieldName);

	/** Create or update the saved data for a specific Match3 level. */
	void UpdateSave(UObject* WorldContextObject, FMatch3LevelSaveData& NewData);

	/** Event for refreshing the UI after save games are updated */
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateUIAfterSave();

	/** Combined save game data for all levels/modes. */
	UPROPERTY()
	class UMatch3SaveGame* InstanceGameData;

	// Should be called by blueprints when the user's ID changes, such as by logging in or out via the game's blueprint menu system.
	UFUNCTION(BlueprintCallable, Category = "Online")
	void RegisterOnlineID(FString NewOnlineID);

	// Called when the user logs in or out on a mobile device.
	UFUNCTION(BlueprintImplementableEvent, Category = "Online")
	void OnLoginChanged(bool bLoggingIn, int32 UserID, int32 UserIndex);

	// Called when the application has just entered the foreground.
	UFUNCTION(BlueprintImplementableEvent, Category = "Online")
	void OnEnteringForeground();

	// Called when the application is about to enter the background.
	UFUNCTION(BlueprintImplementableEvent, Category = "Online")
	void OnEnteringBackground();

	// Internal function to bind to viewport resizing delegate
	void OnViewportResize_Internal(FViewport* Viewport, uint32 ID);

	// Blueprint event to use to respond to viewport resizing, since FViewport* is not a valid Blueprint pin type
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnViewportResize();

protected:
	FString GetSaveSlotName() const;
	FString SaveGamePrefix;
	FString DefaultSaveGameSlot;

private:
	FDelegateHandle LoginChangedHandle;
	FDelegateHandle EnteringForegroundHandle;
	FDelegateHandle EnteringBackgroundHandle;
	FDelegateHandle ViewportHandle;
};
