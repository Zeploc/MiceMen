// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Grid/MM_GridElement.h"
#include "MM_Mouse.generated.h"

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
	void MoveAlongPath(TArray<FVector> _Path);

	UFUNCTION(BlueprintImplementableEvent)
	void BI_MoveAlongPath(const TArray<FVector>& _Path);


	int GetTeam() {
		return  iTeam;
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

protected:
	UPROPERTY(BlueprintReadOnly)
		int iTeam = -1;
};
