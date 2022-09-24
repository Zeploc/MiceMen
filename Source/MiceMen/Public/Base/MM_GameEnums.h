// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "MM_GameEnums.generated.h"


/** Gameplay mode to choose how the game plays, and which are AI or Players */
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

/** The team a mice is in that a player controls */
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

/** The difficulty for the AI determining how well they play */
UENUM(BlueprintType)
enum class EAIDifficulty : uint8
{
	E_NONE			UMETA(DisplayName = "None"),

	E_BASIC			UMETA(DisplayName = "Basic"),
	E_ADVANCED		UMETA(DisplayName = "Advanced"),

	E_MAX			UMETA(DisplayName = "Max"),
};