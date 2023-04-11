// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ItemActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ActorComponent/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Inventory/InventoryItemInstance.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 开启网络同步
	bReplicates = true;
	// 开启移动同步
	SetReplicateMovement(true);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);
}


// 通过数据 实例化 Actor
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 指向背包元素的指针 && 静态数据类 都存在
		if (!IsValid(ItemInstance) && IsValid(ItemStaticDataClass))
		{
			ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Init(ItemStaticDataClass);
			
			MulticastCollisionEnabled(true);
			
	        InitInternal();
		}
	}
}

// 背包装备时触发函数 ，并且这个发生在服务器
void AItemActor::Init(UInventoryItemInstance* InInstance)
{
	// 服务器也要初始化  UInventoryComponent::EquipItemInstance -> UInventoryItemInstance::OnEquipped
	ItemInstance = InInstance;
	
	InitInternal();
}



void AItemActor::OnRep_ItemInstance(UInventoryItemInstance* OldItemInstance)
{
	// 保护
	if (IsValid(ItemInstance) && IsValid(OldItemInstance))
	{
		InitInternal();
	}
}

void AItemActor::InitInternal()
{
	// 此函数 发生在服务器
	
}

// 声明一些以后会用到虚方法
void AItemActor::OnEquipped()
{
	ItemState = EItemState::Equipped;

	
	MulticastCollisionEnabled(false);
};
void AItemActor::OnUnEquipped()
{
	ItemState = EItemState::None;
	MulticastCollisionEnabled(false);
};
// 丢弃
void AItemActor::OnDropped()
{
	// 首先要知道，物品在咱们手上时候，已经关闭了碰撞  OnEquipped() 函数，所以我们要在 物体接触到地面位置后, 开启碰撞
	
	ItemState = EItemState::Dropped;
	// 物品分离
	GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (AActor* ActorOwner = GetOwner())
	{
		const FVector Location = GetActorLocation();
		const FVector Forward = ActorOwner->GetActorForwardVector();

		const float droppItemDist = 100.f;
		const float droppItemTraceDist = 10000.f;

		const FVector TraceStart = Location + Forward * droppItemDist; // 水平一段距离
		const FVector TraceEnd = TraceStart - FVector::UpVector * droppItemTraceDist; // 垂直向下

		
		// 设想忽略
		TArray<AActor*> ActorsToIgnore = {ActorOwner};
		FHitResult TraceHit;

		static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugInventory"));
		const bool bShowTraversal = CVar->GetInt() > 0;
		EDrawDebugTrace::Type DebugDrawType = bShowTraversal ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		// 开始解决这个问题
		FVector TargetLocation = TraceEnd;
		
		// 放到地面
		if (UKismetSystemLibrary::LineTraceSingleByProfile(this, TraceStart, TraceEnd, TEXT("worldStatic"), true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			if (TraceHit.bBlockingHit)
			{
				TargetLocation = TraceHit.Location;
			}
		}
		SetActorLocation(TargetLocation);

		MulticastCollisionEnabled(true);
	}

};

bool AItemActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	// 是否 有数据 被序列化
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (IsValid(ItemInstance))
	{
		// UActorChannel* Channel：指定传输数据
		// `|=` 是按位或赋值运算符 , 运算符只对整数类型有效
		WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void AItemActor::MulticastCollisionEnabled_Implementation(bool bFlag)
{
	if (bFlag)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// 是否生成重叠事件（Overlap Event）。其中，`true`表示要开启碰撞重叠事件的生成，`false`表示不开启。
		// 开启碰撞查询
		SphereComponent->SetGenerateOverlapEvents(true);
	} else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		FGameplayEventData EventPlayload;
		EventPlayload.Instigator = this;
		// OptionalObject 任意对象
		EventPlayload.OptionalObject = ItemInstance; // 指向背包元素的指针
		EventPlayload.EventTag = UInventoryComponent::EquipItemActorTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, UInventoryComponent::EquipItemActorTag, EventPlayload);
	}
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AItemActor, ItemInstance);
	DOREPLIFETIME(AItemActor, ItemState);
}
