// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonMovementCheckUtils.generated.h"

struct FMoverSyncState;
/**
 * 
 */
UCLASS()
class COMMONMOVER_API UCommonMovementCheckUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static bool IsFalling(const FMoverSyncState& SyncState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static bool IsWalking(const FMoverSyncState& SyncState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static bool IsFlying(const FMoverSyncState& SyncState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|State")
	static bool IsSwimming(const FMoverSyncState& SyncState);
};
