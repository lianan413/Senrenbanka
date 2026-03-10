#include "SenrenbankaDialogueComponent.h"

#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Algo/Reverse.h"

using FHttpRequestPtr = TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>;
using FHttpResponsePtr = TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>;

USenrenbankaDialogueComponent::USenrenbankaDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	MaxTotalChars = 6000;
	LastAffinityDelta = 0.f;

	if (ApiUrl.IsEmpty())
	{
		ApiUrl = TEXT("http://localhost:1234/v1/chat/completions");
	}

	if (ModelName.IsEmpty())
	{
		ModelName = TEXT("qwen2.5-7b");
	}

	if (SystemPrompt.IsEmpty())
	{
		SystemPrompt = TEXT("你是一个温柔的日常系女主角，会用简洁自然的中文与玩家对话。");
	}
}

void USenrenbankaDialogueComponent::ResetConversation()
{
	MessageHistory.Empty();
	LastAffinityDelta = 0.f;
	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Conversation reset."));
}

float USenrenbankaDialogueComponent::GetLastAffinityDelta() const
{
	return LastAffinityDelta;
}

void USenrenbankaDialogueComponent::RequestLLMResponse(const FString& UserText, const FString& NPCName)
{
	if (ApiUrl.IsEmpty())
	{
		const FString ErrorText = TEXT("（对话服务未配置：ApiUrl 为空。）");
		OnLLMResponse.Broadcast(ErrorText);
		return;
	}

	CachedUserText = UserText;
	CachedNPCName = NPCName;

	// === 多轮对话：将本轮 user 消息写入历史 ===
	if (MessageHistory.Num() == 0)
	{
		FLLMChatMessage SystemMsg;
		SystemMsg.Role = TEXT("system");
		SystemMsg.Content = FString::Printf(
			TEXT("%s\n当前对话对象：%s。请始终保持该角色的第一人称语气，用自然中文回答。"),
			*SystemPrompt,
			*NPCName);
		SystemMsg.Content += TEXT("\n每次回答时，请先输出自然的中文正文。\n然后在最后单独输出一行，格式必须严格为：\n[AFFINITY_DELTA:n]\n其中 n 必须是一个整数，范围只能是 -5 到 5。\n不要对这个标签做解释，不要输出多余括号或中文说明。");
		MessageHistory.Add(SystemMsg);
	}

	{
		FLLMChatMessage UserMsg;
		UserMsg.Role = TEXT("user");
		UserMsg.Content = UserText;
		MessageHistory.Add(UserMsg);
	}

	// === 上下文截断：按字符总长度限制 ===
	TArray<FLLMChatMessage> TrimmedMessages;
	int32 TotalChars = 0;

	for (int32 Index = MessageHistory.Num() - 1; Index >= 0; --Index)
	{
		const FLLMChatMessage& Msg = MessageHistory[Index];
		const int32 Len = Msg.Content.Len();

		if (TotalChars + Len > MaxTotalChars)
		{
			break;
		}

		TotalChars += Len;
		TrimmedMessages.Add(Msg);
	}

	Algo::Reverse(TrimmedMessages);

	// 如需永久丢弃过旧历史，可取消下面的注释：
	// MessageHistory = TrimmedMessages;

	// === 构造 JSON 请求体 ===
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ApiUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	if (!ApiKey.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("model"), ModelName);

	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	for (const FLLMChatMessage& Msg : TrimmedMessages)
	{
		TSharedRef<FJsonObject> MsgObj = MakeShared<FJsonObject>();
		MsgObj->SetStringField(TEXT("role"), Msg.Role);
		MsgObj->SetStringField(TEXT("content"), Msg.Content);

		MessagesArray.Add(MakeShared<FJsonValueObject>(MsgObj));
	}

	RootObject->SetArrayField(TEXT("messages"), MessagesArray);
	RootObject->SetNumberField(TEXT("temperature"), 0.7);

	FString BodyString;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&BodyString);
	FJsonSerializer::Serialize(RootObject, Writer);

	Request->SetContentAsString(BodyString);

	// 使用 Lambda 适配 HTTP 回调类型
	Request->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bWasSuccessful)
		{
			int32 StatusCode = 0;
			FString Body;
			if (Resp.IsValid())
			{
				StatusCode = Resp->GetResponseCode();
				Body = Resp->GetContentAsString();
			}
			HandleHttpRequestCompleted(bWasSuccessful, StatusCode, Body);
		});

	Request->ProcessRequest();
}

