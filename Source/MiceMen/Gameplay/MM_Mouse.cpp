// Copyright Alex Coultas, Mice Men Example Project


#include "Gameplay/MM_Mouse.h"

// Sets default values
AMM_Mouse::AMM_Mouse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMM_Mouse::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMM_Mouse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMM_Mouse::SetupMouse(int _iTeam)
{
	iTeam = _iTeam;
}

void AMM_Mouse::MoveAlongPath(TArray<FVector> _Path)
{
	BI_MoveAlongPath(_Path);
}

