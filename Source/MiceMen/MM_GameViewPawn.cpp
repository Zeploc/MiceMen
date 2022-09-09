// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameViewPawn.h"
#include "CineCameraComponent.h"

// Sets default values
AMM_GameViewPawn::AMM_GameViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GameCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("Game Camera"));
}

// Called when the game starts or when spawned
void AMM_GameViewPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMM_GameViewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMM_GameViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

