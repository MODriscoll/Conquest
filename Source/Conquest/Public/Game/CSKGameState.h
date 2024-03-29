// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Conquest.h"
#include "GameFramework/GameStateBase.h"
#include "CSKGameState.generated.h"

class ABoardManager;
class ACastle;
class ACSKPawn;
class ACSKPlayerController;
class ACSKPlayerState;
class ACSKGameMode;
class ATile;
class ATower;
class USpell;
class USpellCard;
class UTowerConstructionData;

/** The state of the games timer (What is currently being timed */
UENUM(BlueprintType)
enum class ECSKTimerState : uint8
{
	/** Counting down action phase */
	ActionPhase,

	/** Counting down quick effect selection */
	QuickEffect,

	/** Counting down bonus spell selection */
	BonusSpell,

	/** Counting down a custom timer (notify via OnCustomTimerFinished) */
	Custom,

	/** Timer is inactive */
	None UMETA(Hidden="true")
};

/** Delegate for when the round state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSKRoundStateChanged, ECSKRoundState, NewState);

/** Delegate for when the custom timer has finished. Passes if timer was skipped */
DECLARE_DYNAMIC_DELEGATE_OneParam(FCSKCustomTimerFinished, bool, bWasSkipped);

/**
 * Tracks state of game and stats about the board
 */
UCLASS(ClassGroup = (CSK))
class CONQUEST_API ACSKGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	ACSKGameState();

protected:

	// Begin AGameStateBase Interface
	virtual void OnRep_ReplicatedHasBegunPlay() override;
	// End AGameStateBase Interface

	// Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End UObject Interface

public:

	/** Do not call this externally. This is used by the game mode to set the board to use */
	void SetMatchBoardManager(ABoardManager* InBoardManager);

	/** Do not call this externally. This is used by the local player controller to set the players pawn */
	void SetLocalPlayersPawn(ACSKPawn* InPlayerPawn);

public:

	/** Get the games board manager */
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardManager* GetBoardManager(bool bErrorCheck = true) const;

	/** Get the local players pawn */
	UFUNCTION(BlueprintPure, Category = "Board")
	ACSKPawn* GetLocalPlayerPawn() const { return LocalPlayerPawn; }

private:

	/** The board of this match */
	UPROPERTY(Transient, Replicated)
	ABoardManager* BoardManager;

	/** The local players pawn. We save this here to
	allow tower and spell actions to move the camera */
	UPROPERTY(Transient)
	ACSKPawn* LocalPlayerPawn;

public:

	/** Sets the state of the match */
	void SetMatchState(ECSKMatchState NewState);

	/** Sets the state of the round */
	void SetRoundState(ECSKRoundState NewState);

public:

	/** Get the state of the match */
	FORCEINLINE ECSKMatchState GetMatchState() const { return MatchState; }

	/** Get the state of the round */
	FORCEINLINE ECSKRoundState GetRoundState() const { return RoundState; }

	/** Get if the match is active */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool IsMatchInProgress() const;

	/** Get if an action phase is active */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool IsActionPhaseActive() const;

	/** Get if the end round phase is active */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool IsEndRoundPhaseActive() const;

	/** Get the player ID of the winner */
	UFUNCTION(BlueprintPure, Category = CSK)
	int32 GetMatchWinnerPlayerID() const { return MatchWinnerPlayerID; }

	/** Get the win condition the player met to win the game */
	UFUNCTION(BlueprintPure, Category = CSK)
	ECSKMatchWinCondition GetMatchWinCondition() const { return MatchWinCondition; }

protected:

	/** Match state notifies */
	void NotifyWaitingForPlayers();
	void NotifyPerformCoinFlip();
	void NotifyMatchStart();
	void NotifyMatchFinished();
	void NotifyPlayersLeaving();
	void NotifyMatchAbort();

	/** Round state notifies */
	void NotifyCollectionPhaseStart();
	void NotifyFirstActionPhaseStart();
	void NotifySecondActionPhaseStart();
	void NotifyEndRoundPhaseStart();

