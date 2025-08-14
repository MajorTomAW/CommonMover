// Author: Tom Werner (MajorT), 2025


#include "Library/CommonMoverLibrary.h"

#include "MoveLibrary/MoverBlackboard.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonMoverLibrary)

float UCommonMoverLibrary::GetBlackboardValueAsFloat(const UMoverBlackboard* Blackboard, FName KeyName)
{
	if (!IsValid(Blackboard))
	{
		return 0.f;
	}

	float Value = 0.f;
	Blackboard->TryGet<float>(KeyName, Value);
	return Value;
}
