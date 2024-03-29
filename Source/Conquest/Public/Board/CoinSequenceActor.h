// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Conquest.h"
#include "GameFramework/Actor.h"
#include "CoinSequenceActor.generated.h"

class ACoin;
class UCameraComponent;

/**
 * Handles the coin flip sequence, including notifying the players to switch
 * to the sequence camera. This needs to be placed in the level somewhere.
 * The sequence works by placing the camera at the flip location while the coin is flipped,
 * then once the flip is done, the camera will move to the focus location until sequence is done
 */
UCLASS()
class CONQUEST_API ACoinSequenceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ACoinSequenceActor();

public:

	// Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	virtual bool HasActiveCameraComponent() const override { return true; }
	// End AActor Interface

public:

	/** Get camera component */
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

private:

	/** The camera component to view coin flip with */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

protected:

	/** The coin this sequence actor will flip */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sequence)
	ACoin* Coin;

	/** The location to use when watching the coin flip (Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sequence, meta = (MakeEditWidget="true"))
	FVector FlipCameraLocation;

	/** The location to use after the coin flip has finished (Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sequence, meta = (MakeEditWidget = "true"))
	FVector FocusCameraLocation;

private:

	/** The location the camera should move to */
	UPROPERTY(Transient)
	FVector CameraDesiredLocation;

public:

	/** Starts the coin sequence */
	void StartCoinSequence();

	/** Finishes the coin sequence */
	void FinishCoinSequence();

public:

	/** If this coin sequence actor is able to start the sequence */
	UFUNCTION(BlueprintPure, Category = Sequence)
	bool CanActivateCoinSequence() const;

private:

	/** Notify to start the coin flip on each client */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StartCoinFlip();

	/** Notify to finish the coin flip on each client */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_FinishCoinFlip();

	/** Callback for when the coin has finished the flip sequence. This version is
	only called on the server and will inform the game mode of the flip result */
	UFUNCTION()
	void ServerHandleCoinFlipFinished(bool bHeads);

	/** Callback for when the coin has finished the flip sequence. This 
	version is called on every client to update the cameras desired location */
	UFUNCTION()
	void ClientHandleCoinFlipFinished(bool bHeads);

private:

	/** If the sequence is running */
	uint8 bIsSequenceRunning : 1;

	#if WITH_EDITORONLY_DATA
	/** If we should allow the coin sequence while in editor. This will
	result in warnings by the game mode that it failed to activate sequence */
	UPROPERTY(EditAnywhere, Category = Sequence)
	uint8 bSkipSequenceInPIE : 1;
	#endif

public:

	/** Set up the sequence and coin before starting the coin flip */
	void SetupCoinSequence();

	/** Clean up the sequence and coin either after finishing or if skipped */
	void CleanupCoinSequence();
};