private:

	/** Notify that match state has just been replicated */
	UFUNCTION()
	void OnRep_MatchState();

	/** Determines which match state change notify to call */
	void HandleMatchStateChange(ECSKMatchState NewState);

	/** Notify that round state has just been replicated */
	UFUNCTION()
	void OnRep_RoundState();

	/** Determines which round state change notify to call */
	void HandleRoundStateChange(ECSKRoundState NewState);

	/** Set the match win details on all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_SetWinDetails(int32 WinnerID, ECSKMatchWinCondition WinCondition);

public:

	/** Event called when the round state has changed */
	UPROPERTY(BlueprintAssignable, Category = CSK)
	FCSKRoundStateChanged OnRoundStateChanged;

protected:

	/** The current state of the match */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_MatchState)
	ECSKMatchState MatchState;

	/** The last match state we were running (client side) */
	UPROPERTY()
	ECSKMatchState PreviousMatchState;

	/** During match, what phase of the round we are up to */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_RoundState)
	ECSKRoundState RoundState;

	/** The last round phase we were running (client side) */
	UPROPERTY()
	ECSKRoundState PreviousRoundState;

	/** The ID of the player who won the match */
	UPROPERTY()
	int32 MatchWinnerPlayerID;

	/** The condition the winner met to win the match */
	UPROPERTY()
	ECSKMatchWinCondition MatchWinCondition;

public:

	/** Get the player ID of whose action phase it is */
	FORCEINLINE int32 GetActionPhasePlayerID() const { return ActionPhasePlayerID; }

	/** Get a player state based off a player ID */
	UFUNCTION(BlueprintPure, Category = CSK)
	ACSKPlayerState* GetPlayerStateWithID(int32 PlayerID) const;

	/** Get the opposings player state based off given player state */
	UFUNCTION(BlueprintPure, Category = CSK)
	ACSKPlayerState* GetOpposingPlayerState(ACSKPlayerState* Player) const;

	/** Get if action phase is timed */
	UFUNCTION(BlueprintPure, Category = Rules)
	bool IsActionPhaseTimed() const { return ActionPhaseTime != -1; }

	/** Activates a custom timer for given duration. This timer will call
	CustomTimerFinishedEvent once completed, which can be bound to using GetCustomTimerFinishedEvent() */
	bool ActivateCustomTimer(int32 InDuration);

	/** Clears custom timer if custom timer is currently active */
	void DeactivateCustomTimer();

	/** Get the custom timer finished event */
	FCSKCustomTimerFinished& GetCustomTimerFinishedEvent() { return CustomTimerFinishedEvent; }

	/** Get if the state timer is currently active */
	UFUNCTION(BlueprintPure, Category = Rules)
	bool IsTimerActive() const { return TimerState != ECSKTimerState::None; }

	/** Get the time remaining for current action taking place (this
	can either action phase turn time, quick effect counter time) */
	UFUNCTION(BlueprintPure, Category = Rules)
	int32 GetCountdownTimeRemaining(bool& bOutIsInfinite) const;

	/** Get the amount of instances of given type of tower active on the board */
	UFUNCTION(BlueprintPure, Category = Rules)
	int32 GetTowerInstanceCount(TSubclassOf<ATower> Tower) const;

	/** Updates the latest action health reports */
	void SetLatestActionHealthReports(const TArray<FHealthChangeReport>& InHealthReports);

	/** Get all the towers that were damaged during the previous action */
	UFUNCTION(BlueprintPure, Category = "CSK|Game")
	TArray<FHealthChangeReport> GetDamageHealthReports(bool bFilterOutDead = false) const;

	/** Get all the towers that were healed during the previous action */
	UFUNCTION(BlueprintPure, Category = "CSK|Game")
	TArray<FHealthChangeReport> GetHealingHealthReports() const;

	/** Get all the towers that were damaged during the previous action that belong to specified player */
	UFUNCTION(BlueprintPure, Category = "CSK|Game")
	TArray<FHealthChangeReport> GetPlayersDamagedHealthReports(ACSKPlayerState* PlayerState, bool bFilterOutDead = false) const;

	/** Get all the towers that were healed during the previous action that belong to specified player */
	UFUNCTION(BlueprintPure, Category = "CSK|Game")
	TArray<FHealthChangeReport> GetPlayersHealingHealthReports(ACSKPlayerState* PlayerState) const;

protected:

	/** Activates the timer for given state */
	void ActivateTickTimer(ECSKTimerState InTimerState, int32 InTime);

	/** Deactivates the timer */
	void DeactivateTickTimer();

	/** Set if tick for timer is enabled/disabled */
	void SetTickTimerEnabled(bool bEnable);

	/** Helper function for adding bonus time to given time clamped by action phase time */
	int32 GetActionTimeBonusApplied(int32 Time) const;

