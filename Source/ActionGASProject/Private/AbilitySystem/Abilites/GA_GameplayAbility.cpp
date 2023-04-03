// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_GameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "ActionGASProject/ActionGASProjectCharacter.h"

void UGA_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 创建GE内容句柄
	FGameplayEffectContextHandle EffectContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();
	for (auto GameplayEffect: OngoingEffectsToJustApplyOnStart)
	{
		// 检查是否有效
		if (!GameplayEffect.Get()) continue;

		if (UAbilitySystemComponent* AbilityComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			// GE实例化
			FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				//激活 GE Spec实例 也就 获得了这个 FActiveGameplayEffectHandle
				FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				// 如果没成功 添加一条消息打印
				if (!ActiveGEHandle.WasSuccessfullyApplied())
				{
					ABILITY_LOG(Log, TEXT("Ability %s Failed to apply startup Effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
					// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT(" Failed to apply jump effect! %s"), * GetNameSafe(Character)) , true, true, FLinearColor::Red, 10.f);
				}
			}
		}
	}

	//IsInstantiated() 如果已经实例化，则为True，对于蓝图始终为True
	// 保存GE句柄，用于删除,  GE生效后，需要手动End移除的
	if (IsInstantiated())
	{
		for (auto GameplayEffect : OngoingEffectsToRemoveOnEnd)
		{
			// 检查是否有效
			if (!GameplayEffect.Get()) continue;

			if (UAbilitySystemComponent* AbilityComponent = ActorInfo->AbilitySystemComponent.Get())
			{
				// GE实例化
				FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
				if (SpecHandle.IsValid())
				{
					//激活 GE Spec实例 也就 获得了这个 FActiveGameplayEffectHandle
					FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

					// 如果没成功 添加一条消息打印
					if (!ActiveGEHandle.WasSuccessfullyApplied())
					{
						ABILITY_LOG(Log, TEXT("Ability %s Failed to apply runtime Effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
						// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT(" Failed to apply jump effect! %s"), * GetNameSafe(Character)) , true, true, FLinearColor::Red, 10.f);
					} else
					{
						RemoveOnEndEffectHandles.Add(ActiveGEHandle);
					}
				}
			}
		}
	}
}

void UGA_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsInstantiated())
	{
		for (FActiveGameplayEffectHandle ActiveEffectHandle: RemoveOnEndEffectHandles)
		{
			if (ActiveEffectHandle.IsValid())
			{
				ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
			}
		}

		RemoveOnEndEffectHandles.Empty();
	}

	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AActionGASProjectCharacter* UGA_GameplayAbility::GetActionGameCharacterFromActorInfo() const
{
	return Cast<AActionGASProjectCharacter>(GetAvatarActorFromActorInfo());
}
