// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryList.h"

#include "ActionGameTypes.h"
#include "Inventory/InventoryItemInstance.h"

void FInventoryList::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	// AddDefaulted_GetRef 在数组的末尾添加一个新项 并返回引用
	FInventoryListItem& Item = Items.AddDefaulted_GetRef();

	// 创建 背包元素并 赋予 数组元素指针 
	Item.ItemInstance = NewObject<UInventoryItemInstance>();
	// 初始化 背包元素 
	Item.ItemInstance->Init(InItemStaticDataClass);

	// 数组元素 标记为脏数据
	// 如果在数组中添加或更改项，则必须调用此函数
	MarkItemDirty(Item);
}

void FInventoryList::AddItem(UInventoryItemInstance* InItemInstance)
{
	// AddDefaulted_GetRef 在数组的末尾添加一个新项 并返回引用
	FInventoryListItem& Item = Items.AddDefaulted_GetRef();
	Item.ItemInstance = InItemInstance;
	// 数组元素 标记为脏数据
	// 如果在数组中添加或更改项，则必须调用此函数
	MarkItemDirty(Item);
}

void FInventoryList::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	for (auto ItemIter = Items.CreateIterator(); ItemIter; ++ItemIter)
	{
		FInventoryListItem& Item = *ItemIter;
		if (Item.ItemInstance && Item.ItemInstance->GetItemStaticData()->IsA(InItemStaticDataClass))
		{
			// 删除
			ItemIter.RemoveCurrent();
			
			// 数组 标记为脏数据
			// 如果在数组中添加或更改项，则必须调用此函数
			MarkArrayDirty();
			break;
		}
	}
}

TArray<FInventoryListItem>& FInventoryList::GetItemsRef()
{
	return Items;
}
