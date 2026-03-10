#include "SenrenbankaSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/UnrealType.h"
#include "SenrenbankaSaveGame.h"
#include "SenrenbankaGameState.h"
#include "SenrenbankaCharacter.h"

void USenrenbankaSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentSaveObject)
	{
		CurrentSaveObject = NewObject<USenrenbankaSaveGame>(this);
	}

	if (CurrentSaveObject)
	{
		CurrentSaveObject->SlotName = CurrentSlotName;
		CurrentSaveObject->UserIndex = CurrentUserIndex;
	}
}

void USenrenbankaSaveSubsystem::EnsureCurrentSaveObject()
{
	if (!CurrentSaveObject)
	{
		CurrentSaveObject = NewObject<USenrenbankaSaveGame>(this);
		if (CurrentSaveObject)
		{
			CurrentSaveObject->SlotName = CurrentSlotName;
			CurrentSaveObject->UserIndex = CurrentUserIndex;
		}
	}
}

void USenrenbankaSaveSubsystem::CreateNewSave()
{
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::CreateNewSave"));

	CurrentSaveObject = NewObject<USenrenbankaSaveGame>(this);
	if (CurrentSaveObject)
	{
		CurrentSaveObject->SlotName = CurrentSlotName;
		CurrentSaveObject->UserIndex = CurrentUserIndex;
		CurrentSaveObject->SavedMapName = TEXT("");
		CurrentSaveObject->PlayerTransform = FTransform::Identity;
		CurrentSaveObject->PlayerCurrentHP = 100.f;
		CurrentSaveObject->TimeData = FSenrenbankaTimeSaveData();

		CurrentSaveObject->FloatData.Empty();
		CurrentSaveObject->IntData.Empty();
		CurrentSaveObject->BoolData.Empty();
		CurrentSaveObject->StringData.Empty();
	}

	PendingLoadedSaveObject = nullptr;
	bHasPendingLoadedData = false;
}

bool USenrenbankaSaveSubsystem::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(CurrentSlotName, CurrentUserIndex);
}

bool USenrenbankaSaveSubsystem::DeleteCurrentSave()
{
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::DeleteCurrentSave Slot=%s Index=%d"), *CurrentSlotName, CurrentUserIndex);

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(CurrentSlotName, CurrentUserIndex);
	if (bDeleted)
	{
		CreateNewSave();
	}
	return bDeleted;
}

bool USenrenbankaSaveSubsystem::GetPlayerCurrentHP(float& OutHP) const
{
	if (UWorld* World = GetWorld())
	{
		if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			if (FFloatProperty* HPProp = FindFProperty<FFloatProperty>(PlayerChar->GetClass(), TEXT("CurrentHP")))
			{
				const void* PropAddr = HPProp->ContainerPtrToValuePtr<void>(PlayerChar);
				OutHP = HPProp->GetFloatingPointPropertyValue(PropAddr);
				return true;
			}
		}
	}

	return false;
}

void USenrenbankaSaveSubsystem::SetPlayerCurrentHP(float NewHP) const
{
	if (UWorld* World = GetWorld())
	{
		if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0))
		{
			if (FFloatProperty* HPProp = FindFProperty<FFloatProperty>(PlayerChar->GetClass(), TEXT("CurrentHP")))
			{
				void* PropAddr = HPProp->ContainerPtrToValuePtr<void>(PlayerChar);
				HPProp->SetFloatingPointPropertyValue(PropAddr, NewHP);
			}
		}
	}
}

bool USenrenbankaSaveSubsystem::SaveCurrentGame()
{
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::SaveCurrentGame Slot=%s Index=%d"), *CurrentSlotName, CurrentUserIndex);

	EnsureCurrentSaveObject();
	if (!CurrentSaveObject)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 地图名
	CurrentSaveObject->SavedMapName = UGameplayStatics::GetCurrentLevelName(World, true);

	// 玩家 Transform + 战斗属性
	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0))
	{
		CurrentSaveObject->PlayerTransform = PlayerChar->GetActorTransform();

		if (const ASenrenbankaCharacter* SRCharacter = Cast<ASenrenbankaCharacter>(PlayerChar))
		{
			CurrentSaveObject->CombatData = SRCharacter->GetCombatSaveDataForSave();
			// 兼容旧字段：PlayerCurrentHP
			CurrentSaveObject->PlayerCurrentHP = CurrentSaveObject->CombatData.CurrentHP;

			const FSenrenbankaCombatSaveData& C = CurrentSaveObject->CombatData;
			UE_LOG(LogTemp, Log, TEXT("SaveSubsystem::SaveCurrentGame Saved CombatData "
				"HP=%.1f BaseAtk=%.1f BaseCrit=%.2f BaseMaxHP=%.1f FinalAtk=%.1f FinalCrit=%.2f FinalMaxHP=%.1f"),
				C.CurrentHP, C.BaseAttack, C.BaseCritRate, C.BaseMaxHP,
				C.FinalAttack, C.FinalCritRate, C.FinalMaxHP);
		}
		else
		{
			// 无法 Cast 到项目角色，保持默认 CombatData，只兼容性写入 PlayerCurrentHP
			CurrentSaveObject->PlayerCurrentHP = 100.f;
		}
	}

	// 时间数据
	if (AGameStateBase* GS = World->GetGameState())
	{
		if (ASenrenbankaGameState* SBGameState = Cast<ASenrenbankaGameState>(GS))
		{
			CurrentSaveObject->TimeData = SBGameState->GetTimeSaveData();
		}
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(CurrentSaveObject, CurrentSlotName, CurrentUserIndex);
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::SaveCurrentGame result=%s"), bSaved ? TEXT("Success") : TEXT("Failure"));
	return bSaved;
}

