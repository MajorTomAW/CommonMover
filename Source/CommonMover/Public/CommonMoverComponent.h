// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"

#include "CommonMoverComponent.generated.h"

/** Fired after the actor lands on a valid surface.
 * The first param is the name of the mode this actor is in after landing.
 * The second param is the hit result from hitting the floor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMoverEvent_OnLanded, const FName&, NextMovementModeName, const FHitResult&, HitResult);

/** Mover component extended with common functionality */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class COMMONMOVER_API UCommonMoverComponent
	: public UMoverComponent
	, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

public:
	UCommonMoverComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin IVisualLoggerDebugSnapshotInterface
#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif
	//~ End IVisualLoggerDebugSnapshotInterface


	//~ Begin UObject Interface
	virtual void BeginPlay() override;
	//~ End UObject Interface

	/** Applies forces to physical objects on impact */
	virtual void OnHandleImpact(const FMoverOnImpactParams& ImpactParams) override;

public:
	/** Override to handle Raft movement copy and work around simulation timing issues */
	bool TeleportImmediately(const FVector& Location, const FRotator& Orientation, const FVector& Velocity);

	/** Called from Movement Modes to notify of landed events */
	void OnLanded(const FName& NextMovementModeName, const FHitResult& HitResult);

	/** Sets up a non-immediate teleport */
	void WaitForTeleport();

	/** Teleports the owner to the given location and switches to falling mode */
	void TeleportAndFall(const FVector& TeleportLocation);

	void SetMovementDisabled(bool bState);

	/** Returns true if the owner is currently falling */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsFalling() const;

	/** Returns true if the owner is currently waiting for a non-immediate teleport */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsTeleporting() const { return bIsTeleporting; };

	/** Returns true if movement for the owner has been disabled */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsMovementDisabled() const { return bDisableMovement; };

	/** Returns true if currently crouching */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsCrouching() const;

	/** Returns true if currently flying (moving through a non-fluid volume without resting on the ground) */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsFlying() const;

	/** Is this actor in an airborne state? (e.g., Flying, Falling) */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsAirborne() const;

	/** Is this actor in a grounded state? (e.g., Walking) */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsOnGround() const;

	/** Is this actor in a Swimming state? (e.g., Swimming) */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsSwimming() const;

	/** Is this actor sliding on an unwalkable slope? */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool IsSlopeSliding() const;

	/** Can this Actor jump? */
	UFUNCTION(BlueprintPure, Category="Mover")
	virtual bool CanJump() const;

	/** Returns the Gameplay Tag Container from the Titan Tags Sync State */
	UFUNCTION(BlueprintPure, Category="Mover")
	FGameplayTagContainer GetTagsFromSyncState() const;

	/** Returns the last recorded ground contact normal, or a zero vector if not on the ground */
	UFUNCTION(BlueprintPure, Category="Mover")
	FVector GetGroundNormal() const;

protected:
	/** Broadcasted when this actor lands on a valid surface. */
	UPROPERTY(BlueprintAssignable, Category = Mover)
	FMoverEvent_OnLanded OnLandedDelegate;

	/** Multiplies the impulse applied to physics objects on collisions to push them harder */
	UPROPERTY(EditAnywhere, Category = Mover, meta=(ClampMin=0))
	float ImpactPhysicsForceMultiplier = 10.0f;

	/** Set to true while the owner waits for a long teleport to complete */
	bool bIsTeleporting = false;

	/** Set to true while movement has been disabled externally */
	bool bDisableMovement = false;
};
