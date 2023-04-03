// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilites/GA_GameplayAbility.h"
#include "GA_Jump.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UGA_Jump : public UGA_GameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Jump();

	// 运行条件
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

// protected:
//
// 	// 比如跳跃时候减血
// 	UPROPERTY(EditDefaultsOnly, Category="Effects")
// 	TSubclassOf<class UGameplayEffect> JumpEffect;
};
