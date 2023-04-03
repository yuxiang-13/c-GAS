// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_Crouch.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Crouch::UGA_Crouch()
{
	// 表示 Gameplay Ability 在当前客户端上首先执行本地预测，并在之后在服务端上同步。
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	// 表示每个 Actor 创建一个新的 Ability 实例。
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
}

bool UGA_Crouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	return Character->CanCrouch();
}

void UGA_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->Crouch();

	// 开始 朝向摄像机方向
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	Movement->bUseControllerDesiredRotation = true;
	Movement->bOrientRotationToMovement = false;
}

void UGA_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	
	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->UnCrouch();
	
	// 停止 朝向摄像机方向
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	Movement->bUseControllerDesiredRotation = false;
	Movement->bOrientRotationToMovement = true;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
