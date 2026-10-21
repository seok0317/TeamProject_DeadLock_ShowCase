// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/DoubleJumpAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/DoubleJumpAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../MovementComponent/BaseMovementComponent.h"
#include "TimerManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UDoubleJumpAbility::UDoubleJumpAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.DoubleJump")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.HasDoubleJumped")));
}

void UDoubleJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bIsEnding = false;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	UCharacterMovementComponent* CMC = AvatarCharacter ? AvatarCharacter->GetCharacterMovement() : nullptr;

	if (AvatarCharacter && ASC && CMC)
	{
		// 1. 이단 점프 잠금 태그 부여
		ASC->SetLooseGameplayTagCount(FGameplayTag::RequestGameplayTag(FName("State.Movement.HasDoubleJumped")), 1);

		// 2. 현재 이동 입력 방향 확인
		FVector InputDir = FVector::ZeroVector;
		if (ActorInfo->IsLocallyControlled())
		{
			InputDir = CMC->GetLastInputVector();
		}
		else
		{
			InputDir = AvatarCharacter->GetReplicatedMovementInput();
		}
		InputDir.Z = 0.0f;
		InputDir.Normalize();

		// 속도제한
		FVector CurrentVelocity = CMC->Velocity;
		CurrentVelocity.Z = 0.0f;

		FVector FinalVelocity = CMC->Velocity + (InputDir * DoubleJumpHorizontalForce);
		FinalVelocity.Z = 0.0f;

		if (FinalVelocity.Size2D() > DoubleJumpHorizontalForce)
		{
			FinalVelocity = FinalVelocity.GetSafeNormal2D() * DoubleJumpHorizontalForce;
		}

		UE_LOG(LogTemp, Warning, TEXT("더블점프"));
		// 수직 점프력을 덮어씌우고 최종 발사
		FinalVelocity.Z = DoubleJumpForce;
		AvatarCharacter->LaunchCharacter(FinalVelocity, true, true);

		GetWorld()->GetTimerManager().SetTimer(AutoMantleTimerHandle, this, &UDoubleJumpAbility::CheckAutoMantle, 0.05f, true);


		// 4. 애니메이션 재생
		if (AvatarCharacter->DoubleJumpMontage)
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				AvatarCharacter->DoubleJumpMontage,
				1.0f,
				NAME_None
			);
			MontageTask->OnCompleted.AddDynamic(this, &UDoubleJumpAbility::OnMontageFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &UDoubleJumpAbility::OnMontageFinished);
			MontageTask->OnCancelled.AddDynamic(this, &UDoubleJumpAbility::OnMontageFinished);
			MontageTask->ReadyForActivation();
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UDoubleJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	GetWorld()->GetTimerManager().ClearTimer(AutoMantleTimerHandle);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDoubleJumpAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDoubleJumpAbility::CheckAutoMantle()
{
	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (AvatarCharacter && CMC && ASC)
	{
		// W키 입력 중인지 확인
		FVector ForwardInput = CMC->GetLastInputVector();
		bool bIsPressingForward = FVector::DotProduct(ForwardInput.GetSafeNormal(), AvatarCharacter->GetActorForwardVector()) > 0.5f;

		// W키를 누르고 있고
		if (bIsPressingForward)
		{
			FVector TargetLoc;
			float WallHeight;
			// 내 앞에 넘을 수 있는 벽이 있다면
			if (CMC->CheckVaulting(TargetLoc, WallHeight))
			{
				// 이단 점프 스스로 취소
				CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

				FGameplayEventData Payload;
				Payload.EventMagnitude = WallHeight; // 벽 높이 전달

				FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
				LocationData->TargetLocation.LiteralTransform = FTransform(TargetLoc);
				LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;

				Payload.TargetData.Add(LocationData);

				FGameplayTag VaultTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Vault"));
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarCharacter, VaultTag, Payload);
			}
		}
	}
}