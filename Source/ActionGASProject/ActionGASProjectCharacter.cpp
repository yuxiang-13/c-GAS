// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGASProjectCharacter.h"

#include "Ability/Componts/AGAbilitySystemComponentBase.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "ActorComponent/AG_CharacterMovementComponent.h"
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

void AActionGASProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AActionGASProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AActionGASProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AActionGASProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AActionGASProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AActionGASProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AActionGASProjectCharacter::TouchStopped);
}


void AActionGASProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AActionGASProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AActionGASProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AActionGASProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AActionGASProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AActionGASProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
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


void AActionGASProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AActionGASProjectCharacter, CharacterData);
}

UFootstepsComponent* AActionGASProjectCharacter::GetFootstepsComponent()
{
	return FootstepsComponent;
}
