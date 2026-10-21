// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_Grapple.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GA_Lash_Grapple.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

UGA_Lash_Grapple::UGA_Lash_Grapple()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 타겟 수집 스코프 모드 사용
	TargetingMode = ESkillTargetingMode::Scope;

}

void UGA_Lash_Grapple::OnCastStarted_Implementation()
{
	Super::OnCastStarted_Implementation();

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	// 타겟 데이터 검증
	if (CachedTargetData.Num() == 0) return;

	TArray<TWeakObjectPtr<AActor>> TargetActors = CachedTargetData.Get(0)->GetActors();
	if (TargetActors.Num() == 0 || !TargetActors[0].IsValid()) return;

	AActor* TargetActor = TargetActors[0].Get();

	// 대시 및 이단 점프 횟수 초기화
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		static const FGameplayTag HasDoubleJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasDoubleJumped"), false);
		static const FGameplayTag HasAirDashedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasAirDashed"), false);
		static const FGameplayTag JumpQueuedTag = FGameplayTag::RequestGameplayTag(FName("Event.Input.JumpQueued"), false);
		
		ASC->SetLooseGameplayTagCount(HasDoubleJumpedTag, 0);
		ASC->SetLooseGameplayTagCount(HasAirDashedTag, 0);
		ASC->SetLooseGameplayTagCount(JumpQueuedTag, 0);

		if (GrappleFlyingEffectClass && HasAuthority(&CurrentActivationInfo))
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddInstigator(AvatarChar, AvatarChar);

			// GC에서 적이 누군지 알 수 있도록 HitResult에 적 액터를 담아 보냄
			FHitResult FakeHit;
			FakeHit.HitObjectHandle = TargetActor;
			Context.AddHitResult(FakeHit);

			ASC->ApplyGameplayEffectToSelf(GrappleFlyingEffectClass->GetDefaultObject<UGameplayEffect>(), GetAbilityLevel(), Context);
		}
	}

	// 타겟 방향으로 상향 45도 발사 로직
	if (CurrentActorInfo->IsLocallyControlled() || HasAuthority(&CurrentActivationInfo))
	{
		FVector StartLoc = AvatarChar->GetActorLocation();
		FVector TargetLoc = TargetActor->GetActorLocation();

		// 타겟을 향하는 수평 벡터
		FVector DirToTarget = (TargetLoc - StartLoc);
		DirToTarget.Z = 0.f;
		DirToTarget.Normalize();

		// 수평 + 수직 더해서 정규화하면 45도 상향 벡터
		FVector LaunchDir = (DirToTarget + FVector(0.f, 0.f, 1.0f)).GetSafeNormal();

		// 고정된 힘으로 정해진 시간 동안 발사 
		UAbilityTask_ApplyRootMotionConstantForce* ForceTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this,
			FName("LashGrappleLaunch"),
			LaunchDir,
			LaunchStrength,
			LaunchDuration,
			false,    // bIsPush
			nullptr,  // StrengthOverTimeCurve
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, // FinishVelocityMode
			FVector::ZeroVector, // FinishSetVelocity
			0.f,      // FinishClampMaxSpeed
			false     // bEnableGravity
		);

		if (ForceTask)
		{
			ForceTask->OnFinish.AddDynamic(this, &UGA_Lash_Grapple::OnGrappleFinished);
			ForceTask->ReadyForActivation();
		}

		AvatarChar->MovementModeChangedDelegate.AddDynamic(this, &UGA_Lash_Grapple::OnMovementModeChanged);
	}
}

void UGA_Lash_Grapple::OnGrappleFinished()
{
	// 비행이 끝나면 공중 상태(Falling)로 전환하여 1번 스킬 연계 준비
	if (ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarChar->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Lash_Grapple::CleanupRope()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Lash_Grapple::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarChar->MovementModeChangedDelegate.RemoveDynamic(this, &UGA_Lash_Grapple::OnMovementModeChanged);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Lash_Grapple::OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	// 공중 상태였다가 지상으로 변했다면 = 착지
	if (PrevMovementMode == MOVE_Falling && MoveComp->MovementMode == MOVE_Walking)
	{
		// 센서 해제
		Character->MovementModeChangedDelegate.RemoveDynamic(this, &UGA_Lash_Grapple::OnMovementModeChanged);

		Character->StopAnimMontage(CastMontage);

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}