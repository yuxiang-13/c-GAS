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
	// 触发脚步
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere)
	EFoot Foot;
	
};
