// Copyright Epic Games, Inc. All Rights Reserved.

#include "SenrenbankaCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InteractableInterface.h"
#include "Senrenbanka.h"

ASenrenbankaCharacter::ASenrenbankaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 交互球体（原神式 E 交互）
	InteractRadius = 200.f;
	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(GetRootComponent());
	InteractSphere->SetSphereRadius(InteractRadius);
	InteractSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ASenrenbankaCharacter::OnInteractSphereBeginOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ASenrenbankaCharacter::OnInteractSphereEndOverlap);

	CurrentInteractIndex = INDEX_NONE;
	NearbyInteractables.Empty();

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASenrenbankaCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

void ASenrenbankaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASenrenbankaCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComp->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASenrenbankaCharacter::Look);
		}

		if (MouseLookAction)
		{
			EnhancedInputComp->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASenrenbankaCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComp->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComp->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
	}
	else
	{
		UE_LOG(LogSenrenbanka, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASenrenbankaCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection,   MovementVector.X);
	}
}

void ASenrenbankaCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASenrenbankaCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ASenrenbankaCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ASenrenbankaCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ASenrenbankaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ASenrenbankaCharacter::OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	if (!OtherActor->Implements<UInteractableInterface>()) return;

	for (const TWeakObjectPtr<AActor>& Ptr : NearbyInteractables)
	{
		if (Ptr.Get() == OtherActor) return;
	}
	NearbyInteractables.Add(OtherActor);
	RemoveInvalidEntries();
	if (CurrentInteractIndex == INDEX_NONE)
	{
		CurrentInteractIndex = 0;
	}
	RebuildCurrentIndexAfterListChanged();
}

void ASenrenbankaCharacter::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || !OtherActor->Implements<UInteractableInterface>()) return;
	NearbyInteractables.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Ptr) { return Ptr.Get() == OtherActor; });
	RemoveInvalidEntries();
	RebuildCurrentIndexAfterListChanged();
}

void ASenrenbankaCharacter::RemoveInvalidEntries()
{
	NearbyInteractables.RemoveAll([](const TWeakObjectPtr<AActor>& Ptr) { return !Ptr.IsValid(); });
}

void ASenrenbankaCharacter::RebuildCurrentIndexAfterListChanged()
{
	const int32 N = NearbyInteractables.Num();
	if (N == 0)
	{
		CurrentInteractIndex = INDEX_NONE;
		return;
	}
	CurrentInteractIndex = FMath::Clamp(CurrentInteractIndex, 0, N - 1);
	if (CurrentInteractIndex < 0) CurrentInteractIndex = 0;
}

AActor* ASenrenbankaCharacter::GetCurrentInteractTarget() const
{
	if (CurrentInteractIndex == INDEX_NONE || NearbyInteractables.Num() == 0) return nullptr;
	if (CurrentInteractIndex < 0 || CurrentInteractIndex >= NearbyInteractables.Num()) return nullptr;
	AActor* Target = NearbyInteractables[CurrentInteractIndex].Get();
	return Target;
}

FText ASenrenbankaCharacter::GetCurrentInteractText() const
{
	AActor* Target = GetCurrentInteractTarget();
	if (!Target) return FText::GetEmpty();
	return IInteractableInterface::Execute_GetInteractText(Target);
}

void ASenrenbankaCharacter::CycleInteractTarget()
{
	const int32 N = NearbyInteractables.Num();
	if (N <= 1) return;
	CurrentInteractIndex = (CurrentInteractIndex + 1) % N;
}

void ASenrenbankaCharacter::TryInteract(APlayerController* InstigatorPC)
{
	AActor* Target = GetCurrentInteractTarget();
	if (Target && InstigatorPC)
	{
		IInteractableInterface::Execute_Interact(Target, InstigatorPC);
	}
}

FSenrenbankaCombatSaveData ASenrenbankaCharacter::GetCombatSaveDataForSave_Implementation() const
{
	// 默认返回一份空的战斗存档数据，具体数值由蓝图子类覆盖
	return FSenrenbankaCombatSaveData();
}

void ASenrenbankaCharacter::ApplyCombatSaveDataFromSave_Implementation(const FSenrenbankaCombatSaveData& InData)
{
	// 默认无操作，具体恢复逻辑由蓝图子类实现
}
