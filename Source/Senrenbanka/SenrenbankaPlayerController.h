// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SenrenbankaPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;
class ASenrenbankaCharacter;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS()
class SENRENBANKA_API ASenrenbankaPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASenrenbankaPlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** E 交互 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Interact;

	/** Q 切换目标 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_CycleTarget;

	/** 交互提示 UI 类（WBP_InteractPrompt） */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InteractPromptWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> InteractPromptWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaSeconds) override;

	void HandleInteract();
	void HandleCycleTarget();
	void UpdateInteractPromptUI();

	/** 蓝图实现：把 InText 赋给提示 UI 的 TextBlock，无需在 C++ 里查控件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void BP_SetInteractPromptText(const FText& InText);

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

};
