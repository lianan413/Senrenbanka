#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AffectionComponent.h"
#include "SenrenbankaGameState.h"
#include "LLMNPCDialogueComponent.generated.h"

class ULLMService;
class ASenrenbankaNPCBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLLMResponse, const FString&, PlayerText, const FString&, NPCText);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SENRENBANKA_API ULLMNPCDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLMNPCDialogueComponent();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FText SystemPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	bool bIncludeTimeOfDayInPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	bool bIncludeAffectionInPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	int32 MaxHistoryEntries;

	TArray<FString> ConversationHistory;

	bool bIsRequestInFlight;

	UAffectionComponent* CachedAffection;

	ASenrenbankaGameState* CachedGameState;

	ASenrenbankaNPCBase* CachedNPCOwner;

	UPROPERTY(BlueprintAssignable, Category = "LLM")
	FOnLLMResponse OnLLMResponse;

	UFUNCTION(BlueprintCallable, Category = "LLM")
	void ResetConversation();

	UFUNCTION(BlueprintCallable, Category = "LLM")
	void SendPlayerText(const FString& PlayerText);

private:
	FString BuildSystemPrompt() const;

	void ParseAffectionTag(const FString& RawResponse, FString& OutCleanText, float& OutAffectionDelta) const;
};

