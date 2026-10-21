// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_GroundStrike.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GA_Lash_GroundStrike.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TeamUtils.h" 
#include "Common/SkillDamageLibrary.h" 
#include "DrawDebugHelpers.h"

UGA_Lash_GroundStrike::UGA_Lash_GroundStrike()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	TargetingMode = ESkillTargetingMode::None;

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.BasicAttack")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.MeleeAttack.Light")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.MeleeAttack.Heavy")));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.InplaceSkill")));

}


void UGA_Lash_GroundStrike::OnCastStarted_Implementation()
{
	Super::OnCastStarted_Implementation();

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement();
	if (!MoveComp) return;

	//  지상 시전
	if (!MoveComp->IsFalling())
	{
		return;
	}
	if (AirborneCastMontage)
	{
		// 부모(StartCast)가 방금 실행한 지상용 몽타주 태스크를 안전하게 중단
		if (CastMontageTask)
		{
			CastMontageTask->OnInterrupted.Clear();
			CastMontageTask->OnCancelled.Clear();
			CastMontageTask->EndTask();
		}

		// 공중 강하용 몽타주로 새 UAbilityTask_PlayMontageAndWait 생성
		CastMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AirborneCastMontage, 1.0f, FName("Loop"));

		// 부모의 종료/취소 콜백 함수를 그대로 연결
		CastMontageTask->OnCompleted.AddDynamic(this, &UGA_Lash_GroundStrike::OnCastCompleted);
		CastMontageTask->OnBlendOut.AddDynamic(this, &UGA_Lash_GroundStrike::OnCastCompleted);
		CastMontageTask->OnInterrupted.AddDynamic(this, &UGA_Lash_GroundStrike::OnCastCancelled);
		CastMontageTask->OnCancelled.AddDynamic(this, &UGA_Lash_GroundStrike::OnCastCancelled);

		CastMontageTask->ReadyForActivation();
	}

	if (CurrentActorInfo->IsLocallyControlled() || HasAuthority(&CurrentActivationInfo))
	{
		FVector StartLoc = AvatarChar->GetActorLocation();

		FVector ForwardDir = AvatarChar->GetActorForwardVector();
		ForwardDir.Z = 0.f;
		ForwardDir.Normalize();

		// 정면 아래 45도 강하 벡터 계산
		FVector DiveDir = (ForwardDir - FVector(0.f, 0.f, 1.f)).GetSafeNormal();

		FVector TraceEnd = StartLoc + (DiveDir * SkillRange);
		FVector TargetLoc = TraceEnd;

		// 사선 아래 바닥면 감지
		FHitResult GroundHit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(AvatarChar);

		if (GetWorld()->LineTraceSingleByChannel(GroundHit, StartLoc, TraceEnd, ECC_Visibility, Params))
		{
			TargetLoc = GroundHit.Location;
			TargetLoc.Z += AvatarChar->GetDefaultHalfHeight();
		}

		float FinalDistance = FVector::Distance(StartLoc, TargetLoc);
		float DiveDuration = FinalDistance / FMath::Max(DiveSpeed, 1.0f);
		DiveDuration = FMath::Clamp(DiveDuration, 0.1f, 2.0f);

		// GAS 표준 Root Motion Task 실행
		DiveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
			this,
			FName("LashDiveTask"),
			TargetLoc,
			DiveDuration,
			false,
			MOVE_Walking,
			false,
			nullptr,
			ERootMotionFinishVelocityMode::SetVelocity,
			FVector::ZeroVector,
			0.f
		);

		if (DiveTask)
		{
			DiveTask->OnTimedOut.AddDynamic(this, &UGA_Lash_GroundStrike::OnDiveCompleted);
			DiveTask->ReadyForActivation();
		}

		//  바닥에 닿는 순간 즉시 반응하도록 델리게이트 연결!
		AvatarChar->MovementModeChangedDelegate.AddDynamic(this, &UGA_Lash_GroundStrike::OnMovementModeChanged);
	}
}

// 바닥에 몸이 닿는 즉시 터짐
void UGA_Lash_GroundStrike::OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return;

	// 공중 -> 지상으로 착지했다면
	if (PrevMovementMode == MOVE_Falling && MoveComp->MovementMode == MOVE_Walking)
	{
		// 중복 방지를 위해 즉시 델리게이트 제거
		Character->MovementModeChangedDelegate.RemoveDynamic(this, &UGA_Lash_GroundStrike::OnMovementModeChanged);

		// 아직 돌고 있는 시간 대기용 루트 모션 태스크 강제 종료
		if (DiveTask)
		{
			DiveTask->EndTask();
			DiveTask = nullptr;
		}

		MontageJumpToSection(FName("Impact"));
	}
}

// 만약 허공에서 멈췄거나 벽에 걸려 지상 판정이 안 났을 때의 이중 안전장치 폴백
void UGA_Lash_GroundStrike::OnDiveCompleted()
{
	DiveTask = nullptr;

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	// 타이머가 다 되었으므로 착지 센서 델리게이트 해제
	AvatarChar->MovementModeChangedDelegate.RemoveDynamic(this, &UGA_Lash_GroundStrike::OnMovementModeChanged);

	MontageJumpToSection(FName("Impact"));
}

