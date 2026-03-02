// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SenrenbankaGameInstance.generated.h"

UCLASS()
class SENRENBANKA_API USenrenbankaGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USenrenbankaGameInstance();

	/** Called when the game instance is initialized (once at startup). */
	virtual void Init() override;

	/** Called when the game is shutting down. */
	virtual void Shutdown() override;
};
