// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "MM_Enums.generated.h"


/** Gameplay mode to choose how it players */
UENUM(BlueprintType)
enum class EGameType : uint8
{
	E_NONE		UMETA(DisplayName = "None"),

	/** Player versing Player, turn based one screen */
	E_PVP		UMETA(DisplayName = "Player VS Player"),
	/** Player versing AI, the AI will automatically take a their turn */
	E_PVAI		UMETA(DisplayName = "Player VS AI"),
	/**  For visualizing the game, AI's take turns making their moves until the game ends */
	E_AIVAI		UMETA(DisplayName = "AI VS AI"),
	/** No column or turn restrictions, for playing around, testing interactions and movement */
	E_SANDBOX	UMETA(DisplayName = "Sandbox"),
	/** Runs an instant test to look for any problems */
	E_TEST		UMETA(DisplayName = "Test"),


	E_MAX		UMETA(DisplayName = "MAX")
};

/** TODO */
UENUM(BlueprintType)
enum class ETeam : uint8
{
	E_NONE			UMETA(DisplayName = "None"),

	E_TEAM_A		UMETA(DisplayName = "Team A"),
	E_TEAM_B		UMETA(DisplayName = "Team B"),

	E_MAX			UMETA(DisplayName = "Max"),
};

/** Increments team, used for iterating through teams */
FORCEINLINE ETeam& operator++(ETeam& Team)
{
	if (Team == ETeam::E_MAX)
	{
		return Team;
	}

	// Get integer type
	using IntType = typename std::underlying_type<ETeam>::type;

	// Convert enum to type
	int EnumInt = static_cast<IntType>(Team);

	// Increment
	EnumInt++;

	// Convert back to enum
	Team = static_cast<ETeam>(EnumInt);

	return Team;
}

FORCEINLINE ETeam operator++(ETeam& Team, int)
{
	ETeam Result = Team;
	++Team;
	return Result;
}

/** TODO */
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