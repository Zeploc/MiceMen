// Copyright Alex Coultas, Mice Men Example Project

#include "Base/MM_FunctionLibrary.h"
#include "Base/MM_Singleton.h"

UMM_Singleton* UMM_FunctionLibrary::MMSingleton = nullptr;

UMM_Singleton* UMM_FunctionLibrary::GetMMSingleton()
{
	if (!MMSingleton)
	{
		// Gets the singleton from the engine
		UMM_Singleton* FoundSingleton = Cast<UMM_Singleton>(GEngine->GameSingleton);

		if (!FoundSingleton)
		{
			return nullptr;
		}
		if (!FoundSingleton->IsValidLowLevel())
		{
			return nullptr;
		}

		// Stores the singleton as a static variable so it only has to cast once
		MMSingleton = FoundSingleton;
	}

	return MMSingleton;
}
