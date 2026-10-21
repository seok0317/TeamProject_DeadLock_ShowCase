// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/VaultAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/VaultAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "../../GameCore/Character/PlayerCharacter.h"
#include "../../MovementComponent/BaseMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"

UVaultAbility::UVaultAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 어빌리티 태그
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Vault")));

	// 파쿠르 도중엔 Vaulting 태그를 소유함 (이 동안 다른 행동 불가)
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Vaulting")));

	// 파쿠르가 켜지면 강제로 취소시킬 태그들 (대시, 이단점프, 슬라이딩 등 모두 캔슬)
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.AirDash")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.DoubleJump")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide")));
}

void UVaultAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bIsEnding = false;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AvatarCharacter);
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (PlayerCharacter && PlayerCharacter->CameraBoom)
	{
		CMC->bUseControllerDesiredRotation = false;
		CMC->bOrientRotationToMovement = false;
		PlayerCharacter->CameraBoom->bUsePawnControlRotation = true;
		PlayerCharacter->CameraBoom->bEnableCameraLag = false;
		
		PlayerCharacter->CameraBoom->CameraLagSpeed = 80.0f;
	}

	

	if (AvatarCharacter && CMC && AvatarCharacter->VaultMontage)
	{
		FVector TargetLoc;
		float WallHeight;

		if (CMC->CheckVaulting(TargetLoc, WallHeight))
		{
			FName SectionName = NAME_None;

			// 에디터에서 배열 세팅이 안됐을 경우의 기본 폴백 이름
			if (VaultThresholds.Num() == 0)
			{
				SectionName = FName("mantle_64");
			}
			else
			{
				// 배열을 순회하며 현재 벽 높이보다 큰 첫 번째 MaxHeight를 찾음
				for (const FVaultAnimThreshold& Threshold : VaultThresholds)
				{
					if (WallHeight <= Threshold.MaxHeight)
					{
						SectionName = Threshold.SectionName;
						break;
					}
				}

				// 만약 벽 높이가 배열의 모든 설정값보다 크다면, 가장 마지막(가장 높은) 섹션을 선택
				if (SectionName == NAME_None)
				{
					SectionName = VaultThresholds.Last().SectionName;
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("[VaultAbility] 벽 높이: %f ➔ 선택된 섹션: %s"), WallHeight, *SectionName.ToString());

			// 3. 몽타주 재생 (파쿠르 애니메이션)
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, AvatarCharacter->VaultMontage, 1.0f, SectionName);
			MontageTask->ReadyForActivation();

			// 동기화 버그 픽스: 몽타주 섹션의 실제 재생 길이를 가져와서 이동 시간에 맞춤
			float ActualMontageDuration = VaultDuration; // 기본값
			int32 SectionIndex = AvatarCharacter->VaultMontage->GetSectionIndex(SectionName);
			if (SectionIndex != INDEX_NONE)
			{
				ActualMontageDuration = AvatarCharacter->VaultMontage->GetSectionLength(SectionIndex);
			}

			if (ActualMontageDuration > 0.0f)
			{
				CMC->SetMovementMode(MOVE_Flying);  
				CMC->Velocity = FVector::ZeroVector;

				APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(AvatarCharacter);
				if (PlayerChar)
				{
					PlayerChar->bUseControllerRotationYaw = false;
					CMC->bOrientRotationToMovement = false;
					if (PlayerChar->CameraBoom) {
						PlayerChar->CameraBoom->bEnableCameraLag = false;
					}
				}

				VaultStartLocation = AvatarCharacter->GetActorLocation();
				VaultTargetLocation = TargetLoc;
				VaultElapsedTime = 0.0f;
				ActualVaultDuration = ActualMontageDuration;

				// 0.01초(10ms)마다 VaultTick 함수를 호출하여 캐릭터를 끌어올림
				GetWorld()->GetTimerManager().SetTimer(VaultTimerHandle, this, &UVaultAbility::VaultTick, 0.01f, true);
			}
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}

	// 0.1초 뒤에 강제 종료 안전장치
	FTimerHandle ForceEndTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ForceEndTimerHandle,
		[this]() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); },
		VaultDuration + 0.1f,
		false
	);
}

void UVaultAbility::VaultTick()
{
	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!AvatarCharacter) return;

	VaultElapsedTime += GetWorld()->GetDeltaSeconds();
	float Alpha = FMath::Clamp(VaultElapsedTime / ActualVaultDuration, 0.0f, 1.0f);

	// 커브에서 가져온 보간 가중치 (0.0 ~ 1.0)
	float CurveValue = VaultTimeMappingCurve->GetFloatValue(Alpha);

	FVector NewLoc;

	// 수평 전진
	// CurveValue가 0.7을 넘어설 때부터 수평
	float XY_Alpha = FMath::Clamp((CurveValue - 0.7f) / 0.3f, 0.0f, 1.0f);
	NewLoc.X = FMath::Lerp(VaultStartLocation.X, VaultTargetLocation.X, XY_Alpha);
	NewLoc.Y = FMath::Lerp(VaultStartLocation.Y, VaultTargetLocation.Y, XY_Alpha);

	// Z 수직 상승
	//XY가 전진하기 전에 이미 목표 높이의 90% 이상 도달해야 역동적
	float Z_Alpha = FMath::Clamp(CurveValue / 0.9f, 0.0f, 1.0f);
	NewLoc.Z = FMath::Lerp(VaultStartLocation.Z, VaultTargetLocation.Z, Z_Alpha);

	// 캐릭터 위치 강제 이동: 벽에 안걸리게
	AvatarCharacter->SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

	// 목표 시간에 도달하면 타이머 끄고 어빌리티 종료
	if (VaultElapsedTime >= ActualVaultDuration)
	{
		GetWorld()->GetTimerManager().ClearTimer(VaultTimerHandle);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UVaultAbility::OnVaultMovementFinished()
{
	// 태스크가 끝나면 정상 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVaultAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	GetWorld()->GetTimerManager().ClearTimer(VaultTimerHandle);

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AvatarCharacter);
	UBaseMovementComponent* CMC = Cast<UBaseMovementComponent>(PlayerCharacter->GetCharacterMovement());

	if (PlayerCharacter && PlayerCharacter->CameraBoom && CMC)
	{
		CMC->bUseControllerDesiredRotation = true;

		PlayerCharacter->CameraBoom->CameraLagSpeed = 15.0f; 
		PlayerCharacter->CameraBoom->bEnableCameraLag = true;
	}

	if (AvatarCharacter && AvatarCharacter->VaultMontage)
	{
		// 파쿠르 몽타주 정지
		AvatarCharacter->StopAnimMontage(AvatarCharacter->VaultMontage);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (!bWasCancelled && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// 무한 등반 방지
		ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.HasVaulted")));
	}

	if (CMC)
	{
		CMC->SetMovementMode(MOVE_Falling);
		CMC->Velocity = FVector::ZeroVector; // 마무리 관성 제거 (던져지는 느낌 방지)
	}
}