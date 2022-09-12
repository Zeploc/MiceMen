// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MM_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MICEMEN_API AMM_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMM_PlayerController();

	virtual void OnPossess(APawn* _Pawn) override;

	void SetupPlayer(int _Team);

	void BeginTurn();

	void TurnEnded();

	UFUNCTION(BlueprintPure)
	int GetCurrentTeam() { return CurrentTeam; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

protected:
	UPROPERTY(BlueprintReadOnly)
		class AMM_GameViewPawn* MMPawn;

	UPROPERTY(BlueprintReadOnly)
		class AMM_GameMode* MMGameMode;

	UPROPERTY(BlueprintReadOnly)
		int CurrentTeam = -1;
};
