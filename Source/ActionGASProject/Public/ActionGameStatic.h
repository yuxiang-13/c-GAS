// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActionGameStatic.generated.h"

class AProjectile;
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

	// 应用径向 伤害
	UFUNCTION(BlueprintCallable, meta=(worldContext = "worldContextObject"))
	static void ApplyRadialDamage(UObject* worldContextObject, AActor* DamageCauser, FVector Location,float Radius, float DamageAmount, TArray<TSubclassOf<class UGameplayEffect>> DamageEffects,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, ETraceTypeQuery TraceType);

	// 发射榴弹
	UFUNCTION(BlueprintCallable, meta=(worldContext = "worldContextObject"))
	static AProjectile* LaunchProjectile(UObject* worldContextObject, TSubclassOf<UProjectilesStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator);
};
