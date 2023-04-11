// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/AG_AnimInstance.h"

#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "ActorComponent/InventoryComponent.h"
#include "DataAssets/CharacterAnimDataAsset.h"
#include "Inventory/InventoryItemInstance.h"

const UItemStaticData* UAG_AnimInstance::GetEquippedItemData() const
{
	AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor());
	UInventoryComponent* InventoryComponent = ActionGASProjectCharacte ? ActionGASProjectCharacte->GetInventoryComponent() : nullptr;

	UInventoryItemInstance* ItemInstance = InventoryComponent ? InventoryComponent->GetEquippedItem() : nullptr;
	return ItemInstance ? ItemInstance->GetItemStaticData() : nullptr;
}


UBlendSpace* UAG_AnimInstance::GetLocomotionBlendSpace() const
{
	if (AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor()))
	{
		// 返回当前装备的 角色动画
		if (const UItemStaticData* ItemData = GetEquippedItemData())
		{
			if (ItemData->CharacterAnimationData.MovementBlendspace)
			{
				return ItemData->CharacterAnimationData.MovementBlendspace;
			}
		}

		// 下面就成了默认动画
		FCharacterData Data = ActionGASProjectCharacte->GetCharacterData();
		// 获取动画数据资产，并返回混合空间
		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.MovementBlendspace;
		}
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.MovementBlendspace : nullptr;
}

UAnimSequence* UAG_AnimInstance::GetIdleAnimation() const
{
	if (AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor()))
	{
		
		// 返回当前装备的 角色动画
		if (const UItemStaticData* ItemData = GetEquippedItemData())
		{
			if (ItemData->CharacterAnimationData.IdleAnimationAsset)
			{
				return ItemData->CharacterAnimationData.IdleAnimationAsset;
			}
		}

		
		FCharacterData Data = ActionGASProjectCharacte->GetCharacterData();
		// 获取动画数据资产，并返回混合空间
		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.IdleAnimationAsset;
		}
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.IdleAnimationAsset : nullptr;
}


UBlendSpace* UAG_AnimInstance::GetCrouchLocomotionBlendSpace() const
{
	if (AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor()))
	{
		
		// 返回当前装备的 角色动画
		if (const UItemStaticData* ItemData = GetEquippedItemData())
		{
			if (ItemData->CharacterAnimationData.CrouchMovementBlendspace)
			{
				return ItemData->CharacterAnimationData.CrouchMovementBlendspace;
			}
		}
		
		
		FCharacterData Data = ActionGASProjectCharacte->GetCharacterData();
		// 获取动画数据资产，并返回混合空间
		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.CrouchMovementBlendspace;
		}
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.CrouchMovementBlendspace : nullptr;
}

UAnimSequence* UAG_AnimInstance::GetCrouchAnimation() const
{
	if (AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor()))
	{
		
		// 返回当前装备的 角色动画
		if (const UItemStaticData* ItemData = GetEquippedItemData())
		{
			if (ItemData->CharacterAnimationData.CrouchAnimationAsset)
			{
				return ItemData->CharacterAnimationData.CrouchAnimationAsset;
			}
		}

		
		FCharacterData Data = ActionGASProjectCharacte->GetCharacterData();
		// 获取动画数据资产，并返回混合空间
		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.CrouchAnimationAsset;
		}
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.CrouchAnimationAsset : nullptr;
}
