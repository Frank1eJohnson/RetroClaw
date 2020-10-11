// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelObjective.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h" 
#include "ClawGameMode.h"
#include "Engine/Engine.h"


ALevelObjective::ALevelObjective()
{
    PrimaryActorTick.bCanEverTick = false;

    // initializes the Bullet's box collision 
    HitCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet Collision"));
    HitCollisionBox->SetBoxExtent(FVector(17.0f, 17.0f, 17.0f));
    HitCollisionBox->SetCollisionProfileName("Trigger");
    HitCollisionBox->SetupAttachment(RootComponent);

    HitCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelObjective::OnOverlapBegin); 
}

void ALevelObjective::BeginPlay()
{
    Super::BeginPlay();
    // the potion has touched claw
    UE_LOG(LogTemp, Warning, TEXT("potion"));

    GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ALevelObjective::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // check it it's claw who's overlapping with the object.
    if (OtherActor && OtherActor->IsA(ARetroClawCharacter::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
    {
        GameModeRef->HandleGameOver(true);
        // destroy the object.
        this->Destroy(); 
    }
}
