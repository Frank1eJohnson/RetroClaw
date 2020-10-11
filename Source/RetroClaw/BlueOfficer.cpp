// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueOfficer.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/BoxComponent.h"
#include "RetroClawCharacter.h"
#include "BlueOfficerBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"


ABlueOfficer::ABlueOfficer()
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

	OfficerHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// initializes the enemy's GunFire CollisionBox
	GunFireCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("GunFireCollisionBox"));
	GunFireCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	GunFireCollisionBox->SetCollisionProfileName("Trigger");
	GunFireCollisionBox->SetupAttachment(RootComponent);

	GunFireCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABlueOfficer::OnOverlapBeginGunFireCollisionBox);
	GunFireCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ABlueOfficer::OnOverlapEndGunFireCollisionBox);

	// initializes the enemy's GunBash CollisionBox
	GunBashCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("GunBashCollisionBox"));
	GunBashCollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	GunBashCollisionBox->SetCollisionProfileName("Trigger");
	GunBashCollisionBox->SetupAttachment(RootComponent); 
}

void ABlueOfficer::UpdateAnimation()
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
	else if (isFiringGun) {
		if (isCrouching)
		{
			DesiredAnimation = CrouchingGunAttackAnimation;
		} 
		else
		{
			DesiredAnimation = GunAttackAnimation;
		}
	}
	else if (isBashingGun) {
		DesiredAnimation = GunBashAnimation;
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

void ABlueOfficer::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("beginplay"));

	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	StartMoving();
}

void ABlueOfficer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); 

	// the movementDirection will routinly be changed from 1 to -1
	// so the enemy will always be moving either to left or to right
	if (!isDead)
	{
		if (isWalking)
		{
			AddMovementInput(FVector(movementDirection, 0.0f, 0.0f), 1);
		} 
	} 

	if (!isDead && OfficerHealth->GetHealth() <= 0) {
		HandleDeath();
	}

	UpdateCharacter();
}

void ABlueOfficer::StartMoving()
{
	isWalking = true;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::ChangeMovementDirection, 1.5f, false);
}

void ABlueOfficer::ChangeMovementDirection()
{
	if (patrols <= 4) {
		patrols++;

		if (isDead)  return; 

		if (isFiringGun || isBashingGun)
		{
			StartMoving();
			return;
		}

		if (movementDirection == 1.0f) movementDirection = -1.0f;
		else movementDirection = 1.0f;  

		if (movementDirection < 0.0f)
		{
			SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
		else if (movementDirection > 0.0f)
		{
			SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
		}

		StartMoving();
	}
	else {
		patrols = 0;

		StopMoving();
	}
}

void ABlueOfficer::StopMoving()
{
	isWalking = false;

	StartIdling();
}

void ABlueOfficer::StartIdling()
{
	isIdling = true;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::StopIdling, 6.0f, false);
}

void ABlueOfficer::StopIdling()
{
	isIdling = false;

	StartMoving();
}

void ABlueOfficer::StartGunAttack()
{
	//UE_LOG(LogTemp, Warning, TEXT("swording"));
	isFiringGun = true;

	// swording animation takes 0.6 second
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::FireBullet, 0.75f, false);

	GetCharacterMovement()->DisableMovement(); 
}

void ABlueOfficer::FireBullet()
{
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));
	if (BulletClass && !isDead)
	{
		UGameplayStatics::SpawnSound2D(this, BulletSound, 1.0f, 1.0f, 0.0f);

		FRotator SpawnRotation = GetSprite()->GetComponentRotation();
		if (movementDirection < 0)
			SpawnRotation.Yaw = 180;
		else SpawnRotation.Yaw = 0;
		FVector SpawnLocation;

		if (GetActorRotation().Yaw < 0) SpawnLocation = GetActorLocation() + FVector(65.0, 0.0f, 35.0f);
		else SpawnLocation = GetActorLocation() + FVector(-65.0, 0.0f, 35.0f);

		if (isCrouching) SpawnLocation.Z -= 40.0f;

		GetWorld()->SpawnActor<ABlueOfficerBullet>(BulletClass, SpawnLocation, SpawnRotation);
	}

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::StopGunAttack, 0.75f, false);
}

