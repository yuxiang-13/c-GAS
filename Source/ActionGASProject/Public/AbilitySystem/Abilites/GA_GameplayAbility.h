// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GameplayAbility.generated.h"

/**
 * 
 */

class AActionGASProjectCharacter;

UCLASS()
class ACTIONGASPROJECT_API UGA_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToRemoveOnEnd;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToJustApplyOnStart;

	// 用于记录GE句柄，方便移除
	TArray<FActiveGameplayEffectHandle> RemoveOnEndEffectHandles;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActionGASProjectCharacter* GetActionGameCharacterFromActorInfo() const;
};
