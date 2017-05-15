// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PaperSprite.h"
#include "Tile.h"
#include "Grid.generated.h"

//´´½¨

USTRUCT(BlueprintType)
struct FTileType
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float Probability;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UMaterialInstanceConstant* TileMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EffectColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTileAbilities Abilities;

	FTileType()
	: Probability(1.0f)
	{
	}
};

UCLASS()
class MATCH3_API AGrid : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AGrid(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ATile*> GameTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTileType> TileLibrary;
	
	/** The size of a space on the grid. Does not include borders or spacing between tiles. */
	UPROPERTY(EditAnywhere, Category = Tile)
	FVector2D TileSize;

	/** Minimum number of matching tiles in a row needed to score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tile)
	int32 MinimumRunLength;

	/** The width of the grid. Needed to calculate tile positions and neighbors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tile)
	int32 GridWidth;

	/** The height of the grid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tile)
	int32 GridHeight;

	/** Spawn a tile and associate it with a specific grid address. */
	ATile* CreateTile(TSubclassOf<class ATile> TileToSpawn, class UMaterialInstanceConstant* TileMaterial, FVector SpawnLocation, int32 SpawnGridAddress, int32 TileTypeID);
	/** Randomly select a type of tile from the grid's library, using the probability values on the tiles. */
	int32 SelectTileFromLibrary();

	/** Get the pointer to the tile at the specified grid address. */
	ATile* GetTileFromGridAddress(int32 GridAddress) const;

	/** Initialize the tiles on the grid*/
	UFUNCTION(BlueprintCallable, Category = Initialization)
	void InitGrid();

	/** Play effects when a move is made. Use this to avoid spamming sounds on tiles. */
	UFUNCTION(BlueprintImplementableEvent, meta = (ExpandEnumAsExecs = "MoveType"), Category = Tile)
	void OnMoveMade(EMatch3MoveType::Type MoveType);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void ReturnMatchSounds(TArray<USoundWave*>& MatchSounds);

	/** Get the world location for a given grid address. */
	UFUNCTION(BlueprintCallable, Category = Tile)
	FVector GetLocationFromGridAddress(int32 GridAddress) const;
	/** Get the world location for a grid address relative to another grid address. Offset between addresses is measured in tiles. */
	FVector GetLocationFromGridAddressWithOffset(int32 GridAddress, int32 XOffsetInTiles, int32 YOffsetInTiles) const;
	/** Get a grid address relative to another grid address. Offset between addresses is measured in tiles. */
	UFUNCTION(BlueprintCallable, Category = Tile)
	bool GetGridAddressWithOffset(int32 InitialGridAddress, int32 XOffset, int32 YOffset, int32 &ReturnGridAddress) const;
	/** Determine if two grid addresses are valid and adjacent. */
	bool AreAddressesNeighbors(int32 GridAddressA, int32 GridAddressB) const;

	void OnTileFinishedFalling(ATile* Tile, int32 LandingGridAddress);
	void OnTileFinishedMatching(ATile* InTile);
	void OnSwapDisplayFinished(ATile* InTile);

	void RespawnTiles();
	void SwapTiles(ATile* A, ATile* B, bool bRepositionTileActors = false);

	/** Tests a move to see if it's permitted. */
	bool IsMoveLegal(ATile* A, ATile* B);

	/** Get list of tiles that will be affected by a bomb's explosion. */
	TArray<ATile*> GetExplosionList(ATile* A) const;
	/** Check for a successful sequence. bMustMatchID can be set to false to ignore matching. MinimumLengthRequired will default to the game's MinimumRunLength setting if negative. */
	TArray<ATile*> FindNeighbors(ATile* StartingTile, bool bMustMatchID = true, int32 RunLength = -1) const;
	/** Find all tiles of a given type. */
	TArray<ATile*> FindTilesOfType(int32 TileTypeID) const;
	/** Execute the result of one or more matches. It is possible, with multiple matches, to have more than one tile type in the array. */
	void ExecuteMatch(const TArray<ATile*>& MatchingTiles);
	/** React to a tile being clicked. */
	void OnTileWasSelected(ATile* NewSelectedTile);

	/** Detects unwinnable states. */
	bool IsUnwinnable();

	/** Establishes the most recent move type for the specified player. */
	void SetLastMove(EMatch3MoveType::Type MoveType);

	/** Tells what type of move the specified player made most recently. */
	EMatch3MoveType::Type GetLastMove();

	/** Gives point value per tile based on move type. Default is 100. */
	UFUNCTION(BlueprintNativeEvent, Category = Game)
	int32 GetScoreMultiplierForMove(EMatch3MoveType::Type LastMoveType);
	virtual int32 GetScoreMultiplierForMove_Implementation(EMatch3MoveType::Type LastMoveType);

	ATile* GetCurrentlySelectedTile() const { return CurrentlySelectedTile; };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tile)
	ATile* CurrentlySelectedTile;

private:
	/** Array of tiles found in the most recent call to IsMoveLegal. */
	TArray<ATile*> LastLegalMatch;
	/** Tiles that are currently falling. */
	TArray<ATile*> FallingTiles;
	/** Tiles that are currently swapping positions with each other. Should be exactly two of them, or zero. */
	TArray<ATile*> SwappingTiles;
	/** After spawning new tiles, which tiles to check for automatic matches. */
	TArray<ATile*> TilesToCheck;
	/** Tiles that are currently reacting to being matches. */
	TArray<ATile*> TilesBeingDestroyed;
	/** The type of move last executed by a given player. */
	TMap<APlayerController*, EMatch3MoveType::Type> LastMoves;
	/** Indicates that we are waiting to complete a swap move. When SwappingTiles is populated by two tiles, we are done. */
	uint32 bPendingSwapMove : 1;
	/** Indicates that we are waiting to complete a swap move. When SwappingTiles is populated by two tiles, we are done. */
	uint32 bPendingSwapMoveSuccess : 1;
};
