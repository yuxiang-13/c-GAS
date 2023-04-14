// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile.h"

#include "ActionGameStatic.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

/*
 *TAutoConsoleVariable是一个宏定义，用于创建控制台命令变量
 * 控制台命令变量在UE5中可以用于动态修改游戏参数，例如控制游戏的采样率、帧率、物理系统参数等。
 * 在运行时，可以通过输入控制台命令（变量名称+变量值）来修改控制台命令变量的值，从而改变游戏参数。
 */
// 调试Debug
static TAutoConsoleVariable<int32> CVarShowProjectiles(TEXT("ShowDebugProjectiles"),
	0, //控制台命令变量的初始值，默认值为0
	TEXT("Draws debug info about Projectiles/n    0: off/n    1: on/n"), //控制台命令变量的帮助文本，可以为NULL。
	ECVF_Cheat); //控制台命令变量的标识位，用于指定变量的使用权限。可用标识位包括：可通过配置文件进行修改、只读变量、命令行参数、不使用外部命令行参数等。
//ECVF_Cheat : 隐藏在控制台中，用户无法更改
//..其他的我也不清楚。需要看文档，就先不说了

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	// 开启移动复制
	SetReplicateMovement(true);
	bReplicates = true;
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->Velocity = FVector::Zero();
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AProjectile::OnProjectileStop);

	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	// 当Actor的bReplicates属性设置为true时，它的所有组件都会默认进行网络同步，不需要单独设置SetIsReplicated(true)
	// StaticMeshComponent->SetIsReplicated(true);
	StaticMeshComponent->SetCollisionProfileName(TEXT("Projectile"));
	StaticMeshComponent->bReceivesDecals = false; // 警用贴花
}

const UProjectilesStaticData* AProjectile::GetProjectilesStaticData() const
{
	if (IsValid(ProjectileDataClass))
	{
		return GetDefault<UProjectilesStaticData>(ProjectileDataClass);
	}
	return nullptr; 
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 获取 榴弹静态数据
	const UProjectilesStaticData* ProjectileData = GetProjectilesStaticData();
	if (ProjectileData && ProjectileMovementComponent)
	{
		if (ProjectileData->StaticMesh)
		{
			StaticMeshComponent->SetStaticMesh(ProjectileData->StaticMesh);
		}
		// 设置 运动
		//  = false 使用世界坐标系进行述投射物的初始速度设置 true 是本地坐标系
		ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
		ProjectileMovementComponent->InitialSpeed = ProjectileData->InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = ProjectileData->MaxSpeed;
		// 投射物的旋转是否跟随投射物的速度方向
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		// 反弹
		ProjectileMovementComponent->bShouldBounce = false;
		// 值为0 投射物将不具备弹性   1具有最大的弹性
		ProjectileMovementComponent->Bounciness = 0.f;
		ProjectileMovementComponent->ProjectileGravityScale = ProjectileData->GravityMultiplayer;

		// 启动飞行
		ProjectileMovementComponent->Velocity = ProjectileData->InitialSpeed * GetActorForwardVector();
	}

	const int32 DebugShowProjectile = CVarShowProjectiles.GetValueOnAnyThread();
	if (DebugShowProjectile)
	{
		DebugDrawPath();
	}
}


void AProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	// 获取 榴弹静态数据
	const UProjectilesStaticData* ProjectileData = GetProjectilesStaticData();
	if (ProjectileData)
	{
		// 调用蓝图库函数 应用GE
		UActionGameStatic::ApplyRadialDamage(this,
			GetOwner(),
			GetActorLocation(),
			ProjectileData->DamageRadius,
			ProjectileData->BaseDamage,
			ProjectileData->Effects,
			ProjectileData->RadialDamageQueryTypes,
			ProjectileData->RadialDamageTraceType
		);
	}
	Destroy();
}

// 画出抛射线
void AProjectile::DebugDrawPath() const
{
	// 获取 榴弹静态数据
	const UProjectilesStaticData* ProjectileData = GetProjectilesStaticData();
	if (ProjectileData)
	{
		// 使用 预测碰撞弹道路径  FPredictProjectilePathParams
		FPredictProjectilePathParams PredictProjectilePathParams;
		// 起始位置，即发射点
		PredictProjectilePathParams.StartLocation = GetActorLocation();
		// 发射速度
		PredictProjectilePathParams.LaunchVelocity = ProjectileData->InitialSpeed * GetActorForwardVector();
		// 碰撞检测通道
		PredictProjectilePathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
		// 是否使用复杂碰撞检测
		PredictProjectilePathParams.bTraceComplex = true;
		// 是否启用碰撞检测
		PredictProjectilePathParams.bTraceWithCollision = true;
		PredictProjectilePathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
		PredictProjectilePathParams.DrawDebugTime = 3.f;
		// 重力 // Optional override of Gravity (if 0, uses WorldGravityZ). 0 也有重力、、
		PredictProjectilePathParams.OverrideGravityZ = ProjectileData->GravityMultiplayer == 0 ? 0.0001 : ProjectileData->GravityMultiplayer;

		// 开始预测
		FPredictProjectilePathResult PredictProjectilePathResult;
		if (UGameplayStatics::PredictProjectilePath(this, PredictProjectilePathParams, PredictProjectilePathResult))
		{
			// 检测碰撞成功
			DrawDebugSphere(GetWorld(), PredictProjectilePathResult.HitResult.Location, 50, 10, FColor::Red);
		}
	}
}

void AProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 获取 榴弹静态数据
	const UProjectilesStaticData* ProjectileData = GetProjectilesStaticData();
	if (ProjectileData)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ProjectileData->OnStopSFX, GetActorLocation(), 0.2f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ProjectileData->OnStapVFX, GetActorLocation());
	}
	
	Super::EndPlay(EndPlayReason);
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile, ProjectileDataClass);
}
