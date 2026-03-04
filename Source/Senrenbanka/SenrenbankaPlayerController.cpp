// Copyright Epic Games, Inc. All Rights Reserved.

#include "SenrenbankaPlayerController.h"
#include "SenrenbankaCharacter.h"
#include "SenrenbankaDialogueComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Senrenbanka.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UObject/ConstructorHelpers.h"

ASenrenbankaPlayerController::ASenrenbankaPlayerController()
{
	DialogueComponent = CreateDefaultSubobject<USenrenbankaDialogueComponent>(TEXT("DialogueComponent"));

	// 从内容加载默认 IMC，保证 C++ GameMode 下移动与转视角可用
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_DefaultRef(TEXT("/Game/Input/IMC_Default.IMC_Default"));
	if (IMC_DefaultRef.Succeeded())
	{
		DefaultMappingContexts.Add(IMC_DefaultRef.Object);
	}
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_MouseLookRef(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	if (IMC_MouseLookRef.Succeeded())
	{
		MobileExcludedMappingContexts.Add(IMC_MouseLookRef.Object);
	}
}

void ASenrenbankaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);

	// 交互提示 UI 常驻
	if (InteractPromptWidgetClass)
	{
		InteractPromptWidget = CreateWidget<UUserWidget>(this, InteractPromptWidgetClass);
		if (InteractPromptWidget)
		{
			InteractPromptWidget->AddToViewport();
			InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogSenrenbanka, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ASenrenbankaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	// E 交互、Q 切换目标
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Interact)
		{
			EIC->BindAction(IA_Interact, ETriggerEvent::Triggered, this, &ASenrenbankaPlayerController::HandleInteract);
		}
		if (IA_CycleTarget)
		{
			EIC->BindAction(IA_CycleTarget, ETriggerEvent::Triggered, this, &ASenrenbankaPlayerController::HandleCycleTarget);
		}
	}
}

void ASenrenbankaPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateInteractPromptUI();
}

void ASenrenbankaPlayerController::HandleInteract()
{
	if (ASenrenbankaCharacter* C = Cast<ASenrenbankaCharacter>(GetPawn()))
	{
		C->TryInteract(this);
	}
}

void ASenrenbankaPlayerController::HandleCycleTarget()
{
	if (ASenrenbankaCharacter* C = Cast<ASenrenbankaCharacter>(GetPawn()))
	{
		C->CycleInteractTarget();
	}
}

void ASenrenbankaPlayerController::UpdateInteractPromptUI()
{
	if (!InteractPromptWidget) return;

	ASenrenbankaCharacter* C = Cast<ASenrenbankaCharacter>(GetPawn());
	if (!C)
	{
		InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	AActor* Target = C->GetCurrentInteractTarget();
	if (!Target)
	{
		InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	InteractPromptWidget->SetVisibility(ESlateVisibility::Visible);
	BP_SetInteractPromptText(C->GetCurrentInteractText());
}

bool ASenrenbankaPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
