// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/GameCore/Combat/Ability/GA_Nexus_AoE.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GA_Nexus_AoE.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameCore/GameRule/Nexus.h"
#include "Common/SkillDamageLibrary.h"

UGA_Nexus_AoE::UGA_Nexus_AoE()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ExplodeEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Skill.Explode"));
}

void UGA_Nexus_AoE::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ANexus* NexusCharacter = Cast<ANexus>(ActorInfo->AvatarActor.Get());
	if (!NexusCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 차징 이펙트 네트워킹 스폰 (서버가 모든 클라이언트에게 뿌림)
	if (ChargingNiagara)
	{
		NexusCharacter->MulticastPlayAoECharging(ChargingNiagara, EffectScale, EffectOffset);
	}

	// 폭발 타이밍 대기 (몽타주 내부의 AnimNotify에서 ExplodeEventTag를 보내면 OnExplosionEvent 실행)
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ExplodeEventTag);
	EventTask->EventReceived.AddDynamic(this, &UGA_Nexus_AoE::OnExplosionEvent);
	EventTask->ReadyForActivation();

	// 몽타주 동시 재생
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Nexus_AoE::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Nexus_AoE::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Nexus_AoE::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주가 없으면 즉시 폭발
		OnExplosionEvent(FGameplayEventData());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Nexus_AoE::OnExplosionEvent(FGameplayEventData Payload)
{
	ANexus* NexusCharacter = Cast<ANexus>(GetAvatarActorFromActorInfo());
	if (!NexusCharacter) return;
	
	// 폭발 타이밍에 기존 차징 이펙트를 끄고 폭발 이펙트 네트워킹 스폰
	NexusCharacter->MulticastStopAoECharging();
	NexusCharacter->MulticastPlayAoEExplosion(ExplosionNiagara, EffectScale, EffectOffset);

#if ENABLE_DRAW_DEBUG
	// 폭발 데미지 반경을 시각적으로 확인할 수 있는 디버그 구체 생성 (3초 동안 빨간색으로 표시, 두께 2.0)
	DrawDebugSphere(GetWorld(), NexusCharacter->GetActorLocation(), DamageRadius, 32, FColor::Red, false, 3.0f, 0, 2.0f);
#endif

	// 반경 내 액터 감지 및 데미지 적용
	if (true) // 기존 조건 무시 (BaseDamage를 무조건 적용하기 위해)
	{
		TArray<AActor*> OverlappedActors;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(NexusCharacter); // 자기 자신은 무시

		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			NexusCharacter->GetActorLocation(),
			DamageRadius,
			TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) },
			ABaseCharacter::StaticClass(),
			ActorsToIgnore,
			OverlappedActors
		);

		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

		for (AActor* OverlappedActor : OverlappedActors)
		{
			ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(OverlappedActor);
			// 적군인지 식별 (TeamIdx 비교)
			if (TargetCharacter && NexusCharacter->GetTeamIdx() != TargetCharacter->GetTeamIdx())
			{
				// 거대한 트리거 스피어(시야, 어그로 등)가 맞는 현상 방지: 실제 캡슐간 수학적 거리 계산
				float Distance = FVector::Distance(NexusCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
				float TargetRadius = TargetCharacter->GetCapsuleComponent() ? TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() : 50.f;
				
				// 넥서스 중심에서 타겟 캡슐 외곽까지의 거리가 반경 이내일 때만 데미지 적용
				if (Distance - TargetRadius <= DamageRadius)
				{
					// 프로젝트 공용 데미지 처리 라이브러리 사용
					USkillDamageLibrary::ApplyDamage(NexusCharacter, TargetCharacter, DamageCoefficient, BaseDamage, TAG_Data_Damage_Spirit);

					// 부가적인 상태이상용 GE가 있다면 추가 적용
					if (DamageEffectClass)
					{
						if (UAbilitySystemComponent* TargetASC = TargetCharacter->ASC.Get())
						{
							FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
							ContextHandle.AddInstigator(NexusCharacter, NexusCharacter);
							FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
							TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						}
					}
				}
			}
		}
	}
}

void UGA_Nexus_AoE::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Nexus_AoE::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Nexus_AoE::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
