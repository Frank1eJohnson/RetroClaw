 // Fill out your copyright notice in the Description page of Project Settings.


#include "ClawGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ClawGameHUD.h"

void AClawGameMode::BeginPlay()
{
	Super::BeginPlay();
	UGameplayStatics::SpawnSound2D(this, LevelSound, 1.0f, 1.0f, 0.0f);
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->bShowMouseCursor = true;
	ClawGameHUD = Cast<UClawGameHUD>(CreateWidget(GetWorld(), ClawGameHUDClass));
	check(ClawGameHUD);
	if (ClawGameHUD != nullptr)
	{
		ClawGameHUD->AddToViewport();
	}
    ClawGameHUD->InitializeHUD(this);
}


void AClawGameMode::ActorDied(AActor* DeadActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Actor Died"));
}


void AClawGameMode::AddScore(int64 AdditionlaScore)
{
	PlayerScore += AdditionlaScore;
	UE_LOG(LogTemp, Warning, TEXT("score is: %i"), PlayerScore);
    OnScoreCountChanged.Broadcast(PlayerScore);
}


int64 AClawGameMode::GetScore()
{
	return PlayerScore;
}


void AClawGameMode::HandleGameOver(bool PlayerWon)
{
    if (PlayerWon)
    {
        //Selecting Game Win Widget
        UUserWidget* GameWinScreen = CreateWidget(GetGameInstance(), GameWinScreenClass);
        //Displaying Game Win Screen
        if (GameWinScreen != nullptr)
        {
            GameWinScreen->AddToViewport();
        }
    }
    else
    {
        //Selecting Game Over Widget
        UUserWidget* GameOverScreen = CreateWidget(GetGameInstance(), GameOverScreenClass);
        //Displaying Game Over Screen
        if (GameOverScreen != nullptr)
        {
            GameOverScreen->AddToViewport();
        }
    }
} 
