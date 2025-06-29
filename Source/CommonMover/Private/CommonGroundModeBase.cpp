// Copyright © 2024 MajorT. All Rights Reserved.


#include "CommonGroundModeBase.h"

#include "CommonBlackboard.h"
#include "CommonMoverComponent.h"

#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "MoveLibrary/GroundMovementUtils.h"
#include "MoveLibrary/MovementUtils.h"

UCommonGroundModeBase::UCommonGroundModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCommonGroundModeBase::ApplyMovement(FMoverTickEndData& OutputState)
{
	// Ensure we have cached floor information before moving
	ValidateFloor();

	// Initialize the move data
	FCommonMoveData WalkData;

	// Initialize the move record
	WalkData.MoveRecord.SetDeltaSeconds(DeltaTime);

	// Apply any movement from a dynamic base
	bool bDidMoveAlongWithBase = ApplyDynamicFloorMovement(OutputState, WalkData.MoveRecord);

	// After handling the dynamic base, check for disabled movement
	if (MutableMoverComponent->IsMovementDisabled())
	{
		CaptureFinalState(CurrentFloor, WalkData.MoveRecord);
		return;
	}

	// Calculate the target orientation for the following moves
	bool bIsOrientationChanging = CalculateOrientationChange(WalkData.TargetOrientQuat);

	// Calculate the move delta
	WalkData.OriginalMoveDelta = ProposedMove->LinearVelocity * DeltaTime;
	WalkData.CurrentMoveDelta = WalkData.OriginalMoveDelta;

	// Floor check result passed to step-up suboperations, so we can use their final floor results if they did a test
	FOptionalFloorCheckResult StepUpFloorResult;

	// Are we moving or re-orienting?
	if (!WalkData.CurrentMoveDelta.IsNearlyZero() ||bIsOrientationChanging)
	{
		// Apply the first move.
		// This will catch any potential collisions or initial penetration
		bool bMovedFreely = ApplyFirstMove(WalkData);

		// Apply any depenetration in case we started in the frame stuck.
		// This will include any catch-up from the first move
		bool bDepenetration = ApplyDepenetrationOnFirstMove(WalkData);

		if (!bDepenetration)
		{
			// If no depenetration was done, we can check for a ramp
			bool bMovedUpRamp = ApplyRampMove(WalkData);

			// Attempt to move up any climbable obstacles
			bool bSteppedUp = ApplyStepUpMove(WalkData, StepUpFloorResult);

			// Did we fail to step up?
			bool bSlidAlongWall = false;
			if (bSteppedUp)
			{
				// Attempt to slide along an unclimbable obstacle
				bSlidAlongWall = ApplySlideAlongWall(WalkData);
			}

			// Search for the floor we've ended up on
			UFloorQueryUtils::FindFloor(
				MovingComponentSet,
				CommonLegacySettings->FloorSweepDistance,
				CommonLegacySettings->MaxWalkSlopeCosine,
				MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
				CurrentFloor);

			// Adjust vertically so we remain in contact with the floor
			bool bAdjustedToFloor = ApplyFloorHeightAdjustment(WalkData);

			// Check if we're falling
			if (HandleFalling(OutputState, WalkData.MoveRecord, CurrentFloor.HitResult, DeltaMs * WalkData.PercentTimeAppliedSoFar))
			{
				// Handle falling captured our output state, so we can return
				return;
			}
		}
	}
	else
	{
		// We don't need to move this frame, but we may still need to adjust to the floor
		// Search for the floor we're standing on
		UFloorQueryUtils::FindFloor(
			MovingComponentSet,
			CommonLegacySettings->FloorSweepDistance,
			CommonLegacySettings->MaxWalkSlopeCosine,
			MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
			CurrentFloor);

		// Copy the current floor hit result
		WalkData.MoveHitResult = CurrentFloor.HitResult;

		// Check if we need to adjust to depenetrate from the floor
		bool bAdjustedToFloor = ApplyIdleCorrections(WalkData);

		// Check if we're falling
		if (HandleFalling(OutputState, WalkData.MoveRecord, CurrentFloor.HitResult, DeltaMs * WalkData.PercentTimeAppliedSoFar))
		{
			// Handle falling captured our output state, so we can return
			return;
		}
	}

	// Capture the final movement state
	CaptureFinalState(CurrentFloor, WalkData.MoveRecord);
}

