#include "SenrenbankaGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Internationalization/Text.h"

ASenrenbankaGameState::ASenrenbankaGameState()
{
	DayOfWeekIndex = 0;   // 周一
	DayIndex = 0;
	CurrentHour = 8;      // 早上 8 点
	CurrentMinute = 0;
	MinutesPerTick = 10;
	SecondsPerGameHour = 10.f;  // 每隔多少秒调用一次 AdvanceTimeOfDay
	CurrentTimeOfDay = 8.f;
	CurrentSegment = ETimeOfDaySegment::Morning;
}

void ASenrenbankaGameState::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimeOfDayTimerHandle,
			this,
			&ASenrenbankaGameState::AdvanceTimeOfDay,
			SecondsPerGameHour,
			true);
	}
}

void ASenrenbankaGameState::AdvanceTimeOfDay()
{
	const ETimeOfDaySegment OldSegment = CurrentSegment;

	CurrentMinute += MinutesPerTick;
	while (CurrentMinute >= 60)
	{
		CurrentMinute -= 60;
		CurrentHour += 1;
	}
	if (CurrentHour >= 24)
	{
		CurrentHour -= 24;
		DayIndex += 1;
		DayOfWeekIndex = (DayOfWeekIndex + 1) % 7;
	}

	CurrentTimeOfDay = CurrentHour + CurrentMinute / 60.f;

	RecalculateSegmentFromTime();

	if (CurrentSegment != OldSegment)
	{
		OnTimeSegmentChanged.Broadcast(CurrentSegment);
	}
}

void ASenrenbankaGameState::RecalculateSegmentFromTime()
{
	const int32 H = CurrentHour;
	if (H >= 6 && H < 12)
	{
		CurrentSegment = ETimeOfDaySegment::Morning;
	}
	else if (H >= 12 && H < 18)
	{
		CurrentSegment = ETimeOfDaySegment::Noon;
	}
	else if (H >= 18 && H < 22)
	{
		CurrentSegment = ETimeOfDaySegment::Evening;
	}
	else
	{
		CurrentSegment = ETimeOfDaySegment::Night;
	}
}

ETimeOfDaySegment ASenrenbankaGameState::GetCurrentTimeSegment() const
{
	return CurrentSegment;
}

FText ASenrenbankaGameState::GetTimeDisplayText() const
{
	static const TCHAR* DayNames[] = { TEXT("周一"), TEXT("周二"), TEXT("周三"), TEXT("周四"), TEXT("周五"), TEXT("周六"), TEXT("周日") };
	const int32 Day = FMath::Clamp(DayOfWeekIndex, 0, 6);
	const FString TimeStr = FString::Printf(TEXT("%02d:%02d"), CurrentHour, CurrentMinute);
	const FString Combined = FString::Printf(TEXT("%s-%s"), DayNames[Day], *TimeStr);
	return FText::FromString(Combined);
}

FText ASenrenbankaGameState::GetSegmentDisplayText() const
{
	switch (CurrentSegment)
	{
	case ETimeOfDaySegment::Morning: return FText::FromString(TEXT("早上"));
	case ETimeOfDaySegment::Noon:    return FText::FromString(TEXT("中午"));
	case ETimeOfDaySegment::Evening: return FText::FromString(TEXT("晚上"));
	case ETimeOfDaySegment::Night:   return FText::FromString(TEXT("凌晨"));
	default:                         return FText::FromString(TEXT("凌晨"));
	}
}

float ASenrenbankaGameState::GetSkyTimeSeconds() const
{
	// 现实 1 秒 = 游戏 1 分钟
	// 游戏一天 24 小时 = 1440 分钟 = 1440 秒
	const float MinutesTotal = static_cast<float>(CurrentHour * 60 + CurrentMinute);
	return FMath::Clamp(MinutesTotal, 0.0f, 1440.0f);
}

float ASenrenbankaGameState::GetSkyTimeNormalized() const
{
	return GetSkyTimeSeconds() / 1440.0f;
}

FSenrenbankaTimeSaveData ASenrenbankaGameState::GetTimeSaveData() const
{
	FSenrenbankaTimeSaveData Data;
	Data.DayIndex = DayIndex;
	Data.WeekDayIndex = DayOfWeekIndex;
	Data.Hour = CurrentHour;
	Data.Minute = CurrentMinute;

	UE_LOG(LogTemp, Log, TEXT("GameState::GetTimeSaveData -> DayIndex=%d WeekDayIndex=%d Time=%02d:%02d"),
		Data.DayIndex, Data.WeekDayIndex, Data.Hour, Data.Minute);

	return Data;
}

