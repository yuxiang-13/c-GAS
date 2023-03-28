// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "ActionGameTypes.h"
#include "GameFramework/Character.h"
#include "ActionGASProjectCharacter.generated.h"

class UAGAbilitySystemComponentBase;
class UAG_AttributeSetBase;
class UGameplayEffect;
class UGameplayAbility;
struct FCharacterData;

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

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

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
public:
	UFUNCTION(BlueprintCallable)
	const FCharacterData& GetCharacterData();

	UFUNCTION(BlueprintCallable)
	void SetCharacterData(const FCharacterData& InCharacterData);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	// 返回脚步组件
	class UFootstepsComponent* GetFootstepsComponent();
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
};

