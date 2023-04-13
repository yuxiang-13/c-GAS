// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ItemActors/WeaponItemActor.h"

#include "GameFramework/Character.h"
#include "Inventory/InventoryItemInstance.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/AG_PhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"

AWeaponItemActor::AWeaponItemActor()
{
}

const UWeaponStaticData* AWeaponItemActor::GetWeaponStaticData() const
{
	// 指向背包元素指针
	return ItemInstance ? Cast<UWeaponStaticData>(ItemInstance->GetItemStaticData()) : nullptr;
}

FVector AWeaponItemActor::GetMuzzleLocation() const
{
	return MeshComponent ? MeshComponent->GetSocketLocation(TEXT("Muzzle")) : GetActorLocation();
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

				MeshComponent = SkeletalComp;
				if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
				{
					MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData->Name);
				} else
				{
					MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
			}
		} else if (WeaponData->StaticMesh)
		{
			UStaticMeshComponent* StaticComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
			StaticComp->RegisterComponent();
			StaticComp->SetStaticMesh(WeaponData->StaticMesh);
			StaticComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			
			MeshComponent = StaticComp;
			if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
			{
				MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData->Name);
			} else
			{
				MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}
}


void AWeaponItemActor::PlayWeaponEffects(const FHitResult& InHitResult)
{
	if (HasAuthority())
	{
		MulticastPlayWeaponEffects(InHitResult);
	} else
	{
		PlayWeaponEffectsIternal(InHitResult);
	}
}

void AWeaponItemActor::MulticastPlayWeaponEffects_Implementation(const FHitResult& InHitResult)
{
	// Owner是一个指向控制该Actor的Controller的引用，它通常用于在多人游戏中跟踪Actor的所有权以及与该Actor相关联的PlayerController。
	// 代理一共三种：
	/*
	 * Authority、Simulated Proxy、Autonomous Proxy
	*
		If (HasAuthority){ //如果在服务端
			// Authority存在于服务器
		} 
		else { //在客户端
			// Autonomous Proxy表示自己客户端控制的角色
			// Simulated Proxy 表示除了 自己客户的角色以外的 其他的角色在该客户端都是模拟的
		} 
	 */
	if (!Owner || Owner->GetLocalRole() != ROLE_AutonomousProxy)
	{
		PlayWeaponEffectsIternal(InHitResult);
	}
}

void AWeaponItemActor::PlayWeaponEffectsIternal(const FHitResult& InHitResult)
{
	// 根据碰撞物 物理材质生成音效
	if (InHitResult.PhysMaterial.Get())
	{
		UAG_PhysicalMaterial* PhysicalMaterial = Cast<UAG_PhysicalMaterial>(InHitResult.PhysMaterial.Get());
		if (PhysicalMaterial)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PhysicalMaterial->PointImpactSound, InHitResult.ImpactPoint, 0.2f);

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PhysicalMaterial->PointImpactVFX, InHitResult.ImpactPoint);
		}
	}

	// 无论是否命中 武器声
	if (const UWeaponStaticData* weaponData = GetWeaponStaticData())
	{
		UGameplayStatics::PlaySoundAtLocation(this, weaponData->AttackSound, GetActorLocation(), 0.2f);
	}
}