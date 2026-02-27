#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AffectionComponent.generated.h"

UENUM(BlueprintType)
enum class EAffectionLevel : uint8
{
	Cold UMETA(DisplayName = "Cold"),
	Neutral UMETA(DisplayName = "Neutral"),
	Warm UMETA(DisplayName = "Warm"),
	Intimate UMETA(DisplayName = "Intimate")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAffectionLevelChanged, EAffectionLevel, NewLevel, EAffectionLevel, OldLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SENRENBANKA_API UAffectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAffectionComponent();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float CurrentAffection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float MinAffection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float MaxAffection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float NeutralThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float WarmThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	float IntimateThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affection")
	EAffectionLevel CurrentAffectionLevel;

	UPROPERTY(BlueprintAssignable, Category = "Affection")
	FAffectionLevelChanged OnAffectionLevelChanged;

	UFUNCTION(BlueprintCallable, Category = "Affection")
	void AddAffection(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Affection")
	void SetAffection(float NewValue);

	UFUNCTION(BlueprintPure, Category = "Affection")
	float GetAffectionValue() const;

	UFUNCTION(BlueprintPure, Category = "Affection")
	EAffectionLevel GetAffectionLevel() const;

protected:
	EAffectionLevel CalculateLevelFromValue(float Value) const;
};

