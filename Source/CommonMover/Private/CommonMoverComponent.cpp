// Copyright © 2024 MajorT. All Rights Reserved.


#include "CommonMover/Public/CommonMoverComponent.h"

#include "CommonMover/Public/GameplayTagSyncState.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "MoveLibrary/FloorQueryUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMoverComponent)

UCommonMoverComponent::UCommonMoverComponent()
{
}

#if ENABLE_VISUAL_LOG
void UCommonMoverComponent::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
}
#endif

void UCommonMoverComponent::BeginPlay()
{
	Super::BeginPlay();

#if ENABLE_VISUAL_LOG
	REDIRECT_TO_VLOG(GetOwner());
#endif
}

void UCommonMoverComponent::OnHandleImpact(const FMoverOnImpactParams& ImpactParams)
{
	// Get the hit component
	UPrimitiveComponent* HitComponent = ImpactParams.HitResult.GetComponent();

	if (IsValid(HitComponent) && HitComponent->IsSimulatingPhysics())
	{
		const FVector ImpactForce = ImpactParams.AttemptedMoveDelta * HitComponent->GetMass() * ImpactPhysicsForceMultiplier;
		HitComponent->AddImpulseAtLocation(ImpactForce, ImpactParams.HitResult.ImpactPoint);

		UE_LOG(LogMover, Warning, TEXT("Applying impact force of %s to %s"), *ImpactForce.ToString(), *HitComponent->GetName());
	}
}

bool UCommonMoverComponent::TeleportImmediately(
	const FVector& Location,
	const FRotator& Orientation,
	const FVector& Velocity)
{
	bool bSuccessfullyWrote = false;
	FMoverSyncState PendingSyncState;

	if (BackendLiaisonComp->ReadPendingSyncState(PendingSyncState))
	{
		if (FMoverDefaultSyncState* DefaultSync = PendingSyncState.SyncStateCollection.FindMutableDataByType<FMoverDefaultSyncState>())
		{
			// Move the actor and reflect this in the official simulation state
			UpdatedComponent->SetWorldLocationAndRotation(Location, Orientation);
			UpdatedComponent->ComponentVelocity = Velocity;
			DefaultSync->SetTransforms_WorldSpace(Location, Orientation, FVector::ZeroVector, nullptr);
			
			bSuccessfullyWrote = (BackendLiaisonComp->WritePendingSyncState(PendingSyncState));
			if (bSuccessfullyWrote)
			{
				FinalizeFrame(&PendingSyncState, &CachedLastAuxState);
			}
		}
	}

	return bSuccessfullyWrote;
}

void UCommonMoverComponent::OnLanded(const FName& NextMovementModeName, const FHitResult& HitResult)
{
	OnLandedDelegate.Broadcast(NextMovementModeName, HitResult);
}

void UCommonMoverComponent::WaitForTeleport()
{
	// Raise the teleport flag
	bIsTeleporting = true;
	
	// Set the teleport movement mode
	//@TODO:
}

void UCommonMoverComponent::TeleportAndFall(const FVector& TeleportLocation)
{
}

void UCommonMoverComponent::SetMovementDisabled(bool bState)
{
	bDisableMovement = true;
}

bool UCommonMoverComponent::IsFalling() const
{
	if (const UCommonLegacyMovementSettings* LegacySettings = FindSharedSettings<UCommonLegacyMovementSettings>())
	{
		return GetMovementModeName() != LegacySettings->AirMovementModeName;
	}

	return false;
}

FGameplayTagContainer UCommonMoverComponent::GetTagsFromSyncState() const
{
	// Get the tags sync state
	const FGameplayTagsSyncState* TagsSyncState = GetSyncState().SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();
	if (ensure(TagsSyncState))
	{
		return TagsSyncState->GetMovementTags();
	}

	return FGameplayTagContainer();
}

FVector UCommonMoverComponent::GetGroundNormal() const
{
	FFloorCheckResult CurrentFloor;
	const UMoverBlackboard* SimBB = GetSimBlackboard();

	if (IsValid(SimBB) && SimBB->TryGet(CommonBlackboard::LastFloorResult, CurrentFloor))
	{
		return CurrentFloor.HitResult.ImpactNormal;
	}

	return FVector::ZeroVector;
}
