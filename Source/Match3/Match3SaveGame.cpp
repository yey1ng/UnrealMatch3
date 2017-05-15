// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Match3.h"
#include "Match3SaveGame.h"
#include "Kismet/GameplayStatics.h"

bool UMatch3SaveGame::LoadCustomInt(FString FieldName, int32& Value) const
{
	const int32* ValuePointer = Match3CustomIntData.Find(FieldName);
	if (ValuePointer != nullptr)
	{
		Value = *ValuePointer;
		return true;
	}
	return false;
}

void UMatch3SaveGame::SaveCustomInt(FString FieldName, int32 Value)
{
	Match3CustomIntData.FindOrAdd(FieldName) = Value;
}

void UMatch3SaveGame::ClearCustomInt(FString FieldName)
{
	Match3CustomIntData.Remove(FieldName);
}

