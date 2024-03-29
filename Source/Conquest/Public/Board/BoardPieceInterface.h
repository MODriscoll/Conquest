// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Conquest.h"
#include "UObject/Interface.h"
#include "BoardPieceInterface.generated.h"

class ACSKPlayerState;
class ATile;
class UHealthComponent;

UINTERFACE(MinimalAPI)
class UBoardPieceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that must be implemented for pieces that can be placed onto tiles.
 */
class CONQUEST_API IBoardPieceInterface
{
	GENERATED_BODY()

public:

	/** Set the player state for the player who owns this board piece */
	virtual void SetBoardPieceOwnerPlayerState(ACSKPlayerState* InPlayerState) PURE_VIRTUAL(IBoardPieceInterface::SetBoardPieceOwnerPlayerState, )

	/** Get the player state for the player who owns this board peice */
	virtual ACSKPlayerState* GetBoardPieceOwnerPlayerState() const PURE_VIRTUAL(IBoardPieceInterface::GetBoardPieceOwnerPlayerState, return nullptr;)

	/** Get this board pieces health component */
	virtual UHealthComponent* GetHealthComponent() const PURE_VIRTUAL(IBoardPieceInterface::GetHealthComponent, return nullptr;)

public:

	/** Event for when this board piece has been placed on given tile (called on clients) */
	virtual void PlacedOnTile(ATile* Tile) { check(Tile != nullptr); }

	/** Event for when this board piece has been removed from the tile it once occupied (called on clients) */
	virtual void RemovedOffTile() {  }

	/** Event for when the tile this board piece is on has been hovered by the player */
	virtual void OnHoverStart() { }

	/** Event for when the tile this board piece is on is no longer hovered by the player */
	virtual void OnHoverFinish() { }

public:

	/** Get information about this board piece to display to the player.*/
	virtual void GetBoardPieceUIData(FBoardPieceUIData& OutData) const { }
};