void UCommonGroundModeBase::ValidateFloor()
{
	// Check if we have cached floor data
	if (!SimBlackboard->TryGet(CommonBlackboard::LastFloorResult, CurrentFloor))
	{
		// Search for the floor data again
		UFloorQueryUtils::FindFloor(
			MovingComponentSet,
			CommonLegacySettings->FloorSweepDistance,
			CommonLegacySettings->MaxWalkSlopeCosine,
			MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
			CurrentFloor);
	}

	// Check if we have a cached relative base
	if (!SimBlackboard->TryGet(CommonBlackboard::LastFoundDynamicMovementBase, OldRelativeBase))
	{
		// Update the floor and base info
		OldRelativeBase = UpdateFloorAndBaseInfo(CurrentFloor);
	}
}

bool UCommonGroundModeBase::ApplyDynamicFloorMovement(FMoverTickEndData& OutputState, FMovementRecord& MoveRecord)
{
	// If we're on a dynamic movement base, attempt to move along with whatever motion is has performed since we last ticked
	/*if (OldRelativeBase.UsesSameBase(StartingSyncState->GetMovementBase(), StartingSyncState->GetMovementBaseBoneName()))
	{
		return UBasedMovementUtils::TryMoveToStayWithBase(UpdatedComponent, UpdatedPrimitive, OldRelativeBase, MoveRecord, false);
	}*/

	return false;
}

bool UCommonGroundModeBase::ApplyFirstMove(FCommonMoveData& WalkData)
{
	// Attempt to move the full amount first
	bool bMoved = UMovementUtils::TrySafeMoveUpdatedComponent(
		MovingComponentSet,
		WalkData.CurrentMoveDelta,
		WalkData.TargetOrientQuat,
		true,
		WalkData.MoveHitResult,
		ETeleportType::None,
		WalkData.MoveRecord);

	// Update the time percentage applied
	WalkData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(WalkData.PercentTimeAppliedSoFar, WalkData.MoveHitResult.Time);

#if ENABLE_VISUAL_LOG
	//@TODO: VLOG
#endif
	
	return bMoved;
}

bool UCommonGroundModeBase::ApplyDepenetrationOnFirstMove(FCommonMoveData& WalkData)
{
	// Were we immediately blocked?
	if (WalkData.MoveHitResult.bStartPenetrating)
	{
		// TODO: handle de-penetration, try moving again, etc.

		return true;
	}

	return false;
}

bool UCommonGroundModeBase::ApplyRampMove(FCommonMoveData& WalkData)
{
	// Have we hit something that we suspect is a ramp?
	if (WalkData.MoveHitResult.IsValidBlockingHit())
	{
		// Check if the hit normal is a ramp
		if ( (WalkData.MoveHitResult.Time > 0.0f)
			&& (WalkData.MoveHitResult.Normal.Z > UE_KINDA_SMALL_NUMBER)
			&& (UFloorQueryUtils::IsHitSurfaceWalkable(WalkData.MoveHitResult, FVector::UpVector, CommonLegacySettings->MaxWalkSlopeCosine)))
		{
			// Compute the deflected move onto the ramp and update the move delta
			// We apply only the time remaining. (1-time applied)
			WalkData.CurrentMoveDelta = UGroundMovementUtils::ComputeDeflectedMoveOntoRamp(
				WalkData.CurrentMoveDelta * (1.0f - WalkData.PercentTimeAppliedSoFar),
				FVector::UpVector,
				WalkData.MoveHitResult,
				CommonLegacySettings->MaxWalkSlopeCosine,
				CurrentFloor.bLineTrace);

			// Move again onto the ramp
			UMovementUtils::TrySafeMoveUpdatedComponent(
				MovingComponentSet,
				WalkData.CurrentMoveDelta,
				WalkData.TargetOrientQuat,
				true,
				WalkData.MoveHitResult,
				ETeleportType::None,
				WalkData.MoveRecord);

			// Update the time percentage applied
			WalkData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(WalkData.PercentTimeAppliedSoFar, WalkData.MoveHitResult.Time);

#if ENABLE_VISUAL_LOG
			//@TODO: VLOG
#endif

			return true;
		}
	}

	// No blocking hit, or no walkable ramp
	return false;
}