bool USenrenbankaSaveSubsystem::LoadCurrentGame()
{
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::LoadCurrentGame Slot=%s Index=%d"), *CurrentSlotName, CurrentUserIndex);

	if (!HasSaveGame())
	{
		return false;
	}

	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(CurrentSlotName, CurrentUserIndex))
	{
		if (USenrenbankaSaveGame* LoadedSave = Cast<USenrenbankaSaveGame>(Loaded))
		{
			CurrentSaveObject = LoadedSave;
			PendingLoadedSaveObject = DuplicateObject<USenrenbankaSaveGame>(LoadedSave, this);
			bHasPendingLoadedData = (PendingLoadedSaveObject != nullptr);

			if (!CurrentSaveObject->SavedMapName.IsEmpty())
			{
				UGameplayStatics::OpenLevel(this, FName(*CurrentSaveObject->SavedMapName));
			}

			return true;
		}
	}

	return false;
}

bool USenrenbankaSaveSubsystem::ApplyPendingLoadedState()
{
	UE_LOG(LogTemp, Log, TEXT("SenrenbankaSaveSubsystem::ApplyPendingLoadedState"));

	if (!bHasPendingLoadedData || !PendingLoadedSaveObject)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bool bAppliedSomething = false;

	// 恢复玩家 Transform 和 HP
	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0))
	{
		PlayerChar->SetActorTransform(PendingLoadedSaveObject->PlayerTransform);

		if (ASenrenbankaCharacter* SRCharacter = Cast<ASenrenbankaCharacter>(PlayerChar))
		{
			const FSenrenbankaCombatSaveData& C = PendingLoadedSaveObject->CombatData;
			UE_LOG(LogTemp, Log, TEXT("SaveSubsystem::ApplyPendingLoadedState Applied CombatData "
				"HP=%.1f BaseAtk=%.1f BaseCrit=%.2f BaseMaxHP=%.1f FinalAtk=%.1f FinalCrit=%.2f FinalMaxHP=%.1f"),
				C.CurrentHP, C.BaseAttack, C.BaseCritRate, C.BaseMaxHP,
				C.FinalAttack, C.FinalCritRate, C.FinalMaxHP);

			SRCharacter->ApplyCombatSaveDataFromSave(C);
		}

		bAppliedSomething = true;
	}

	// 恢复时间数据
	if (AGameStateBase* GS = World->GetGameState())
	{
		if (ASenrenbankaGameState* SBGameState = Cast<ASenrenbankaGameState>(GS))
		{
			const FSenrenbankaTimeSaveData& TimeData = PendingLoadedSaveObject->TimeData;
			SBGameState->ApplyTimeSaveData(TimeData);
			bAppliedSomething = true;
		}
	}

	bHasPendingLoadedData = false;
	PendingLoadedSaveObject = nullptr;

	return bAppliedSomething;
}

// 通用键值接口

void USenrenbankaSaveSubsystem::SetFloatValue(FName Key, float Value)
{
	EnsureCurrentSaveObject();
	if (CurrentSaveObject)
	{
		CurrentSaveObject->FloatData.Add(Key, Value);
	}
}

bool USenrenbankaSaveSubsystem::GetFloatValue(FName Key, float& OutValue) const
{
	if (CurrentSaveObject)
	{
		if (const float* Found = CurrentSaveObject->FloatData.Find(Key))
		{
			OutValue = *Found;
			return true;
		}
	}
	return false;
}

void USenrenbankaSaveSubsystem::SetIntValue(FName Key, int32 Value)
{
	EnsureCurrentSaveObject();
	if (CurrentSaveObject)
	{
		CurrentSaveObject->IntData.Add(Key, Value);
	}
}

bool USenrenbankaSaveSubsystem::GetIntValue(FName Key, int32& OutValue) const
{
	if (CurrentSaveObject)
	{
		if (const int32* Found = CurrentSaveObject->IntData.Find(Key))
		{
			OutValue = *Found;
			return true;
		}
	}
	return false;
}

void USenrenbankaSaveSubsystem::SetBoolValue(FName Key, bool Value)
{
	EnsureCurrentSaveObject();
	if (CurrentSaveObject)
	{
		CurrentSaveObject->BoolData.Add(Key, Value);
	}
}

bool USenrenbankaSaveSubsystem::GetBoolValue(FName Key, bool& OutValue) const
{
	if (CurrentSaveObject)
	{
		if (const bool* Found = CurrentSaveObject->BoolData.Find(Key))
		{
			OutValue = *Found;
			return true;
		}
	}
	return false;
}

void USenrenbankaSaveSubsystem::SetStringValue(FName Key, const FString& Value)
{
	EnsureCurrentSaveObject();
	if (CurrentSaveObject)
	{
		CurrentSaveObject->StringData.Add(Key, Value);
	}
}

bool USenrenbankaSaveSubsystem::GetStringValue(FName Key, FString& OutValue) const
{
	if (CurrentSaveObject)
	{
		if (const FString* Found = CurrentSaveObject->StringData.Find(Key))
		{
			OutValue = *Found;
			return true;
		}
	}
	return false;
}

FSenrenbankaTimeSaveData USenrenbankaSaveSubsystem::GetSavedTimeData() const
{
	if (CurrentSaveObject)
	{
		return CurrentSaveObject->TimeData;
	}
	return FSenrenbankaTimeSaveData();
}

FString USenrenbankaSaveSubsystem::GetSavedMapName() const
{
	if (CurrentSaveObject)
	{
		return CurrentSaveObject->SavedMapName;
	}
	return FString();
}

