// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "MM_PlayerController.h"
#include "Grid/MM_GridManager.h"
#include "Grid/MM_GridTransform.h"
#include "MM_GameViewPawn.h"

AMM_GameMode::AMM_GameMode()
{	
	GridManagerClass = AMM_GridManager::StaticClass();
	PlayerControllerClass = AMM_PlayerController::StaticClass();
	DefaultPawnClass = AMM_GameViewPawn::StaticClass();
}

class AMM_GridManager* AMM_GameMode::GetGridManager()
{
	return GridManager;
}

void AMM_GameMode::BeginPlay()
{
	Super::BeginPlay();

	SetupGridManager();
}

bool AMM_GameMode::SetupGridManager()
{
	if (!GetWorld())
		return false;
	
	// Check valid manager class
	if (!GridManagerClass)
		GridManagerClass = AMM_GridManager::StaticClass();

	// Default Spawn grid at world zero
	FTransform SpawnTransform = FTransform::Identity;

	// Try find GridTransform for override transform
	AActor* FoundGridTransform = UGameplayStatics::GetActorOfClass(GetWorld(), AMM_GridTransform::StaticClass());
	if (FoundGridTransform)
		SpawnTransform = FoundGridTransform->GetActorTransform();

	// Spawn Grid manager
	GridManager = GetWorld()->SpawnActor<AMM_GridManager>(GridManagerClass, SpawnTransform);
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Grid Manager!"));
		return false;
	}

	return true;
}
