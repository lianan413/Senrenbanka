#include "ScheduleComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"

UScheduleComponent::UScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bTeleportInsteadOfWalking = true;
	CachedGameState = nullptr;
	LastAppliedSegment = ETimeOfDaySegment::Morning;
}

void UScheduleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		CachedGameState = World->GetGameState<ASenrenbankaGameState>();
	}

	if (CachedGameState)
	{
		LastAppliedSegment = CachedGameState->GetCurrentTimeSegment();
		ApplyScheduleForSegment(LastAppliedSegment);
	}
}

void UScheduleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedGameState)
	{
		return;
	}

	const ETimeOfDaySegment CurrentSegment = CachedGameState->GetCurrentTimeSegment();
	if (CurrentSegment != LastAppliedSegment)
	{
		LastAppliedSegment = CurrentSegment;
		ApplyScheduleForSegment(LastAppliedSegment);
	}
}

void UScheduleComponent::ApplyScheduleForCurrentSegment()
{
	if (CachedGameState)
	{
		ApplyScheduleForSegment(CachedGameState->GetCurrentTimeSegment());
	}
}

void UScheduleComponent::ApplyScheduleForSegment(ETimeOfDaySegment Segment)
{
	AActor* const* TargetPtr = TimeToLocationTargets.Find(Segment);
	if (!TargetPtr)
	{
		return;
	}

	AActor* Target = *TargetPtr;
	if (!Target)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (bTeleportInsteadOfWalking)
	{
		Owner->SetActorLocation(Target->GetActorLocation());
	}
}

