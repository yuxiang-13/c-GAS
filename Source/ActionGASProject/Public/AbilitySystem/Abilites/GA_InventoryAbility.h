// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "AbilitySystem/Abilites/GA_GameplayAbility.h"
#include "GA_InventoryAbility.generated.h"


class UInventoryComponent;
class UInventoryItemInstance;
class UItemStaticData;
class UWeaponStaticData;
class AItemActor;
class AWeaponItemActor;

UCLASS()
class ACTIONGASPROJECT_API UGA_InventoryAbility : public UGA_GameplayAbility
{
	GENERATED_BODY()
public:
	// 在 能力被授予角色时  触发
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

// 封装对外接口 方便后续处理逻辑
	UFUNCTION(BlueprintPure) // 背包
	UInventoryComponent* GetInventoryComponent() const;
	
	UFUNCTION(BlueprintPure) // 装备的 背包元素
	UInventoryItemInstance* GetEquippedItemInstance() const;
	
	UFUNCTION(BlueprintPure) // 道具 基础静态数据
	const UItemStaticData* GetEquippedItemStaticData() const;
	
	UFUNCTION(BlueprintPure) // 装备 武器静态数据
	const UWeaponStaticData* GetEquippedItemWeaponStaticData() const;
	
	UFUNCTION(BlueprintPure) // 道具蓝图实例
	AItemActor* GetEquippedItemActor() const;
	
	UFUNCTION(BlueprintPure) // 装备蓝图实例
	AWeaponItemActor* GetEquippedWeaponItemActor() const;
	
protected:
	// 背包
	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;
};
