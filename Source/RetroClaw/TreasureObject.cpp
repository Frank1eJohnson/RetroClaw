// Fill out your copyright notice in the Description page of Project Settings.


#include "TreasureObject.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "Components/CapsuleComponent.h" 
#include "Engine/Engine.h"

ATreasureObject::ATreasureObject()
{
	// initializes the enemy's box collision 
	attackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	attackCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	attackCollisionBox->SetCollisionProfileName("Trigger");
	attackCollisionBox->SetupAttachment(RootComponent);

	attackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATreasureObject::OnOverlapBegin);
	attackCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ATreasureObject::OnOverlapEnd);
}

void ATreasureObject::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("started treasure"));
}

void ATreasureObject::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ATreasureObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
