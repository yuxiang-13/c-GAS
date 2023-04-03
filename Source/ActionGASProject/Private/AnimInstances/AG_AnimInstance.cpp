// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/AG_AnimInstance.h"

#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "DataAssets/CharacterAnimDataAsset.h"

UBlendSpace* UAG_AnimInstance::GetLocomotionBlendSpace() const
{
	if (AActionGASProjectCharacter* ActionGASProjectCharacte = Cast<AActionGASProjectCharacter>(GetOwningActor()))
	{
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
		FCharacterData Data = ActionGASProjectCharacte->GetCharacterData();
		// 获取动画数据资产，并返回混合空间
		if (Data.CharacterAnimDataAsset)
		{
			return Data.CharacterAnimDataAsset->CharacterAnimationData.CrouchAnimationAsset;
		}
	}

	return DefaultCharacterAnimDataAsset ? DefaultCharacterAnimDataAsset->CharacterAnimationData.CrouchAnimationAsset : nullptr;
}
