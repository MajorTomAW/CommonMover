// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonMovementCheckUtils.generated.h"

struct FSimulationTickParams;
struct FMoverSyncState;

#define MY_API COMMONMOVER_API

/** */
UCLASS(MinimalAPI)
class UCommonMovementCheckUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static MY_API bool IsFalling(const FSimulationTickParams& TickParams);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static MY_API bool IsWalking(const FSimulationTickParams& TickParams);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static MY_API bool IsFlying(const FSimulationTickParams& TickParams);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static MY_API bool IsSwimming(const FSimulationTickParams& TickParams);
};

#undef MY_API
