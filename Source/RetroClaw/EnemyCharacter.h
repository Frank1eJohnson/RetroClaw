// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "HealthComponent.h"
#include "Sound/SoundBase.h"
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

	// The animation to play after death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* DeadAnimation;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation(); 

	void StartMovement();
	void ChangeMovementDirection();

	void StartSwording();
	void DealDamage();
	void StopSwording(); 

	bool isSwording = false;
	bool isDead = false;

	// movementDirection will be multiplied by world vector
	// 0 will result in no movement, 1 is right
	// movement and -1 is left movement.
	float movementDirection = 1.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* ClawCelebrationSound;

public:
	AEnemyCharacter(); 

	class AActor* ClawCharacter; 

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<UDamageType> DamageType;

	UHealthComponent* EnemyHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* attackCollisionBox;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void HandleDeath();

	void DestroyEnemy();
};
