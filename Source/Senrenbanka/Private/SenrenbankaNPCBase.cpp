#include "SenrenbankaNPCBase.h"
#include "AffectionComponent.h"
#include "ScheduleComponent.h"
#include "LLMNPCDialogueComponent.h"

ASenrenbankaNPCBase::ASenrenbankaNPCBase()
{
	AffectionComponent = CreateDefaultSubobject<UAffectionComponent>(TEXT("AffectionComponent"));
	ScheduleComponent = CreateDefaultSubobject<UScheduleComponent>(TEXT("ScheduleComponent"));
	DialogueComponent = CreateDefaultSubobject<ULLMNPCDialogueComponent>(TEXT("DialogueComponent"));
}

UAffectionComponent* ASenrenbankaNPCBase::GetAffectionComponent() const
{
	return AffectionComponent;
}

UScheduleComponent* ASenrenbankaNPCBase::GetScheduleComponent() const
{
	return ScheduleComponent;
}

ULLMNPCDialogueComponent* ASenrenbankaNPCBase::GetDialogueComponent() const
{
	return DialogueComponent;
}

