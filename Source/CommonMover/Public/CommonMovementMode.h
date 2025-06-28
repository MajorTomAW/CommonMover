// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagSyncState.h"
#include "MovementMode.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"

#include "CommonMovementMode.generated.h"

class UCommonMoverComponent;

/** Data struct that holds utility data for moving the updated component during simulation ticks. */
struct FCommonMoveData
{
	/** Original move delta for this simulation frame. */
	FVector OriginalMoveDelta;

	/** Move delta for the current stage of the simulation frame. */
	FVector CurrentMoveDelta;

	/** Target orientation that we want the updated component to achieve. */
	FQuat TargetOrientQuat;

	/** HitResult to hold any potential collision response data as we move the simulated component. */
	FHitResult MoveHitResult;

	/** Record of all the movement we've incurred so far in the far in the frame. */
	FMovementRecord MoveRecord;

	/** Percentage of the simulation time slice we've used so far while moving. */
	float PercentTimeAppliedSoFar = 0.0f;
};

/** Provides a common structure for movement modes. */
UCLASS()
class COMMONMOVER_API UCommonMovementMode
	: public UBaseMovementMode
	, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

public:
	UCommonMovementMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementMode Interface

	/** Generates the movement data that will be consumed by the simulation tick */
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

	/** Runs (or re-runs) the simulation and moves the updated component. */
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
	//~ End UBaseMovementMode Interface

	virtual void OnRegistered(const FName ModeName) override;
	virtual void OnUnregistered() override;

	//~ Begin IVisualLoggerDebugSnapshotInterface
#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(struct FVisualLogEntry* Snapshot) const override;
#endif
	//~ End IVisualLoggerDebugSnapshotInterface

protected:
	/** Prepares and validates all the data needed for the Simulation Tick and saves it into transient variables */
	virtual bool PrepareSimulationData(const FSimulationTickParams& Params);

	/** Builds the output sync states and saves them into transient variables */
	virtual void BuildSimulationOutputStates(FMoverTickEndData& OutputState);

	/** Checks if character movement has been disabled at the component level and cancels simulation if needed */
	virtual bool CheckIfMovementIsDisabled();

	/** Handles any additional prerequisite work that needs to be done before the simulation moves the updated component */
	virtual void PreMove(FMoverTickEndData& OutputState);

	/** Handles most of the actual movement, including collision recovery  */
	virtual void ApplyMovement(FMoverTickEndData& OutputState);

	/** Handles any additional behaviors after the updated component's final position and velocity have been computed */
	virtual void PostMove(FMoverTickEndData& OutputState);

	/** Attempts to teleport the updated component */
	virtual bool AttemptTeleport(const FVector& TeleportPos, const FRotator& TeleportRot, const FVector& PriorVelocity);

	/** Utility function to help keep track of the percentage of the time slice applied so far during move substages */
	float UpdateTimePercentAppliedSoFar(float PreviousTimePct, float LastCollisionTime) const;

	/** Calculates the target orientation Quat for the movement. Returns true if there is a change in orientation. */
	virtual bool CalculateOrientationChange(FQuat& TargetOrientQuat);

protected:
	/** Tag to add while this mode is active */
	UPROPERTY(Category="Tags", EditAnywhere, BlueprintReadWrite)
	FGameplayTag ModeTag;



	///////////////////////////////////////////////////////////////
	// Transient variables used by the simulation stages
	// @NOTE: These should be considered invalidated outside of OnSimulationTick()
	// and aren't meant to persist between simulation frames

	/** Mutable pointer to the Mover component */
	UPROPERTY()
	TObjectPtr<UCommonMoverComponent> MutableMoverComponent;

	/** Pointers to the updated components */
	FMovingComponentSet MovingComponentSet;

	/** Pointer to the legacy movement settings */
	UPROPERTY()
	TObjectPtr<const UCommonLegacyMovementSettings> CommonLegacySettings;

	/** Non-mutable pointers to the starting sync states */
	const FMoverDefaultSyncState* StartingSyncState;
	const FGameplayTagsSyncState* TagsSyncState;

	/** Mutable pointer to the blackboard */
	UPROPERTY()
	TObjectPtr<UMoverBlackboard> SimBlackboard;

	/** Non-mutable pointers to the input structs */
	const FCharacterDefaultInputs* KinematicInputs;

	/** Pointer to the proposed move for this simulation step */
	const FProposedMove* ProposedMove;

	/** Mutable pointers to the output sync states */
	FMoverDefaultSyncState* OutDefaultSyncState;
	FGameplayTagsSyncState* OutTagsSyncState;

	/** Utility velocity values */
	FVector StartingVelocity;

	/** Utility time values */
	float DeltaMs;
	float DeltaTime;
	float CurrentSimulationTime;
};
