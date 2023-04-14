// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 让此文件支持反射
#include "ActionGameTypes.generated.h"

class AItemActor;
class UGameplayEffect;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FCharacterData
{
	// ue5 GENERATED_BODY 和 GENERATED_USTRUCT_BODY 区别
	// 需要注意的是，GENERATED_BODY和GENERATED_USTRUCT_BODY宏定义虽然用途不同，但其本质上是相似的。
	// 它们都是利用了UE5中的反射机制，通过宏定义来自动生成一些代码
	//GENERATED_BODY宏定义在类声明中使用，可以用于生成类的一些标准方法（例如Construct、Tick、BeginPlay等）
	//GENERATED_USTRUCT_BODY宏定义在结构体声明中使用，用于生成结构体的元数据和一些标准方法（例如初始化、复制、比较等
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GAS")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GAS")
	TArray<TSubclassOf<class UGameplayAbility>> Abilitys;

	// 动画数据资产
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation")
	class UCharacterAnimDataAsset* CharacterAnimDataAsset = nullptr;
};


// 动画结构体
USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
	// ue5 GENERATED_BODY 和 GENERATED_USTRUCT_BODY 区别
	// 需要注意的是，GENERATED_BODY和GENERATED_USTRUCT_BODY宏定义虽然用途不同，但其本质上是相似的。
	// 它们都是利用了UE5中的反射机制，通过宏定义来自动生成一些代码
	//GENERATED_BODY宏定义在类声明中使用，可以用于生成类的一些标准方法（例如Construct、Tick、BeginPlay等）
	//GENERATED_USTRUCT_BODY宏定义在结构体声明中使用，用于生成结构体的元数据和一些标准方法（例如初始化、复制、比较等
	GENERATED_USTRUCT_BODY()

	// 混合空间
	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* MovementBlendspace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequence* IdleAnimationAsset = nullptr;

	// 蹲伏动画
	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* CrouchMovementBlendspace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequence* CrouchAnimationAsset = nullptr;
};


// 左右枚举
UENUM(BlueprintType)
enum class EFoot: uint8
{
	LEFT UMETA(DisplayName = "Left"),
	RIGHT UMETA(DispalyName = "Right"),
};


USTRUCT(BlueprintType)
struct FMotionWarpingTargetByLocationAndRotation
{
	// ue5 GENERATED_BODY 和 GENERATED_USTRUCT_BODY 区别
	// 需要注意的是，GENERATED_BODY和GENERATED_USTRUCT_BODY宏定义虽然用途不同，但其本质上是相似的。
	// 它们都是利用了UE5中的反射机制，通过宏定义来自动生成一些代码
	//GENERATED_BODY宏定义在类声明中使用，可以用于生成类的一些标准方法（例如Construct、Tick、BeginPlay等）
	//GENERATED_USTRUCT_BODY宏定义在结构体声明中使用，用于生成结构体的元数据和一些标准方法（例如初始化、复制、比较等
	GENERATED_USTRUCT_BODY()

	FMotionWarpingTargetByLocationAndRotation()
	{
	}

	FMotionWarpingTargetByLocationAndRotation(FName InName, FVector InLocation, FRotator InRotation)
		: Name(InName)
		  , Location(InLocation)
		  , Rotation(InRotation)

	{
	}

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;
};


// BlueprintType是用于声明C++的结构体、枚举、类等是允许在蓝图（Blueprint）中使用的。
// 具有BlueprintType修饰符的C++类型可以在蓝图中使用，并且可以直接在蓝图中实例化使用。
// 
//Blueprintable是用于声明C++的类是允许在蓝图中派生的。
//如果一个C++类使用了Blueprintable修饰符，那么该类可以被蓝图继承，并且可以在蓝图中创建和实例化对象。
// 项目静态数据
UCLASS(BlueprintType, Blueprintable)
class UItemStaticData : public UObject
{
	GENERATED_BODY()

public:
	// 通用参数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Name;

	// 指定道具 BP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItemActor> ItemActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachmentSocket = NAME_None;

	// 是否可以进行装备标识
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanBeEquipped = false;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FCharacterAnimationData CharacterAnimationData;

	// 新增道具可触发的GA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	// 道具装备时 的GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffects;
};

UCLASS(BlueprintType, Blueprintable)
class UWeaponStaticData : public UItemStaticData
{
	GENERATED_BODY()

public:
	// 伤害GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	// 武器骨骼 和 网格
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USkeletalMesh* SkeletalMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UStaticMesh* StaticMesh;

	// 射击蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AttackMontage;

	// 射击间隔
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRate;
	
	// 基础伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	// 音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USoundBase* AttackSound;
};


UENUM()
enum class EItemState: uint8
{
	None UMETA(DisplayName = "None"),
	Equipped UMETA(DisplayName = "Equipped"),
	Dropped UMETA(DisplayName = "Dropped"),
};


// 移动方向
UENUM()
enum class EMovementDirectionType: uint8
{
	None UMETA(DisplayName = "None"),
	OrientToMovement UMETA(DisplayName = "OrientToMovement"),
	Strafe UMETA(DisplayName = "Strafe"),
};


// 榴弹
UCLASS(BlueprintType, Blueprintable)
class UProjectilesStaticData : public UObject
{
	GENERATED_BODY()
public:
	// 基础伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;
	// 半径伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageRadius;
	// 重力
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GravityMultiplayer = 1.f;
	// 初始速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InitialSpeed = 3000.f;
	// 最大速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpeed = 3000.f;

	
	// 静态网格
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UStaticMesh* StaticMesh;
	// 生效GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> Effects;
	// 碰撞检测类型 用于球型爆炸查询
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> RadialDamageQueryTypes;
	// 普通直线检测
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ETraceTypeQuery> RadialDamageTraceType;

	// 爆炸特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* OnStapVFX = nullptr;
	// 爆炸声音
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USoundBase* OnStopSFX = nullptr;
};