void USenrenbankaDialogueComponent::HandleFakeResponse()
{
	// 不再使用假回复逻辑，此函数保留仅为兼容性占位
}

void USenrenbankaDialogueComponent::HandleHttpRequestCompleted(bool bWasSuccessful, int32 StatusCode, const FString& ResponseString)
{
	if (!bWasSuccessful)
	{
		OnLLMResponse.Broadcast(TEXT("（对话服务连接失败，请稍后再试。）"));
		return;
	}

	if (StatusCode < 200 || StatusCode >= 300)
	{
		const FString ErrorText = FString::Printf(
			TEXT("（对话服务返回错误：HTTP %d）%s"),
			StatusCode,
			*ResponseString);
		OnLLMResponse.Broadcast(ErrorText);
		return;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		OnLLMResponse.Broadcast(TEXT("（对话服务响应解析失败。）"));
		return;
	}

	FString Reply;

	const TArray<TSharedPtr<FJsonValue>>* ChoicesArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray && ChoicesArray->Num() > 0)
	{
		const TSharedPtr<FJsonObject> FirstChoiceObject = (*ChoicesArray)[0]->AsObject();
		if (FirstChoiceObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* MessageObjectPtr = nullptr;
			if (FirstChoiceObject->TryGetObjectField(TEXT("message"), MessageObjectPtr) && MessageObjectPtr && MessageObjectPtr->IsValid())
			{
				(*MessageObjectPtr)->TryGetStringField(TEXT("content"), Reply);
			}
		}
	}

	// 退路：有些实现直接在根上给 content
	if (Reply.IsEmpty())
	{
		RootObject->TryGetStringField(TEXT("content"), Reply);
	}

	if (Reply.IsEmpty())
	{
		OnLLMResponse.Broadcast(TEXT("（未能从模型响应中提取回复内容。）"));
		return;
	}

	// 解析好感度标签并清理正文
	FString CleanReply;
	float ParsedAffinityDelta = 0.f;
	ExtractAffinityDeltaAndCleanReply(Reply, CleanReply, ParsedAffinityDelta);
	LastAffinityDelta = ParsedAffinityDelta;

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent::HandleHttpRequestCompleted RawReply=\"%s\" CleanReply=\"%s\" AffinityDelta=%.2f"),
		*Reply, *CleanReply, LastAffinityDelta);

	// 将模型回复写入历史（使用清理后的文本）
	FLLMChatMessage AssistantMsg;
	AssistantMsg.Role = TEXT("assistant");
	AssistantMsg.Content = CleanReply;
	MessageHistory.Add(AssistantMsg);

	OnLLMResponse.Broadcast(CleanReply);
}

bool USenrenbankaDialogueComponent::ExtractAffinityDeltaAndCleanReply(const FString& RawReply, FString& OutCleanReply, float& OutAffinityDelta) const
{
	OutCleanReply = RawReply;
	OutAffinityDelta = 0.f;

	const FString TagPrefix = TEXT("[AFFINITY_DELTA:");

	int32 StartIndex = RawReply.Find(TagPrefix, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	if (StartIndex == INDEX_NONE)
	{
		return false;
	}

	int32 SearchFrom = StartIndex + TagPrefix.Len();
	int32 EndIndex = RawReply.Find(TEXT("]"), ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchFrom);
	if (EndIndex == INDEX_NONE || EndIndex <= SearchFrom)
	{
		return false;
	}

	const int32 ValueLen = EndIndex - SearchFrom;
	FString NumberStr = RawReply.Mid(SearchFrom, ValueLen).TrimStartAndEnd();

	float ParsedValue = 0.f;
	if (!LexTryParseString(ParsedValue, *NumberStr))
	{
		return false;
	}

	OutAffinityDelta = FMath::Clamp(ParsedValue, -5.f, 5.f);

	// 移除标签部分
	FString Clean = RawReply.Left(StartIndex) + RawReply.Mid(EndIndex + 1);
	Clean.TrimStartAndEndInline();
	OutCleanReply = Clean;

	return true;
}


