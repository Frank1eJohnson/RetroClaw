// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "PaperFlipbookComponent.h" 
#include "Components/CapsuleComponent.h"  
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h" 


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
	GetCharacterMovement()->MaxWalkSpeed = 150.0f;
	GetCharacterMovement()->MaxFlySpeed = 150.0f;

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
}

//////////////////////////////////////////////////////////////////////////
// Animation

void AEnemyCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;

	if (isSwording) {
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

	//StartMovementTimer();
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
	UE_LOG(LogTemp, Warning, TEXT("%f"), DeltaSeconds);

	AddMovementInput(FVector(movementDirection, 0.0f, 0.0f), 1);
	
	UpdateCharacter();
}

void AEnemyCharacter::StartMovementTimer()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemyCharacter::ChangeMovementDirection, 1.63f, false);
}

void AEnemyCharacter::ChangeMovementDirection()
{
	if (movementDirection == 1.0f)
		movementDirection = -1.0f;
	else if (movementDirection == -1.0f)
		movementDirection = 1.0f;

	StartMovementTimer();
}

void AEnemyCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}