bool UCommonGroundModeBase::ApplyStepUpMove(FCommonMoveData& WalkData, FOptionalFloorCheckResult& StepUpFloorResult)
{
	// Are we hitting something?
	if (WalkData.MoveHitResult.IsValidBlockingHit())
	{
		// Is this a surface we can step up on?
		if (UGroundMovementUtils::CanStepUpOnHitSurface(WalkData.MoveHitResult))
		{
			// Hit a barrier or unwalkable surface, try to step up and onto it
			const FVector PreStepUpLocation = MovingComponentSet.UpdatedComponent->GetComponentLocation();
			const FVector DownwardDir = -MutableMoverComponent->GetUpDirection();

			if (!UGroundMovementUtils::TryMoveToStepUp(MovingComponentSet, DownwardDir, CommonLegacySettings->MaxStepHeight, CommonLegacySettings->MaxWalkSlopeCosine, CommonLegacySettings->FloorSweepDistance, WalkData.OriginalMoveDelta * (1.f - WalkData.PercentTimeAppliedSoFar), WalkData.MoveHitResult, CurrentFloor, false, &StepUpFloorResult, WalkData.MoveRecord))
			{
				// Update the time percentage
				// WalkData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(WalkData.PercentTimeAppliedSoFar, WalkData.MoveHitResult.Time);

#if ENABLE_VISUAL_LOG
				//@TODO: VLOG
#endif

				return true;
			}
		}
		else if (WalkData.MoveHitResult.Component.IsValid()
			&& !WalkData.MoveHitResult.Component.Get()->CanCharacterStepUp(Cast<APawn>(WalkData.MoveHitResult.GetActor())))
		{
			return true;
		}
	}

	// either not a blocking obstacle or not a climbable obstacle
	return false;
}

bool UCommonGroundModeBase::ApplySlideAlongWall(FCommonMoveData& WalkData)
{
	// Are we hitting something?
	if (WalkData.MoveHitResult.IsValidBlockingHit())
	{
		// Tell the mover component to handle the impact
		FMoverOnImpactParams ImpactParams(DefaultModeNames::Walking, WalkData.MoveHitResult, WalkData.OriginalMoveDelta);
		MutableMoverComponent->HandleImpact(ImpactParams);

		// Slide along the wall
		const float SlidePct = 1.0f - WalkData.PercentTimeAppliedSoFar;

		float SlideAmount = UGroundMovementUtils::TryWalkToSlideAlongSurface(
			MovingComponentSet,
			WalkData.OriginalMoveDelta,
			SlidePct,
			WalkData.TargetOrientQuat,
			WalkData.MoveHitResult.Normal,
			WalkData.MoveHitResult,
			true,
			WalkData.MoveRecord,
			CommonLegacySettings->MaxWalkSlopeCosine,
			CommonLegacySettings->MaxStepHeight);

		// Update the time percentage
		WalkData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(WalkData.PercentTimeAppliedSoFar, SlideAmount);

#if ENABLE_VISUAL_LOG
		//@TODO: VLOG
#endif

		return true;
	}

	// Not a blocking obstacle, can't slide along
	return false;
}

bool UCommonGroundModeBase::ApplyFloorHeightAdjustment(FCommonMoveData& WalkData)
{
	// Ensure we're standing on a walkable floor
	if (CurrentFloor.IsWalkableFloor())
	{

#if ENABLE_VISUAL_LOG
		const FVector ArrowStart = MovingComponentSet.UpdatedPrimitive->GetComponentLocation();
#endif

		// Adjust our height to match the floor
		UGroundMovementUtils::TryMoveToAdjustHeightAboveFloor(
			MovingComponentSet,
			CurrentFloor,
			CommonLegacySettings->MaxWalkSlopeCosine,
			WalkData.MoveRecord);

#if ENABLE_VISUAL_LOG
		//@TODO: VLOG
#endif

		return true;
	}

	// Couldn't adjust, the floor is not walkable
	return false;
}

