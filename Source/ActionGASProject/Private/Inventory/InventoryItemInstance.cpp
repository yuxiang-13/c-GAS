// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItemInstance.h"

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
		ItemActor->FinishSpawning(Transform);

		// 检测骨架
		if (ACharacter* Character = Cast<ACharacter>(InOwer))
		{
			if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
			{
				ItemActor->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
			}
		}
		

	}
}

void UInventoryItemInstance::OnUnEquipped()
{
	if (ItemActor)
	{
		ItemActor->Destroy();
		ItemActor = nullptr;
	}
}

void UInventoryItemInstance::OnDropped()
{
	if (ItemActor)
	{
		ItemActor->OnDropped();
	}
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemInstance, ItemStaticDataClass);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventoryItemInstance, bEquipped, COND_None, REPNOTIFY_Always); 
	DOREPLIFETIME(UInventoryItemInstance, ItemActor);
}
