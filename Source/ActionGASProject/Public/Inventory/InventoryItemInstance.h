// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.generated.h"

/**
 * 
 */

class UItemStaticData;
class AItemActor;

// 把这个暴漏给蓝图
UCLASS(BlueprintType, Blueprintable)
class ACTIONGASPROJECT_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()
public:
	// 添加个用于初始化的函数
	virtual void Init(TSubclassOf<UItemStaticData> InItemStaticDataClass);
	
	//该函数用于检查一个对象是否适用于网络同步。
	virtual bool IsSupportedForNetworking() const override { return true; };

	// 返回这个默认的对象
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const UItemStaticData* GetItemStaticData() const;
	
	// 属性 网络同步
	// 用于自动同步变量到客户端和服务器之间，如果在服务器上修改了变量，它会自动同步到各个客户端。这个过程是自动的，不需要做其他的设置。
	UPROPERTY(Replicated)
	TSubclassOf<UItemStaticData> ItemStaticDataClass;

	// 客户端 监听 服务器 网络属性变化
	UPROPERTY(ReplicatedUsing= OnRep_Equipped)
	bool bEquipped = false;

	UFUNCTION()
	void OnRep_Equipped();

	// 声明一些以后会用到虚方法
	virtual void OnEquipped(AActor* InOwer = nullptr);
	virtual void OnUnEquipped();

protected:
	UPROPERTY(Replicated)
	AItemActor* ItemActor = nullptr;

	// 返回用于网络复制的属性，这需要由具有本机复制属性的所有参与者类重写 声明自己要同步什么东西
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
