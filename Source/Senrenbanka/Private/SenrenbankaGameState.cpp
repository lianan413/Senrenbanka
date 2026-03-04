#include "SenrenbankaGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Internationalization/Text.h"

ASenrenbankaGameState::ASenrenbankaGameState()
{
	DayOfWeekIndex = 0;   // 周一
	CurrentHour = 11;     // 早上 8 点
	CurrentMinute = 50;
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

