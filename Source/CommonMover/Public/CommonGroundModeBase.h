// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonMovementMode.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "CommonGroundModeBase.generated.h"

/** Base class for all ground movement modes.
 * Establishes a common simulation structure to handle slopes, stairs, and other obstacles.
 */
UCLASS(Abstract)
class COMMONMOVER_API UCommonGroundModeBase : public UCommonMovementMode
{
	GENERATED_BODY()

public:
	UCommonGroundModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UCommonMovementMOde
	virtual void ApplyMovement(FMoverTickEndData& OutputState) override;
	//~ End UCommonMovementMode

	/** Validates the floor prior to any movement */
	virtual void ValidateFloor(float FloorSweepDistance, float MaxWalkableSlopeCosine);

	/** Attempts to move the updated comp along any dynamically moving floor it is standing on */
	virtual bool ApplyDynamicFloorMovement(FMoverTickEndData& OutputState, FMovementRecord& MoveRecord);

	/** Applies the first free movement. Returns true if the updated component was successfully moved. */
	virtual bool ApplyFirstMove(FCommonMoveData& WalkData);

	/** Attempts to de-penetrate the updated component prior to its first move */
	virtual bool ApplyDepenetrationOnFirstMove(FCommonMoveData& WalkData);

	/** Calculates ramp deflection and moves the updated component up a ramp */
	virtual bool ApplyRampMove(FCommonMoveData& WalkData, float MaxWalkableSlopeCosine);

	/** Attempts to move the updated component over a climbable obstacle */
	virtual bool ApplyStepUpMove(FCommonMoveData& WalkData, FOptionalFloorCheckResult& StepUpFloorResult, float MaxWalkableSlopeCosine, float MaxStepHeight, float FloorSweepDistance);

	/** Attempts to slide the updated component along a wall or other blocking, unclimbable obstacle */
	virtual bool ApplySlideAlongWall(FCommonMoveData& WalkData, float MaxWalkableSlopeCosine, float MaxStepHeight);

	/** Attempts to adjust the character vertically so it contacts the floor */
	virtual bool ApplyFloorHeightAdjustment(FCommonMoveData& WalkData, float MaxWalkableSlopeCosine);

	/** Applies corrections to the updated component's position while not moving. */
	virtual bool ApplyIdleCorrections(FCommonMoveData& WalkData);

	/** Handles any movement mode transitions as a result of falling */
	virtual bool HandleFalling(FMoverTickEndData & OutputState, FMovementRecord & MoveRecord, FHitResult & Hit, float TimeAppliedSoFar);

	/** Captures the final movement state for the simulation frame and updates the output default sync state */
	void CaptureFinalState(const FFloorCheckResult& FloorResult, bool bDidAttemptMovement, const FMovementRecord& Record) const;

	/** Updates and returns the floor and base info data structures */
	FRelativeBaseInfo UpdateFloorAndBaseInfo(const FFloorCheckResult& FloorResult) const;

	/** Returns the name of the movement mode that will handle falling*/
	virtual const FName& GetFallingModeName() const;

protected:
	///////////////////////////////////////////////////////////////
	// Transient variables used by the simulation stages
	// @NOTE: These should be considered invalidated outside of OnSimulationTick()
	// and aren't meant to persist between simulation frames

	/** Floor info structs */
	FFloorCheckResult CurrentFloor;
	FRelativeBaseInfo OldRelativeBase;
};