private:

	/** Updates action phase properties, including activating timer */
	void UpdateActionPhaseProperties();

	/** Handles ticking the timer */
	void TickTimer();

	/** Handles when timer has finished */
	void HandleTickTimerFinished();

	/** Executes the custom timer finished event only if bound */
	void ExecuteCustomTimerFinishedEvent(bool bWasSkipped);

	/** Generates a new array containing health reports filtered by passed in arguments */
	TArray<FHealthChangeReport> QueryLatestHealthReports(bool bDamaged, ACSKPlayerState* InOwner, bool bExcludeDead) const;

protected:

	/** ID of the player who won the coin toss */
	UPROPERTY(Transient, Replicated)
	int32 CoinTossWinnerPlayerID;

	/** ID of the player whose action phase it is */
	UPROPERTY(Transient)
	int32 ActionPhasePlayerID;

	/** The reason why the timer is currently ticking */
	UPROPERTY(Transient, Replicated)
	ECSKTimerState TimerState;

	/** The time remaining for the current timer state */
	UPROPERTY(Transient, Replicated)
	int32 TimeRemaining;

	/** The amount of time the action phase had before entering a different timer state */
	UPROPERTY(Transient)
	int32 NewActionPhaseTimeRemaining;

	/** If the timer is paused, this doesn't pause the actual timer
	but instead skips TickTimer whenever the callback is executed */
	UPROPERTY(Transient)
	uint32 bTimerPaused : 1;

	/** Lookup table for how many instances of a certain tower exists on the board */
	UPROPERTY(Transient)
	TMap<TSubclassOf<ATower>, int32> TowerInstanceTable;

	/** The health reports from the latest action */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = "CSK|Game")
	TArray<FHealthChangeReport> LatestActionHealthReports;

private:

	/** Handle for the timers tick */
	FTimerHandle Handle_TickTimer;

	/** Event for when the custom timer has finished */
	FCSKCustomTimerFinished CustomTimerFinishedEvent;

public:

	/** Notify that a move request has been confirmed and is starting */
	void HandleMoveRequestConfirmed();

	/** Notify that the current move request has finished */
	void HandleMoveRequestFinished();

	/** Notify that a new tower has been placed on the map */
	void HandleBuildRequestConfirmed(ATile* TargetTile);

	/** Notify that the current build request has finished */
	void HandleBuildRequestFinished(ATower* NewTower);

	/** Notify that a spell has been cast and will soon start */
	void HandleSpellRequestConfirmed(EActiveSpellContext Context, ATile* TargetTile);

	/** Notify that the current spell request has finished */
	void HandleSpellRequestFinished(EActiveSpellContext Context);

	/** Notify that a quick effect is being selected */
	void HandleQuickEffectSelectionStart(bool bNullify);

	/** Notify that a bonus spell is being targeted */
	void HandleBonusSpellSelectionStart();

