// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItemInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "ActionGameStatic.h"
#include "Actors/ItemActor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

void UInventoryItemInstance::Init(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	ItemStaticDataClass = InItemStaticDataClass;
}

const UItemStaticData* UInventoryItemInstance::GetItemStaticData() const
{
	// 函数库  创建并返回 默认的对象
	return UActionGameStatic::GetItemStaticData(ItemStaticDataClass);
}

void UInventoryItemInstance::OnRep_Equipped()
{
}

void UInventoryItemInstance::OnEquipped(AActor* InOwer)
{
	if (InOwer->GetWorld())
	{
		const UItemStaticData* StaticData = GetItemStaticData();
		
		FTransform Transform;
		// 1 使用SpawnActor方法创建Actor时，UE4会立即创建一个Actor对象并从内存池中为其分配内存。
		// 2 SpawnActorDeferred 只会创建一个虚拟的Actor对象并不会为其分配内存,需要创建时候，直接调用 FinishSpawning
		ItemActor = InOwer->GetWorld()->SpawnActorDeferred<AItemActor>(StaticData->ItemActorClass, Transform, InOwer);
		ItemActor->Init(this);
		ItemActor->OnEquipped();
		ItemActor->FinishSpawning(Transform);

		// 检测骨架
		if (ACharacter* Character = Cast<ACharacter>(InOwer))
		{
			if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
			{
				ItemActor->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
			}
		}
		
		TryGrantAbilities(InOwer);
		TryApplyEffects(InOwer);
		bEquipped = true;
	}
}

void UInventoryItemInstance::OnUnEquipped(AActor* InOwer)
{
	if (ItemActor)
	{
		ItemActor->Destroy();
		ItemActor = nullptr;
	}
	TryRemoveAbilities(InOwer);
	// OnUnEquipped 函数只会在 背包component中 服务器上触发
	bEquipped = false;
	TryRemoveEffects(InOwer);
}

void UInventoryItemInstance::OnDropped(AActor* InOwer)
{
	if (ItemActor)
	{
		ItemActor->OnDropped();
	}
	TryRemoveAbilities(InOwer);
	// OnUnEquipped 函数只会在 背包component中 服务器上触发
	bEquipped = false;
	TryRemoveEffects(InOwer);
}

AItemActor* UInventoryItemInstance::GetItemActor() const
{
	return ItemActor;
}

// 只能服务器 赋予能力
void UInventoryItemInstance::TryGrantAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			// 从静态数据中赋予能力
			const UItemStaticData* StaticData = GetItemStaticData();
			for (auto ItemAbility : StaticData->GrantedAbilities)
			{
				// 开始  给予能力
				GrantedAbilityHandles.Add(AbilityComponent->GiveAbility(FGameplayAbilitySpec(ItemAbility)));
			}
		}
	}
}

void UInventoryItemInstance::TryRemoveAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			// 从静态数据中赋予能力
			const UItemStaticData* StaticData = GetItemStaticData();
			for (auto AbilityHandle : GrantedAbilityHandles)
			{
				// 取消能力
				AbilityComponent->ClearAbility(AbilityHandle);
			}

			GrantedAbilityHandles.Empty();
		}
	}
}

void UInventoryItemInstance::TryApplyEffects(AActor* InOwner)
{
	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
	{
		const UItemStaticData* ItemStaticData = GetItemStaticData();
		
		// 创建GE内容句柄
		FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
		
		for (auto GameplayEffect : ItemStaticData->OngoingEffects)
		{
			// 检查是否有效
			if (!GameplayEffect.Get()) continue;

			// GE实例化
			FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				//激活 GE Spec实例 也就 获得了这个 FActiveGameplayEffectHandle
				FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				// 如果没成功 添加一条消息打印
				if (!ActiveGEHandle.WasSuccessfullyApplied())
				{
					ABILITY_LOG(Log, TEXT("Item %s Failed to apply runtime Effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
					// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT(" Failed to apply jump effect! %s"), * GetNameSafe(Character)) , true, true, FLinearColor::Red, 10.f);
				} else
				{
					OnGoingEffectHandles.Add(ActiveGEHandle);
				}
			}
		}
	}
}

void UInventoryItemInstance::TryRemoveEffects(AActor* InOwner)
{
	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
	{
		for (FActiveGameplayEffectHandle ActiveEffectHandle: OnGoingEffectHandles)
		{
			if (ActiveEffectHandle.IsValid())
			{
				AbilityComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
			}
		}

		OnGoingEffectHandles.Empty();
	}
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemInstance, ItemStaticDataClass);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventoryItemInstance, bEquipped, COND_None, REPNOTIFY_Always); 
	DOREPLIFETIME(UInventoryItemInstance, ItemActor);
}
