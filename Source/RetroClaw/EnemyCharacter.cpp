// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "PaperFlipbookComponent.h" 
#include "Components/CapsuleComponent.h"  
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h" 
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"


AEnemyCharacter::AEnemyCharacter() 
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;
	GetCharacterMovement()->MaxFlySpeed = 100.0f;
	
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	//UE_LOG(LogTemp, Warning, TEXT("swording"));

	EnemyHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// initializes the enemy's box collision 
	attackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	attackCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	attackCollisionBox->SetCollisionProfileName("Trigger");
	attackCollisionBox->SetupAttachment(RootComponent);

	attackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapBegin);
}


// will be called when the Claw character enter the collision box
void AEnemyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("begin overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "overlap Begin");
	if (OtherActor && OtherComp->IsA(UCapsuleComponent::StaticClass()) && !isDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("overlapping"));

		if (OtherActor && OtherActor != this && OtherActor->IsA(ARetroClawCharacter::StaticClass()))
		{
			ClawCharacter = OtherActor;
			StartSwording();
		}
		
	}
}

//////////////////////////////////////////////////////////////////////////
// Animation

void AEnemyCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;

	if (isDead) {
		DesiredAnimation = DeadAnimation;
	}
	else if (isSwording) {
		// if swording then render the sword animation.
		DesiredAnimation = SwordingAnimation;
	}
	else {
		// else render running or idle animation
		DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? WalkingAnimation : IdleAnimation;
	}

	// Are we moving or standing still?
	if (GetSprite()->GetFlipbook() != DesiredAnimation)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void AEnemyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("beginplay"));

	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	StartMovement();
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); 

	// the movementDirection will routinly be changed from 1 to -1
	// so the enemy will always be moving either to left or to right
	if (isDead)
	{ 
		AddMovementInput(FVector(-1.0f, 0.0f, 0.0f), 1);
	}
	else 
	{
		AddMovementInput(FVector(movementDirection, 0.0f, 0.0f), 1);
	}

	if (!isDead && EnemyHealth->GetHealth() <= 0) {
		HandleDeath();
	}
	
	UpdateAnimation();
}


// starts the timer for the swording animation
void AEnemyCharacter::StartSwording()
{
	//for animation
	isSwording = true;

	// swording animation takes 0.6 second
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::DealDamage, 0.5f, false);

	GetCharacterMovement()->DisableMovement(); 
}

// starts the timer for the swording animation
void AEnemyCharacter::DealDamage()
{
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{ 
		UGameplayStatics::ApplyDamage(ClawCharacter, 20, GetOwner()->GetInstigatorController(), this, DamageType);
	} 

	// swording animation takes 0.6 second
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StopSwording, 0.1f, false); 
}

// called when the timer for the swording animation ends
void AEnemyCharacter::StopSwording()
{ 
	// if he's still overlapping claw, then he keeps attacking.
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{ 
		StartSwording();
	} else {
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		isSwording = false;
	}
} 

void AEnemyCharacter::StartMovement()
{
	//UE_LOG(LogTemp, Error, TEXT("started timer"));

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::ChangeMovementDirection, 2.2f, false);
}

void AEnemyCharacter::ChangeMovementDirection()
{
	if (isDead)
	{
		return;
	}

	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		StartMovement();
		return;
	}

	//if moving right reverse and vice versa
	movementDirection *= -1.0f;
	
	if (movementDirection < 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f)); 
	}
	else if (movementDirection > 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f)); 
	}
	
	StartMovement();
}  

void AEnemyCharacter::HandleDeath()
{
	isDead = true; 

	UGameplayStatics::SpawnSound2D(this, DeathSound, 1.0f, 1.0f, 0.0f);

	if (ClawCelebrationSound != nullptr)
	{
		UGameplayStatics::SpawnSound2D(this, ClawCelebrationSound, 1.0f, 1.0f, 0.0f);
	}

	//TODO: Disable Enemy movement and make him fall  

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::DestroyEnemy, 0.6f, false);

	//SetActorLocation(GetActorLocation() + FVector(0.0f, 4.0f, 0.0f));
	this->SetActorEnableCollision(false);
}

void AEnemyCharacter::DestroyEnemy()
{
	this->Destroy();
}
