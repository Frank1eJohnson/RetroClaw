// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class RETROCLAW_API AEnemyCharacter : public APaperCharacter
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay();

protected:
	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	// The animation to play while walking (patrolling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* WalkingAnimation;

	// The animation to play while swording (attacking with sword)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* SwordingAnimation;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation();

	void UpdateCharacter();

	void StartMovementTimer();
	void ChangeMovementDirection();
	void StartIdlenessTimer();

	void StartSwording();
	void StopSwording();

	bool isSwording = false;

	// movementDirection will be multiplied by world vector
	// 0 will result in no movement, 1 is right
	// movement and -1 is left movement.
	float movementDirection = 1.0f;

public:
	AEnemyCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* attackCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AActor* ClawCharacter;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
						
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};
