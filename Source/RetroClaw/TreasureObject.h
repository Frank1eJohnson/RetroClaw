// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperFlipbookActor.h"
#include "Sound/SoundBase.h"
#include "TreasureObject.generated.h"


class AClawGameMode;

UCLASS()
class RETROCLAW_API ATreasureObject : public APaperFlipbookActor
{
	GENERATED_BODY()
	
	virtual void BeginPlay();

public:
	ATreasureObject();

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* ScoreCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int64 TreasureObjectScore;

	AClawGameMode* GameModeRef;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* CollectedSound;
};
