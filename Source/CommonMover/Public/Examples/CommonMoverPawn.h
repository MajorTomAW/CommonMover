// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "MoverSimulationTypes.h"
#include "GameFramework/Pawn.h"
#include "CommonMoverPawn.generated.h"

class UCapsuleComponent;
class UCommonMoverComponent;

UCLASS(Abstract, NotPlaceable)
class COMMONMOVER_API ACommonMoverPawn
	: public APawn
	, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	ACommonMoverPawn();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//~ Begin IMoverInputProducerInterface
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;
	//~ End IMoverInputProducerInterface

	/** Returns the mover component. */
	UFUNCTION(BlueprintPure, Category=Mover)
	UCommonMoverComponent* GetMoverComponent() const { return CommonMoverComponent; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);

private:
	/** Mover component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess=true))
	TObjectPtr<UCommonMoverComponent> CommonMoverComponent;

	/** Player collision capsule */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mover Pawn", meta = (AllowPrivateAccess=true))
	TObjectPtr<UCapsuleComponent> PlayerCapsule;

	/* last nonzero movement intent input */
	FVector LastAffirmativeMoveInput = FVector::ZeroVector;

	/* cached move variables */
	FVector CachedMoveInputIntent = FVector::ZeroVector;
};
