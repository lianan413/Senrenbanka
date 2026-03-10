#pragma once

#include "CoreMinimal.h"
#include "SenrenbankaSaveTypes.generated.h"

USTRUCT(BlueprintType)
struct FSenrenbankaTimeSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 DayIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 WeekDayIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Hour = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Minute = 0;
};

USTRUCT(BlueprintType)
struct FSenrenbankaCombatSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float CurrentHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float BaseAttack = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float BaseCritRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float BaseMaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float FinalAttack = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float FinalCritRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float FinalMaxHP = 100.f;
};


