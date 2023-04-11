// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ItemActors/WeaponItemActor.h"

#include "Inventory/InventoryItemInstance.h"

AWeaponItemActor::AWeaponItemActor()
{
}

const UWeaponStaticData* AWeaponItemActor::GetWeaponStaticData() const
{
	// 指向背包元素指针
	return ItemInstance ? Cast<UWeaponStaticData>(ItemInstance->GetItemStaticData()) : nullptr;
}

void AWeaponItemActor::InitInternal()
{
	Super::InitInternal();
	
	if (const UWeaponStaticData* WeaponData = GetWeaponStaticData())
	{
		if (WeaponData->SkeletalMesh)
		{
			USkeletalMeshComponent* SkeletalComp = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), TEXT("MeshComponent"));
			if (SkeletalComp)
			{
				// 注册 注册后actor才能进行获取以及其他操作
				SkeletalComp->RegisterComponent();
				SkeletalComp->SetSkeletalMesh(WeaponData->SkeletalMesh);
				SkeletalComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

				MeshComponent = SkeletalComp;
			}
		} else if (WeaponData->StaticMesh)
		{
			UStaticMeshComponent* StaticComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
			StaticComp->RegisterComponent();
			StaticComp->SetStaticMesh(WeaponData->StaticMesh);
			StaticComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			
			MeshComponent = StaticComp;
		}
	}
}
