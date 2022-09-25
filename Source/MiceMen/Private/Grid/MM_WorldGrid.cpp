// Copyright Alex Coultas, Mice Men Example Project

#include "Grid/MM_WorldGrid.h"

// Sets default values
AMM_WorldGrid::AMM_WorldGrid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMM_WorldGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMM_WorldGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
