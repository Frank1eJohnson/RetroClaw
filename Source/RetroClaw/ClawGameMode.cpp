 // Fill out your copyright notice in the Description page of Project Settings.


#include "ClawGameMode.h"
#include "Kismet/GameplayStatics.h"


void AClawGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::SpawnSound2D(this, LevelSound, 1.0f, 1.0f, 0.0f);
}


void AClawGameMode::ActorDied(AActor* DeadActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Actor Died"));
}


void AClawGameMode::AddScore(int64 AdditionlaScore)
{
	PlayerScore += AdditionlaScore;
	UE_LOG(LogTemp, Warning, TEXT("score is: %i"), PlayerScore);
}


int64 AClawGameMode::GetScore()
{
	return PlayerScore;
}


void AClawGameMode::HandleGameOver(bool PlayerWon)
{
	// to be implemented in the future
}


void AClawGameMode::HandleGameStart()
{
	// to be implemented in the future
}
