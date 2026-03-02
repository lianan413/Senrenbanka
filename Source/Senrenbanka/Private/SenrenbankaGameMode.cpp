// Fill out your copyright notice in the Description page of Project Settings.

#include "SenrenbankaGameMode.h"
#include "SenrenbankaCharacter.h"
#include "SenrenbankaPlayerController.h"
#include "SenrenbankaGameState.h"
#include "SenrenbankaPlayerState.h"
#include "UObject/ConstructorHelpers.h"

ASenrenbankaGameMode::ASenrenbankaGameMode()
{
	PlayerControllerClass = ASenrenbankaPlayerController::StaticClass();
	GameStateClass = ASenrenbankaGameState::StaticClass();
	PlayerStateClass = ASenrenbankaPlayerState::StaticClass();
	DefaultPawnClass = ASenrenbankaCharacter::StaticClass();
}
