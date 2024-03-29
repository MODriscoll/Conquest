// Fill out your copyright notice in the Description page of Project Settings.

#include "ConquestFunctionLibrary.h"
#include "Conquest.h"
#include "Board/BoardManager.h"
#include "Board/CoinSequenceActor.h"
#include "Board/Tile.h"
#include "Game/CSKGameInstance.h"
#include "Game/CSKGameMode.h"
#include "Game/CSKGameState.h"
#include "Game/CSKPawn.h"
#include "Resources/SpellCard.h"

#include "UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

bool FMapSelectionDetails::IsValid() const
{
	FString TempString = MapFileName;
	if (GEngine && GEngine->MakeSureMapNameIsValid(TempString))
	{
		return true;
	}

	return false;
}

bool UConquestFunctionLibrary::IsWithEditor()
{
	#if WITH_EDITOR
	return true;
	#else
	return false;
	#endif
}

bool UConquestFunctionLibrary::IsOnMobile()
{
	#if PLATFORM_IOS || PLATFORM_ANDROID
	return true;
	#else
	return false;
	#endif
}

UCSKGameInstance* UConquestFunctionLibrary::GetCSKGameInstance(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetGameInstance<UCSKGameInstance>() : nullptr;
}

ACSKGameMode* UConquestFunctionLibrary::GetCSKGameMode(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetAuthGameMode<ACSKGameMode>() : nullptr;
}

ACSKGameState* UConquestFunctionLibrary::GetCSKGameState(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetGameState<ACSKGameState>() : nullptr;
}

ABoardManager* UConquestFunctionLibrary::GetMatchBoardManager(const UObject* WorldContextObject, bool bWarnIfNull)
{
	const ACSKGameState* GameState = GetCSKGameState(WorldContextObject);
	return GameState ? GameState->GetBoardManager(bWarnIfNull) : nullptr;
}

ABoardManager* UConquestFunctionLibrary::FindMatchBoardManager(const UObject* WorldContextObject, bool bWarnIfNotFound)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		for (TActorIterator<ABoardManager> It(World); It; ++It)
		{
			return *It;
		}
	}

	if (bWarnIfNotFound)
	{
		UE_LOG(LogConquest, Warning, TEXT("FindMatchBoardManager: Was not able to find a board manager in world %s"), *World->GetPathName());
	}

	return nullptr;
}

ACoinSequenceActor* UConquestFunctionLibrary::FindCoinSequenceActor(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		for (TActorIterator<ACoinSequenceActor> It(World); It; ++It)
		{
			return *It;
		}
	}

	return nullptr;
}

ACSKPawn* UConquestFunctionLibrary::GetLocalPlayersCSKPawn(const UObject* WorldContextObject)
{
	ACSKGameState* GameState = GetCSKGameState(WorldContextObject);
	return GameState ? GameState->GetLocalPlayerPawn() : nullptr;
}

void UConquestFunctionLibrary::MoveLocalPlayerToTile(const UObject* WorldContextObject, ATile* Tile, float TravelTime, bool bCancellable)
{
	if (Tile)
	{
		ACSKPawn* PlayersPawn = GetLocalPlayersCSKPawn(WorldContextObject);
		if (PlayersPawn)
		{
			PlayersPawn->TravelToLocation(Tile->GetActorLocation(), TravelTime, bCancellable);
		}
	}
}

bool UConquestFunctionLibrary::AreTilesWithingRange(const ATile* T1, const ATile* T2, int32 Range, int32& OutDistance)
{
	OutDistance = -1;

	if (T1 && T2)
	{
		OutDistance = FHexGrid::HexDisplacement(T1->GetGridHexValue(), T2->GetGridHexValue());
		return OutDistance <= Range;
	}

	return false;
}

int32 UConquestFunctionLibrary::AccumulateHealthReportDeltas(const TArray<FHealthChangeReport>& Reports)
{
	int32 Total = 0;

	for (const FHealthChangeReport& Repo : Reports)
	{
		Total += Repo.Delta;
	}

	return Total;
}

bool UConquestFunctionLibrary::CanActivateSpell(TSubclassOf<USpell> Spell, const ACSKPlayerState* CastingPlayer, const ATile* TargetTile)
{
	if (Spell)
	{
		// Can Activate Spell expects these to be valid
		if (!CastingPlayer || !TargetTile)
		{
			return false;
		}

		const USpell* DefaultSpell = Spell.GetDefaultObject();
		if (DefaultSpell && DefaultSpell->CanActivateSpell(CastingPlayer, TargetTile))
		{
			return true;
		}
	}

	return false;
}

void UConquestFunctionLibrary::AddWidgetToViewport(UUserWidget* Widget, int32 ZOrder)
{
	if (Widget && !Widget->IsInViewport())
	{
		Widget->AddToViewport(ZOrder);
	}
}

void UConquestFunctionLibrary::RemoveWidgetFromParent(UUserWidget* Widget)
{
	if (Widget && Widget->IsInViewport())
	{
		Widget->RemoveFromParent();
	}
}

FString UConquestFunctionLibrary::GetSecondsAsHourString(float Seconds)
{
	// Number of hours
	int32 NumHours = FMath::FloorToInt(Seconds / 3600.f);

	// Number of minutes
	int32 NumMinutes = FMath::FloorToInt(Seconds / 60.f);

	// Number of seconds
	int32 NumSeconds = FMath::FloorToInt(Seconds - static_cast<float>((NumMinutes * 60)));
		
	return FString::Printf(TEXT("%01d:%02d:%02d"), NumHours, NumMinutes, NumSeconds);
}
