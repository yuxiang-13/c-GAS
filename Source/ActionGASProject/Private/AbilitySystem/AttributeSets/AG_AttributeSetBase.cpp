// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

//属性变化后（PostGameplayEffectExecute）
// PostGameplayEffectExecute(const FGameplayEffectModCallbackData & Data)只是在由Instant类型的GameplayEffect对某个Attribute的BaseValue!!修改之后!!才会触发。
// 这里可以进一步做一些Attribute相关的操作。
// 当postgameplayeffectexecute调用的时候，只发生在服务器上。
/*
* 例如，在示例项目中我们令生命值的Attribute减去最终伤害值的Meta Attribute。如果有护盾的Attribute的话，我们可以在这里先让护盾值减去伤害值，
* 然后再把剩余伤害（如果还有的话）应用到生命值上。示例项目也在这个位置来应用受击动画，显示伤害飘字，并且为击杀者赋予经验和金币奖励。
* 从设计上说，伤害值的Meta Attribute将始终通过Instant类型的GameplayEffect来设置，并且永远不需要通过Attribute的设置器（setter）来设置。
  其他一些仅由Instant类型的GameplayEffect来改变其BaseValue的Attributes，比如法力值和体力值，也可以在这里通过其最大值对应的Attributes来进行Clamp截取操作。
**注意：**当调用PostGameplayEffectExecute()，对Attribute的修改就已经生效了，但是还没有复制回客户端，所以在此处进行Clamp截取操作的话实际上不会进行两次值的复制。
* 客户端仅收到截取过后的结果（最终值）就是最终被Clamp限制的那个值。
 */
void UAG_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// 传递GE处理属性范围
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

//属性变化前（PreAttributeChange）
//PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)是AttributeSet中重要的函数方法之一，主要是响应Attribute中的CurrentValue的修改发生之前的调用。
//这里最好就是去做一些对输入的限制和调整，利用NewValue来将可能会被应用到CurrentValue上的修改限制到某个合理的区间范围。
/*
 *注意Epic的对PreAttributeChange()的注释提到，不要去使用它来处理游玩相关的事件，而只是把它用作数值的修正和处理。
 *监听Attribute的变化而产生的和游玩相关的事件（译者注：比如说生命值、弹药数等属性的UI响应事件）
 *的推荐的处理方案是使用UAbilitySystemComponent::GetGameplayAttributeValueChangeDelegate(FGameplayAttribute Attribute)
 *（Responding to Attribute Changes）。
 */
void UAG_AttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
	}
}

void UAG_AttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	// 这是一个辅助宏,位于AttributeSet类提供好的，可在RepNotify函数中使用，以处理客户端将进行预测性修改的属性。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, Health, OldHealth);
}

void UAG_AttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	// 这是一个辅助宏,位于AttributeSet类提供好的，可在RepNotify函数中使用，以处理客户端将进行预测性修改的属性。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAG_AttributeSetBase, MaxHealth, OldMaxHealth);
}

void UAG_AttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 当使用条件同步时，只有满足了条件所对应的情况才会进行属性同步，这个例子中 COND_OwnerOnly 对应其中一种条件值得枚举值，表示属性只会同步给Actor 的拥有者（Onwer）。
	// 还有其它条件枚举值可以选择：

	// REPNOTIFY_Always是告诉OnRep函数当本地值和服务器下发的值相同的时候也去进行相应的触发。默认情况下（即这里不用REPNOTIFY_Always的情况下）这两个值一样的时候是不会触发OnRep函数的。
	// 如果某个Attribute不需要复制，类似Meta Attribute，那么OnRep和GetLifetimeReplicatedProps这两步的设置是可以跳过的。
	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAG_AttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
}
