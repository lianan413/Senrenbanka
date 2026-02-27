#include "SenrenbankaGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASenrenbankaGameState::ASenrenbankaGameState()
{
	CurrentTimeOfDay = 8.f;
	SecondsPerGameHour = 10.f;
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
	CurrentTimeOfDay += 1.f;
	if (CurrentTimeOfDay >= 24.f)
	{
		CurrentTimeOfDay -= 24.f;
	}

	ETimeOfDaySegment OldSegment = CurrentSegment;
	ETimeOfDaySegment NewSegment = ETimeOfDaySegment::Night;

	const float T = CurrentTimeOfDay;
	if (T >= 6.f && T < 12.f)
	{
		NewSegment = ETimeOfDaySegment::Morning;
	}
	else if (T >= 12.f && T < 18.f)
	{
		NewSegment = ETimeOfDaySegment::Noon;
	}
	else if (T >= 18.f && T < 22.f)
	{
		NewSegment = ETimeOfDaySegment::Evening;
	}
	else
	{
		NewSegment = ETimeOfDaySegment::Night;
	}

	if (NewSegment != OldSegment)
	{
		CurrentSegment = NewSegment;
		OnTimeSegmentChanged.Broadcast(CurrentSegment);
	}
}

ETimeOfDaySegment ASenrenbankaGameState::GetCurrentTimeSegment() const
{
	return CurrentSegment;
}

