// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MM_GameViewPawn.generated.h"

UCLASS()
class MICEMEN_API AMM_GameViewPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMM_GameViewPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCineCameraComponent* GameCamera;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* _NewController);

	void BeginGrab();
	void EndGrab();

	void HandleGrab();

	UFUNCTION(BlueprintPure)
	class AMM_GridManager* GetGridManager();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float InteractTraceDistance = 100000.0f;


protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_ColumnControl* CurrentColumn;

	UPROPERTY(BlueprintReadOnly)
		class AMM_PlayerController* MMPlayerController;


	UPROPERTY(BlueprintReadOnly)
		class AMM_GameMode* MMGameMode;

	class AMM_GridManager* GridManager;

	FVector HitColumnOffset;

};
