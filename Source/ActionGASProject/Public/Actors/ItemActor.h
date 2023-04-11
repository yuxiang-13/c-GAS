// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UInventoryItemInstance;
class USphereComponent;

UCLASS()
class ACTIONGASPROJECT_API AItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemActor();

	void Init(UInventoryItemInstance* InInstance);

	// 声明一些以后会用到虚方法
	virtual void OnEquipped();
	virtual void OnUnEquipped();
	// 丢弃
	virtual void OnDropped();

	// ReplicateSubobjects 函数的作用是将一个 UObject 类型的子对象在网络上进行同步
	/*
	当 ReplicateSubobjects 为 false 时，子对象不会被同步到客户端，这可以提高网络性能。
	当 ReplicateSubobjects 为 true 时，如果一个 UObject 类型的对象在服务器上创建或修改了它的子对象，这些修改会被传播到与该对象相关联的客户端。这使得客户端能够与服务器上的对象保持同步
	
		参数1 UActorChannel* Channel：指定传输数据
	 */
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	// 指向背包元素的指针
	// UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_ItemInstance)
	UInventoryItemInstance* ItemInstance = nullptr;

	// 1 注意：这个同步的变量在服务器端发生改变后，第一步是该变量的新值将传输到客户端。然后客户端接收到新值后，才会触发`ReplicatedUsing`函数
	UFUNCTION()
	void OnRep_ItemInstance(UInventoryItemInstance* OldItemInstance);

	// 提供子类虚方法，当道具被装备时进行触发
	virtual void InitInternal();
	
	UPROPERTY(Replicated)
	TEnumAsByte<EItemState> ItemState = EItemState::None;

	// 碰撞
	UPROPERTY()
	USphereComponent* SphereComponent = nullptr;


	// 广播开启碰撞
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCollisionEnabled(bool bFlag);


	//DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SixParams( FComponentBeginOverlapSignature, UPrimitiveComponent, OnComponentBeginOverlap,
	// UPrimitiveComponent*, OverlappedComponent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult &, SweepResult);

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	// 声明一个静态数据类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemStaticData> ItemStaticDataClass;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