void UGA_Lash_GroundStrike::OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;

	// 지상 제자리 시전용 타격 처리
	if (EventTag.MatchesTag(SkillEventParentTag))
	{
		ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (!AvatarChar) return;

		FVector ImpactLocation = AvatarChar->GetActorLocation() - FVector(0.f, 0.f, AvatarChar->GetDefaultHalfHeight());
		ExecuteStrikeDamage(ImpactLocation);
	}
}

void UGA_Lash_GroundStrike::ExecuteStrikeDamage(const FVector& ImpactLocation)
{
	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector ForwardDir = AvatarChar->GetActorForwardVector();
	ForwardDir.Z = 0.f;
	ForwardDir.Normalize();

	// 디버그 드로우
#if ENABLE_DRAW_DEBUG
	if (AvatarChar && World)
	{
		FColor DebugColor = FColor::Orange;
		float DebugLifeTime = 2.0f;

		FVector CenterEnd = ImpactLocation + (ForwardDir * StrikeRadius);
		DrawDebugLine(World, ImpactLocation, CenterEnd, DebugColor, false, DebugLifeTime, 0, 3.0f);

		FVector LeftDir = ForwardDir.RotateAngleAxis(-ConeAngleDegrees * 0.5f, FVector::UpVector);
		FVector RightDir = ForwardDir.RotateAngleAxis(ConeAngleDegrees * 0.5f, FVector::UpVector);

		FVector LeftEnd = ImpactLocation + (LeftDir * StrikeRadius);
		FVector RightEnd = ImpactLocation + (RightDir * StrikeRadius);

		DrawDebugLine(World, ImpactLocation, LeftEnd, DebugColor, false, DebugLifeTime, 0, 3.0f);
		DrawDebugLine(World, ImpactLocation, RightEnd, DebugColor, false, DebugLifeTime, 0, 3.0f);

		int32 Segments = 10;
		float AngleStep = ConeAngleDegrees / Segments;
		FVector PrevPoint = LeftEnd;

		for (int32 i = 1; i <= Segments; ++i)
		{
			float CurrentAngle = (-ConeAngleDegrees * 0.5f) + (AngleStep * i);
			FVector ArcDir = ForwardDir.RotateAngleAxis(CurrentAngle, FVector::UpVector);
			FVector CurrentPoint = ImpactLocation + (ArcDir * StrikeRadius);
			DrawDebugLine(World, PrevPoint, CurrentPoint, DebugColor, false, DebugLifeTime, 0, 3.0f);
			PrevPoint = CurrentPoint;
		}
	}
#endif

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!MyASC) return;

	if (ImpactCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = ImpactLocation;         // 이펙트가 생성될 위치 
		CueParams.Normal = ForwardDir;               // 이펙트가 바라볼 방향 
		CueParams.Instigator = AvatarChar;           // 시전 자
		CueParams.EffectContext = MyASC->MakeEffectContext();

		// 일회성(Burst) 이펙트이므로 ExecuteGameplayCue 사용
		MyASC->ExecuteGameplayCue(ImpactCueTag, CueParams);
	}

	// 구체 트레이스 (채널 방식이 아닌 오브젝트 타입 방식으로 변경!)
	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(StrikeRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GroundStrikeTrace), false, AvatarChar);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	// SweepMultiByChannel 대신 SweepMultiByObjectType 사용
	bool bHit = World->SweepMultiByObjectType(HitResults, ImpactLocation, ImpactLocation, FQuat::Identity, ObjectQueryParams, SphereShape, QueryParams);

	if (!bHit) return;

	// 부채꼴 판정
	float ConeHalfAngleRad = FMath::DegreesToRadians(ConeAngleDegrees * 0.5f);
	float ConeDotThreshold = FMath::Cos(ConeHalfAngleRad);
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* TargetDataArray = new FGameplayAbilityTargetData_ActorArray();

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && SkillTeamUtils::IsHostile(AvatarChar, HitActor))
		{
			FVector DirToTarget = HitActor->GetActorLocation() - ImpactLocation;
			DirToTarget.Z = 0.f;

			if (!DirToTarget.IsNearlyZero())
			{
				DirToTarget.Normalize();
				float DotResult = FVector::DotProduct(ForwardDir, DirToTarget);

				if (DotResult >= ConeDotThreshold)
				{
					TargetDataArray->TargetActorArray.AddUnique(HitActor);
				}
			}
			else
			{
				TargetDataArray->TargetActorArray.AddUnique(HitActor);
			}
		}
	}

	// 데미지 일괄 적용
	if (TargetDataArray->TargetActorArray.Num() > 0)
	{
		TargetDataHandle.Add(TargetDataArray);
		TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetDataHandle, 0);

		for (AActor* TargetActor : TargetActors)
		{
			USkillDamageLibrary::ApplyDamage(AvatarChar, TargetActor, SpiritCoefficient, BaseDamage, TAG_Data_Damage_Spirit);

			if (DamageEffectClass)
			{
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
				{
					FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
					Context.AddInstigator(AvatarChar, AvatarChar);

					FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Context);
					if (SpecHandle.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
	else
	{
		delete TargetDataArray;
	}
}
