// Fill out your copyright notice in the Description page of Project Settings.


#include "ClawBullet.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "Components/CapsuleComponent.h" 
#include "ClawGameMode.h"
#include "Engine/Engine.h"

AClawBullet::AClawBullet()
{
	PrimaryActorTick.bCanEverTick = false;

	// initializes the Bullet's box collision 
	HitCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet Collision"));
	HitCollisionBox->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	HitCollisionBox->SetCollisionProfileName("Trigger");
	HitCollisionBox->SetupAttachment(RootComponent);

	HitCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AClawBullet::OnOverlapBegin);
	HitCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AClawBullet::OnOverlapEnd);
}

void AClawBullet::BeginPlay()
{
	Super::BeginPlay();

	GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	

	UE_LOG(LogTemp, Warning, TEXT("started bullet"));
}

void AClawBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// check it it's claw who's overlapping with the score object.
	if (OtherActor && OtherActor->IsA(AEnemyCharacter::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		// the bullet has hitted an enemy 

		// decrease the enemy health.
		//UGameplayStatics::ApplyDamage(OtherActor, Damage, GetOwner()->GetInstigatorController(), this, DamageType);

		// destroy actor.
		//this->Destroy();
	}
}

void AClawBullet::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
