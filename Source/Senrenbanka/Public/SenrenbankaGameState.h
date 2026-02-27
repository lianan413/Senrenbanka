#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SenrenbankaGameState.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDaySegment : uint8
{
	Morning UMETA(DisplayName = "Morning"),
	Noon UMETA(DisplayName = "Noon"),
	Evening UMETA(DisplayName = "Evening"),
	Night UMETA(DisplayName = "Night")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeSegmentChanged, ETimeOfDaySegment, NewSegment);

UCLASS()
class SENRENBANKA_API ASenrenbankaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASenrenbankaGameState();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	float CurrentTimeOfDay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	float SecondsPerGameHour;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	ETimeOfDaySegment CurrentSegment;

	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay")
	FOnTimeSegmentChanged OnTimeSegmentChanged;

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	ETimeOfDaySegment GetCurrentTimeSegment() const;

	void AdvanceTimeOfDay();

private:
	FTimerHandle TimeOfDayTimerHandle;
};

