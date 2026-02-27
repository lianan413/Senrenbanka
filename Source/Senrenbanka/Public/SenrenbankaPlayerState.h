#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SenrenbankaPlayerState.generated.h"

UCLASS()
class SENRENBANKA_API ASenrenbankaPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ASenrenbankaPlayerState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString PlayerDisplayName;
};

