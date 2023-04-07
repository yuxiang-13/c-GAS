// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 让这个文件支持反射
#include "InventoryList.generated.h"


class UInventoryItemInstance;
class UItemStaticData;


/* TArray可以支持基本网络同步，但是不保证正确同步，所以要重定义其 序列化和反序列化
 * 所以，我们要用让数组 派生自 FFastArraySerializerItem 或 FFastArraySerializer
 *
 *如果想要在网络上传输单个TArray元素，则需要 先创建一个继承自FFastArraySerializerItem的类，
 *并在其中实现序列化Serialize函数和反序列化Deserialize函数并将它们绑定到TArray元素的数据类型上。
 *
 *同理 要在网络上传输整个TArray，则需要创建一个继承自FFastArraySerializer的类
 *
 *需要注意的是，FFastArraySerializer 与普通的 TArray 数据类型不同，它没有提供一些常用的数组操作方法。
 *所以，在使用时，需要在自定义结构体类中自行添加需要的方法来操作数据
 */

// 背包 元素
USTRUCT(BlueprintType)
struct FInventoryListItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()
public:
	
	UPROPERTY()
	UInventoryItemInstance* ItemInstance = nullptr;
};

// 背包 TAttay
USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()
public:
	//序列化
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		// <元素, 结构体名>
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryListItem, FInventoryList>( Items, DeltaParms, *this );
	}

	// 需要注意的是，FFastArraySerializer 与普通的 TArray 数据类型不同，它没有提供一些常用的数组操作方法。
	// 所以，在使用时，需要在自定义结构体类中自行添加需要的方法来操作数据

	void AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);
	void RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	TArray<FInventoryListItem>& GetItemsRef();
protected:

	// 属性加 UPROPERTY 标签 才能序列化，所以 这里必须加，否则FFastArraySerializer::FastArrayDeltaSerialize<FInventoryListItem, FInventoryList>( Items, DeltaParms, *this ); 无法通过
	UPROPERTY()
	TArray<FInventoryListItem> Items;
};


// 在UE中，结构体类型模板特征（TStructOpsTypeTraits）用于为特定类型的结构体   提供自定义的序列化、反序列化和其他操作。
template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { withNetDeltaSerializer = true };
};

