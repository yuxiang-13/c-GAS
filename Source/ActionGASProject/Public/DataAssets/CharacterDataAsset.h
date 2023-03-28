// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

/**
 * 
 */
// BlueprintType是用于声明C++的结构体、枚举、类等是允许在蓝图（Blueprint）中使用的。
// 具有BlueprintType修饰符的C++类型可以在蓝图中使用，并且可以直接在蓝图中实例化使用。
// 
//Blueprintable是用于声明C++的类是允许在蓝图中派生的。
//如果一个C++类使用了Blueprintable修饰符，那么该类可以被蓝图继承，并且可以在蓝图中创建和实例化对象。
UCLASS(BlueprintType, Blueprintable)
class ACTIONGASPROJECT_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FCharacterData CharacterData;
};
