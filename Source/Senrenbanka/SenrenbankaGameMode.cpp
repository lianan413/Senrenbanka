// Copyright Epic Games, Inc. All Rights Reserved.

#include "SenrenbankaGameMode.h"
#include "SenrenbankaCharacter.h"
#include "SenrenbankaPlayerController.h"
#include "SenrenbankaGameState.h"
#include "SenrenbankaPlayerState.h"

ASenrenbankaGameMode::ASenrenbankaGameMode()
{
	DefaultPawnClass = ASenrenbankaCharacter::StaticClass();
	PlayerControllerClass = ASenrenbankaPlayerController::StaticClass();
	GameStateClass = ASenrenbankaGameState::StaticClass();
	PlayerStateClass = ASenrenbankaPlayerState::StaticClass();
}
