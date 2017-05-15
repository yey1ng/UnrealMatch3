// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PaperSpriteActor.h"
#include "PaperSpriteComponent.h"
#include "Tile.generated.h"

UENUM()
namespace ETileState
{
	enum Type
	{
		ETS_Normal,
		ETS_Falling,
		ETS_PendingDelete
	};
}

UENUM(BlueprintType)
namespace EMatch3MoveType
{
	enum Type
	{
		MT_None,
		MT_Failure,
		MT_Standard,
		MT_MoreTiles,
		MT_Combo,
		MT_Bomb,
		MT_AllTheBombs,
		MT_MAX
	};
}

USTRUCT()
struct FTileAbilities
{
	GENERATED_USTRUCT_BODY();

	bool CanExplode() { return bExplodes; }
	bool CanSwap() { return (!bPreventSwapping && !bExplodes); }

protected:
	/** Tile explodes when selected (change this!) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bExplodes : 1;

	/** Tile can't be selected as part of a normal swapping move. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bPreventSwapping : 1;

public:
	/** Power rating of a bomb. What this means is determined in GameMode code, and can consider what kind of bomb this is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BombPower;
};

/**
*
*/
UCLASS()
class MATCH3_API ATile : public APaperSpriteActor
{
	GENERATED_BODY()

public:
	ATile();

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	/** When a tile is touched. */
	UFUNCTION()
	void TilePress(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

	/** When the user's finger moves over a tile. */
	UFUNCTION()
	void TileEnter(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

	/** Mouse surrogate for TilePress. */
	UFUNCTION()
	void TilePress_Mouse(AActor* ClickedActor, FKey ButtonClicked);

	/** Mouse surrogate for TileEnter. */
	UFUNCTION()
	void TileEnter_Mouse(AActor* MousedOverActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Events")
	void PlaySelectionEffect(bool bTurnEffectOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Events")
	void StartFallingEffect();

	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Events")
	void StopFallingEffect();

	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Events")
	void SetTileMaterial(class UMaterialInstanceConstant* TileMaterial);
	virtual void SetTileMaterial_Implementation(class UMaterialInstanceConstant* TileMaterial);

	/** Called when a match has been made, and says what type of move led to the match. */
	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Events")
	void OnMatched(EMatch3MoveType::Type MoveType);
	virtual void OnMatched_Implementation(EMatch3MoveType::Type MoveType);

	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Events")
	void OnSwapMove(ATile* OtherTile, bool bMoveWillSucceed);
	virtual void OnSwapMove_Implementation(ATile* OtherTile, bool bMoveWillSucceed);

	void StartFalling(bool bUseCurrentWorldLocation = false);

	USoundWave* GetMatchSound();


	UFUNCTION()
	void TickFalling();

	void FinishFalling();

	void SetGridAddress(int32 NewLocation);
	int32 GetGridAddress() const;

	UPROPERTY(BlueprintReadOnly)
	int32 TileTypeID;

	UPROPERTY()
	TEnumAsByte<ETileState::Type> TileState;

	UPROPERTY(BlueprintReadOnly)
	FTileAbilities Abilities;

protected:
	float TotalFallingTime;
	float FallingStartTime;
	FVector FallingStartLocation;
	FVector FallingEndLocation;
	FTimerHandle TickFallingHandle;

	/** Location on the grid as a 1D key/value. To find neighbors, ask the grid. */
	UPROPERTY(BlueprintReadOnly, Category = Tile)
	int32 GridAddress;

	/** Location where we will land on the grid as a 1D key/value. Used while falling. */
	int32 LandingGridAddress;

	/** The grid that owns this tile. Currently, this is set by casting the object that spawned the tile. */
	UPROPERTY(BlueprintReadOnly, Category = Tile)
	class AGrid* Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio, Meta = (BlueprintProtected=True))
	USoundWave* MatchSound;
};
