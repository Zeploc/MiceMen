// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MM_ColumnControl.generated.h"

UCLASS()
class MICEMEN_API AMM_ColumnControl : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMM_ColumnControl();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UBoxComponent* GrabbableBox;

	void SetupColumn(int _ColumnID, class AMM_GridManager* _GridManager);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	int ControllingColumn = -1;
};
