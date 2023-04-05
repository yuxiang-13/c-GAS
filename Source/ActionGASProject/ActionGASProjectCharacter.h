// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "ActionGameTypes.h"
#include "GameFramework/Character.h"

#include "ActionGASProjectCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
// class UInputAction;
class UAGAbilitySystemComponentBase;
class UAG_AttributeSetBase;
class UGameplayEffect;
class UGameplayAbility;
struct FCharacterData;
// class UInputMappingContext;

class UAG_MotionWarpingComponent;
class UAG_CharacterMovementComponent;

UCLASS(config=Game)
class AActionGASProjectCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	// AActionGASProjectCharacter();
	AActionGASProjectCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	// 绑定 Attribute属性变化
	void OnMaxMovementSpeedChanged(const FOnAttributeChangeData& Data);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
public:
	// 对自己应用GE
	bool ApplyGameplayEffectToSelf(const TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);
public:
	virtual void PawnClientRestart() override;
protected:
	// 授予  ->  能力 GA
	void GiveAbilities();
	// 准备应用的GE列表
	void ApplyStartupEffects();

	// 角色切换
	/*
	 * 对于ASC位于PlayerState上的玩家控制角色，我通常服务器 在Pawn的PosessedBy（）函数中初始化，
	 * 客户端 在OnRep_PlayerState（）函数进行初始化。
	 * 这样可以 确保客户端上已经存在PlayerState了。
	 */
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// 初始化技能组件 和 属性
	UPROPERTY(EditDefaultsOnly)
	UAGAbilitySystemComponentBase* AbilitySystemComponent;
	
	// 说明该属性不会保存或者从磁盘加载，相当于该值是其他值通过计算而得，没有存储的必要。
	// 或者在网络传输序列化时，不希望这些字段被记录或传输，这时候我们就可以在这些字段上面加上transient关键字。
	UPROPERTY(Transient)
	UAG_AttributeSetBase* AttributeSet;

	// 运动扭曲组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=MotionWarp)
	UAG_MotionWarpingComponent* AGMotionWarpingComponent;

	// 这个变量 去保存 咱们在默认构造中 强行更改的 运动组件，实际就是记录下来，因为之前只是绑定上，没有做记录
	UAG_CharacterMovementComponent* AGCharacterMovementComponent;
public:
	UFUNCTION(BlueprintCallable)
	const FCharacterData& GetCharacterData();

	UFUNCTION(BlueprintCallable)
	void SetCharacterData(const FCharacterData& InCharacterData);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	// 返回脚步组件
	class UFootstepsComponent* GetFootstepsComponent();

	// 添加Get
	UAG_MotionWarpingComponent* GetAGMotionWarpingComponent() const;
protected:
	// 角色数据要开启网络复制
	UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
	FCharacterData CharacterData;

	UFUNCTION()
	void OnRep_CharacterData();

	// 子类去实现的虚方法，方便之后重写此方法修改数据
	virtual void InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication = false);

	// 数据资产蓝图类
	UPROPERTY(EditDefaultsOnly)
	class UCharacterDataAsset* CharacterDataAsset;

	// 脚步声组件
	UPROPERTY(BlueprintReadOnly)
	class UFootstepsComponent* FootstepsComponent;

	// EnhancedInput
protected:
	// 增强输入的上下文
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="EnhancedInput")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveForwardInputAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveRightInputAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* TurnInputAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* LookUpInputAction;
	
	UPROPERTY(EditDefaultsOnly)
	UInputAction* JumpInputAction;
	
	UPROPERTY(EditDefaultsOnly)
	UInputAction* CrouchInputAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction* SprintInputAction;
	
	void OnMoveForwardAction(const FInputActionValue& Value);
	void OnMoveRightAction(const FInputActionValue& Value);
	void OnTurnAction(const FInputActionValue& Value);
	void OnLookUpAction(const FInputActionValue& Value);
	void OnJumpActionStart(const FInputActionValue& Value);
	void OnJumpActionEnded(const FInputActionValue& Value);

	// 下蹲
	void OnCrouchActionStart(const FInputActionValue& Value);
	void OnCrouchActionEnded(const FInputActionValue& Value);

	// 冲刺
	void OnSprintActionStart(const FInputActionValue& Value);
	void OnSprintActionEnded(const FInputActionValue& Value);

	// 重写 着陆的 命中事件  降落时调用，根据命中结果执行操作。触发OnLanded事件
	virtual void Landed(const FHitResult& Hit) override;

	// 覆盖 开始下蹲 下蹲结束
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
protected:
	//用于触发 Gameplay Event 的 Tags 
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag JumpEVentTag;
	
	// Gameplay Tags容器
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer InAirTags;
	
	// Gameplay Tags容器
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer CrouchTags;
	
	// Gameplay Tags容器
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer SprintTags;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> CrouchStateEffect;


protected:
	FDelegateHandle MaxMovementSpeedChangedDelegateHandle;
};

