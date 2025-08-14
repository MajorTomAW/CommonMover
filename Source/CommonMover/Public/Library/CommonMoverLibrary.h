// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CommonMoverLibrary.generated.h"

class UMoverBlackboard;
struct FSimulationTickParams;
struct FMoverSyncState;

#define MY_API COMMONMOVER_API

/** Blueprint library for exposing things to blueprint */
UCLASS(MinimalAPI)
class UCommonMoverLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//@TODO: Make a K2 Node for this to allow all possible data types from blueprint ?
	/** Returns a current blackboard value as a float, by its blackboard key. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|Blackboard")
	static MY_API float GetBlackboardValueAsFloat(const UMoverBlackboard* Blackboard, FName KeyName);
};

#undef MY_API
