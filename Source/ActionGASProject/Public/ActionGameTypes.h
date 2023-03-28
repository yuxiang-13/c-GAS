// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 让此文件支持反射
#include "ActionGameTypes.generated.h"

USTRUCT(BlueprintType)
struct FCharacterData
{
	// ue5 GENERATED_BODY 和 GENERATED_USTRUCT_BODY 区别
	// 需要注意的是，GENERATED_BODY和GENERATED_USTRUCT_BODY宏定义虽然用途不同，但其本质上是相似的。
	// 它们都是利用了UE5中的反射机制，通过宏定义来自动生成一些代码
	//GENERATED_BODY宏定义在类声明中使用，可以用于生成类的一些标准方法（例如Construct、Tick、BeginPlay等）
	//GENERATED_USTRUCT_BODY宏定义在结构体声明中使用，用于生成结构体的元数据和一些标准方法（例如初始化、复制、比较等
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GAS")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GAS")
	TArray<TSubclassOf<class UGameplayAbility>> Abilitys;
};





