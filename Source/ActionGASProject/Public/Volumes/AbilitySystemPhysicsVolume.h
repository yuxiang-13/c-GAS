// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PhysicsVolume.h"
#include "AbilitySystemPhysicsVolume.generated.h"


class UGameplayEffect;
class UGameplayAbility;

// 保存进入体积的actor
USTRUCT()
struct FAbilityVolumeEnteredActorInfo
{
	GENERATED_USTRUCT_BODY()

	TArray<FGameplayAbilitySpecHandle> AppliedAbilities;
	TArray<FActiveGameplayEffectHandle> AppliedEffects;
};

UCLASS()
class ACTIONGASPROJECT_API AAbilitySystemPhysicsVolume : public APhysicsVolume
{
	GENERATED_BODY()

protected:
	// 角色在体积内 应用效果
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApply;
	// 角色进入体积内 发送的Event数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> GameplayEventsToSendOnEnter;
	// 角色进入体积内 获得一些能力
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> OngoingAbilitiesToGive;
	// 角色进入体积内 获得一些 永久性能力
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> PermanentAbilitiesToGive;
	
	// 角色在离开体积内 应用效果
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayEffect>> OnExitEffectsToApply;
	// 角色离开体积内 发送的Event数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> GameplayEventsToSendOnExit;

	// 展示体积形状 Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawbug = false;

	// 按map记录进入体积的actor
	TMap<AActor*, FAbilityVolumeEnteredActorInfo> EnteredActorsInfoMap;
public:
	AAbilitySystemPhysicsVolume();

	virtual void ActorEnteredVolume(AActor* Other) override;
	virtual void ActorLeavingVolume(AActor* Other) override;

	// 绘制
	virtual void Tick(float DeltaSeconds) override;
};
