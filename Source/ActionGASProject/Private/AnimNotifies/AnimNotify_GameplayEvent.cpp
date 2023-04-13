
#include "AnimNotifies/AnimNotify_GameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "ActionGASProject/ActionGASProjectCharacter.h"

void UAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	
	check(MeshComp);

	// 获取角色
	if (AActionGASProjectCharacter* Character = Cast<AActionGASProjectCharacter>(MeshComp->GetOwner()))
	{
		// 发送GameplayEvent
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Character, Payload.EventTag, Payload);
	}
}
