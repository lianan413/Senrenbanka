#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LLMService.generated.h"

UCLASS()
class SENRENBANKA_API ULLMService : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULLMService();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void SendChatRequest(
		const FString& SystemPrompt,
		const TArray<FString>& ConversationHistory,
		const FString& UserInput,
		TFunction<void(bool bSuccess, const FString& AssistantText)> Callback);

private:
	FString ApiBaseUrl;
	FString ModelName;
	FString ApiKey;
};

