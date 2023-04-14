// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/ActionGamePlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActionGASProject/ActionGASProjectGameMode.h"

void AActionGamePlayerController::RestartPlayerIn(float InTime)
{
	// 状态变为 旁观者 不能操纵和移动角色,玩家的视角变为幽灵状态
	ChangeState(NAME_Spectating);

	GetWorld()->GetTimerManager().SetTimer(RestartPlayerTimerHandle, this, &AActionGamePlayerController::RestartPlayer, InTime, false);
}


// 复活角色
void AActionGamePlayerController::RestartPlayer()
{
	if (UWorld* world = GetWorld())
	{
		if (AActionGASProjectGameMode* GameMode = Cast<AActionGASProjectGameMode>(world->GetAuthGameMode()))
		{
			GameMode->RestartPlayer(this);
		}
	}
}

void AActionGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn))
	{
		// 监听死亡 Event Tag
		FOnGameplayEffectTagCountChanged& GameplayEffectTagCountChanged = AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), EGameplayTagEventType::NewOrRemoved));
		DeathStateTagDelegate = GameplayEffectTagCountChanged.AddUObject(this, &AActionGamePlayerController::OnPawnDeathStateChanged);
	}
}

// 取消控制时，清空监听
void AActionGamePlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	if (DeathStateTagDelegate.IsValid())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), EGameplayTagEventType::NewOrRemoved));
		}
	}
}

void AActionGamePlayerController::OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 玩家身上存在死亡标签
	if (NewCount > 0)
	{
		if (UWorld* world = GetWorld())
		{
			if (AActionGASProjectGameMode* GameMode = Cast<AActionGASProjectGameMode>(world->GetAuthGameMode()))
			{
				// 倒计时后进行复活
				GameMode->NotifyPlayerDied(this);
			}
		}

		// 防止多次触发，清空监听
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), EGameplayTagEventType::NewOrRemoved));
		}
	}
}
