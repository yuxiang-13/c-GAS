// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/GA_Vault.h"

#include "ActionGASProject/ActionGASProjectCharacter.h"
#include "ActorComponent/AG_MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_Vault::UGA_Vault()
{
	// 1 用于表示某个Gameplay Ability的网络执行策略
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	/*
	*enum Type
	{
		// 表示 Gameplay Ability 在当前客户端上首先执行本地预测，并在之后在服务端上同步。如果客户端和服务端状态相同，则不需要额外的状态纠正操作。
		// 该选项适用于对实时性有严格要求的 Gameplay Ability 。例如角色的移动、攻击等。
		LocalPredicted		UMETA(DisplayName = "Local Predicted"),

		// 表示 Gameplay Ability 只在当前客户端本地执行。在网络环境下，该选项适用于无需影响其他客户端和服务端状态的 Gameplay Ability。
		LocalOnly			UMETA(DisplayName = "Local Only"),

		// 表示 Gameplay Ability 在服务端上先执行，然后在客户端上同步。该选项适用于只能由服务端上当前 Actor 引发的影响其他 Actor 的 Gameplay Ability 。
		ServerInitiated		UMETA(DisplayName = "Server Initiated"),

		// 表示 Gameplay Ability 只在服务端执行，客户端无法执行抵用。该选项适用于只对服务端状态造成影响的 Gameplay Ability 。
		ServerOnly			UMETA(DisplayName = "Server Only"),
	};
	* 
	*/

	//1 InstancingPolicy 表示Gameplay Ability实例化策略的枚举值
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	/*
		*enum Type
		{
			// 表示 Gameplay Ability 使用已经存在的 Ability 实例。相当于将所有使用该 Ability 的对象共享同一个 Ability 实例。
			// 适用于那些不需要创建新实例的特定 Ability。由于不需要创建新实例，可以减少计算和资源开销
			NonInstanced,
	
			// 表示每个 Actor 创建一个新的 Ability 实例。适用于每个 Actor 运行自己的实例的情况，例如一些身体动作或类似玩家控制的操作。
			InstancedPerActor,
	
			// 表示每次执行 Ability 都创建一个新的 Ability 实例。适用于那些多个 Actor 运行不同实例的 Ability 类，
			例如施法者对多个目标进行施法，每个目标都需要一个新的 Ability 实例。
			InstancedPerExecution,
		};
	 */
}

