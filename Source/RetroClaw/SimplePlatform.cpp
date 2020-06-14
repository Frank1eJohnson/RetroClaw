// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePlatform.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h" 
#include "RetroClawCharacter.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/Actor.h"

ASimplePlatform::ASimplePlatform()
{
	PrimaryActorTick.bCanEverTick = false;

	// initializes the underneath collision box. 
	underneathCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("underneath Collision"));
	underneathCollisionBox->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	underneathCollisionBox->SetCollisionProfileName("Trigger");
	underneathCollisionBox->SetupAttachment(RootComponent);

	underneathCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASimplePlatform::OnOverlapBegin);
	underneathCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ASimplePlatform::OnOverlapEnd);

	baseCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("base Collision"));
	baseCollisionBox->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	baseCollisionBox->SetCollisionProfileName("Trigger");
	baseCollisionBox->SetupAttachment(RootComponent);
}


void ASimplePlatform::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// check it it's claw who's overlapping with the box.
	if (OtherActor && OtherActor->IsA(ARetroClawCharacter::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("enter"));
		//SetActorEnableCollision(false);
		
		baseCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ASimplePlatform::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// check it it's claw who's overlapping with the box.
	if (OtherActor && OtherActor->IsA(ARetroClawCharacter::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("exit"));
		//SetActorEnableCollision(true);

		baseCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
