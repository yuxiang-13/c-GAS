// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_GameplayEvent.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
protected:
	// 触发的 GameplayEvent
	UPROPERTY(EditAnywhere)
	FGameplayEventData Payload;
};
