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
	attackCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapEnd);
}


// will be called when the Claw character enter the collision box
void AEnemyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("begin overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "overlap Begin");
	if (OtherActor && OtherComp->IsA(UCapsuleComponent::StaticClass()) && !isDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("overlapping"));
		
		AActor* MyOwner = GetOwner();
		if (OtherActor && OtherActor != this && OtherActor->IsA(ARetroClawCharacter::StaticClass()))
		{
			ClawCharacter = OtherActor;
			StartSwording();
		}
		
	}
}

// will supposedly be called when the Claw character exit the collision box
// it's not being called at the moment, maybe an off signature dunno
void AEnemyCharacter::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//UE_LOG(LogTemp, Warning, TEXT("end overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, "overlap End");
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
	StartMovementTimer();
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (isSwording) {
		//UE_LOG(LogTemp, Warning, TEXT("swording"));
	}
	else {
		//UE_LOG(LogTemp, Warning, TEXT("not swording"));
	} 

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
	
	UpdateCharacter();
}


// starts the timer for the swording animation
void AEnemyCharacter::StartSwording()
{
	//UE_LOG(LogTemp, Warning, TEXT("swording"));
	isSwording = true;

	// swording animation takes 0.6 second
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StopSwording, 0.6f, false);

	StartDamaging();

	GetCharacterMovement()->DisableMovement();
	//GetCharacterMovement()->StopMovementImmediately();
}

// called when the timer for the swording animation ends
void AEnemyCharacter::StopSwording()
{
	//StopDamaging();
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		//UE_LOG(LogTemp, Warning, TEXT("overlapping"));
		StartSwording();
	} else {
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		isSwording = false;
	}
}

// somehow this method gets called twice for a single hit.
void AEnemyCharacter::StartDamaging()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StopDamaging, 0.5f, false);
}

void AEnemyCharacter::StopDamaging()
{
	// checks if enemy is overlapping claw
	if (ClawCharacter != nullptr && attackCollisionBox->IsOverlappingActor(ClawCharacter))
	{
		//UE_LOG(LogTemp, Warning, TEXT("hit"));
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, "hitted claw"); 

		UGameplayStatics::ApplyDamage(ClawCharacter, 20, GetOwner()->GetInstigatorController(), this, DamageType);
	}
	else {
		//UE_LOG(LogTemp, Warning, TEXT("else"));
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "didn't hit claw");
	}
} 


void AEnemyCharacter::StartMovementTimer()
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
		StartMovementTimer();
		return;
	}

	if (movementDirection == 1.0f)
		movementDirection = -1.0f;
	else if (movementDirection == -1.0f)
		movementDirection = 1.0f;
	
	//UE_LOG(LogTemp, Error, TEXT("ended timer"));


	if (movementDirection < 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f)); 
	}
	else if (movementDirection > 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f)); 
	}
	

	//SetActorLocation(GetActorLocation() + FVector(-1.0f * movementDirection * 80.0f, 0.0f, 0.0f)); 

	StartIdlenessTimer();
}

// don't know why but unreal enters crashloop without this 
// function, so here it is.
void AEnemyCharacter::StartIdlenessTimer()
{
	//UE_LOG(LogTemp, Error, TEXT("started idleness timer"));

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::StartMovementTimer, 0.5f, false);
	//StartMovementTimer();
}

void AEnemyCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();
}

void AEnemyCharacter::HandleDeath()
{
	isDead = true;

	//GetCharacterMovement()->DisableMovement(); 

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
