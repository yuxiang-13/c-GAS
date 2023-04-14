// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameStatic.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actors/Projectile.h"
#include "Kismet/KismetSystemLibrary.h"

/*
 *TAutoConsoleVariable是一个宏定义，用于创建控制台命令变量
 * 控制台命令变量在UE5中可以用于动态修改游戏参数，例如控制游戏的采样率、帧率、物理系统参数等。
 * 在运行时，可以通过输入控制台命令（变量名称+变量值）来修改控制台命令变量的值，从而改变游戏参数。
 */
// 调试Debug
static TAutoConsoleVariable<int32> CVarShowRadialDamage(TEXT("ShowDebugRadialDamage"),
	0, //控制台命令变量的初始值，默认值为0
	TEXT("Draws debug info about RadialDamage/n    0: off/n    1: on/n"), //控制台命令变量的帮助文本，可以为NULL。
	ECVF_Cheat); //控制台命令变量的标识位，用于指定变量的使用权限。可用标识位包括：可通过配置文件进行修改、只读变量、命令行参数、不使用外部命令行参数等。
//ECVF_Cheat : 隐藏在控制台中，用户无法更改
//..其他的我也不清楚。需要看文档，就先不说了



const UItemStaticData* UActionGameStatic::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{
	if (IsValid(ItemDataClass))
	{
		// 在Unreal Engine 4中，GetDefaultObject()函数用于获取一个类的默认对象，这个默认对象在编译时被创建，并且在整个游戏实例中只有一个。
		// 每次调用GetDefaultObject()函数时，它都会返回同一个默认对象的引用，而不是生成新的对象。
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}

// 先找出 爆炸范围内的人，再从爆炸中心 到 被炸物 发 射线检测
void UActionGameStatic::ApplyRadialDamage(UObject* worldContextObject, AActor* DamageCauser, FVector Location,float Radius,
	float DamageAmount, TArray<TSubclassOf<UGameplayEffect>> DamageEffects,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, ETraceTypeQuery TraceType)
{
	// 承受GE伤害的人
	TArray<AActor*> OutActors;
	// 忽略伤害的人
	TArray<AActor*> ActorsToIgnore = { DamageCauser };

	// ClassFilter 如果设置，将只返回此类或其子类的结果。
	UKismetSystemLibrary::SphereOverlapActors(worldContextObject, Location, Radius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);
	const bool bDebug = static_cast<bool>(CVarShowRadialDamage.GetValueOnAnyThread());

	for (AActor* Actor: OutActors)
	{
		// 从爆炸中心 开始检测被炸敌人的距离
		FHitResult HitResult;
		if (UKismetSystemLibrary::LineTraceSingle(worldContextObject, Location, Actor->GetActorLocation(), TraceType, true, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
		{
			// 线性检测 发生碰撞

			AActor* Target = HitResult.GetActor();
			if (Target == Actor)
			{
				if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
				{
					FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
					// 施法者
					EffectContext.AddInstigator(DamageCauser, DamageCauser);
					// 应用GE
					for (auto Effect: DamageEffects)
					{
						//是否以应用应用GE效果 （绘制调试信息）
						bool bWasApplied = false;
						
						FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(Effect, 1, EffectContext);
						if (SpecHandle.IsValid())
						{
							// 1 AbilityComponent->RegisterGameplayTagEvent  注册GameplayTag并监听事件

							// 2 动态改变的属性值 GE.h中 属性 TMap<FGameplayTag, float> SetByCallerNameMagnitudes 这个 可以在 行时动态地修改属性值，是一个Map
							// 我们只需要把   TMap->FGameplayTag  传导值 TMap->float： 就可以通过GE触发时候，给予属性改变
							UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")),
								-DamageAmount);

							// 上面只是设置GE的参数，还需要手动激活 GE实例   注意 AbilityComponent 是Target，被炸到的敌人，应用GE
							FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

							if (ActiveGEHandle.WasSuccessfullyApplied())
							{
								bWasApplied = true;
							}
						}

						
						// 绘制调试信息
						if (bWasApplied)
						{
							DrawDebugLine(worldContextObject->GetWorld(), Location, Actor->GetActorLocation(), bWasApplied ? FColor::Green : FColor::Red, false, 4.0f, 0, 1);
							DrawDebugSphere(worldContextObject->GetWorld(), HitResult.Location, 16, 16, bWasApplied ? FColor::Green : FColor::Red, false, 4.0f, 0, 1);
							DrawDebugString(worldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::White, 0, false, 1.f);
						}
					}
				}
			}
			else
			{
				// 绘制调试信息
				if (bDebug)
				{
					DrawDebugLine(worldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.0f, 0, 1);
					DrawDebugSphere(worldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.0f, 0, 1);
					DrawDebugString(worldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::Red, 0, false, 1.f);
				}
			}
		}
		else
		{
			// 线性检测 发生碰撞 未  发生碰撞
			
			// 绘制调试信息
			if (bDebug)
			{
				DrawDebugLine(worldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.0f, 0, 1);
				DrawDebugSphere(worldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.0f, 0, 1);
				DrawDebugString(worldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(HitResult.GetActor()), nullptr, FColor::Red, 0, false, 1.f);
			}
		}
	}

	// 画出爆炸范围 
	if (bDebug) {
		DrawDebugSphere(worldContextObject->GetWorld(), Location, Radius, 16, FColor::White, false, 4.f, 0, 1.f);
	}
}

AProjectile* UActionGameStatic::LaunchProjectile(UObject* worldContextObject,
	TSubclassOf<UProjectilesStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator)
{
	UWorld* world = worldContextObject ? worldContextObject->GetWorld() : nullptr;
	// 服务器生成榴弹
	if (world && world->IsServer())
	{
		Transform.SetScale3D(FVector(0.5));
		// 开始生成
		// world->SpawnActor ------------ 延迟生成Actor 好提前预设一些信息，之后需要手动调用 FinishSpawning函数以生成Actor对象
		// world->SpawnActorDeferred ---- 直接生成Actor
		if (AProjectile* Projectile = world->SpawnActorDeferred<AProjectile>(AProjectile::StaticClass(), Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			// 提前设置静态数据
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->FinishSpawning(Transform);
			return Projectile;
		}
	}
	
	return nullptr;
}
