#pragma once

#include "CoreMinimal.h"
#include "SenrenbankaItemTypes.generated.h"

USTRUCT(BlueprintType)
struct FSenrenbankaInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;
};

