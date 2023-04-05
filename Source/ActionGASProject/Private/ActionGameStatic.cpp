// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameStatic.h"

const UItemStaticData* UActionGameStatic::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{
	if (IsValid(ItemDataClass))
	{
		// 创建并返回 默认的对象
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}