void ASenrenbankaGameState::ApplyTimeSaveData(const FSenrenbankaTimeSaveData& InData)
{
	UE_LOG(LogTemp, Log, TEXT("GameState::ApplyTimeSaveData <- DayIndex=%d WeekDayIndex=%d Time=%02d:%02d"),
		InData.DayIndex, InData.WeekDayIndex, InData.Hour, InData.Minute);

	UE_LOG(LogTemp, Log, TEXT("GameState::ApplyTimeSaveData begin suppress refresh"));
	bSuppressDailyEnemyRefreshBroadcast = true;

	const ETimeOfDaySegment OldSegment = CurrentSegment;

	DayIndex = InData.DayIndex;
	DayOfWeekIndex = InData.WeekDayIndex;
	CurrentHour = InData.Hour;
	CurrentMinute = InData.Minute;

	CurrentTimeOfDay = CurrentHour + CurrentMinute / 60.f;

	RecalculateSegmentFromTime();

	if (CurrentSegment != OldSegment)
	{
		OnTimeSegmentChanged.Broadcast(CurrentSegment);
	}

	bSuppressDailyEnemyRefreshBroadcast = false;
	UE_LOG(LogTemp, Log, TEXT("GameState::ApplyTimeSaveData end suppress refresh"));
}

void ASenrenbankaGameState::AdvanceGameTimeByMinutes(int32 MinutesToAdvance)
{
	if (MinutesToAdvance == 0)
	{
		return;
	}

	// 计算推进前的全局分钟数
	const int32 OldDayIndex = DayIndex;
	const int32 OldHour = CurrentHour;
	const int32 OldMinute = CurrentMinute;

	const int32 OldTotalMinutes = OldDayIndex * 1440 + OldHour * 60 + OldMinute;
	const int32 NewTotalMinutes = OldTotalMinutes + MinutesToAdvance;

	UE_LOG(LogTemp, Log, TEXT("GameState::AdvanceGameTimeByMinutes OldDay=%d OldTime=%02d:%02d NewTotalMinutes=%d (+%d)"),
		OldDayIndex, OldHour, OldMinute, NewTotalMinutes, MinutesToAdvance);

	// 检查是否跨过任何每天 4:00（240 分钟）节点
	const int32 FirstDayToCheck = OldTotalMinutes / 1440;
	const int32 LastDayToCheck = NewTotalMinutes / 1440;

	for (int32 Day = FirstDayToCheck; Day <= LastDayToCheck; ++Day)
	{
		const int32 ThresholdMinutes = Day * 1440 + 240; // 每天 4:00
		if (ThresholdMinutes > OldTotalMinutes && ThresholdMinutes <= NewTotalMinutes)
		{
			if (!bSuppressDailyEnemyRefreshBroadcast)
			{
				UE_LOG(LogTemp, Log, TEXT("GameState::AdvanceGameTimeByMinutes Daily enemy refresh triggered for day %d"), Day);
				OnDailyEnemyRefresh.Broadcast(Day);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("GameState::AdvanceGameTimeByMinutes Daily enemy refresh suppressed during save-load apply, day %d"), Day);
			}
		}
	}

	// 更新 DayIndex / Hour / Minute
	const int32 NewDayIndex = NewTotalMinutes / 1440;
	const int32 MinutesWithinDay = NewTotalMinutes - NewDayIndex * 1440;
	const int32 NewHour = MinutesWithinDay / 60;
	const int32 NewMinute = MinutesWithinDay % 60;

	const int32 DeltaDays = NewDayIndex - OldDayIndex;

	DayIndex = NewDayIndex;

	// WeekDayIndex 按 7 天循环
	if (DeltaDays != 0)
	{
		int32 NewWeekDay = DayOfWeekIndex + DeltaDays;
		NewWeekDay %= 7;
		if (NewWeekDay < 0)
		{
			NewWeekDay += 7;
		}
		DayOfWeekIndex = NewWeekDay;
	}

	CurrentHour = NewHour;
	CurrentMinute = NewMinute;
	CurrentTimeOfDay = CurrentHour + CurrentMinute / 60.f;

	const ETimeOfDaySegment OldSegment = CurrentSegment;
	RecalculateSegmentFromTime();

	if (CurrentSegment != OldSegment)
	{
		OnTimeSegmentChanged.Broadcast(CurrentSegment);
	}

	UE_LOG(LogTemp, Log, TEXT("GameState::AdvanceGameTimeByMinutes NewDay=%d NewWeekDay=%d NewTime=%02d:%02d"),
		DayIndex, DayOfWeekIndex, CurrentHour, CurrentMinute);
}

