// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGASProjectCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputTriggers.h"
#include "Ability/Componts/AGAbilitySystemComponentBase.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "ActorComponent/AG_CharacterMovementComponent.h"
#include "ActorComponent/AG_MotionWarpingComponent.h"
#include "ActorComponent/FootstepsComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "DataAssets/CharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// AActionGASProjectCharacter

AActionGASProjectCharacter::AActionGASProjectCharacter(const FObjectInitializer& ObjectInitializer)
	// 用我们自定义的移动组件 替换 默认的移动组件
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UAG_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	
	// 记录下来
	AGCharacterMovementComponent = Cast<UAG_CharacterMovementComponent>(GetCharacterMovement());
	// 创建动作扭曲组件
	AGMotionWarpingComponent = CreateDefaultSubobject<UAG_MotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// 初始化能力和属性集
	AbilitySystemComponent = CreateDefaultSubobject<UAGAbilitySystemComponentBase>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// 以下是GE网络同步的前提
	/*
	* GAS组件网络模式必须是Mixed或Full，才会完整地同步GE在AbilitySystemComponent.h中有EGameplayEffectReplicationMode的定义，
	* 可以通过UAbilitySystemComponent::SetReplicationMode函数设置组件实例的网络模式。
	*
	* 根据以上三种模式的注释可知（minimal gameplay effect info指的是GameplayTag和GameplayCue）
		Minimal不会完整地同步GE到客户端
		Mixed只会完整地同步GE到客户端的owner代理（proxy），其它代理是不完整同步
		Full会完整地同步GE到客户端的所有代理
	 */
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAG_AttributeSetBase>(TEXT("AttributeSet"));

	FootstepsComponent = CreateDefaultSubobject<UFootstepsComponent>(TEXT("FootstepsComponent"));

	// 绑定客户端监听属性变化
	//GetGameplayAttributeValueChangeDelegate 是用于注册属性值发生变化的回调函数（Delegate），当属性值发生变化时，会触发该回调函数。
	// 客户端和服务器之间需要同步游戏状态，包括属性值的变化。因此，该回调函数在客户端和服务器上都会被响应
	// 当客户端的属性值发生变化时，客户端会触发该回调函数，并向服务器发送属性值变化的通知。而服务器也会监听该回调函数，并更新游戏状态，然后将状态同步给所有的客户端。
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &AActionGASProjectCharacter::OnMaxMovementSpeedChanged);
}

void AActionGASProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	
}


// 用于在组件初始化后对组件进行一些操作。该函数被定义在Actor类中，它会在Actor及其组件（如静态网格组件、动画组件、声音组件等）完成初始化后立即被调用。
// 例如，如果我们需要在Actor中的某个组件初始化后对其进行设置、调整或其他操作，我们可以通过实现PostInitializeComponents函数来实现。
// 这样，在组件初始化完成后，PostInitializeComponents函数会被调用，我们就可以在该函数中对组件进行需要的操作。
void AActionGASProjectCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 这个 客户端服务器都会触发，必要都触发，因为两边都要初始化CharacterData。尤其是第一次初始化，
	//不能完全依照RPC让客户端等待服务器下发，因为不是同步的东西，很可能服务器下发时候，你客户端还没创建完成
	// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT("-0000 - ->>>> %f"), 0.0f) , true, true, FLinearColor::Red, 10.f);
	// 根据数据资产 初始化 角色数据
	if (IsValid(CharacterDataAsset))
	{
		SetCharacterData(CharacterDataAsset->CharacterData);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AActionGASProjectCharacter::OnMaxMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	// 设置 Attribute属性变化，保持客户端服务器的同步
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;

	// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT("-0000 - ->>>> %f"), 0.0f) , true, true, FLinearColor::Red, 10.f);
}

void AActionGASProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	
	if (UEnhancedInputComponent * PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveForwardInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveForwardInputAction, ETriggerEvent::Triggered, this, &AActionGASProjectCharacter::OnMoveForwardAction);
		}
		if (MoveRightInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveRightInputAction, ETriggerEvent::Triggered, this, &AActionGASProjectCharacter::OnMoveRightAction);
		}
		if (TurnInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(TurnInputAction, ETriggerEvent::Triggered, this, &AActionGASProjectCharacter::OnTurnAction);
		}
		if (LookUpInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookUpInputAction, ETriggerEvent::Triggered, this, &AActionGASProjectCharacter::OnLookUpAction);
		}
		if (JumpInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AActionGASProjectCharacter::OnJumpActionStart);
			PlayerEnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &AActionGASProjectCharacter::OnJumpActionEnded);
		}

		// 绑定蹲伏
		if (CrouchInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &AActionGASProjectCharacter::OnCrouchActionStart);
			PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Completed, this, &AActionGASProjectCharacter::OnCrouchActionEnded);
		}

		// 绑定冲刺
		if (SprintInputAction)
		{
			PlayerEnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &AActionGASProjectCharacter::OnSprintActionStart);
			PlayerEnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &AActionGASProjectCharacter::OnSprintActionEnded);
		}
	}
}

// 实现纯虚函数的 接口
UAbilitySystemComponent* AActionGASProjectCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionGASProjectCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 只在服务器执行此函数
	// 初始化 服务器  GAS
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	GiveAbilities();
	ApplyStartupEffects();
}

// 授予  ->  能力 GA
void AActionGASProjectCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		// for (auto DefaultAbility : DefaultAbilities)
		for (auto DefaultAbility : CharacterData.Abilitys)
		{
			// 授予  ->  能力 GA
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility));
		}
	}
}

// 准备应用的GE列表
void AActionGASProjectCharacter::ApplyStartupEffects()
{
	// if (GetLocalRole() == ROLE_Authority && DefaultAttributeSet && AttributeSet)
	if (GetLocalRole() == ROLE_Authority)
	{
		//创建 GE效果上下文 实际就是GE的Params配置
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		//  AddSourceObject()是EffectContext中的一个方法，用于向该效果上下文中添加一个源对象。源对象一般是指需要被处理的目标对象 暂时设置de
		EffectContext.AddSourceObject(this);

		// 默认的GE列表
		// for (const TSubclassOf<UGameplayEffect> CharacterEffect : DefaultEffects)
		for (const TSubclassOf<UGameplayEffect> CharacterEffect : CharacterData.Effects)
		{
			// 自己写的。。。。
			ApplyGameplayEffectToSelf(CharacterEffect, EffectContext);
		}
	}
}

void AActionGASProjectCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// 初始化 客户端  GAS
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool AActionGASProjectCharacter::ApplyGameplayEffectToSelf(const TSubclassOf<UGameplayEffect> Effect,
	FGameplayEffectContextHandle InEffectContext)
{
	if (!Effect.Get()) return false;

	// The GameplayEffectSpec (GESpec)可以认为是GameplayEffects的实例化
	// 在应用一个GameplayEffect时，会先从GameplayEffect中创建一个GameplayEffectSpec出来，然后实际上是把GameplayEffectSpec应用给目标
	// 从GameplayEffects创建GameplayEffectSpecs会用到UAbilitySystemComponent::MakeOutgoingSpec()（BlueprintCallable）。
	// GameplayEffectSpecs不是必须立即应用。通常是将GameplayEffectSpec传递给由技能创建的子弹，然后当子弹击中目标时将具体的技能效果应用给目标。
	// 当GameplayEffectSpecs成功被应用后，它会返回一个新的结构体FActiveGameplayEffect
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);

	if (SpecHandle.IsValid())
	{
		// 应用 FGameplayEffect ！Spec！ Handle
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		// 返回应用效果的结果
		return ActiveGEHandle.WasSuccessfullyApplied();
	}
	return false;
}

const FCharacterData& AActionGASProjectCharacter::GetCharacterData()
{
	return CharacterData;
}

// 服务器去 设置
void AActionGASProjectCharacter::SetCharacterData(const FCharacterData& InCharacterData)
{
	CharacterData = InCharacterData;
	InitFromCharacterData(CharacterData);
}


void AActionGASProjectCharacter::InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication)
{
}

