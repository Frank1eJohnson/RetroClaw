// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ClawBullet.generated.h"


class AClawGameMode;

UCLASS()
class RETROCLAW_API AClawBullet : public APaperSpriteActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementSpeed = 1300;

	UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category = Damage, meta = (AllowPrivateAccess = "true"))
	float Damage = 100;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* HitCollisionBox;

	AClawGameMode* GameModeRef;

public:
	// class constructor
	AClawBullet();


protected:
	// called when actor is spawned
	virtual void BeginPlay() override;


};