void ABlueOfficer::StopGunAttack()
{
	if (ClawCharacter != nullptr && GunFireCollisionBox->IsOverlappingActor(ClawCharacter) && GunFireCollisionBox->IsOverlappingComponent(ClawCapsuleComponent))
	{
		if (GunBashCollisionBox->IsOverlappingActor(ClawCharacter) && GunBashCollisionBox->IsOverlappingComponent(ClawCapsuleComponent))
		{
			isFiringGun = false;

			StartGunBash();
		}
		else 
		{
			ARetroClawCharacter* clawCharacter;
			clawCharacter = Cast<ARetroClawCharacter>(ClawCharacter);

			if (clawCharacter->isCrouching)
			{
				isCrouching = true;
			}
			else
			{
				isCrouching = false;
			}

			StartGunAttack();
		} 
	}
	else {
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		isFiringGun = false;
	}
}

void ABlueOfficer::StartGunBash()
{
	isBashingGun = true;
	
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::DealDamage, 0.4f, false);

	GetCharacterMovement()->DisableMovement();
}

void ABlueOfficer::DealDamage()
{
	if (ClawCharacter && ClawCapsuleComponent && GunBashCollisionBox->IsOverlappingActor(ClawCharacter) && GunBashCollisionBox->IsOverlappingComponent(ClawCapsuleComponent))
	{
		UGameplayStatics::ApplyDamage(ClawCharacter, 15, GetOwner()->GetInstigatorController(), this, DamageType);
	}

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::StopGunBash, 0.4f, false);
}

void ABlueOfficer::StopGunBash()
{
	if (ClawCharacter != nullptr && GunFireCollisionBox->IsOverlappingActor(ClawCharacter) && GunFireCollisionBox->IsOverlappingComponent(ClawCapsuleComponent))
	{
		if (GunBashCollisionBox->IsOverlappingActor(ClawCharacter) && GunBashCollisionBox->IsOverlappingComponent(ClawCapsuleComponent))
		{
			StartGunBash();
		}
		else
		{ 
			ARetroClawCharacter* clawCharacter;
			clawCharacter = Cast<ARetroClawCharacter>(ClawCharacter);

			if (clawCharacter->isCrouching)
			{
				isCrouching = true;
			}
			else
			{
				isCrouching = false;
			}
			StartGunAttack();
		}
	}
	else {
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		isBashingGun = false;
	}
}

void ABlueOfficer::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();
}

void ABlueOfficer::OnOverlapBeginGunFireCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("begin overlap"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "overlap Begin");
	if (OtherActor && OtherActor->IsA(ARetroClawCharacter::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()) && !isDead)
	{
		//UE_LOG(LogTemp, Warning, TEXT("overlapping"));
		
		if (OtherActor != GetOwner())
		{
			ClawCharacter = OtherActor;
			ClawCapsuleComponent = OtherComp;

			if (GunBashCollisionBox->IsOverlappingActor(OtherActor) && GunBashCollisionBox->IsOverlappingComponent(OtherComp))
			{
				StartGunBash();
			}
			else {
				ARetroClawCharacter* clawCharacter;
				clawCharacter = Cast<ARetroClawCharacter>(OtherActor);

				if (clawCharacter->isCrouching)
				{
					isCrouching = true;
				} 
				else 
				{
					isCrouching = false;
				}

				StartGunAttack();
			} 
		} 
	}
}

void ABlueOfficer::OnOverlapEndGunFireCollisionBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ABlueOfficer::HandleDeath()
{
	isDead = true;

	UGameplayStatics::SpawnSound2D(this, DeathSound, 1.0f, 1.0f, 0.0f);

	if (ClawCelebrationSound != nullptr)
	{
		UGameplayStatics::SpawnSound2D(this, ClawCelebrationSound, 1.0f, 1.0f, 0.0f);
	}

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ABlueOfficer::DestroyOfficer, 0.6f, false);

	this->SetActorEnableCollision(false);
}

void ABlueOfficer::DestroyOfficer()
{
	this->Destroy();
}
