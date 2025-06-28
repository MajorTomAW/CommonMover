// Copyright © 2024 MajorT. All Rights Reserved.


#include "CommonMover/Public/CommonMovementMode.h"

#include "CommonMover/Public/CommonMoverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMovementMode)

UCommonMovementMode::UCommonMovementMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, StartingVelocity(ForceInit)
	, DeltaMs(0.0f)
	, DeltaTime(0.0f)
	, CurrentSimulationTime(0.0f)
{
	StartingSyncState = nullptr;
	TagsSyncState = nullptr;
	SimBlackboard = nullptr;
	KinematicInputs = nullptr;
	ProposedMove = nullptr;

	OutDefaultSyncState = nullptr;
	OutTagsSyncState = nullptr;

	SharedSettingsClasses.Add(UCommonLegacyMovementSettings::StaticClass());
}

void UCommonMovementMode::GenerateMove_Implementation(
	const FMoverTickStartData& StartState,
	const FMoverTimeStep& TimeStep,
	FProposedMove& OutProposedMove) const
{
	// Stub
}

void UCommonMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	// Prepare the simulation data
	if (!PrepareSimulationData(Params))
	{
		UE_LOG(LogMover, Error, TEXT("Couldn't prepare move simulation data for [%s]"), *GetNameSafe(this));
		return;
	}

	// Build simulation output states
	BuildSimulationOutputStates(OutputState);

	// Check if movement is allowed
	if (CheckIfMovementIsDisabled())
	{
		// Update the output sync state
		OutDefaultSyncState->SetTransforms_WorldSpace(
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			FVector::ZeroVector,
			nullptr);

		// Update the component velocity
		MovingComponentSet.UpdatedComponent->ComponentVelocity = FVector::ZeroVector;

		// Give back all the time to the next state
		OutputState.MovementEndState.RemainingMs = 0.0f;
		return;
	}

	// Handle anything else that needs to happen before we start moving
	PreMove(OutputState);

	// Move the updated component
	ApplyMovement(OutputState);

	// Handle anything else after the final location and velocity has been computed
	PostMove(OutputState);
	
#if ENABLE_VISUAL_LOG
	{
		const FVector LogLoc = MovingComponentSet.UpdatedComponent->GetComponentLocation();
		const FRotator LogRot = MovingComponentSet.UpdatedComponent->GetComponentRotation();
		const FVector LogVel = MovingComponentSet.UpdatedComponent->GetComponentVelocity();
		const float LogSpeed = LogVel.Size();

		//UE_VLOG(this, LogMOver, Log, TEXT("Final State:\nCurrent:[%s]\nNext[%s]\nLoc[%s]\nRot[%s]\nVel[%s]\nSpd[%f]"), *Params.StartState.SyncState.MovementMode.ToString(), *OutputState.MovementEndState.NextModeName.ToString(), *LogLoc.ToCompactString(), *LogRot.ToCompactString(), *LogVel.ToCompactString(), LogSpeed);
	}
#endif
}

bool UCommonMovementMode::PrepareSimulationData(const FSimulationTickParams& Params)
{
	// Get the mover component
	MutableMoverComponent = Cast<UCommonMoverComponent>(GetMoverComponent());

	if (!IsValid(MutableMoverComponent.Get()))
	{
		UE_LOG(LogMover, Error, TEXT("[%hs]: Couldn't get the mover component"), __FUNCTION__);
		return false;
	}

	// Get the updated component set
	MovingComponentSet = Params.MovingComps;

	// If the updated component is not valid, cancel the simulation
	if (!MovingComponentSet.UpdatedComponent.IsValid() || !MovingComponentSet.UpdatedPrimitive.IsValid())
	{
		UE_LOG(LogMover, Error, TEXT("[%hs]: Updated component is not valid"), __FUNCTION__);
		return false;
	}

	// Get the sync states
	StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	TagsSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();

	// Get the input structs
	KinematicInputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// Get the proposed move
	ProposedMove = &Params.ProposedMove;

	// Get the blackboard
	if (const UMoverComponent* OwnerComp = GetMoverComponent())
	{
		SimBlackboard = OwnerComp->GetSimBlackboard_Mutable();
	}

	// Get the velocity
	StartingVelocity = StartingSyncState->GetVelocity_WorldSpace();

	// Get the time deltas
	DeltaMs = Params.TimeStep.StepMs;
	DeltaTime = Params.TimeStep.StepMs * 0.001f;
	CurrentSimulationTime = Params.TimeStep.BaseSimTimeMs;
	
	return true;
}

