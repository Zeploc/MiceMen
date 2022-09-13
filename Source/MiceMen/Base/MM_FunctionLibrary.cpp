// Copyright Alex Coultas, Mice Men Example Project


#include "Base/MM_FunctionLibrary.h"
#include "MM_Singleton.h"

UMM_Singleton* UMM_FunctionLibrary::MMSingleton = nullptr;

UMM_Singleton* UMM_FunctionLibrary::GetMMSingleton()
{
	if (!MMSingleton)
	{
		UMM_Singleton* FoundSingleton = Cast<UMM_Singleton>(GEngine->GameSingleton);

		if (!FoundSingleton)
			return nullptr;
		if (!FoundSingleton->IsValidLowLevel())
			return nullptr;

		MMSingleton = FoundSingleton;
	}

	return MMSingleton;
}
