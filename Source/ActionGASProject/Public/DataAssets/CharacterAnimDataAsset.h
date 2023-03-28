// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Engine/DataAsset.h"
#include "CharacterAnimDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UCharacterAnimDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 动画数据结构体
	UPROPERTY(EditDefaultsOnly)
	FCharacterAnimationData CharacterAnimationData;

	
};
