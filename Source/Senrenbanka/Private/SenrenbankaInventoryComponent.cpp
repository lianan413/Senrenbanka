#include "SenrenbankaInventoryComponent.h"

USenrenbankaInventoryComponent::USenrenbankaInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxSlots = 32;
}

bool USenrenbankaInventoryComponent::AddItemById(FName ItemId, int32 Count, bool bStackable, int32 MaxStack)
{
	if (Count <= 0 || ItemId.IsNone())
	{
		return false;
	}

	if (MaxStack <= 0)
	{
		MaxStack = 1;
	}

	int32 Remaining = Count;
	bool bAddedAny = false;

	// 堆叠逻辑
	if (bStackable)
	{
		// 先填充已有堆
		for (FSenrenbankaInventoryEntry& Entry : Items)
		{
			if (Entry.ItemId == ItemId && Entry.Count < MaxStack && Remaining > 0)
			{
				const int32 Space = MaxStack - Entry.Count;
				const int32 ToAdd = FMath::Min(Space, Remaining);
				Entry.Count += ToAdd;
				Remaining -= ToAdd;
				bAddedAny = bAddedAny || ToAdd > 0;

				if (Remaining <= 0)
				{
					break;
				}
			}
		}

		// 再创建新格子
		while (Remaining > 0 && Items.Num() < MaxSlots)
		{
			const int32 ToAdd = FMath::Min(MaxStack, Remaining);
			FSenrenbankaInventoryEntry NewEntry;
			NewEntry.ItemId = ItemId;
			NewEntry.Count = ToAdd;
			Items.Add(NewEntry);

			Remaining -= ToAdd;
			bAddedAny = true;
		}
	}
	else
	{
		// 非堆叠：每格一个
		while (Remaining > 0 && Items.Num() < MaxSlots)
		{
			FSenrenbankaInventoryEntry NewEntry;
			NewEntry.ItemId = ItemId;
			NewEntry.Count = 1;
			Items.Add(NewEntry);

			--Remaining;
			bAddedAny = true;
		}
	}

	return bAddedAny;
}

bool USenrenbankaInventoryComponent::AddItemFromDataAsset(USenrenbankaItemDataAsset* ItemData, int32 Count)
{
	if (!ItemData || Count <= 0)
	{
		return false;
	}

	const bool bResult = AddItemById(ItemData->ItemId, Count, ItemData->bStackable, ItemData->MaxStack);

	UE_LOG(LogTemp, Log, TEXT("Inventory: AddItemFromDataAsset ItemId=%s Count=%d Result=%s"),
		*ItemData->ItemId.ToString(),
		Count,
		bResult ? TEXT("Success") : TEXT("Failed"));

	return bResult;
}

int32 USenrenbankaInventoryComponent::GetItemCount(FName ItemId) const
{
	int32 Total = 0;
	if (ItemId.IsNone())
	{
		return 0;
	}

	for (const FSenrenbankaInventoryEntry& Entry : Items)
	{
		if (Entry.ItemId == ItemId)
		{
			Total += Entry.Count;
		}
	}

	return Total;
}

bool USenrenbankaInventoryComponent::RemoveItemById(FName ItemId, int32 Count)
{
	if (Count <= 0 || ItemId.IsNone())
	{
		return false;
	}

	if (GetItemCount(ItemId) < Count)
	{
		UE_LOG(LogTemp, Log, TEXT("Inventory: RemoveItemById ItemId=%s Count=%d Result=Failed (not enough)"),
			*ItemId.ToString(), Count);
		return false;
	}

	int32 Remaining = Count;

	for (int32 Index = 0; Index < Items.Num() && Remaining > 0; ++Index)
	{
		FSenrenbankaInventoryEntry& Entry = Items[Index];
		if (Entry.ItemId != ItemId)
		{
			continue;
		}

		const int32 ToRemove = FMath::Min(Entry.Count, Remaining);
		Entry.Count -= ToRemove;
		Remaining -= ToRemove;

		if (Entry.Count <= 0)
		{
			Items.RemoveAt(Index);
			--Index;
		}
	}

	const bool bSuccess = (Remaining == 0);

	UE_LOG(LogTemp, Log, TEXT("Inventory: RemoveItemById ItemId=%s Count=%d Result=%s"),
		*ItemId.ToString(),
		Count,
		bSuccess ? TEXT("Success") : TEXT("Failed"));

	return bSuccess;
}

bool USenrenbankaInventoryComponent::HasItem(FName ItemId, int32 Count) const
{
	if (Count <= 0 || ItemId.IsNone())
	{
		return false;
	}

	return GetItemCount(ItemId) >= Count;
}

TArray<FSenrenbankaInventoryEntry> USenrenbankaInventoryComponent::GetItemsCopy() const
{
	return Items;
}

void USenrenbankaInventoryComponent::LoadItemsFromSave(const TArray<FSenrenbankaInventoryEntry>& InItems)
{
	Items = InItems;
	UE_LOG(LogTemp, Log, TEXT("Inventory: LoadItemsFromSave Count=%d"), Items.Num());
}

