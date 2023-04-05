// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Abilites/GA_GameplayAbility.h"
#include "GA_Vault.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UGA_Vault : public UGA_GameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Vault();

	// 该函数用于检查是否可以应用某个游戏玩法能力。
	// CommitCheck 函数可以检查玩家是否当前处于可以施放技能的状态，或者是否满足使用技能所需的必要资源或条件。如果通过了检查，游戏玩法能力将被成功应用。
	virtual bool CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags) override;

	// 激活
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 结束
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 用于环境检测的
// 水平识别 用于识别 障碍物末端
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float HorizontalTraceRadius = 30.f; // 半径
	
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float HorizontalTraceLength = 500.f;

	// 执行 次数
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float HorizontalTraceCount = 5.f;

	// 水平射线之间的间隔
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float HorizontalTraceStep = 30.f;


	
// 垂直识别  用于识别 障碍物末端
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float VerticalTraceRadius = 30.f; // 半径
// 垂直射线之间的间隔
	UPROPERTY(EditDefaultsOnly, Category=HorizontalTrace)
	float VerticalTraceStep = 30.f; // 半径

// 指定碰撞通道
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
// 指定蒙太奇
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* VaultMontage = nullptr;
// 缓存目标点
	FVector JumpToLocation;
	FVector JumpOverLocation;
// 忽略一些碰撞 防止条约过程中，出现因为碰撞问题，产生的不协调动作问题（比如卡进去了啥的）
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannelsToIgnore;

// 创建 蒙太奇任务 （这个是引擎提供好的， 可以执行播放蒙太奇 和 等待蒙太奇完成）
	UAbilityTask_PlayMontageAndWait* MontageTask = nullptr;
};
