#include "LLMService.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

ULLMService::ULLMService()
{
	ApiBaseUrl = TEXT("http://127.0.0.1:1234/v1/chat/completions");
	ModelName = TEXT("qwen2.5-7b");
	ApiKey = TEXT("");
}

void ULLMService::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ULLMService initialized. BaseUrl=%s, Model=%s"), *ApiBaseUrl, *ModelName);
}

void ULLMService::SendChatRequest(
	const FString& SystemPrompt,
	const TArray<FString>& ConversationHistory,
	const FString& UserInput,
	TFunction<void(bool bSuccess, const FString& AssistantText)> Callback)
{
	if (ApiBaseUrl.IsEmpty())
	{
		if (Callback)
		{
			Callback(false, TEXT(""));
		}
		return;
	}

	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("model"), ModelName);

	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	// System message
	{
		TSharedRef<FJsonObject> SystemMessage = MakeShared<FJsonObject>();
		SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
		SystemMessage->SetStringField(TEXT("content"), SystemPrompt);
		MessagesArray.Add(MakeShared<FJsonValueObject>(SystemMessage));
	}

	// User message: history + current input
	FString CombinedHistory = FString::Join(ConversationHistory, TEXT("\n"));
	FString UserContent = CombinedHistory;
	if (!UserInput.IsEmpty())
	{
		if (!UserContent.IsEmpty())
		{
			UserContent += TEXT("\n");
		}
		UserContent += UserInput;
	}

	{
		TSharedRef<FJsonObject> UserMessage = MakeShared<FJsonObject>();
		UserMessage->SetStringField(TEXT("role"), TEXT("user"));
		UserMessage->SetStringField(TEXT("content"), UserContent);
		MessagesArray.Add(MakeShared<FJsonValueObject>(UserMessage));
	}

	RootObject->SetArrayField(TEXT("messages"), MessagesArray);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(RootObject, Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ApiBaseUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	if (!ApiKey.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	}

	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!Callback)
			{
				return;
			}

			if (!bWasSuccessful || !Response.IsValid())
			{
				Callback(false, TEXT(""));
				return;
			}

			const int32 StatusCode = Response->GetResponseCode();
			if (StatusCode != 200)
			{
				Callback(false, TEXT(""));
				return;
			}

			const FString ResponseString = Response->GetContentAsString();

			TSharedPtr<FJsonObject> JsonRoot;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
			if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
			{
				Callback(false, TEXT(""));
				return;
			}

			const TArray<TSharedPtr<FJsonValue>>* ChoicesArray = nullptr;
			if (!JsonRoot->TryGetArrayField(TEXT("choices"), ChoicesArray) || !ChoicesArray || ChoicesArray->Num() == 0)
			{
				Callback(false, TEXT(""));
				return;
			}

			TSharedPtr<FJsonObject> ChoiceObject = (*ChoicesArray)[0]->AsObject();
			if (!ChoiceObject.IsValid())
			{
				Callback(false, TEXT(""));
				return;
			}

			TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField(TEXT("message"));
			if (!MessageObject.IsValid())
			{
				Callback(false, TEXT(""));
				return;
			}

			FString AssistantText;
			if (!MessageObject->TryGetStringField(TEXT("content"), AssistantText))
			{
				Callback(false, TEXT(""));
				return;
			}

			Callback(true, AssistantText);
		});

	Request->ProcessRequest();
}

