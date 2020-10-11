// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "ClawPotion.generated.h"

class AClawGameMode;

UCLASS()
class RETROCLAW_API AClawPotion : public APaperSpriteActor
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = Damage, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditAnywhere, Category = Damage, meta = (AllowPrivateAccess = "true"))
    float Damage = -20;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* HitCollisionBox;

    AClawGameMode* GameModeRef;

public:
    // class constructor
    AClawPotion();


protected:
    // called when actor is spawned
    virtual void BeginPlay() override;

};
