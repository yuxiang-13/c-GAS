// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Components/ActorComponent.h"
#include "FootstepsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONGASPROJECT_API UFootstepsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFootstepsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	// 左右脚插槽名称
	UPROPERTY(EditDefaultsOnly)
	FName LeftFootSocketName = TEXT("foot_l_Socket");
	// 左右脚插槽名称
	UPROPERTY(EditDefaultsOnly)
	FName RightFootSocketName = TEXT("foot_r_Socket");
public:
	void HandleFootstep(EFoot Foot);

};