void UCommonMovementMode::BuildSimulationOutputStates(FMoverTickEndData& OutputState)
{
	OutDefaultSyncState = &OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	
	OutTagsSyncState = &OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FGameplayTagsSyncState>();
	OutTagsSyncState->ClearTags();
}

void UCommonMovementMode::OnRegistered(const FName ModeName)
{
	Super::OnRegistered(ModeName);

	// Get the common legacy settings
	CommonLegacySettings = GetMoverComponent()->FindSharedSettings<UCommonLegacyMovementSettings>();
	ensureMsgf(CommonLegacySettings, TEXT("Failed to find instance of CommonLegacyMovementSettings on %s. Movement may not function properly."),
		*GetPathNameSafe(this));

#if ENABLE_VISUAL_LOG
	REDIRECT_TO_VLOG(GetMoverComponent()->GetOwner());
#endif
}

void UCommonMovementMode::OnUnregistered()
{
	// Release the shared settings pointers
	CommonLegacySettings = nullptr;
	
	Super::OnUnregistered();
}

bool UCommonMovementMode::CheckIfMovementIsDisabled()
{
	return MutableMoverComponent->IsMovementDisabled();
}

void UCommonMovementMode::PreMove(FMoverTickEndData& OutputState)
{
	// Stub
}

void UCommonMovementMode::ApplyMovement(FMoverTickEndData& OutputState)
{
	// Stub
}

void UCommonMovementMode::PostMove(FMoverTickEndData& OutputState)
{
	// Add the movement mode tag
	OutTagsSyncState->AddTag(ModeTag);
}

bool UCommonMovementMode::AttemptTeleport(
	const FVector& TeleportPos,
	const FRotator& TeleportRot,
	const FVector& PriorVelocity)
{
	if (MovingComponentSet.UpdatedComponent->GetOwner()->TeleportTo(TeleportPos, TeleportRot))
	{
		OutDefaultSyncState->SetTransforms_WorldSpace(
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			PriorVelocity,
			nullptr);

		MovingComponentSet.UpdatedComponent->ComponentVelocity = PriorVelocity;
		return true;
	}

	return false;
}

float UCommonMovementMode::UpdateTimePercentAppliedSoFar(
	float PreviousTimePct,
	float LastCollisionTime) const
{
	return PreviousTimePct + ( (1.0f - PreviousTimePct) * LastCollisionTime );
}

bool UCommonMovementMode::CalculateOrientationChange(FQuat& TargetOrientQuat)
{
	// Set the move direction intent on the output sync state
	OutDefaultSyncState->MoveDirectionIntent = ProposedMove->bHasDirIntent ? ProposedMove->DirectionIntent : FVector::ZeroVector;

	// Get the start orientation
	const FRotator StartingOrient = StartingSyncState->GetOrientation_WorldSpace();
	FRotator TargetOrient = StartingOrient;

	// Apply orientation changes, if any
	if (!ProposedMove->AngularVelocity.IsZero())
	{
		TargetOrient += (ProposedMove->AngularVelocity * DeltaTime);
	}

	// Return the quat and whether orientation changed
	TargetOrientQuat = TargetOrient.Quaternion();
	return TargetOrient != StartingOrient;
}

#if ENABLE_VISUAL_LOG
void UCommonMovementMode::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
}
#endif