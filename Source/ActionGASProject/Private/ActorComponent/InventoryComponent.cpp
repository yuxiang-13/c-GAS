// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActionGameTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/ActorChannel.h"
#include "Inventory/InventoryItemInstance.h"
#include "Kismet/KismetSystemLibrary.h"
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


// 初始化静态变量
FGameplayTag UInventoryComponent::EquipItemActorTag;
FGameplayTag UInventoryComponent::DropItemTag;
FGameplayTag UInventoryComponent::EquipNextTag;
FGameplayTag UInventoryComponent::UnEquipTag;

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

	// 为游戏增加GameplayTags的代理，在 游戏启动时自动触发
	// 注意 是 UGameplayTagsManager::::Get() 不是 UGameplayTagsManager（）.Get()
	UGameplayTagsManager::Get().OnLastChanceToAddNativeTags().AddUObject(this, &UInventoryComponent::AddInventoryTags);
}

void UInventoryComponent::AddInventoryTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	/**
	*将给定的名称注册为游戏标签，并跟踪它是否直接从代码中引用
	*这只能在引擎初始化期间调用，复制之前需要锁定表
	*/
	UInventoryComponent::EquipItemActorTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipItem"), TEXT("Equip Item Form Item Actor Event"));
	UInventoryComponent::DropItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.DropItem"), TEXT("Drop Equipped Item"));
	UInventoryComponent::EquipNextTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipNext"), TEXT("Try Equip Next Item"));
	UInventoryComponent::UnEquipTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.UnEquip"), TEXT("UnEquip Current Item"));

	// 使用完了，给移除
	TagsManager.OnLastChanceToAddNativeTags().RemoveAll(this);
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

	
	// 代理，我们要绑定 代理，代理传递的参数是 GameplayEventData,里面有各种标签，这样，标签代表不同处理
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		// 绑定 GameplayEvent 响应事件代理。这样相应的事件被触发时，回调函数就会被执行
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipItemActorTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::DropItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipNextTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::UnEquipTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
	}
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	// 默认第一个是默认装备的武器
	// if (InventoryList.GetItemsRef().Num())
	// {
	// 	EquipItem(InventoryList.GetItemsRef()[0].ItemInstance->ItemStaticDataClass);
	// }

	// UnEquipItem();
	//DropItem()
	// DropItem();
}


void UInventoryComponent::GameplayEventCallback(const FGameplayEventData* Playload)
{
	ENetRole NetRole = GetOwnerRole();
	
	if (NetRole == ROLE_Authority)
	{
		HandleGameplayEventInternal(*Playload);
	} else if (NetRole == ROLE_AutonomousProxy) // 自主代理，玩家自己
	{
		ServerHandleGameplayEvent(*Playload);
	}
}

void UInventoryComponent::ServerHandleGameplayEvent_Implementation(FGameplayEventData PayLoad)
{
	HandleGameplayEventInternal(PayLoad);
}

void UInventoryComponent::HandleGameplayEventInternal(FGameplayEventData PayLoad)
{
	ENetRole NetRole = GetOwnerRole();
	if (NetRole == ROLE_Authority)
	{
		// 获取事件中的 Tag
		// FGameplayTag EventTag = PayLoad.EventTag;
		if (PayLoad.EventTag == UInventoryComponent::EquipItemActorTag)
		{
			if (const UInventoryItemInstance* ItemInstance = Cast<UInventoryItemInstance>(PayLoad.OptionalObject))
			{
				// 手动把 const 强转
				// AddItemInstance(ItemInstance);
				AddItemInstance(const_cast<UInventoryItemInstance*>(ItemInstance));
				if (PayLoad.Instigator)
				{
					// AItemActor::OnSphereOverlap 转 this，我们要删除他
					const_cast<AActor*>(PayLoad.Instigator.Get())->Destroy();
				}
			}
		} else if (PayLoad.EventTag == UInventoryComponent::EquipNextTag)
		{
			EquipNext();
		} else if (PayLoad.EventTag == UInventoryComponent::DropItemTag)
		{
			DropItem();
		} else if (PayLoad.EventTag == UInventoryComponent::UnEquipTag)
		{
			UnEquipItem();
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
			
			WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UInventoryComponent::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemStaticDataClass);
	}
}


void UInventoryComponent::AddItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemInstance);
	}
}


// 装备下一件装备
void UInventoryComponent::EquipNext()
{
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();

	const bool bNoItems = Items.Num() == 0;

	if (bNoItems) return;

	UInventoryItemInstance* TargetItem = CurrentItem;
	// 有些物品无法装备，所以需要遍历,找出合适的装备
	for (auto Item: Items)
	{
		// 检测是否可以装备
		if (Item.ItemInstance->GetItemStaticData()->bCanBeEquipped)
		{
			// 不是当前装备
			if (Item.ItemInstance != CurrentItem)
			{
				TargetItem = Item.ItemInstance;
				break;
			}
		}
	}

	if (CurrentItem == TargetItem)
	{
		return;
	} else
	{
		// 卸载老装备
		UnEquipItem();	
		EquipItemInstance(TargetItem);	
	}
}

// 通过参数判断背包中是否存在此装备，再进行穿戴
void UInventoryComponent::EquipItemInstance(UInventoryItemInstance* InItemInstance)
{	
	// 只在服务器上初始化
	if (GetOwner()->HasAuthority())
	{		
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance == InItemInstance)
			{
				// 直接触发 背包元素的 装戴虚方法
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				break;
			}
		}
	}
}

void UInventoryComponent::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItem(InItemStaticDataClass);
	}
}

// 通过参数判断背包中是否存在此装备，再进行穿戴
void UInventoryComponent::EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	// 只在服务器上初始化
	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance->ItemStaticDataClass == InItemStaticDataClass)
			{
				// 直接触发 背包元素的 装戴虚方法
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				break;
			}
		}
	}
}

void UInventoryComponent::UnEquipItem()
{
	// 只在服务器上初始化
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnUnEquipped(GetOwner());
			CurrentItem = nullptr;
		}
	}
}

void UInventoryComponent::DropItem()
{
	// 只在服务器上初始化
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnDropped(GetOwner());
			// 背包数据中移除这个元素
			RemoveItem(CurrentItem->ItemStaticDataClass);
			
			CurrentItem = nullptr;
		}
	}
}

UInventoryItemInstance* UInventoryComponent::GetEquippedItem() const
{
	return CurrentItem;
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
			const UItemStaticData* ItemStaticData = ItemInstance->GetItemStaticData();
			if (IsValid(ItemInstance) && IsValid(ItemStaticData))
			{
				if (GetOwner()->HasAuthority())
				{
					// 获取默认 成员对象
					GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,FString::Printf(TEXT(" server Item: %s"), *ItemStaticData->Name.ToString()));
				} else
				{
					// 获取默认 成员对象
					GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,FString::Printf(TEXT(" client Item: %s"), *ItemStaticData->Name.ToString()));
				}
			}
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInventoryComponent, InventoryList);
	DOREPLIFETIME(UInventoryComponent, CurrentItem);
}

