// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Match3.h"
#include "Match3GameMode.h"
#include "Grid.h"
#include "Kismet/GameplayStatics.h"
#include "Tile.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	
	// We are going to use a Scene Component to base our Tile on. The PaperSpriteActor should have a RenderComponent as the default root, so we are going to attach it to our new root.

		if(RootComponent)
		{
			RootComponent->SetMobility(EComponentMobility::Movable);
		}
	
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	
	Grid = Cast<AGrid>(GetOwner());

	// Set our class up to handle touch events.
	OnInputTouchBegin.AddUniqueDynamic(this, &ATile::TilePress);
	OnInputTouchEnter.AddUniqueDynamic(this, &ATile::TileEnter);
}

// Called every frame
void ATile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ATile::SetTileMaterial_Implementation(class UMaterialInstanceConstant* MaterialToUse)
{
	GetRenderComponent()->SetMaterial(0, MaterialToUse);
}


void ATile::TilePress(ETouchIndex::Type FingerIndex, AActor* TouchedActor)
{
	// We clicked or touched the tile.
	if (!UGameplayStatics::IsGamePaused(this) && Grid)
	{
		Grid->OnTileWasSelected(this);
	}
}

void ATile::TileEnter(ETouchIndex::Type FingerIndex, AActor* TouchedActor)
{
	// We have moved into the tile's space while we had a different tile selected. This is the same as pressing the tile directly.
	// Note that we need to make sure it's a different actual tile (i.e. not NULL) because deselecting a tile by touching it twice will then trigger the TileEnter event and re-select it.
	if (!UGameplayStatics::IsGamePaused(this) && Grid)
	{
		ATile* CurrentlySelectedTile = Grid->GetCurrentlySelectedTile();
		if (CurrentlySelectedTile && (CurrentlySelectedTile != this))
		{
			TilePress(FingerIndex, TouchedActor);
		}
	}
}

void ATile::TilePress_Mouse(AActor* ClickedActor, FKey ButtonClicked)
{
	TilePress(ETouchIndex::Touch1, ClickedActor);
}

void ATile::TileEnter_Mouse(AActor* MousedOverActor)
{
	// This is meant to simulate finger-swiping, so ignore if the mouse isn't clicked.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->IsInputKeyDown(EKeys::LeftMouseButton))
		{
			TileEnter(ETouchIndex::Touch1, MousedOverActor);
		}
	}
}

void ATile::OnMatched_Implementation(EMatch3MoveType::Type MoveType)
{
	Grid->OnTileFinishedMatching(this);
}

void ATile::OnSwapMove_Implementation(ATile* OtherTile, bool bMoveWillSucceed)
{
	Grid->OnSwapDisplayFinished(this);
}

void ATile::StartFalling(bool bUseCurrentWorldLocation)
{
	float FallDistance = 0;

	FallingStartTime = GetWorld()->GetTimeSeconds();
	FallingStartLocation = GetActorLocation();
	// Tiles fall at a fixed rate of 120 FPS.
	GetWorldTimerManager().SetTimer(TickFallingHandle, this, &ATile::TickFalling, 0.001f, true);
	check(Grid);

	if (!bUseCurrentWorldLocation)
	{
		// Fall from where we are on the grid to where we are supposed to be on the grid.
		int32 YOffset = 0;
		int32 HeightAboveBottom = 1;
		while (true)
		{
			++YOffset;
			if (Grid->GetGridAddressWithOffset(GetGridAddress(), 0, -YOffset, LandingGridAddress))
			{
				if (ATile* TileBelow = Grid->GetTileFromGridAddress(LandingGridAddress))
				{
					// We're not off the grid, so check to see what is in this space and react to it.
					if (TileBelow->TileState == ETileState::ETS_Falling)
					{
						// This space contains a falling tile, so continue to fall through it, but note that the tile will land underneath us, so we need to leave a gap for it.
						++HeightAboveBottom;
						continue;
					}
					else if (TileBelow->TileState == ETileState::ETS_PendingDelete)
					{
						// This space contains a tile that is about to be deleted. We can fall through this space freely.
						continue;
					}
				}
				else
				{
					// The space below is empty, but is on the grid. We can fall through this space freely.
					continue;
				}
			}
			// This space is off the grid or contains a tile that is staying. Go back one space and stop.
			YOffset -= HeightAboveBottom;
			Grid->GetGridAddressWithOffset(GetGridAddress(), 0, -YOffset, LandingGridAddress);
			break;
		}
		FallDistance = Grid->TileSize.Y * YOffset;
		FallingEndLocation = FallingStartLocation;
		FallingEndLocation.Z -= FallDistance;
	}
	else
	{
		// Fall from where we are physically to where we are supposed to be on the grid.
		LandingGridAddress = GetGridAddress();
		FallingEndLocation = Grid->GetLocationFromGridAddress(LandingGridAddress);
		FallDistance = FallingStartLocation.Z - FallingEndLocation.Z;
	}
	AMatch3GameMode* CurrentGameMode = Cast<AMatch3GameMode>(UGameplayStatics::GetGameMode(this));
	TotalFallingTime = 0.0f;
	if (CurrentGameMode && (CurrentGameMode->TileMoveSpeed > 0.0f))
	{
		TotalFallingTime = FallDistance / CurrentGameMode->TileMoveSpeed;
	}
	if (TotalFallingTime <= 0.0f)
	{
		TotalFallingTime = 0.75f;
	}
	StartFallingEffect();
}

void ATile::TickFalling()
{
	AMatch3GameMode* CurrentGameMode = Cast<AMatch3GameMode>(UGameplayStatics::GetGameMode(this));
	if (CurrentGameMode)
	{
		check(Grid);
		check(TotalFallingTime > 0.0f);
		float FallCompleteFraction = (GetWorld()->GetTimeSeconds() - FallingStartTime) / TotalFallingTime;

		// Stop falling if we're at the final location. Otherwise, continue to move.
		if (FallCompleteFraction >= 1.0f)
		{
			FinishFalling();
		}
		else
		{
			FVector NewLocation = FMath::Lerp(FallingStartLocation, FallingEndLocation, FallCompleteFraction);
			SetActorLocation(NewLocation);
		}
	}
	else
	{
		// Error. Stop ticking this function. Move the tile to the final location.
		FinishFalling();
	}
}

void ATile::FinishFalling()
{
	GetWorldTimerManager().ClearTimer(TickFallingHandle);
	SetActorLocation(FallingEndLocation);
	Grid->OnTileFinishedFalling(this, LandingGridAddress);
	StopFallingEffect();
}


void ATile::SetGridAddress(int32 NewLocation)
{
	GridAddress = NewLocation;
}

int32 ATile::GetGridAddress() const
{
	return GridAddress;
}

USoundWave* ATile::GetMatchSound()
{
	return MatchSound;
}

