// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class SENRENBANKA_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class SENRENBANKA_API IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	FText GetInteractText() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void Interact(APlayerController* InstigatorPC);
};
