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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* _NewController);


	void BeginGrab();
	void EndGrab();

	void HandleGrab();

	void TurnEnded();

	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();
	UFUNCTION(BlueprintPure)
		AMM_GameMode* GetGamemode();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float InteractTraceDistance = 100000.0f;


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

};
