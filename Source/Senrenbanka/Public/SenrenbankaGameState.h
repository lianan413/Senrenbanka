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

	/** 0..6 对应 周一..周日 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	int32 DayOfWeekIndex;

	/** 0..23 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	int32 CurrentHour;

	/** 0..59 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	int32 CurrentMinute;

	/** 每次 AdvanceTimeOfDay() 前进多少分钟，默认 10 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	int32 MinutesPerTick;

	/** 小时的浮点表示，与 CurrentHour + CurrentMinute 同步，UI 可不直接用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	float CurrentTimeOfDay;

	/** 每次时间 tick 间隔（秒），即每隔多少秒调用一次 AdvanceTimeOfDay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	float SecondsPerGameHour;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	ETimeOfDaySegment CurrentSegment;

	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay")
	FOnTimeSegmentChanged OnTimeSegmentChanged;

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	ETimeOfDaySegment GetCurrentTimeSegment() const;

	/** 返回类似：周三-08:40 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetTimeDisplayText() const;

	/** 返回：早上 / 中午 / 晚上 / 凌晨 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetSegmentDisplayText() const;

	void AdvanceTimeOfDay();

private:
	/** 根据当前小时更新 CurrentSegment */
	void RecalculateSegmentFromTime();

	FTimerHandle TimeOfDayTimerHandle;
};

