// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryList.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONGASPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	/*
	InitializeComponent 和 BeginPlay 都是用于在组件中进行初始化的重要函数。尽管它们的目的类似，但它们之间有一些关键的区别。
	
	1 InitializeComponent 是在创建组件时自动调用的函数，用于对组件进行初始化。该函数在构造函数之后调用，并且可以用于设置组件的属性、创建子组件等操作。
	通常，InitializeComponent 可以用于设置组件的初始状态，以便该组件可以正常运行。

	2 BeginPlay 是在组件所属的 Actor 的 BeginPlay 函数中调用的函数。它表示 Actor 已经准备好开始游戏，并且组件也可以开始工作了。在 BeginPlay
	 */
	virtual void InitializeComponent() override;

	// ReplicateSubobjects 函数的作用是将一个 UObject 类型的子对象在网络上进行同步
	/*
	当 ReplicateSubobjects 为 false 时，子对象不会被同步到客户端，这可以提高网络性能。
	当 ReplicateSubobjects 为 true 时，如果一个 UObject 类型的对象在服务器上创建或修改了它的子对象，这些修改会被传播到与该对象相关联的客户端。这使得客户端能够与服务器上的对象保持同步
	
		参数1 UActorChannel* Channel：指定传输数据
	 */
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	//蓝图指定 用于初始化 上面这个 InventoryList
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UItemStaticData>> DefaultItems;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
