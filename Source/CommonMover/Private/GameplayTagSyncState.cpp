// Copyright © 2024 MajorT. All Rights Reserved.


#include "CommonMover/Public/GameplayTagSyncState.h"

FGameplayTagsSyncState::FGameplayTagsSyncState()
{
}

FMoverDataStructBase* FGameplayTagsSyncState::Clone() const
{
	FGameplayTagsSyncState* CopyPtr = new FGameplayTagsSyncState(*this);
	return CopyPtr;
}

bool FGameplayTagsSyncState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bool bSuccess = FMoverDataStructBase::NetSerialize(Ar, Map, bOutSuccess);
	MovementTags.NetSerialize(Ar, Map, bSuccess);
	return bSuccess;
}

UScriptStruct* FGameplayTagsSyncState::GetScriptStruct() const
{
	return FGameplayTagsSyncState::StaticStruct();
}

void FGameplayTagsSyncState::ToString(FAnsiStringBuilderBase& Out) const
{
	FMoverDataStructBase::ToString(Out);
	Out.Appendf("Tags[%s] \n", *MovementTags.ToStringSimple());
}

bool FGameplayTagsSyncState::ShouldReconcile(const FMoverDataStructBase& AuthorityState) const
{
	const FGameplayTagsSyncState* AuthoritySyncState = static_cast<const FGameplayTagsSyncState*>(&AuthorityState);

	// Reconcile if the tags don't match
	return MovementTags != AuthoritySyncState->GetMovementTags();
}

void FGameplayTagsSyncState::Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct)
{
	// Copy from authority
	const FGameplayTagsSyncState* AuthoritySyncState = static_cast<const FGameplayTagsSyncState*>(&From);
	MovementTags = AuthoritySyncState->GetMovementTags();
}

bool FGameplayTagsSyncState::HasTagExact(const FGameplayTag& Tag) const
{
	return MovementTags.HasTagExact(Tag);
}

bool FGameplayTagsSyncState::HasTagAny(const FGameplayTag& Tag) const
{
	return MovementTags.HasTag(Tag);
}

void FGameplayTagsSyncState::AddTag(const FGameplayTag& Tag)
{
	MovementTags.AddTag(Tag);
}

void FGameplayTagsSyncState::RemoveTag(const FGameplayTag& Tag)
{
	MovementTags.RemoveTag(Tag);
}

void FGameplayTagsSyncState::ClearTags()
{
	MovementTags.Reset();
}
