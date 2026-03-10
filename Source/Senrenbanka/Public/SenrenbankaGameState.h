#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SenrenbankaSaveTypes.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDailyEnemyRefresh, int32, RefreshDayIndex);

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

	/** 全局第几天（从 0 开始计数） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeOfDay")
	int32 DayIndex;

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

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnDailyEnemyRefresh OnDailyEnemyRefresh;

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	ETimeOfDaySegment GetCurrentTimeSegment() const;

	/** 返回类似：周三-08:40 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetTimeDisplayText() const;

	/** 返回：早上 / 中午 / 晚上 / 凌晨 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetSegmentDisplayText() const;

	// Sky 时间（用于 BP_StylizedSky）：0~1440 秒
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetSkyTimeSeconds() const;

	// Sky 时间归一化：0~1
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetSkyTimeNormalized() const;

	// 手动推进游戏时间（用于睡觉跳时等）
	UFUNCTION(BlueprintCallable, Category = "Time")
	void AdvanceGameTimeByMinutes(int32 MinutesToAdvance);

	// 存档用时间读取/应用接口
	UFUNCTION(BlueprintCallable, Category = "Save")
	FSenrenbankaTimeSaveData GetTimeSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ApplyTimeSaveData(const FSenrenbankaTimeSaveData& InData);

	void AdvanceTimeOfDay();

private:
	/** 根据当前小时更新 CurrentSegment */
	void RecalculateSegmentFromTime();

	FTimerHandle TimeOfDayTimerHandle;
};

