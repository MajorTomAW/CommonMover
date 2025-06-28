// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoverTypes.h"

#include "GameplayTagSyncState.generated.h"

/** Extends the mover sync state to provide gameplay tag tracking. */
USTRUCT(BlueprintType)
struct COMMONMOVER_API FGameplayTagsSyncState : public FMoverDataStructBase
{
	GENERATED_BODY()

public:
	FGameplayTagsSyncState();
	virtual ~FGameplayTagsSyncState() override = default;

	//~ Begin FMoverDataStructBase Interface
	virtual FMoverDataStructBase* Clone() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	//~ End FMoverDataStructBase Interface

public:
	/** Returns the movement tags container */
	const FGameplayTagContainer& GetMovementTags() const { return MovementTags; };

	/** Returns true if the sync state contains the exact leaf tag */
	bool HasTagExact(const FGameplayTag& Tag) const;

	/** Returns true if the sync state contains the tag as part of its hierarchy */
	bool HasTagAny(const FGameplayTag& Tag) const;

	/** Adds a tag to the sync state */
	void AddTag(const FGameplayTag& Tag);

	/** Removes a tag from the sync state */
	void RemoveTag(const FGameplayTag& Tag);

	/** Clears all tags from the sync state */
	void ClearTags();

protected:

	/** Tags container */
	FGameplayTagContainer MovementTags;
};

template<>
struct TStructOpsTypeTraits< FGameplayTagsSyncState > : public TStructOpsTypeTraitsBase2< FGameplayTagsSyncState >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};