private:

	/** Handle movement request confirmation client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleMoveRequestConfirmed();

	/** Handle movement request finished client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleMoveRequestFinished();

	/** Handle build request confirmation client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleBuildRequestConfirmed(ATile* TargetTile);

	/** Handle build request finished client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleBuildRequestFinished(ATower* NewTower);

	/** Handle spell request confirmation client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleSpellRequestConfirmed(EActiveSpellContext Context, ATile* TargetTile);

	/** Handle spell request finished client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleSpellRequestFinished(EActiveSpellContext Context);

	/** Handle quick effect selection client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleQuickEffectSelection(bool bNullify);

	/** Handle bonus spell selection client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleBonusSpellSelection();

public:

	/** If given player has moved the required amount of tiles this turn */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool HasPlayerMovedRequiredTiles(const ACSKPlayerController* Controller) const;

	/** Get the remaining amount of tiles the given player is allowed to move */
	UFUNCTION(BlueprintPure, Category = CSK)
	int32 GetPlayersNumRemainingMoves(const ACSKPlayerState* PlayerState) const;

	/** Get the tiles the given player is able to move to. Can optionally
	pathfind to each tile to guarantee that the tile can be reached */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool GetTilesPlayerCanMoveTo(const ACSKPlayerController* Controller, TArray<ATile*>& OutTiles, bool bPathfind = false) const;

	/** Get the tiles the given player is able to build tiles on. 
	This assumes player is able to build at least one tower */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool GetTilesPlayerCanBuildOn(const ACSKPlayerController* Controller, TArray<ATile*>& OutTiles);

	/** If given player can build or destroy the given tower */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool CanPlayerBuildTower(const ACSKPlayerController* Controller, TSubclassOf<UTowerConstructionData> TowerTemplate) const;

	/** If given player can build or destroy anymore towers this turn */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool CanPlayerBuildMoreTowers(const ACSKPlayerController* Controller) const;

	/** Get all the towers the given player can build */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool GetTowersPlayerCanBuild(const ACSKPlayerController* Controller, TArray<TSubclassOf<UTowerConstructionData>>& OutTowers) const;

	/** Get the remaining amount of spells the given player is allowed to cast */
	UFUNCTION(BlueprintPure, Category = CSK)
	int32 GetPlayerNumRemainingSpellCasts(const ACSKPlayerState* PlayerState, bool& bOutInfinite) const;

	/** If given player is able to afford given spell. This will check dynamic cost along with static cost.
	This does not check if spell is able to be cast at tile, so be sure to check that before calling this function */
	UFUNCTION(BlueprintPure, Category = CSK)
	bool CanPlayerCastSpell(const ACSKPlayerController* Controller, ATile* TargetTile,
		TSubclassOf<USpellCard> SpellCard, int32 SpellIndex, int32 AdditionalMana) const;

	/** Get all towers that can be built this match */
	FORCEINLINE const TArray<TSubclassOf<UTowerConstructionData>>& GetAvailableTowers() const { return AvailableTowers; }

protected:

	/** Updates the rules variables by cloning rules establish by game mode */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = CSK)
	void UpdateRules();

	/** Helper function for checking if given player can build or destroy given tower */
	bool CanPlayerBuildTower(const ACSKPlayerState* PlayerState, TSubclassOf<UTowerConstructionData> TowerTemplate) const;
	
protected:

	/** Cached action phase timer used to reset action phase time each round */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 ActionPhaseTime;

	/** The minimum amount of tiles a player must move each action phase */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MinTileMovements;

	/** The maximum amount of tiles a player can move each action round */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MaxTileMovements;

	/** The max number of NORMAL towers players are allowed to build */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MaxNumTowers;

	/** The max number of duplicated NORMAL towers a player can have built at once */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MaxNumDuplicatedTowers;

	/** The max amount of duplicated types of all NORMAL towers player can have built at once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Rules)
	int32 MaxNumDuplicatedTowerTypes;

	/** The max number of LEGENDARY towers a player can have built at once */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MaxNumLegendaryTowers;

	/** The max range from the players castle they can build from */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	int32 MaxBuildRange;

	/** The towers supported for this match */
	// TODO: See CSKGameMode.h (ln 412) for a TODO
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category = Rules)
	TArray<TSubclassOf<UTowerConstructionData>> AvailableTowers;

public:

	/** Notify that the given player has reached their opponents portal */
	void HandlePortalReached(ACSKPlayerController* Controller, ATile* ReachedPortal);

	/** Notify that the given player has destroyed their opponents castle */
	void HandleCastleDestroyed(ACSKPlayerController* Controller, ACastle* DestroyedCastle);

	/** Notify that a building has been destroyed */
	void HandleTowerDestroyed(ATower* DestroyedTower, bool bByRequest);

private:

	/** Handle portal reached client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandlePortalReached(ACSKPlayerState* Player, ATile* ReachedPortal);

	/** Handle castle destroyed client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleCastleDestroyed(ACSKPlayerState* Player, ACastle* DestroyedCastle);

	/** Handle tower destroyed client side */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_HandleTowerDestroyed(ATower* DestroyedTower, bool bByRequest);

public:

	/** Get the total time of the match. If match is
	still running, how long the match has been in session */
	UFUNCTION(BlueprintPure, Category = CSK)
	float GetMatchTimeSeconds() const;

	/** Get the current round being played */
	FORCEINLINE int32 GetRound() const { return RoundsPlayed; }

protected:

	/** The time when the match started (Coin Flip) */
	float MatchStartTime;

	/** The time when the match finished */
	float MatchEndTime;

	/** How many rounds have been played */
	UPROPERTY(BlueprintReadOnly, Category = Stats)
	int32 RoundsPlayed;
};
