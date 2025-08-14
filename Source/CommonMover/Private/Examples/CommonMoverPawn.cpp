// Copyright © 2024 MajorT. All Rights Reserved.


#include "Examples/CommonMoverPawn.h"

#include "CommonMoverComponent.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"


ACommonMoverPawn::ACommonMoverPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCapsule = CreateDefaultSubobject<UCapsuleComponent>("PlayerCapsule");
	SetRootComponent(PlayerCapsule);
	PlayerCapsule->InitCapsuleSize(34.0f, 88.0f);
	PlayerCapsule->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	PlayerCapsule->CanCharacterStepUpOn = ECB_No;
	PlayerCapsule->SetShouldUpdatePhysicsVolume(true);
	PlayerCapsule->SetCanEverAffectNavigation(false);
	PlayerCapsule->bDynamicObstacle = true;

	CommonMoverComponent = CreateDefaultSubobject<UCommonMoverComponent>("MoverComponent");

	SetReplicatingMovement(false);

#if WITH_EDITORONLY_DATA
	// Cooked builds may have some MoverComponent data missing, resulting in a non-functional actor.
	bOptimizeBPComponentData = false;
#endif
}

void ACommonMoverPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::MoveCompleted);
	}
}

void ACommonMoverPawn::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	float DeltaMs = static_cast<float>(SimTimeMs);
	FCharacterDefaultInputs& DefaultKinematicInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

	static const FCharacterDefaultInputs DoNothingInput;

	if (Controller == nullptr)
	{
		if (GetLocalRole() == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
		{
			DefaultKinematicInputs = DoNothingInput;
		}

		return;
	}

	if (GetMoverComponent()->IsMovementDisabled())
	{
		DefaultKinematicInputs = DoNothingInput;
	}

	DefaultKinematicInputs.ControlRotation = FRotator::ZeroRotator;

	// Copy control rotation
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DefaultKinematicInputs.ControlRotation = PC->GetControlRotation();
	}

	// force the forward input intent if autowalk is on
	FVector MoveInputIntent = CachedMoveInputIntent;
	

	// use only the control rotation yaw to avoid tapering our inputs if looking at the character from a too low or too high angle
	FRotator ControlFacing = FRotator::ZeroRotator;
	ControlFacing.Yaw = DefaultKinematicInputs.ControlRotation.Yaw;

	// set the move input
	DefaultKinematicInputs.SetMoveInput(EMoveInputType::DirectionalIntent, ControlFacing.RotateVector(MoveInputIntent));

	// check if we have a nonzero input
	static float RotationMagMin(1e-3);
	const bool bHasAffirmativeMoveInput = (DefaultKinematicInputs.GetMoveInput().Size() >= RotationMagMin);

	// Figure out the orientation intent for the character
	DefaultKinematicInputs.OrientationIntent = FVector::ZeroVector;
	if (bHasAffirmativeMoveInput)
	{
		// set the orientation intent to the movement direction
		DefaultKinematicInputs.OrientationIntent = DefaultKinematicInputs.GetMoveInput();

		// save the last nonzero input
		LastAffirmativeMoveInput = DefaultKinematicInputs.OrientationIntent;
	}
	else
	{
		// no input intent, so keep the last orientation from input
		DefaultKinematicInputs.OrientationIntent = LastAffirmativeMoveInput;
	}

	// cancel out any z intent to keep the actor vertical
	DefaultKinematicInputs.OrientationIntent = DefaultKinematicInputs.OrientationIntent.GetSafeNormal2D();
	
	// Convert inputs to be relative to the current movement base (depending on options and state)
	DefaultKinematicInputs.bUsingMovementBase = false;
	
	// get the Mover component
	if (const UMoverComponent* MoverComp = GetMoverComponent())
	{
		// do we have a base?
		if (UPrimitiveComponent* MovementBase = MoverComp->GetMovementBase())
		{
			FName MovementBaseBoneName = MoverComp->GetMovementBaseBoneName();

			FVector RelativeMoveInput, RelativeOrientDir;


			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, DefaultKinematicInputs.GetMoveInput(), RelativeMoveInput);
			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, DefaultKinematicInputs.OrientationIntent, RelativeOrientDir);

			DefaultKinematicInputs.SetMoveInput(DefaultKinematicInputs.GetMoveInputType(), RelativeMoveInput);
			DefaultKinematicInputs.OrientationIntent = RelativeOrientDir;

			DefaultKinematicInputs.bUsingMovementBase = true;
			DefaultKinematicInputs.MovementBase = MovementBase;
			DefaultKinematicInputs.MovementBaseBoneName = MovementBaseBoneName;
		}
	}
}

void ACommonMoverPawn::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// set up the input vector. We flip the axis so they correspond with the expected Mover input
	CachedMoveInputIntent.X = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
	CachedMoveInputIntent.Y = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
}

void ACommonMoverPawn::MoveCompleted(const FInputActionValue& Value)
{
	// zero out the cached input
	CachedMoveInputIntent = FVector::ZeroVector;
}
