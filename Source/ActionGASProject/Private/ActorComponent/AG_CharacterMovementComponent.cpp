// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent//AG_CharacterMovementComponent.h"

#include "AbilitySystemComponent.h"

/*
 *TAutoConsoleVariable是一个宏定义，用于创建控制台命令变量
 * 控制台命令变量在UE5中可以用于动态修改游戏参数，例如控制游戏的采样率、帧率、物理系统参数等。
 * 在运行时，可以通过输入控制台命令（变量名称+变量值）来修改控制台命令变量的值，从而改变游戏参数。
 */
// 调试Debug
static TAutoConsoleVariable<int32> CVarShowTraversal(TEXT("ShowTraversal"),
	0, //控制台命令变量的初始值，默认值为0
	TEXT("Draws debug info about Traversal/n    0: off/n    1: on/n"), //控制台命令变量的帮助文本，可以为NULL。
	ECVF_Cheat); //控制台命令变量的标识位，用于指定变量的使用权限。可用标识位包括：可通过配置文件进行修改、只读变量、命令行参数、不使用外部命令行参数等。
//ECVF_Cheat : 隐藏在控制台中，用户无法更改
//..其他的我也不清楚。需要看文档，就先不说了

// 这里一层层遍历，应该触发的GA
bool UAG_CharacterMovementComponent::TryTraversal(UAbilitySystemComponent* ASC)
{
	// 遍历GA
	for (TSubclassOf<UGameplayAbility> AbilityClass : TraversalAbilitiesOrdered)
	{
		//1 用于主动激活与指定 class(Tag)匹配的技能。
		//2 如果bAllowRemoteActivation为true，它将远程激活本地/服务器功能，如果为false，它将仅尝试本地激活功能。
		
		if (ASC->TryActivateAbilityByClass(AbilityClass, true))
		{
			// 寻找并获取技能 实例
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
			// 查看这个技能是否被激活，当然，每个技能内部自己做判断，是否可以被触发
			if (Spec && Spec->IsActive())
			{
				return true;
			}
		}
	}

	return false;
}
