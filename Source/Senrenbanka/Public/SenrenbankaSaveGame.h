#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SenrenbankaSaveTypes.h"
#include "SenrenbankaSaveGame.generated.h"

UCLASS()
class SENRENBANKA_API USenrenbankaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USenrenbankaSaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 UserIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SavedMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FTransform PlayerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float PlayerCurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FSenrenbankaTimeSaveData TimeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FSenrenbankaCombatSaveData CombatData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, float> FloatData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, int32> IntData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, bool> BoolData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, FString> StringData;
};

