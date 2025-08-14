// Author: Tom Werner (MajorT), 2025


#include "Library/CommonMovementCheckUtils.h"

#include "CommonMoverComponent.h"
#include "MoverSimulationTypes.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMovementCheckUtils)

bool UCommonMovementCheckUtils::IsFalling(const FSimulationTickParams& TickParams)
{
	const UCommonMoverComponent* CommonMover =
		Cast<UCommonMoverComponent>(TickParams.MovingComps.MoverComponent.Get());

	return (TickParams.StartState.SyncState.MovementMode == DefaultModeNames::Falling) ||
		(CommonMover->IsFalling());
}

bool UCommonMovementCheckUtils::IsWalking(const FSimulationTickParams& TickParams)
{
	const UCommonMoverComponent* CommonMover =
		Cast<UCommonMoverComponent>(TickParams.MovingComps.MoverComponent.Get());

	return (TickParams.StartState.SyncState.MovementMode == DefaultModeNames::Walking) ||
		(CommonMover->IsOnGround());
}

bool UCommonMovementCheckUtils::IsFlying(const FSimulationTickParams& TickParams)
{
	const UCommonMoverComponent* CommonMover =
		Cast<UCommonMoverComponent>(TickParams.MovingComps.MoverComponent.Get());

	return (TickParams.StartState.SyncState.MovementMode == DefaultModeNames::Flying) ||
		(CommonMover->IsFlying());
}

bool UCommonMovementCheckUtils::IsSwimming(const FSimulationTickParams& TickParams)
{
	const UCommonMoverComponent* CommonMover =
		Cast<UCommonMoverComponent>(TickParams.MovingComps.MoverComponent.Get());

	return (TickParams.StartState.SyncState.MovementMode == DefaultModeNames::Swimming) ||
		(CommonMover->IsSwimming());
}
