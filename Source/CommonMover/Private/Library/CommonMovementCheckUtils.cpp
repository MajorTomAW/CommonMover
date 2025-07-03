// Copyright © 2025 Playton. All Rights Reserved.


#include "Library/CommonMovementCheckUtils.h"

#include "MoverSimulationTypes.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMovementCheckUtils)

bool UCommonMovementCheckUtils::IsFalling(const FMoverSyncState& SyncState)
{
	return SyncState.MovementMode == DefaultModeNames::Falling;
}

bool UCommonMovementCheckUtils::IsWalking(const FMoverSyncState& SyncState)
{
	return SyncState.MovementMode == DefaultModeNames::Walking;
}

bool UCommonMovementCheckUtils::IsFlying(const FMoverSyncState& SyncState)
{
	return SyncState.MovementMode == DefaultModeNames::Flying;
}

bool UCommonMovementCheckUtils::IsSwimming(const FMoverSyncState& SyncState)
{
	return SyncState.MovementMode == DefaultModeNames::Swimming;
}
