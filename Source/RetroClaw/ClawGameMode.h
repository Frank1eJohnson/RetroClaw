// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "GameFramework/GameModeBase.h"
#include "ClawGameHUD.h"	
#include "ClawGameMode.generated.h"

/**
 * 
 */
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreCountChanged, int32, ScoreCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthPaneChanged, int32, HealthPane);

UCLASS()
class RETROCLAW_API AClawGameMode : public AGameModeBase
{
	GENERATED_BODY()
	

private:
	int64 PlayerScore = 0;


public:
	void HandleGameOver(bool PlayerWon);
	void ActorDied(AActor* DeadActor);

	void AddScore(int64 AdditionlaScore);
	int64 GetScore();

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<class UUserWidget> ClawGameHUDClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Runtime")
	class UClawGameHUD* ClawGameHUD;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Delegates")
	FOnScoreCountChanged OnScoreCountChanged;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Delegates")
	FOnHealthPaneChanged OnHealthPaneChanged;
	

protected:

	virtual void BeginPlay() override;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* LevelSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> GameOverScreenClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> GameWinScreenClass;


};
