#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SenrenbankaNPCBase.generated.h"

class UAffectionComponent;
class UScheduleComponent;
class ULLMNPCDialogueComponent;

UCLASS(Blueprintable)
class SENRENBANKA_API ASenrenbankaNPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASenrenbankaNPCBase();

	UFUNCTION(BlueprintCallable, Category = "NPC")
	UAffectionComponent* GetAffectionComponent() const;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	UScheduleComponent* GetScheduleComponent() const;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	ULLMNPCDialogueComponent* GetDialogueComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UAffectionComponent* AffectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UScheduleComponent* ScheduleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	ULLMNPCDialogueComponent* DialogueComponent;
};

