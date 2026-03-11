#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SenrenbankaItemTypes.h"
#include "SenrenbankaItemDataAsset.h"
#include "SenrenbankaInventoryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SENRENBANKA_API USenrenbankaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USenrenbankaInventoryComponent();

	// 背包条目数组
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FSenrenbankaInventoryEntry> Items;

	// 最大槽位数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxSlots = 32;

	// 直接通过 ItemId 添加物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemById(FName ItemId, int32 Count, bool bStackable, int32 MaxStack);

	// 从 DataAsset 添加物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemFromDataAsset(USenrenbankaItemDataAsset* ItemData, int32 Count);

	// 获取某个 ItemId 的总数量
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	// 移除指定数量的某物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemById(FName ItemId, int32 Count);

	// 是否至少拥有指定数量
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(FName ItemId, int32 Count) const;

	// 复制当前背包条目（用于存档）
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FSenrenbankaInventoryEntry> GetItemsCopy() const;

	// 从存档恢复背包条目
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void LoadItemsFromSave(const TArray<FSenrenbankaInventoryEntry>& InItems);
};

