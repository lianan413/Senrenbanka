#include "LLMNPCDialogueComponent.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "LLMService.h"
#include "SenrenbankaNPCBase.h"

ULLMNPCDialogueComponent::ULLMNPCDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIncludeTimeOfDayInPrompt = true;
	bIncludeAffectionInPrompt = true;
	MaxHistoryEntries = 10;
	bIsRequestInFlight = false;

	CachedAffection = nullptr;
	CachedGameState = nullptr;
	CachedNPCOwner = nullptr;
}

void ULLMNPCDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedNPCOwner = Cast<ASenrenbankaNPCBase>(GetOwner());
	if (CachedNPCOwner)
	{
		CachedAffection = CachedNPCOwner->GetAffectionComponent();
	}

	if (UWorld* World = GetWorld())
	{
		CachedGameState = World->GetGameState<ASenrenbankaGameState>();
	}
}

void ULLMNPCDialogueComponent::ResetConversation()
{
	ConversationHistory.Empty();
	bIsRequestInFlight = false;
}

void ULLMNPCDialogueComponent::SendPlayerText(const FString& PlayerText)
{
	if (PlayerText.IsEmpty() || bIsRequestInFlight)
	{
		return;
	}

	ConversationHistory.Add(FString::Printf(TEXT("Player: %s"), *PlayerText));
	if (MaxHistoryEntries > 0)
	{
		while (ConversationHistory.Num() > MaxHistoryEntries)
		{
			ConversationHistory.RemoveAt(0);
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		OnLLMResponse.Broadcast(PlayerText, TEXT("World not available."));
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		OnLLMResponse.Broadcast(PlayerText, TEXT("GameInstance not available."));
		return;
	}

	ULLMService* Service = GameInstance->GetSubsystem<ULLMService>();
	if (!Service)
	{
		OnLLMResponse.Broadcast(PlayerText, TEXT("LLM service not available."));
		return;
	}

	const FString SystemPrompt = BuildSystemPrompt();

	bIsRequestInFlight = true;

	TWeakObjectPtr<ULLMNPCDialogueComponent> WeakThis(this);

	Service->SendChatRequest(
		SystemPrompt,
		ConversationHistory,
		PlayerText,
		[WeakThis, PlayerText](bool bSuccess, const FString& AssistantText)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			ULLMNPCDialogueComponent* StrongThis = WeakThis.Get();
			StrongThis->bIsRequestInFlight = false;

			if (!bSuccess || AssistantText.IsEmpty())
			{
				StrongThis->OnLLMResponse.Broadcast(PlayerText, TEXT("LLM request failed."));
				return;
			}

			FString CleanText;
			float AffectionDelta = 0.f;
			StrongThis->ParseAffectionTag(AssistantText, CleanText, AffectionDelta);

			if (StrongThis->CachedAffection && !FMath::IsNearlyZero(AffectionDelta))
			{
				StrongThis->CachedAffection->AddAffection(AffectionDelta);
			}

			if (!CleanText.IsEmpty())
			{
				StrongThis->ConversationHistory.Add(FString::Printf(TEXT("NPC: %s"), *CleanText));

				if (StrongThis->MaxHistoryEntries > 0)
				{
					while (StrongThis->ConversationHistory.Num() > StrongThis->MaxHistoryEntries)
					{
						StrongThis->ConversationHistory.RemoveAt(0);
					}
				}
			}

			StrongThis->OnLLMResponse.Broadcast(PlayerText, CleanText);
		});
}

FString ULLMNPCDialogueComponent::BuildSystemPrompt() const
{
	FString Prompt = SystemPromptText.ToString();

	if (bIncludeTimeOfDayInPrompt)
	{
		FString SegmentText = TEXT("Unknown");

		if (CachedGameState)
		{
			switch (CachedGameState->GetCurrentTimeSegment())
			{
			case ETimeOfDaySegment::Morning:
				SegmentText = TEXT("Morning");
				break;
			case ETimeOfDaySegment::Noon:
				SegmentText = TEXT("Noon");
				break;
			case ETimeOfDaySegment::Evening:
				SegmentText = TEXT("Evening");
				break;
			case ETimeOfDaySegment::Night:
				SegmentText = TEXT("Night");
				break;
			default:
				break;
			}
		}

		Prompt += FString::Printf(TEXT("\n[Context] Current time of day segment: %s."), *SegmentText);
	}

	if (bIncludeAffectionInPrompt)
	{
		FString AffectionText = TEXT("Unknown");

		if (CachedAffection)
		{
			switch (CachedAffection->GetAffectionLevel())
			{
			case EAffectionLevel::Cold:
				AffectionText = TEXT("Cold");
				break;
			case EAffectionLevel::Neutral:
				AffectionText = TEXT("Neutral");
				break;
			case EAffectionLevel::Warm:
				AffectionText = TEXT("Warm");
				break;
			case EAffectionLevel::Intimate:
				AffectionText = TEXT("Intimate");
				break;
			default:
				break;
			}
		}

		Prompt += FString::Printf(TEXT("\n[Context] Current affection level: %s."), *AffectionText);
	}

	Prompt += TEXT("\n[Instruction] At the end of each reply, append one of [AFF_UP_SMALL], [AFF_UP_BIG], [AFF_DOWN_SMALL], [AFF_DOWN_BIG], or [AFF_NONE] to indicate how the player's affection should change.");

	return Prompt;
}

void ULLMNPCDialogueComponent::ParseAffectionTag(const FString& RawResponse, FString& OutCleanText, float& OutAffectionDelta) const
{
	FString Text = RawResponse;
	OutAffectionDelta = 0.f;

	auto HandleTag = [&Text, &OutAffectionDelta](const FString& Tag, float Delta)
	{
		if (Text.Contains(Tag))
		{
			Text = Text.Replace(*Tag, TEXT(""), ESearchCase::IgnoreCase);
			OutAffectionDelta = Delta;
			return true;
		}
		return false;
	};

	if (!HandleTag(TEXT("[AFF_UP_BIG]"), 3.f))
	{
		if (!HandleTag(TEXT("[AFF_UP_SMALL]"), 1.f))
		{
			if (!HandleTag(TEXT("[AFF_DOWN_BIG]"), -3.f))
			{
				if (!HandleTag(TEXT("[AFF_DOWN_SMALL]"), -1.f))
				{
					HandleTag(TEXT("[AFF_NONE]"), 0.f);
				}
			}
		}
	}

	Text.TrimStartAndEndInline();
	OutCleanText = Text;
}