// 客户端接收后 设置
void AActionGASProjectCharacter::OnRep_CharacterData()
{
	InitFromCharacterData(CharacterData, true);
}

UFootstepsComponent* AActionGASProjectCharacter::GetFootstepsComponent()
{
	return FootstepsComponent;
}

UAG_MotionWarpingComponent* AActionGASProjectCharacter::GetAGMotionWarpingComponent() const
{
	return AGMotionWarpingComponent;
}


// 客户端重启  绑定上下文
void AActionGASProjectCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 增强输入子系统
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			// 设置输入上下文以及优先级
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


void AActionGASProjectCharacter::OnMoveForwardAction(const FInputActionValue& Value)
{
	const float Magnitude = Value.GetMagnitude();
	if ((Controller != nullptr) && (Magnitude != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Magnitude);
	}
}
void AActionGASProjectCharacter::OnMoveRightAction(const FInputActionValue& Value)
{
	const float Magnitude = Value.GetMagnitude();
	if ( (Controller != nullptr) && (Magnitude != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Magnitude);
	}
}
void AActionGASProjectCharacter::OnTurnAction(const FInputActionValue& Value)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Value.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}
void AActionGASProjectCharacter::OnLookUpAction(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AActionGASProjectCharacter::OnJumpActionStart(const FInputActionValue& Value)
{
	// // Jump();
	// // 通过事件 触发Jump
	// FGameplayEventData PayLoad;
	// PayLoad.Instigator = this;
	// PayLoad.EventTag = JumpEVentTag;
	//
	// // 触发EventTag
	// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, JumpEVentTag, PayLoad);

	AGCharacterMovementComponent->TryTraversal(AbilitySystemComponent);
}

void AActionGASProjectCharacter::OnJumpActionEnded(const FInputActionValue& Value)
{
	// StopJumping();

	
}

void AActionGASProjectCharacter::OnCrouchActionStart(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		//1 用于主动激活与指定标签(Tag)匹配的技能。
		//2 如果bAllowRemoteActivation为true，它将远程激活本地/服务器功能，如果为false，它将仅尝试本地激活功能。
		AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchTags, true);
	}
}

void AActionGASProjectCharacter::OnCrouchActionEnded(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&CrouchTags);
	}
}

void AActionGASProjectCharacter::OnSprintActionStart(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		//1 用于主动激活与指定标签(Tag)匹配的技能。
		//2 如果bAllowRemoteActivation为true，它将远程激活本地/服务器功能，如果为false，它将仅尝试本地激活功能。
		AbilitySystemComponent->TryActivateAbilitiesByTag(SprintTags, true);
	}
}

void AActionGASProjectCharacter::OnSprintActionEnded(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&SprintTags);
	}
}

void AActionGASProjectCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (AbilitySystemComponent)
	{
		// 根据InAirTags容器列表中每个Tag，移除对应的GE
		AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
	}
}

void AActionGASProjectCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	// 检查是否有效
	if (!CrouchStateEffect.Get()) return;

	if (AbilitySystemComponent)
	{
		// 创建GE内容句柄
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		
		// GE实例化
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(CrouchStateEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			//激活 GE Spec实例 也就 获得了这个 FActiveGameplayEffectHandle
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			// 如果没成功 添加一条消息打印
			if (!ActiveGEHandle.WasSuccessfullyApplied())
			{
				ABILITY_LOG(Log, TEXT("Ability %s Failed to apply Crouch Effect %s"), *GetName(), *GetNameSafe(CrouchStateEffect));
				// UKismetSystemLibrary::PrintString(this,  FString::Printf(TEXT(" Failed to apply jump effect! %s"), * GetNameSafe(Character)) , true, true, FLinearColor::Red, 10.f);
			}
		}
	}
}

void AActionGASProjectCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (AbilitySystemComponent && CrouchStateEffect.Get())
	{
		// 通过GE源文件 删除GE
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(CrouchStateEffect, AbilitySystemComponent);
	}
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
}


void AActionGASProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AActionGASProjectCharacter, CharacterData);
}


