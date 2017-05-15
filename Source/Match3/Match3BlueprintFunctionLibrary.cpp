// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Match3.h"
#include "Match3GameMode.h"
#include "Match3BlueprintFunctionLibrary.h"

APlayerController* UMatch3BlueprintFunctionLibrary::GetLocalPlayerController(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController->IsLocalController())
			{
				// For this project, we will only ever have one local player.
				return PlayerController;
			}
		}
	}
	return nullptr;
}

FString UMatch3BlueprintFunctionLibrary::GetOnlineAccountID(APlayerController* PlayerController)
{
	if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->UniqueId.IsValid())
	{
		return PlayerController->PlayerState->UniqueId->GetHexEncodedString();
	}
	return FString();
}

bool UMatch3BlueprintFunctionLibrary::IsGameActive(UObject* WorldContextObject)
{
	if (AMatch3GameMode* GameMode = Cast<AMatch3GameMode>(UGameplayStatics::GetGameMode(WorldContextObject)))
	{
		if (GameMode->IsGameActive())
		{
			return true;
		}
	}
	return false;
}

void UMatch3BlueprintFunctionLibrary::PauseGameTimer(UObject* WorldContextObject, bool bPause)
{
	if (AMatch3GameMode* GameMode = Cast<AMatch3GameMode>(UGameplayStatics::GetGameMode(WorldContextObject)))
	{
		GameMode->PauseGameTimer(bPause);
	}
}