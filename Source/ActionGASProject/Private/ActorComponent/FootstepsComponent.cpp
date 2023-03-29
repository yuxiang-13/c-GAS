// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/FootstepsComponent.h"

#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/AG_PhysicalMaterial.h"

/*
 *TAutoConsoleVariable是一个宏定义，用于创建控制台命令变量
 * 控制台命令变量在UE5中可以用于动态修改游戏参数，例如控制游戏的采样率、帧率、物理系统参数等。
 * 在运行时，可以通过输入控制台命令（变量名称+变量值）来修改控制台命令变量的值，从而改变游戏参数。
 */
// 调试Debug
static TAutoConsoleVariable<int32> CVarShowFootsteps(TEXT("ShowFootsteps"),
	0, //控制台命令变量的初始值，默认值为0
	TEXT("Draws debug info about footsteps/n    0: off/n    1: on/n"), //控制台命令变量的帮助文本，可以为NULL。
	ECVF_Cheat); //控制台命令变量的标识位，用于指定变量的使用权限。可用标识位包括：可通过配置文件进行修改、只读变量、命令行参数、不使用外部命令行参数等。
//ECVF_Cheat : 隐藏在控制台中，用户无法更改
//..其他的我也不清楚。需要看文档，就先不说了


// Sets default values for this component's properties
UFootstepsComponent::UFootstepsComponent()
{

	// 关闭Tick事件
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UFootstepsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UFootstepsComponent::HandleFootstep(EFoot Foot)
{
	if (AActionGASProjectCharacter* Character =Cast<AActionGASProjectCharacter>(GetOwner()))
	{
		// Debug
		int32 DebugShowFootsteps = CVarShowFootsteps.GetValueOnAnyThread();
		
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			// 获取脚插槽位置
			const FVector SocketLocation = Mesh->GetSocketLocation(Foot == EFoot::LEFT ? LeftFootSocketName : RightFootSocketName);

			// 开始射线检测
			FHitResult HitResult;
			const FVector Location = SocketLocation + FVector::UpVector * 20;
			// 配置 射线信息
			FCollisionQueryParams QueryParams;
			QueryParams.bReturnPhysicalMaterial = true;
			QueryParams.AddIgnoredActor(Character);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location+FVector::UpVector * -50.f, ECollisionChannel::ECC_WorldStatic, QueryParams))
			{
				if (HitResult.bBlockingHit)
				{
					if (HitResult.PhysMaterial.Get())
					{
						// 获取物理材质
						UAG_PhysicalMaterial* PhysicalMateria = Cast<UAG_PhysicalMaterial>(HitResult.PhysMaterial.Get());
						if (PhysicalMateria)
						{
							// 播放声音
							UGameplayStatics::PlaySoundAtLocation(this, PhysicalMateria->FootstepSound, Location, 0.1f, 1.f);

							if (DebugShowFootsteps > 0)
							{
								// 4. TestBaseActor：表示当前绘制文本的基准Actor对象，可以理解为文本的父对象。
								// 6. Duration：表示绘制文本的持续时间，一般使用-1表示一直持续。
								DrawDebugString(GetWorld(), Location, PhysicalMateria->GetName(), nullptr, FColor::White, 4.f);
							}
						}

						if (DebugShowFootsteps > 0)
						{
							// 2. Center：表示球体的中心位置。
							// 3. Radius：表示球体的半径。
							// 4. Segments：表示球体的分段数，分段越高，球体边缘越细腻。
							// 6. persistentlines 是否一直显示
							DrawDebugSphere(GetWorld(), Location, 16, 16, FColor::Red,false, 4.f);
						}
					} else
					{
						// 碰撞物没有物理资产
						if (DebugShowFootsteps > 0)
						{
							//2. LineStart：表示直线的起始点。
							//3. LineEnd：表示直线的终点。
							//5. bPersistentLines：是否一直显示
							//6. LifeTime：表示直线在屏幕上显示的最长时间，单位为秒。时间到后会自动消失
							//7. DepthPriority：表示直线的渲染深度，默认为0，值越大，则渲染优先级越高。
							//8. Thickness：表示直线的线条粗细，默认为1.0。
							DrawDebugLine(GetWorld(), Location, Location + FVector::UpVector * -50.f, FColor::Red, false, 4.f, 0, 1);
						}
					}
				} else
				{
					// 没发生碰撞
					if (DebugShowFootsteps > 0)
					{
						DrawDebugLine(GetWorld(), Location, Location + FVector::UpVector * -50.f, FColor::Red, false, 4.f, 0, 1);
						DrawDebugSphere(GetWorld(), Location, 16, 16, FColor::Red,false, 4.f);
					}
				}
			}
		}
	}
}

