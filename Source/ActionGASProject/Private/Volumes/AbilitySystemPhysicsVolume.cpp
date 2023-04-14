// Fill out your copyright notice in the Description page of Project Settings.


#include "Volumes/AbilitySystemPhysicsVolume.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAbilitySystemPhysicsVolume::AAbilitySystemPhysicsVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	
}

void AAbilitySystemPhysicsVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);
	// 服务器检测进入
	if (!HasAuthority()) return;
	
	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		// 赋予永久能力
		for (auto Ability: PermanentAbilitiesToGive)
		{
			// 创建GA实例 给 Other 的技能组件
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
		}
		// 记录进入的人
		EnteredActorsInfoMap.Add(Other);
		
		// 赋予 再体积内才激活的 能力
		for (auto Ability: OngoingAbilitiesToGive)
		{
			// 创建GA实例 给 Other 的技能组件
			FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
			EnteredActorsInfoMap[Other].AppliedAbilities.Add(AbilityHandle);
		}

		// 赋予 再体积内才激活的 GE
		for (auto GameplayEffect: OngoingEffectsToApply)
		{
			// 创建GE配置
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(Other, Other);

			// 创建GE实例
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				// 应用GE
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				if (ActiveGEHandle.WasSuccessfullyApplied())
				{
					EnteredActorsInfoMap[Other].AppliedEffects.Add(ActiveGEHandle);
				}
			}
		}

		// 激活 事件 (进入体积时)
		for (FGameplayTag EventTag: GameplayEventsToSendOnEnter)
		{
			// 创建事件 并激活
			FGameplayEventData EventPayload;
			EventPayload.EventTag = EventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
}

// 应用离开时的 事件  以及  恢复我们赋予的 一些GA和GE
void AAbilitySystemPhysicsVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);
	// 服务器检测进入
	if (!HasAuthority()) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		// 离开时 恢复我们赋予的一些GA和GE
		if (EnteredActorsInfoMap.Find(Other))
		{
			// 移除GE
			for (auto GameplayEffectHandle : EnteredActorsInfoMap[Other].AppliedEffects)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(GameplayEffectHandle);
			}

			for (auto GameplayAbilityHandle : EnteredActorsInfoMap[Other].AppliedAbilities)
			{
				AbilitySystemComponent->ClearAbility(GameplayAbilityHandle);
			}

			EnteredActorsInfoMap.Remove(Other);
		}

		//  离开时 激活的GE
		for (auto GameplayEffect: OnExitEffectsToApply)
		{
			// 创建GE配置
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(Other, Other);

			// 创建GE实例
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				// 应用GE
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		
		// 激活 事件 (离开体积时)
		for (auto EventTag: GameplayEventsToSendOnExit)
		{
			// 创建事件 并激活
			FGameplayEventData EventPayload;
			EventPayload.EventTag = EventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
	
}

void AAbilitySystemPhysicsVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDrawbug)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), GetBounds().BoxExtent, FColor::Red, false, 0, 0, 5);
	}
}
