#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SenrenbankaSaveTypes.h"
#include "SenrenbankaSaveSubsystem.generated.h"

class USenrenbankaSaveGame;

UCLASS()
class SENRENBANKA_API USenrenbankaSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY()
	TObjectPtr<USenrenbankaSaveGame> CurrentSaveObject;

	UPROPERTY()
	TObjectPtr<USenrenbankaSaveGame> PendingLoadedSaveObject;

	UPROPERTY()
	bool bHasPendingLoadedData = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString CurrentSlotName = TEXT("MainSlot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CurrentUserIndex = 0;

public:
	UFUNCTION(BlueprintCallable, Category = "Save")
	void CreateNewSave();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasSaveGame() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteCurrentSave();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadCurrentGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool ApplyPendingLoadedState();

	// 通用键值接口
	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	void SetFloatValue(FName Key, float Value);

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	bool GetFloatValue(FName Key, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	void SetIntValue(FName Key, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	bool GetIntValue(FName Key, int32& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	void SetBoolValue(FName Key, bool Value);

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	bool GetBoolValue(FName Key, bool& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	void SetStringValue(FName Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Save|Data")
	bool GetStringValue(FName Key, FString& OutValue) const;

	// 辅助读取
	UFUNCTION(BlueprintPure, Category = "Save")
	FSenrenbankaTimeSaveData GetSavedTimeData() const;

	UFUNCTION(BlueprintPure, Category = "Save")
	FString GetSavedMapName() const;

protected:
	void EnsureCurrentSaveObject();

	bool GetPlayerCurrentHP(float& OutHP) const;
	void SetPlayerCurrentHP(float NewHP) const;
};

