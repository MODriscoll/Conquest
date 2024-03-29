// Fill out your copyright notice in the Description page of Project Settings.

#include "CSKPawnMovement.h"
#include "CSKPawn.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

UCSKPawnMovement::UCSKPawnMovement()
{
	bIsTravelling = false;
	bCanCacelTravel = false;

	TravelToTime = 2.f;
}

void UCSKPawnMovement::AddInputVector(FVector WorldVector, bool bForce)
{
	// If tracking an actor or when executing a travel task that can't be cancelled, ignore input
	bool bCancelTravelTask = bIsTravelling && !bCanCacelTravel;
	if (!IsTrackingActor() && !bCancelTravelTask)
	{
		Super::AddInputVector(WorldVector, bForce);
	}
}

FVector UCSKPawnMovement::ConsumeInputVector()
{
	FVector InputVector = Super::ConsumeInputVector();
	if (InputVector != FVector::ZeroVector)
	{
		// If consuming the input vector, it means the player was allowed to
		// move thus cancel the transition goal we might have been travelling to
		bIsTravelling = false;
	}

	return InputVector;
}

bool UCSKPawnMovement::IsMoveInputIgnored() const
{
	bool bResult = Super::IsMoveInputIgnored();
	if (!bResult)
	{
		// If executing travel task that can't be cancelled
		bool bCancelTravelTask = bIsTravelling && !bCanCacelTravel;
		bResult = IsTrackingActor() || bCancelTravelTask;
	}

	return bResult;
}

void UCSKPawnMovement::ApplyControlInputToVelocity(float DeltaTime)
{
	if (IsTrackingActor())
	{
		UpdateTrackTaskVelocity(DeltaTime);
	}
	else if (bIsTravelling)
	{
		UpdateTravelTaskVelocity(DeltaTime);
	}
	else
	{
		Super::ApplyControlInputToVelocity(DeltaTime);
	}
}

void UCSKPawnMovement::TravelToLocation(const FVector& Location, float TravelTime, bool bCancellable)
{
	if (IsTrackingActor())
	{
		return;
	}

	if (!bIsTravelling || bCanCacelTravel)
	{
		bIsTravelling = true;
		bCanCacelTravel = bCancellable;

		TravelFrom = UpdatedComponent->GetComponentLocation();
		TravelGoal = Location;
		TravelToTime = TravelTime;
		TravelElapsedTime = 0.f;
	}
}

void UCSKPawnMovement::TrackActor(AActor* ActorToTrack, bool bIgnoreIfStatic)
{
	if (TrackedActor != ActorToTrack)
	{
		// Need to remove prerequisite tick
		if (TrackedActor)
		{
			RemoveTickPrerequisiteActor(TrackedActor);
			
			// Also reset velocity we were gained while tracking it
			Velocity = FVector::ZeroVector;
		}
	}

	// There is one more thing to check, set it to null
	// as actor may exist but not be valid for us to use
	TrackedActor = nullptr;

	if (ActorToTrack)
	{
		// Can track an actor that doesn't have a transform
		USceneComponent* Root = ActorToTrack->GetRootComponent();
		if (!Root)
		{
			return;
		}

		// We might not want to track actors that don't move
		if (bIgnoreIfStatic && Root->Mobility == EComponentMobility::Static)
		{
			return;
		}

		TrackedActor = ActorToTrack;

		// We want to tick after the actor has, so we can use it's latest position
		AddTickPrerequisiteActor(TrackedActor);

		// Override travelling task
		bIsTravelling = false;
	}
}

void UCSKPawnMovement::StopTrackingActor()
{
	TrackActor(nullptr);
}

// TODO: Maybe Move this to being executed in TickComponent (might need to copy FloatingPawnsMovements Tick and just add it somewhere)
void UCSKPawnMovement::UpdateTravelTaskVelocity(float DeltaTime)
{
	// Travels to location based on time we want to take
	TravelElapsedTime = FMath::Clamp(TravelElapsedTime + (DeltaTime / TravelToTime), 0.f, 1.f);

	float AlphaAlongTack = FMath::InterpSinInOut(0.f, 1.f, TravelElapsedTime);

	// Move along track
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FVector NewLocation = FMath::Lerp(TravelFrom, TravelGoal, AlphaAlongTack);

	// Dividing by delta time here as later in TickComponent, velocity will get multiplied by it (which we want to negate)
	Velocity = (NewLocation - OldLocation) / DeltaTime;

	// Have we finished travelling (at 1, we should be at our destination)
	if (TravelElapsedTime == 1.f)
	{
		bIsTravelling = false;

		ACSKPawn* Pawn = Cast<ACSKPawn>(GetPawnOwner());
		if (Pawn)
		{
			Pawn->OnTravelTaskFinished.Broadcast(Pawn);
		}
	}

	// Consume input for this frame. This could also cancel out this transition (if cancellable)
	ConsumeInputVector();
}

// TODO: Maybe Move this to being executed in TickComponent (might need to copy FloatingPawnsMovements Tick and just add it somewhere)
void UCSKPawnMovement::UpdateTrackTaskVelocity(float DeltaTime)
{
	check(TrackedActor);

	FVector TargetLocation = TrackedActor->GetActorLocation();
	FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	FVector Displacement = TargetLocation - ComponentLocation;
	
	float Distance = Displacement.Size();

	// Travel faster if further away, but maintain focus if within range
	const float TravelDilation = 3.f; // TODO: make this a variable?
	float Scale = Distance > (MaxSpeed / 4.f) ? (GetMaxSpeed() * TravelDilation) : (Distance / DeltaTime);
	
	Velocity = Displacement.GetSafeNormal() * Scale;

	// Consume input for this frame
	ConsumeInputVector();
}
