#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SenrenbankaDialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct FLLMChatMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "LLM")
	FString Role; // "system" / "user" / "assistant"

	UPROPERTY(BlueprintReadWrite, Category = "LLM")
	FString Content;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLLMResponseReceived, const FString&, ResponseText);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SENRENBANKA_API USenrenbankaDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USenrenbankaDialogueComponent();

	UPROPERTY(BlueprintAssignable, Category = "LLM")
	FOnLLMResponseReceived OnLLMResponse;

	// LLM 配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString ApiUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString ApiKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString ModelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	FString SystemPrompt;

	// 历史消息列表
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLM")
	TArray<FLLMChatMessage> MessageHistory;

	// 历史截断上限（按总字符数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM")
	int32 MaxTotalChars;

	// 最近一次解析出的好感度变化（-5~5）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LLM")
	float LastAffinityDelta = 0.f;

	// 请求接口
	UFUNCTION(BlueprintCallable, Category = "LLM")
	void RequestLLMResponse(const FString& UserText, const FString& NPCName);

	// 重置对话（清空历史）
	UFUNCTION(BlueprintCallable, Category = "LLM")
	void ResetConversation();

	// 读取最近一次的好感度增量
	UFUNCTION(BlueprintPure, Category = "LLM")
	float GetLastAffinityDelta() const;

protected:
	// 最近一次请求的缓存数据
	UPROPERTY()
	FString CachedUserText;

	UPROPERTY()
	FString CachedNPCName;

	FTimerHandle FakeResponseTimerHandle;

	UFUNCTION()
	void HandleFakeResponse();

	// HTTP 完成回调（仅 C++ 内部使用，具体实现放在 cpp 中）
	void HandleHttpRequestCompleted(bool bWasSuccessful, int32 StatusCode, const FString& ResponseBody);

	// 解析 [AFFINITY_DELTA:n] 标签并清理回复文本
	bool ExtractAffinityDeltaAndCleanReply(const FString& RawReply, FString& OutCleanReply, float& OutAffinityDelta) const;
};

