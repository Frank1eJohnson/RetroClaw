// Copyright Epic Games, Inc. All Rights Reserved.

#include "RetroClawCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnemyCharacter.h"
#include "ClawBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// ARetroClawCharacter

ARetroClawCharacter::ARetroClawCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->SetUsingAbsoluteRotation(true);
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	ClawHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// initializes the enemy's box collision 
	attackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	attackCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	attackCollisionBox->SetCollisionProfileName("Trigger");
	attackCollisionBox->SetupAttachment(RootComponent); 

	BulletSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Spawn Point"));
	BulletSpawnLocation->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}


//////////////////////////////////////////////////////////////////////////
// Animation

void ARetroClawCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;

	if (isDead) {
		DesiredAnimation = DeadAnimation;
	}
	else if (isHurt) {
		DesiredAnimation = HurtAnimation;
	}
	else if (GetCharacterMovement()->IsFalling()) {
		// if falling then render falling animation.
		DesiredAnimation = JumpingAnimation;
	}
	else if (isSwording) {
		// if swording then render the sword animation.
		DesiredAnimation = SwordingAnimation;
	}
	else if (isPistoling) {
		DesiredAnimation = PistolingAnimation;
	}
	else {
		// else render running or idle animation
		DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	}

	// Are we moving or standing still?
	if( GetSprite()->GetFlipbook() != DesiredAnimation )
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void ARetroClawCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!isDead && ClawHealth->GetHealth() <= 0) {
		HandleDeath();
	}

	UpdateCharacter();
}


//////////////////////////////////////////////////////////////////////////
// Input

void ARetroClawCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARetroClawCharacter::MoveRight);
	PlayerInputComponent->BindAction("Sword", IE_Pressed, this, &ARetroClawCharacter::StartSwording);
	PlayerInputComponent->BindAction("Pistol", IE_Pressed, this, &ARetroClawCharacter::StartPistoling);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARetroClawCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARetroClawCharacter::TouchStopped);
}

void ARetroClawCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

// starts the timer for the swording animation
void ARetroClawCharacter::StartSwording()
{
	if (isSwording == false && GetCharacterMovement()->IsFalling() == false)
	{
		isSwording = true;

		FixAnimationChangeOffset(43.0, true);
		
		StartDamaging();

		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopSwording, 0.6f, false);

		//GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
}

// called when the timer for the swording animation ends
void ARetroClawCharacter::StopSwording()
{
	isSwording = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	FixAnimationChangeOffset(43.0, false);
}

// somehow this method gets called twice for a single hit.
void ARetroClawCharacter::StartDamaging()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopDamaging, 0.3f, false);
}

void ARetroClawCharacter::StopDamaging()
{
	TSet<AActor*> OverlappingActors;

	attackCollisionBox->GetOverlappingActors(OverlappingActors);
	for (auto& Actor : OverlappingActors)
	{
		if (Actor->IsA(AEnemyCharacter::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("1"));

			UGameplayStatics::ApplyDamage(Actor, 300, GetOwner()->GetInstigatorController(), this, DamageType);

			break;
		}
	}
}

void ARetroClawCharacter::StartPistoling()
{
	isPistoling = true;

	FixAnimationChangeOffset(43.0, true);

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::SpawnBullet, 0.3f, false);
}

void ARetroClawCharacter::SpawnBullet()
{
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));
	if (BulletClass)
	{
		FRotator SpawnRotation = GetActorRotation();
		FVector SpawnLocation;

		if (GetActorRotation().Yaw >= 0) SpawnLocation = GetActorLocation() + FVector(70.0, 0.0f, 25.0f);
		else SpawnLocation = GetActorLocation() + FVector(-70.0, 0.0f, 25.0f);

		GetWorld()->SpawnActor<AClawBullet>(BulletClass, SpawnLocation, SpawnRotation);
	}

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopPistoling, 0.45f, false);
}

void ARetroClawCharacter::StopPistoling()
{
	FixAnimationChangeOffset(43.0, false);
	isPistoling = false;
}


// start hurt and stop hurt are implemented but not used yet. I realised it would 
// require either a major refactor of the entire project or very nasty code to use them.
void ARetroClawCharacter::StartHurt()
{
	isHurt = true;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopHurt, 0.1f, false);
}

void ARetroClawCharacter::StopHurt()
{
	isHurt = false;
}

void ARetroClawCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void ARetroClawCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void ARetroClawCharacter::UpdateCharacter()
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

void ARetroClawCharacter::FixAnimationChangeOffset(float offset, bool animationBegin)
{
	if (animationBegin)
	{
		if (GetActorRotation().Yaw >= 0)
		{
			//SetActorLocation(GetActorLocation() + FVector(10.0f, 0.0f, 0.0f));
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(offset, 0.0f, 0.0f));
		}
		else
		{
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(-offset, 0.0f, 0.0f));
		}
	}
	else
	{
		if (GetActorRotation().Yaw >= 0)
		{
			//SetActorLocation(GetActorLocation() + FVector(10.0f, 0.0f, 0.0f));
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(-offset, 0.0f, 0.0f));
		}
		else
		{
			GetSprite()->SetWorldLocation(GetSprite()->GetComponentLocation() + FVector(offset, 0.0f, 0.0f));
		}
	}
}

void ARetroClawCharacter::HandleDeath()
{
	isDead = true;

	GetCharacterMovement()->DisableMovement(); 
	FVector ClawLocation = GetActorLocation();

	//TODO: Disable Claw movement and make him fall 
	//SetActorLocation(ClawLocation + FVector(0.0f, 1.0f, 0.0f));
}
