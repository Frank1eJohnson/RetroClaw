// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "HealthComponent.h"
#include "Sound/SoundBase.h"
#include "BlueOfficer.generated.h"


class APaperSpriteActor;

UCLASS()
class RETROCLAW_API ABlueOfficer : public APaperCharacter
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

	// The animation to play while firing the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* GunAttackAnimation;

	// The animation to play while firing the gun while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* CrouchingGunAttackAnimation;

	// The animation to play while bashing the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* GunBashAnimation;

	// The animation to play when getting hurt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* HurtAnimation;

	// The animation to play after death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* DeadAnimation;
	

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation();

	void UpdateCharacter();

	void StartMoving();
	void ChangeMovementDirection();
	void StopMoving();

	void StartIdling();
	void StopIdling();

	void StartGunAttack();
	void FireBullet();
	void StopGunAttack();

	void StartGunBash();
	void DealDamage();
	void StopGunBash();

	bool isWalking = true;
	bool isIdling = true;
	bool isFiringGun = false;
	bool isBashingGun = false;
	bool isDead = false;
	bool isHurt = false;

	int patrols = 0;

	// movementDirection will be multiplied by world vector
	// 0 will result in no movement, 1 is right
	// movement and -1 is left movement.
	float movementDirection = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* BulletSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* ClawCelebrationSound;

public:
	ABlueOfficer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APaperSpriteActor> BulletClass;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<UDamageType> DamageType;

	UHealthComponent* OfficerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* GunFireCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* GunBashCollisionBox;

private:
	class AActor* ClawCharacter;

	class UPrimitiveComponent* ClawCapsuleComponent;

	UFUNCTION()
	void OnOverlapBeginGunFireCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEndGunFireCollisionBox(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HandleDeath();

	void DestroyOfficer();
};
