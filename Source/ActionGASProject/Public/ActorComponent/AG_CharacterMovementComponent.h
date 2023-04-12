// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AG_CharacterMovementComponent.generated.h"

/**
 * 
 */

class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class ACTIONGASPROJECT_API UAG_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	// Traversal 遍历;树的遍历 踹我搜
	bool TryTraversal(UAbilitySystemComponent* ASC);

	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void SetMovementDirectionType(EMovementDirectionType InMovementDirectionType);

	// 强制的
	UFUNCTION()
	void OnEnforcedStrafeTagChange(const FGameplayTag CallbackTag, int32 NewCount);
	
public:
	UFUNCTION(BlueprintPure)
	EMovementDirectionType GetMovementDirectionType() const;

protected:
	// 能力列表，就是不同情况下 我们能选择去触发的各种 跳跃能力
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayAbility>> TraversalAbilitiesOrdered;

	// 移动方向
	UPROPERTY(EditAnywhere)
	EMovementDirectionType MovementDirectionType;

	void HandleMovementDirection();
};
