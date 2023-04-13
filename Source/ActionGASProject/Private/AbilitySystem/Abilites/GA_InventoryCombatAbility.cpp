// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_InventoryCombatAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Inventory/ItemActors/WeaponItemActor.h"
#include "Kismet/KismetSystemLibrary.h"


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

const bool UGA_InventoryCombatAbility::GetWeaponToFocusTraceResult(float TraceDistance, ETraceTypeQuery TraceType,
	FHitResult& OutHitResult)
{
	AWeaponItemActor* WeaponItemActor = GetEquippedWeaponItemActor();
	AActionGASProjectCharacter* ActionCharacter = GetActionGameCharacterFromActorInfo();
	// 获取角色摄像机位置
	const FTransform& CameraTransform = ActionCharacter->GetFollowCamera()->GetComponentTransform();

	// 摄像机（朝向）终点位置
	const FVector FocusTraceEnd = CameraTransform.GetLocation() + CameraTransform.GetRotation().Vector() * TraceDistance;
	// 射线忽略
	TArray<AActor*> ActorsToIgnore = { GetAvatarActorFromActorInfo() };

	FHitResult FocusHit;
	// 注意，参数是 FocusHit
	UKismetSystemLibrary::LineTraceSingle(this, CameraTransform.GetLocation(), FocusTraceEnd, TraceType, false, ActorsToIgnore, EDrawDebugTrace::None, FocusHit, true);


	// 枪口位置
	FVector MuzzleLocation = WeaponItemActor->GetMuzzleLocation();
	// 枪口位置 + 枪口朝向摄像机碰撞点位置的单位向量 * 距离
	const FVector WeaponTraceEnd = MuzzleLocation + (FocusHit.Location - MuzzleLocation).GetSafeNormal() * TraceDistance;
	// 注意，参数是 OutHitResult
	UKismetSystemLibrary::LineTraceSingle(this, MuzzleLocation, WeaponTraceEnd, TraceType, false, ActorsToIgnore, EDrawDebugTrace::None, OutHitResult, true);

	return OutHitResult.bBlockingHit;
}