bool UCommonGroundModeBase::ApplyIdleCorrections(FCommonMoveData& WalkData)
{
	if (WalkData.MoveHitResult.bStartPenetrating)
	{
		// The floor check failed because it started in penetration
		// We don't want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
		WalkData.MoveHitResult.TraceEnd = WalkData.MoveHitResult.TraceStart + FVector(0.f, 0.f, 2.4f);

		// Compute the depenetration adjustment vector
		const FVector RequestedAdjustment = UMovementUtils::ComputePenetrationAdjustment(WalkData.MoveHitResult);

		// Set up the component flags for the depenetration test
		constexpr EMoveComponentFlags IncludeBlockingOverlapsWithoutEvents = (MOVECOMP_NeverIgnoreBlockingOverlaps | MOVECOMP_DisableBlockingOverlapDispatch);
		EMoveComponentFlags MoveComponentFlags = MOVECOMP_NoFlags;
		MoveComponentFlags = (MoveComponentFlags | IncludeBlockingOverlapsWithoutEvents);

		// Move the component to resolve the penetration
		UMovementUtils::TryMoveToResolvePenetration(MovingComponentSet, MoveComponentFlags, RequestedAdjustment, WalkData.MoveHitResult, MovingComponentSet.UpdatedComponent->GetComponentQuat(), WalkData.MoveRecord);

#if ENABLE_VISUAL_LOG
		const FVector ArrowEnd = WalkData.MoveHitResult.bBlockingHit ? WalkData.MoveHitResult.Location : WalkData.MoveHitResult.TraceEnd;
		const FColor ArrowColor = WalkData.MoveHitResult.bBlockingHit ? FColor::Red : FColor::Green;

		//@TODO: VLOG
#endif

		return true;
	}

	// No initial penetration
	return false;
}

bool UCommonGroundModeBase::HandleFalling(
	FMoverTickEndData& OutputState,
	FMovementRecord& MoveRecord,
	FHitResult& Hit,
	float TimeAppliedSoFar)
{
	if (!CurrentFloor.IsWalkableFloor() && !Hit.bStartPenetrating)
	{
		// No floor or not walkable, so let us let the airborne movement mode deal with it
		OutputState.MovementEndState.NextModeName = GetFallingModeName();

		// Set the remaining time
		OutputState.MovementEndState.RemainingMs = DeltaMs - TimeAppliedSoFar;

		// Update the move record's delta seconds
		MoveRecord.SetDeltaSeconds((DeltaMs - OutputState.MovementEndState.RemainingMs) * 0.001f);

		// Capture the final movement state
		CaptureFinalState(CurrentFloor, MoveRecord);

		// Update the last fall time on the blackboard
		SimBlackboard->Set(CommonBlackboard::LastFallTime, CurrentSimulationTime);

#if ENABLE_VISUAL_LOG
		//@TODO: VLOG
#endif

		return true;
	}

	return false;
}

void UCommonGroundModeBase::CaptureFinalState(const FFloorCheckResult& FloorResult, const FMovementRecord& Record) const
{
	FRelativeBaseInfo BaseInfo = UpdateFloorAndBaseInfo(FloorResult);

	// TODO: Update Main/large movement record with substeps from our local record

	if (BaseInfo.HasRelativeInfo())
	{
		OutDefaultSyncState->SetTransforms_WorldSpace(
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			Record.GetRelevantVelocity(),
			BaseInfo.MovementBase.Get(),
			BaseInfo.BoneName);
	}
	else
	{
		OutDefaultSyncState->SetTransforms_WorldSpace(
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			Record.GetRelevantVelocity(),
			nullptr);
	}

	MovingComponentSet.UpdatedComponent->ComponentVelocity = OutDefaultSyncState->GetVelocity_WorldSpace();
}

FRelativeBaseInfo UCommonGroundModeBase::UpdateFloorAndBaseInfo(const FFloorCheckResult& FloorResult) const
{
	FRelativeBaseInfo ReturnBaseInfo;

	SimBlackboard->Set(CommonBlackboard::LastFloorResult, FloorResult);

	if (FloorResult.IsWalkableFloor() && UBasedMovementUtils::IsADynamicBase(FloorResult.HitResult.GetComponent()))
	{
		ReturnBaseInfo.SetFromFloorResult(FloorResult);

		SimBlackboard->Set(CommonBlackboard::LastFoundDynamicMovementBase, ReturnBaseInfo);
	}
	else
	{

		SimBlackboard->Invalidate(CommonBlackboard::LastFoundDynamicMovementBase);
	}

	return ReturnBaseInfo;
}

const FName& UCommonGroundModeBase::GetFallingModeName() const
{
	return CommonLegacySettings->AirMovementModeName;
}
