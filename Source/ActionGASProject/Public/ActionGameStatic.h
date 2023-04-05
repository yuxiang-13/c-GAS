// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActionGameStatic.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UActionGameStatic : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static const UItemStaticData* GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass);
	
};
