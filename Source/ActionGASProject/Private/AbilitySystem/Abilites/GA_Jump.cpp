// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_Jump.h"

#include "AbilitySystemComponent.h"
#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_Jump::UGA_Jump()
{
	// 1 用于表示某个Gameplay Ability的网络执行策略
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	/*
	*enum Type
	{
		// 表示 Gameplay Ability 在当前客户端上首先执行本地预测，并在之后在服务端上同步。如果客户端和服务端状态相同，则不需要额外的状态纠正操作。
		// 该选项适用于对实时性有严格要求的 Gameplay Ability 。例如角色的移动、攻击等。
		LocalPredicted		UMETA(DisplayName = "Local Predicted"),

		// 表示 Gameplay Ability 只在当前客户端本地执行。在网络环境下，该选项适用于无需影响其他客户端和服务端状态的 Gameplay Ability。
		LocalOnly			UMETA(DisplayName = "Local Only"),

		// 表示 Gameplay Ability 在服务端上先执行，然后在客户端上同步。该选项适用于只能由服务端上当前 Actor 引发的影响其他 Actor 的 Gameplay Ability 。
		ServerInitiated		UMETA(DisplayName = "Server Initiated"),

		// 表示 Gameplay Ability 只在服务端执行，客户端无法执行抵用。该选项适用于只对服务端状态造成影响的 Gameplay Ability 。
		ServerOnly			UMETA(DisplayName = "Server Only"),
	};
	* 
	*/

	//1 InstancingPolicy 表示Gameplay Ability实例化策略的枚举值
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	/*
		*enum Type
		{
			// 表示 Gameplay Ability 使用已经存在的 Ability 实例。相当于将所有使用该 Ability 的对象共享同一个 Ability 实例。
			// 适用于那些不需要创建新实例的特定 Ability。由于不需要创建新实例，可以减少计算和资源开销
			NonInstanced,
	
			// 表示每个 Actor 创建一个新的 Ability 实例。适用于每个 Actor 运行自己的实例的情况，例如一些身体动作或类似玩家控制的操作。
			InstancedPerActor,
	
			// 表示每次执行 Ability 都创建一个新的 Ability 实例。适用于那些多个 Actor 运行不同实例的 Ability 类，
			例如施法者对多个目标进行施法，每个目标都需要一个新的 Ability 实例。
			InstancedPerExecution,
		};
	 */
}

bool UGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 1 ActorInfo 是一个包含有关 Ability 和执行者（Actor）的信息的结构体
	// 该结构体中的 AvatarActor 变量存储着指向执行 Actor 的指针（通常是 Pawn 或 Character）

	// 2 第一个参数是需要转换的对象，第二个参数是一个 ECastCheckedType 枚举，以指定转换失败时程序如何处理

	/*
	*	enum Type
		{
			// 表示不会抛出异常或触发断言，而是返回空指针（Null），表示类型转换失败。
			NullAllowed,
			// 示在类型转换失败时，立即触发断言（assertion），停止程序的执行，并显示错误信息。
			NullChecked
		};
	 * 
	 */
	const ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	return Character->CanJump();
}

// 激活GA
void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 需要在具有预测能力的Actor中  检查该Actor是否具有运行权限。
	// 用 HasAuthorityOrPredictionKey函数用来确定当前Actor是否具有服务器运行权限。
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// 开始执行 能力,失败 直接返回
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			return;
		}
		// 执行基类中的方法，就可以把下面的 GE生效代码删除了
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

		AActionGASProjectCharacter * Character = CastChecked<AActionGASProjectCharacter>(ActorInfo->AvatarActor.Get());
		Character->Jump();
	}
}
