// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "Sound/SoundBase.h"
#include "BlueOfficerBullet.generated.h"

/**
 * 
 */
UCLASS()
class RETROCLAW_API ABlueOfficerBullet : public APaperSpriteActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementSpeed = 1300;

	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category = Damage, meta = (AllowPrivateAccess = "true"))
	float Damage = 30;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* HitCollisionBox;

	class AClawGameMode* GameModeRef;

public:
	// class constructor
	ABlueOfficerBullet();


protected:
	// called when actor is spawned
	virtual void BeginPlay() override;

};
