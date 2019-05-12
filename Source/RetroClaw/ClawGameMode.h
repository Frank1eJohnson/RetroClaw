// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ClawGameMode.generated.h"

/**
 * 
 */
UCLASS()
class RETROCLAW_API AClawGameMode : public AGameModeBase
{
	GENERATED_BODY()
	

private:

	void HandleGameStart();
	void HandleGameOver(bool PlayerWon);


public:

	void ActorDied(AActor* DeadActor);

protected:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void GameStart();
	UFUNCTION(BlueprintImplementableEvent)
	void GameOver(bool PlayerWon);
};
