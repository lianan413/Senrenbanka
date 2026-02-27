#include "AffectionComponent.h"

UAffectionComponent::UAffectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MinAffection = 0.f;
	MaxAffection = 100.f;
	NeutralThreshold = 25.f;
	WarmThreshold = 50.f;
	IntimateThreshold = 80.f;

	CurrentAffection = 0.f;
	CurrentAffectionLevel = EAffectionLevel::Cold;
}

void UAffectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAffectionComponent::AddAffection(float Delta)
{
	const float OldValue = CurrentAffection;
	const EAffectionLevel OldLevel = CurrentAffectionLevel;

	CurrentAffection = FMath::Clamp(OldValue + Delta, MinAffection, MaxAffection);

	const EAffectionLevel NewLevel = CalculateLevelFromValue(CurrentAffection);
	if (NewLevel != OldLevel)
	{
		CurrentAffectionLevel = NewLevel;
		OnAffectionLevelChanged.Broadcast(NewLevel, OldLevel);
	}
}

void UAffectionComponent::SetAffection(float NewValue)
{
	const EAffectionLevel OldLevel = CurrentAffectionLevel;

	CurrentAffection = FMath::Clamp(NewValue, MinAffection, MaxAffection);
	const EAffectionLevel NewLevel = CalculateLevelFromValue(CurrentAffection);

	if (NewLevel != OldLevel)
	{
		CurrentAffectionLevel = NewLevel;
		OnAffectionLevelChanged.Broadcast(NewLevel, OldLevel);
	}
}

float UAffectionComponent::GetAffectionValue() const
{
	return CurrentAffection;
}

EAffectionLevel UAffectionComponent::GetAffectionLevel() const
{
	return CurrentAffectionLevel;
}

EAffectionLevel UAffectionComponent::CalculateLevelFromValue(float Value) const
{
	if (Value >= IntimateThreshold)
	{
		return EAffectionLevel::Intimate;
	}
	if (Value >= WarmThreshold)
	{
		return EAffectionLevel::Warm;
	}
	if (Value >= NeutralThreshold)
	{
		return EAffectionLevel::Neutral;
	}

	return EAffectionLevel::Cold;
}

