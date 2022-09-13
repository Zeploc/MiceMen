// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Grid/MM_GridElement.h"
#include "MM_Mouse.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseMovementEndDelegate, class AMM_Mouse*, OwningMouse);

/**
 * The mouse actor
 */
UCLASS()
class MICEMEN_API AMM_Mouse : public AMM_GridElement
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMM_Mouse();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetupMouse(int _iTeam);

	/**
	 * Override for visual movement,
	 * call MouseMovementEndDelegate once complete, or game will halt.
	 *
	 * @param _Path - The movements to make
	 */
	UFUNCTION(BlueprintNativeEvent)
	void BN_StartMovement(const TArray<FVector>& _Path);

	void MouseComplete();



	int GetTeam() {
		return  iTeam;
	};
	bool isMouseComplete() {
		return  bMouseComplete;
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UFUNCTION(BlueprintImplementableEvent)
		void BI_MouseComplete();
public:	
	/** Executed when the movement has been made, to continue the next event */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FMouseMovementEndDelegate MouseMovementEndDelegate;

protected:
	UPROPERTY(BlueprintReadOnly)
		int iTeam = -1;
	UPROPERTY(BlueprintReadOnly)
		bool bMouseComplete = false;
};
