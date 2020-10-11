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
#include "BlueOfficer.h"
#include "ClawGameMode.h"
#include "ClawBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "HealthComponent.h"
#include "ClawGameMode.h"

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
	GetCharacterMovement()->JumpZVelocity = 800.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MaxFlySpeed = 500.0f;

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
	else if (isSwording) {
		// using the sword mid air
		if (GetCharacterMovement()->IsFalling()) {
			DesiredAnimation = JumpSwordingAnimation;
		}
		// using the sword while crouching
		else if (isCrouching) {
			DesiredAnimation = CrouchSwordingAnimation;
		}
		// using the sword on the ground
		else {
			DesiredAnimation = SwordingAnimation;
		}
	}
	else if (isPistoling) {
		// firing the pistol mid air
		if (GetCharacterMovement()->IsFalling()) {
			DesiredAnimation = JumpPistolingAnimation;
		}
		// firing the pistol while crouching
		else if (isCrouching) {
			DesiredAnimation = CrouchPistolingAnimation;
		}
		// firing the pistol on the ground
		else {
			DesiredAnimation = PistolingAnimation;
		}
	}
	else if (GetCharacterMovement()->IsFalling()) {
		// if falling then render falling animation.
		DesiredAnimation = JumpingAnimation;
	}
	else if (isCrouching) {
		DesiredAnimation = CrouchingAnimation;
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

	if (currentHealth != ClawHealth->GetHealth()) {
		if (currentHealth > ClawHealth->GetHealth())
		{
			UGameplayStatics::SpawnSound2D(this, ClawHurtSound, 1.0f, 1.0f, 0.0f);
			StartHurt();
		} 
		GameModeRef->OnHealthPaneChanged.Broadcast(ClawHealth->GetHealth());
		currentHealth = ClawHealth->GetHealth();
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
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ARetroClawCharacter::CrouchClaw);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ARetroClawCharacter::StopCrouching);
	PlayerInputComponent->BindAction("Sword", IE_Pressed, this, &ARetroClawCharacter::StartSwording);
	PlayerInputComponent->BindAction("Pistol", IE_Pressed, this, &ARetroClawCharacter::StartPistoling); 
}

void ARetroClawCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

void ARetroClawCharacter::CrouchClaw()
{
	if (GetCharacterMovement()->IsFalling() == false)
	{
		clawCapsuleComponent->SetCapsuleHalfHeight(32.0f);
		//clawCapsuleComponent->SetRelativeLocation(clawCapsuleComponent->GetRelativeLocation() + FVector(0.0f, 0.0f, -30.0f));
		//GetSprite()->SetRelativeLocation(GetSprite()->GetRelativeLocation() + FVector(0.0f, 0.0f, 30.0f));

		isCrouching = true;
		GetCharacterMovement()->DisableMovement(); 
	}
}

void ARetroClawCharacter::StopCrouching()
{
	if (isCrouching == true)
	{
		clawCapsuleComponent->SetCapsuleHalfHeight(52.4f);
		//clawCapsuleComponent->SetRelativeLocation(clawCapsuleComponent->GetRelativeLocation() + FVector(0.0f, 0.0f, 30.0f));
		//GetSprite()->SetRelativeLocation(GetSprite()->GetRelativeLocation() + FVector(0.0f, 0.0f, -30.0f));

		isCrouching = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking); 
	}
}

// starts the timer for the swording animation
void ARetroClawCharacter::StartSwording()
{
	if (isSwording == false)
	{
		isSwording = true;
		UGameplayStatics::SpawnSound2D(this, ClawSwordSound, 1.0f, 1.0f, 0.0f);

		if (GetCharacterMovement()->IsFalling() == false || isCrouching == true)
		{ 
			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::DealDamage, 0.3f, false);

			//GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}
		// using the sword mid air
		else
		{ 
			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::DealDamage, 0.3f, false);
		}
	}
}

void ARetroClawCharacter::DealDamage()
{
	TSet<UPrimitiveComponent*> OverlappingComponents;
	
	attackCollisionBox->GetOverlappingComponents(OverlappingComponents);
	
	for (auto& Component : OverlappingComponents)
	{
		if (Component->IsA(UCapsuleComponent::StaticClass()))
		{ 
			if (Component->GetOwner()->IsA(AEnemyCharacter::StaticClass()) || Component->GetOwner()->IsA(ABlueOfficer::StaticClass()))
			{
				UGameplayStatics::ApplyDamage(Component->GetOwner(), 300, GetOwner()->GetInstigatorController(), this, DamageType);
			}
		}
	}

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopSwording, 0.3f, false);
}

// called when the timer for the swording animation ends
void ARetroClawCharacter::StopSwording()
{
	isSwording = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking); 
}


void ARetroClawCharacter::StartPistoling()
{
	if (isPistoling == false)
	{
		isPistoling = true;

		// firing the pistol on the ground
		if (GetCharacterMovement()->IsFalling() == false)
		{ 
			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::SpawnBullet, 0.3f, false);

			GetCharacterMovement()->DisableMovement();
		}
		// firing the pistol mid air
		else 
		{ 
			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::SpawnBullet, 0.2f, false);
		}
	}
}

void ARetroClawCharacter::SpawnBullet()
{
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));
	if (BulletClass)
	{
		if (ammo > 0)
		{
			ammo--;

			UGameplayStatics::SpawnSound2D(this, pistolFiringSound, 1.0f, 1.0f, 0.0f);

			FRotator SpawnRotation = GetActorRotation();
			FVector SpawnLocation;

			if (GetActorRotation().Yaw >= 0) SpawnLocation = GetActorLocation() + FVector(70.0, 0.0f, 25.0f);
			else SpawnLocation = GetActorLocation() + FVector(-70.0, 0.0f, 25.0f);

			if (GetCharacterMovement()->IsFalling() == true) {
				SpawnLocation += FVector(0.0, 0.0f, -20.0f);
			} 
			else if (isCrouching == true)
			{
				SpawnLocation += FVector(0.0, 0.0f, -40.0f);
			}

			GetWorld()->SpawnActor<AClawBullet>(BulletClass, SpawnLocation, SpawnRotation);
		}
		else
		{
			UGameplayStatics::SpawnSound2D(this, EmptyPistolSound, 1.0f, 1.0f, 0.0f);
		}
	}

	if (GetCharacterMovement()->IsFalling()) {
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopPistoling, 0.3f, false);
	}
	else {
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopPistoling, 0.45f, false);
	}
	
}

void ARetroClawCharacter::StopPistoling()
{
	isPistoling = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking); 
}


// start hurt and stop hurt are implemented but not used yet. I realised it would 
void ARetroClawCharacter::StartHurt()
{
	isHurt = true;
	GetCharacterMovement()->DisableMovement();

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARetroClawCharacter::StopHurt, 0.2f, false);
}

void ARetroClawCharacter::StopHurt()
{
	isHurt = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
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

void ARetroClawCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	clawCapsuleComponent = Cast<UCapsuleComponent>(RootComponent);
}

void ARetroClawCharacter::HandleDeath()
{
	isDead = true;

	GetCharacterMovement()->DisableMovement(); 
	FVector ClawLocation = GetActorLocation();

	//TODO: Disable Claw movement and make him fall 
	//SetActorLocation(ClawLocation + FVector(0.0f, 1.0f, 0.0f));
	GameModeRef->HandleGameOver(false);
	UGameplayStatics::SpawnSound2D(this, ClawDeathSound, 1.0f, 1.0f, 0.0f);

	// disable claw's movement and input
	this->TurnOff();
	this->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}
