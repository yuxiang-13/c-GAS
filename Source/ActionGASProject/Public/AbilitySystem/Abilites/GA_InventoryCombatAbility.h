// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_InventoryAbility.h"
#include "GA_InventoryCombatAbility.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UGA_InventoryCombatAbility : public UGA_InventoryAbility
{
	GENERATED_BODY()
public:
	// 获取GE实例
	UFUNCTION(BlueprintPure)
	FGameplayEffectSpecHandle GetWeaponEffectSpec(const FHitResult& InHitResult);
};
