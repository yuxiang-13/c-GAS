// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/InventoryComponent.h"

#include "ActionGameTypes.h"
#include "Engine/ActorChannel.h"
#include "Inventory/InventoryItemInstance.h"
#include "Net/UnrealNetwork.h"

/*
 *TAutoConsoleVariable是一个宏定义，用于创建控制台命令变量
 * 控制台命令变量在UE5中可以用于动态修改游戏参数，例如控制游戏的采样率、帧率、物理系统参数等。
 * 在运行时，可以通过输入控制台命令（变量名称+变量值）来修改控制台命令变量的值，从而改变游戏参数。
 */
// 调试Debug
static TAutoConsoleVariable<int32> CVarShowInventory(TEXT("ShowDebugInventory"),
	0, //控制台命令变量的初始值，默认值为0
	TEXT("Draws debug info about Inventory/n    0: off/n    1: on/n"), //控制台命令变量的帮助文本，可以为NULL。
	ECVF_Cheat); //控制台命令变量的标识位，用于指定变量的使用权限。可用标识位包括：可通过配置文件进行修改、只读变量、命令行参数、不使用外部命令行参数等。
//ECVF_Cheat : 隐藏在控制台中，用户无法更改
//..其他的我也不清楚。需要看文档，就先不说了


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// 在继承自 UActorComponent 的组件中，
	// 默认情况下，如果不将 bWantsInitializeComponent 设置为 true，则在创建组件时不会自动调用 InitializeComponent 方法。
	// 如果您的组件需要在创建时进行初始化，则应将 bWantsInitializeComponent 设置为 true
	bWantsInitializeComponent = true;

	// 使用 SetIsReplicatedByDefault 方法 开启 成员属性的网络同步，默认是不开启的
	SetIsReplicatedByDefault(true);
	// ...
}

void UInventoryComponent::InitializeComponent()
{
	// 只在服务器上初始化
	if (GetOwner()->HasAuthority())
	{
		for (auto ItemClass : DefaultItems)
		{
			InventoryList.AddItem(ItemClass);
		}
	}
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	// 是否 有数据 被序列化
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// 开始遍历背包，复制其中每一个 item元素
	for (FInventoryListItem Item : InventoryList.GetItemsRef())
	{
		// 获取实例
		UInventoryItemInstance* ItemInstance = Item.ItemInstance;
		if (IsValid(ItemInstance))
		{
			// UActorChannel* Channel：指定传输数据
			WroteSomething = Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bShowDebug = CVarShowInventory.GetValueOnGameThread() != 0;
	if (bShowDebug)
	{
		for (FInventoryListItem Item : InventoryList.GetItemsRef())
		{
			// 获取实例
			UInventoryItemInstance* ItemInstance = Item.ItemInstance;
			if (IsValid(ItemInstance))
			{
				// 获取默认 成员对象
				const UItemStaticData* ItemStaticData =  ItemInstance->GetItemStaticData();
				if (GetOwner()->GetLocalRole() == ROLE_Authority)
				{
					GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("Server Item: %s  id== %d"), *ItemStaticData->Name.ToString(), ItemStaticData->Id));
				} else
				{
					GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("Client Item: %s  id== %d"), *ItemStaticData->Name.ToString(), ItemStaticData->Id));
				}
			}
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInventoryComponent, InventoryList);
}

