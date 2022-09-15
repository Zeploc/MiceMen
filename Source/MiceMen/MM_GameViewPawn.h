// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MM_GameViewPawn.generated.h"

class AMM_ColumnControl;
class AMM_GridManager;
class AMM_PlayerController;
class AMM_GameMode;

UCLASS()
class MICEMEN_API AMM_GameViewPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMM_GameViewPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCineCameraComponent* GameCamera;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void BeginTurn();

	void AddColumnAsGrabbable(int Column);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* _NewController);


	void BeginGrab();
	void EndGrab();

	void HandleGrab();

	void ColumnAdjusted(bool _TurnComplete);
	void TurnEnded();

	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();
	UFUNCTION(BlueprintPure)
		AMM_GameMode* GetGamemode();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float InteractTraceDistance = 100000.0f;

	/**
	 * Maximum times the same column can be moved by the player.
	 * Superseded if all mouse on that same column
	 * Stored in pawn to have the ability to change it per player
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int SameColumnMax = 6;

protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_ColumnControl* CurrentColumn;

	UPROPERTY(BlueprintReadOnly)
		AMM_PlayerController* MMPlayerController;


	UPROPERTY(BlueprintReadOnly)
		AMM_GameMode* MMGameMode;

	AMM_GridManager* GridManager;

	FVector HitColumnOffset;

	UPROPERTY(BlueprintReadOnly)
	bool bTurnActive = false;

	TArray<AMM_ColumnControl*> CurrentColumnControls;

	FDelegateHandle CurrentColumnDelegateHandle;

	int LastMovedColumn = -1;
	int SameMovedColumnCount = 0;
};
