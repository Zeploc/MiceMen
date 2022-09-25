// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "MM_GridEnums.generated.h"

/** Used for mice and columns to indicate movement directions */
UENUM(BlueprintType)
enum class EDirection : uint8
{
	E_NONE			UMETA(DisplayName = "None"),

	E_RIGHT			UMETA(DisplayName = "Right"),
	E_LEFT			UMETA(DisplayName = "Left"),
	E_UP			UMETA(DisplayName = "Up"),
	E_DOWN			UMETA(DisplayName = "Down"),

	E_MAX			UMETA(DisplayName = "Max"),
};