bool UGA_Vault::CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	// 如果检测通过
	if (! Super::CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	AActionGASProjectCharacter* Character = GetActionGameCharacterFromActorInfo();
	if (! IsValid(Character))
	{
		return false;
	}

	const FVector StartLocation = Character->GetActorLocation();
	const FVector ForwardVector = Character->GetActorForwardVector();
	// 跳跃 向上
	const FVector UpVector = Character->GetActorUpVector();

	// 设想忽略
	TArray<AActor*> ActorsToIgnore = {Character};

	// 获取调试器中的 变量 (根据这个变量，显示 隐藏 射线)
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugTraversal"));
	const bool bShowTraversal = CVar->GetInt() > 0;
	EDrawDebugTrace::Type DebugDrawType = bShowTraversal ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool bJumpToLocationSet = false;
	int32 JumpToLocationIdx = INDEX_NONE;

	// 跟踪索引
	int i = 0;
	FHitResult TraceHit;
	// 跳跃的最大距离 (先设置成固定的，后需会根据 到障碍物的距离进行设置)
	float MaxJumpDistance = HorizontalTraceLength;
	// 开始横向走线 从地面开始 并向上一层层的发射
	for (; i < HorizontalTraceCount; ++i)
	{
		// 起始位置 = 地面位置 + （i 乘以 （向上向量 乘以 步长））
		const FVector TraceStart = StartLocation + i * UpVector * HorizontalTraceStep;
		const FVector TraceEnd = TraceStart + ForwardVector * HorizontalTraceLength;

		// 球形 检测 （不用线性是因为 会根据点检测，检测不全 漏掉一些障碍物）
		if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, TraceEnd, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			// ******
			// 检测到碰撞物了
			// ******
			
// 第一次循环，初始化看看是否有一个障碍物，有就满足起跳条件了
			//JumpToLocationIdx == INDEX_NONE 表示 第一次检测 脚前 检测,并且不是最后那次检测
			if (JumpToLocationIdx == INDEX_NONE && (i < HorizontalTraceCount - 1))
			{
				JumpToLocationIdx = i;
				JumpToLocation = TraceHit.Location;
// 第二次循环，这个障碍物 是否别第二次 射线也检测到，检测成功，就可以初始化最大跳跃距离
			} else if (JumpToLocationIdx == (i - 1))
			{
				// 这次循环表示： 脚前检测 确实有障碍物，并且射线检测高度提升一次后，也产生了碰撞
				// 那就可以初始化这次 最大跳跃距离了（这个距离是 射线起始点 和 碰撞点 距离）是 float
				MaxJumpDistance = FVector::Dist2D(TraceHit.Location, TraceStart);
				break;
			}
		} else
		{
			// ******
			// 未 检测到碰撞物
			// ******

			// 看，如果我们记录着 起跳索引，那就有两种情况
			// 1: 第一种，检测到 脚前水平检测 确实存在障碍物，但是接下来的检测未检测到障碍物，那就说明障碍物高度不高,可以起跳
			// 2: 第一种，未检测到 脚前水平检测，那就继续循环
			if (JumpToLocationIdx != INDEX_NONE)
			{
				break;
			}
		}
	}

	if (JumpToLocationIdx == INDEX_NONE)
	{
		return false;
	}

	// 算出 人物起始点 和 脚射线 碰撞物点 的距离
	const float DistanceToJumpTo = FVector::Dist2D(StartLocation, JumpToLocation);

	// 这里表示  (float - float)
	// MaxJumpDistance : 意味着第二次射线检测结果到 腰的距离
	// DistanceToJumpTo : 意味着人物起始点 和 脚射线 碰撞物点 的距离
	// 也就是说，圆锥障碍物，如果是正立的圆锥，那就可以跳，倒立的圆锥，那就算了，不能跳，表示必须是有个坡度的 或者垂直的
	const float MaxVerticalTraceDistance = MaxJumpDistance - DistanceToJumpTo; 
	if (MaxVerticalTraceDistance < 0)
	{
		return false;
	}

	
// 开始计算垂直走线 检测我们跳跃到障碍物的 尽头，或者是否可以越过障碍物
	// 已经是检测完了， i ++ 后已经等于最大检测次数了 ,如果是最大次数，减回正确的步骤数
	if (i == HorizontalTraceCount)
	{
		i = HorizontalTraceCount - 1;
	}
	// 计算出 水平起跳点 和 水平目标点的 Z高度 （这个不是最终的高度 因为目标位置只是第二次 检测到障碍物的 射线）
	const float VerticalTraceLength = FMath::Abs(JumpToLocation.Z - (StartLocation + i * UpVector * HorizontalTraceStep).Z);

	// 起始 垂直点 = 水平目标点的向上一顿距离的点
	FVector VerticalStartLocation = JumpToLocation + UpVector * VerticalTraceLength;
	i = 0;

	// 算出垂直射线的 数量
	const float VerticalTraceCount = MaxVerticalTraceDistance / VerticalTraceStep;
	
	bool bJumpOverLocationSet = false;
	for (; i <= VerticalTraceCount; i++)
	{
		// 射线起点 = 起始 垂直点 + 向上向量 乘以 间隔
		const FVector TraceStart = VerticalStartLocation + (i * ForwardVector * VerticalTraceStep);
		const FVector TraceEnd = TraceStart - (UpVector * VerticalTraceLength);
		if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, TraceEnd, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			// *************
			// 成功碰撞了
			// *************
			
			// 最终跳跃位置 设置为撞击点
			JumpOverLocation = TraceHit.ImpactPoint;
			// i== 0 第一次垂直碰撞了
			if (i == 0)
			{
				// 这时，因为我们确定了水平撞击点，也确定了这个点 垂直方向上，与障碍物的Z，那就直接设置成 水平撞击点位置，这下连Z轴都确定了
				JumpToLocation = JumpOverLocation;
				// ok了，已经存在第一次垂直碰撞点了
				bJumpOverLocationSet = true;
			}
		}
	}

	if (!bJumpOverLocationSet)
	{
		return false;
	}

	
