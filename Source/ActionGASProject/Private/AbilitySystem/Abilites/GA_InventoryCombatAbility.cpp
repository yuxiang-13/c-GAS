// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_InventoryCombatAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActionGASProject/ActionGASProjectCharacter.h"


FGameplayEffectSpecHandle UGA_InventoryCombatAbility::GetWeaponEffectSpec(const FHitResult& InHitResult)
{
	// 获取技能组件
	if (UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UWeaponStaticData* WeaponStaticData = GetEquippedItemWeaponStaticData())
		{
			// 创建伤害GE实例 并给与参数
			FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
			FGameplayEffectSpecHandle OueSpec = AbilityComponent->MakeOutgoingSpec(WeaponStaticData->DamageEffect, 1, EffectContext);
			// 1 AbilityComponent->RegisterGameplayTagEvent  注册GameplayTag并监听事件

			// 2 动态改变的属性值 GE.h中 属性 TMap<FGameplayTag, float> SetByCallerNameMagnitudes 这个 可以在 行时动态地修改属性值，是一个Map
			// 我们只需要把   TMap->FGameplayTag  传导值 TMap->float： 就可以通过GE触发时候，给予属性改变
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OueSpec, FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")), -WeaponStaticData->BaseDamage);
			return OueSpec;
		}
	}
	return FGameplayEffectSpecHandle();
}
