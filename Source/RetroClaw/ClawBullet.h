// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "ClawBullet.generated.h"

/**
 * 
 */
UCLASS()
class RETROCLAW_API AClawBullet : public APaperSpriteActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = Damage, meta = (AllowPrivateAccess = "true"))
	float MovementSpeed = 1300;

	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category = Damage, meta = (AllowPrivateAccess = "true"))
	float Damage = 101;


public:
	// class constructor
	AClawBullet();


protected:
	// called when actor is spawned
	virtual void BeginPlay() override;

	
};
