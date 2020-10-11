// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "LevelObjective.generated.h"

class AClawGameMode;

UCLASS()
class RETROCLAW_API ALevelObjective : public APaperSpriteActor
{
	GENERATED_BODY()
	

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* HitCollisionBox;

	AClawGameMode* GameModeRef;

public:
	// class constructor
	ALevelObjective(); 

protected:
	// called when actor is spawned
	virtual void BeginPlay() override;

};
