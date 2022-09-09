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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
