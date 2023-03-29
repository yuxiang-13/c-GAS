// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Step.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UAnimNotify_Step : public UAnimNotify
{
	GENERATED_BODY()
public:
	// 更新代码：使用UAnimInstance::GetNotifyName()代替UAnimNotify::Notify()。新的接口和旧的接口功能相同，但是使用方式和参数有所不同。
	// 触发脚步
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
	
	UPROPERTY(EditAnywhere)
	EFoot Foot;
	
};
