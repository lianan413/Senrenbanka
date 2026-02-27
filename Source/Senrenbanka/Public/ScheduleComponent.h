#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SenrenbankaGameState.h"
#include "ScheduleComponent.generated.h"

class AActor;
class ASenrenbankaGameState;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SENRENBANKA_API UScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScheduleComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	TMap<ETimeOfDaySegment, AActor*> TimeToLocationTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	bool bTeleportInsteadOfWalking;

	ASenrenbankaGameState* CachedGameState;

	ETimeOfDaySegment LastAppliedSegment;

	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void ApplyScheduleForCurrentSegment();

protected:
	void ApplyScheduleForSegment(ETimeOfDaySegment Segment);
};

