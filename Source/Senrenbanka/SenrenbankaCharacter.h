// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Logging/LogMacros.h"
#include "SenrenbankaSaveTypes.h"
#include "SenrenbankaCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS()
class SENRENBANKA_API ASenrenbankaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** 交互检测球体（原神式 E 交互） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractSphere;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> NearbyInteractables;

	UPROPERTY()
	int32 CurrentInteractIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	float InteractRadius;

protected:

	/** Enhanced Input: 默认映射上下文（如 IMC_Default） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 移动 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** 视角 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** 跳跃 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** Mouse Look（手柄为 LookAction，鼠标可用此） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLookAction;

	/** 处理移动 */
	void Move(const FInputActionValue& Value);
	/** 处理视角 */
	void Look(const FInputActionValue& Value);

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	/** Constructor */
	ASenrenbankaCharacter();

protected:

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** 当前可交互目标（用于 UI 与 E 键） */
	UFUNCTION(BlueprintPure, Category = "Interact")
	AActor* GetCurrentInteractTarget() const;

	UFUNCTION(BlueprintPure, Category = "Interact")
	FText GetCurrentInteractText() const;

	/** Q 切换目标 */
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void CycleInteractTarget();

	/** E 与当前目标交互 */
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void TryInteract(APlayerController* InstigatorPC);

	UFUNCTION()
	void OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void RebuildCurrentIndexAfterListChanged();
	void RemoveInvalidEntries();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// 战斗属性存档桥接接口（默认实现交由蓝图覆盖）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	FSenrenbankaCombatSaveData GetCombatSaveDataForSave() const;
	virtual FSenrenbankaCombatSaveData GetCombatSaveDataForSave_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	void ApplyCombatSaveDataFromSave(const FSenrenbankaCombatSaveData& InData);
	virtual void ApplyCombatSaveDataFromSave_Implementation(const FSenrenbankaCombatSaveData& InData);
};

