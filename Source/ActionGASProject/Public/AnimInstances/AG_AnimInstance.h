// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AG_AnimInstance.generated.h"


class UItemStaticData;

UCLASS()
class ACTIONGASPROJECT_API UAG_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
protected:
	const UItemStaticData* GetEquippedItemData() const;
	
	// 返回 混合空间动画资源
	// UFUNCTION(BlueprintCallable, meta=(NotBlueprintThreadSafe))
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	class UBlendSpace* GetLocomotionBlendSpace() const;

	// 返回 待机动画资源
	// UFUNCTION(BlueprintCallable, meta=(NotBlueprintThreadSafe))
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	class UAnimSequence* GetIdleAnimation() const;

	
	// 返回 混合空间动画资源
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	class UBlendSpace* GetCrouchLocomotionBlendSpace() const;

	// 返回 蹲伏动画资源
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	class UAnimSequence* GetCrouchAnimation() const;
	
	
	// 提供 默认 动画数据资产
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation")
	class UCharacterAnimDataAsset* DefaultCharacterAnimDataAsset;
};
