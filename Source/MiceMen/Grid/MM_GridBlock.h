// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "MM_GridElement.h"
#include "MM_GridBlock.generated.h"

/**
 * A blocked grid item where nothing else can go through
 */
UCLASS()
class MICEMEN_API AMM_GridBlock : public AMM_GridElement
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMM_GridBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
