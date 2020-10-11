// Fill out your copyright notice in the Description page of Project Settings.


#include "Spikes.h"

#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h" 
#include "ClawGameMode.h"
#include "Templates/Casts.h"
#include "Kismet/GameplayStatics.h"	
#include "BlueOfficer.h"


ASpikes::ASpikes()
{
	// initializes the Bullet's box collision 
	HitCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spike Collision"));
	HitCollisionBox->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	HitCollisionBox->SetCollisionProfileName("Trigger");
	HitCollisionBox->SetupAttachment(RootComponent);

	HitCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASpikes::OnOverlapBegin);
}
void ASpikes::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));

	GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ASpikes::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor->IsA(ARetroClawCharacter::StaticClass()) || OtherActor->IsA(AEnemyCharacter::StaticClass()) || OtherActor->IsA(ABlueOfficer::StaticClass())) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(OtherActor, SpikeDamage, GetInstigatorController(), this, DamageType);
	}
};
