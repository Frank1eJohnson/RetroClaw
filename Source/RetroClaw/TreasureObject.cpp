// Fill out your copyright notice in the Description page of Project Settings.


#include "TreasureObject.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "Components/CapsuleComponent.h" 
#include "ClawGameMode.h"
#include "Engine/Engine.h"

ATreasureObject::ATreasureObject()
{
	// initializes the enemy's box collision 
	ScoreCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ScoreCollision"));
	ScoreCollisionBox->SetBoxExtent(FVector(7.0f, 7.0f, 7.0f));
	ScoreCollisionBox->SetCollisionProfileName("Trigger");
	ScoreCollisionBox->SetupAttachment(RootComponent);

	ScoreCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATreasureObject::OnOverlapBegin);
	ScoreCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ATreasureObject::OnOverlapEnd);
}

void ATreasureObject::BeginPlay()
{
	Super::BeginPlay();

	GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	UE_LOG(LogTemp, Warning, TEXT("started treasure system"));
	UE_LOG(LogTemp, Warning, TEXT("started treasure system"));
}

void ATreasureObject::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// increase the score in the game mode.
	GameModeRef->AddScore(TreasureObjectScore);

	// TODO: create animation for the score object flying to the score.

	// destroy actor.

}

void ATreasureObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
} 
