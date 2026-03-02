// Fill out your copyright notice in the Description page of Project Settings.

#include "SenrenbankaGameInstance.h"
#include "Senrenbanka.h"

USenrenbankaGameInstance::USenrenbankaGameInstance()
{
}

void USenrenbankaGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogSenrenbanka, Log, TEXT("USenrenbankaGameInstance::Init - Game instance ready."));
}

void USenrenbankaGameInstance::Shutdown()
{
	Super::Shutdown();
	UE_LOG(LogSenrenbanka, Log, TEXT("USenrenbankaGameInstance::Shutdown - Game instance shutting down."));
}
