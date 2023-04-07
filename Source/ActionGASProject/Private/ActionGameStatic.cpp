// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameStatic.h"

const UItemStaticData* UActionGameStatic::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{
	if (IsValid(ItemDataClass))
	{
		// 在Unreal Engine 4中，GetDefaultObject()函数用于获取一个类的默认对象，这个默认对象在编译时被创建，并且在整个游戏实例中只有一个。
		// 每次调用GetDefaultObject()函数时，它都会返回同一个默认对象的引用，而不是生成新的对象。
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}