//  okokok 此时我们设置了第一次碰撞点的位置（水平和高度上的位置都设置了）
// 开始寻找 障碍物水平最后那个 起跳Over点
	
	// 射线起始点，是障碍物第一个攀爬点 + 向前一个 垂直间距的 距离
	const FVector TraceStart = JumpOverLocation + ForwardVector + VerticalTraceStep;
	if (UKismetSystemLibrary::SphereTraceSingleForObjects(this, TraceStart, JumpOverLocation, HorizontalTraceRadius, TraceObjectTypes, true, ActorsToIgnore, DebugDrawType, TraceHit, true))
	{
		// 记录最终碰撞点
		JumpOverLocation = TraceHit.ImpactPoint;
	}
	// 画出 攀爬点和最终跳跃点
	if (bShowTraversal)
	{
		DrawDebugSphere(GetWorld(), JumpToLocation, 15, 16 , FColor::White, false, 7);
		DrawDebugSphere(GetWorld(), JumpOverLocation, 15, 16 , FColor::White, false, 7);
	}
	return true;
}

void UGA_Vault::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 需要 手动执行 刚刚写好的那个检测函数
	if (!CommitCheck(Handle, ActorInfo, ActivationInfo, nullptr))
	{
		K2_EndAbility();
		return;
	}

	AActionGASProjectCharacter* Character = GetActionGameCharacterFromActorInfo();
	
	UCharacterMovementComponent* CharacterMovement = Character ? Character->GetCharacterMovement() : nullptr;
	if (CharacterMovement)
	{
		// 设置飞行模式
		CharacterMovement->SetMovementMode(MOVE_Flying);
		// CharacterMovement->GravityScale = 0;
		UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT("-MOVE_Flying - ->>>> %f")) , true, true, FLinearColor::Red, 10.f);
		
	}

	UCapsuleComponent* CapsuleComponent = Character ? Character->GetCapsuleComponent() : nullptr;
	if (CapsuleComponent)
	{
		for (ECollisionChannel Channel: CollisionChannelsToIgnore)
		{
			CapsuleComponent->SetCollisionResponseToChannel(Channel, ECollisionResponse::ECR_Ignore);
		}
	}

	UAG_MotionWarpingComponent* MotionWarpingComponent = Character ? Character->GetAGMotionWarpingComponent() : nullptr;
	if (MotionWarpingComponent)
	{
		// 设置动作扭曲的 第一个目标点
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("JumpToLocation"), JumpToLocation, Character->GetActorRotation());
		// 设置动作扭曲的 第二个目标点
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("JumpOverLocation"), JumpOverLocation, Character->GetActorRotation());

		MotionWarpingComponent->SendWarpPointsToClients();
	}

	// 播放蒙太奇
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, VaultMontage);
	// 打断时 直接GA停止
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Vault::K2_EndAbility);
	// 完成 直接GA停止
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Vault::K2_EndAbility);
	// 中断 直接GA停止
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Vault::K2_EndAbility);
	// 取消 直接GA停止
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Vault::K2_EndAbility);

	// 激活任务
	MontageTask->ReadyForActivation();
}

void UGA_Vault::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	
	// 结束任务
	if (IsValid(MontageTask))
	{
		MontageTask->EndTask();
	}
	AActionGASProjectCharacter* Character = GetActionGameCharacterFromActorInfo();
	
	UCharacterMovementComponent* CharacterMovement = Character ? Character->GetCharacterMovement() : nullptr;
	if (CharacterMovement)
	{
		
		CharacterMovement->SetMovementMode(MOVE_Walking);
		// CharacterMovement->GravityScale = 1.75;
		// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT("-MOVE_Walking - ->>>> %f")) , true, true, FLinearColor::Red, 10.f);

	}
	// 恢复碰撞
	UCapsuleComponent* CapsuleComponent = Character ? Character->GetCapsuleComponent() : nullptr;
	if (CapsuleComponent)
	{
		for (ECollisionChannel Channel: CollisionChannelsToIgnore)
		{
			CapsuleComponent->SetCollisionResponseToChannel(Channel, ECollisionResponse::ECR_Block);
		}
	}
	// 清除动作扭曲的 目标点
	UAG_MotionWarpingComponent* MotionWarpingComponent = Character ? Character->GetAGMotionWarpingComponent() : nullptr;
	if (MotionWarpingComponent)
	{
		MotionWarpingComponent->RemoveWarpTarget(TEXT("JumpToLocation"));
		MotionWarpingComponent->RemoveWarpTarget(TEXT("JumpOverLocation"));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
