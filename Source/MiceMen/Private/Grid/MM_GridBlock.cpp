// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridBlock.h"

// Sets default values
AMM_GridBlock::AMM_GridBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMM_GridBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMM_GridBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

