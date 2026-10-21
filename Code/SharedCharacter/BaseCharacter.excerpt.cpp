// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/GameCore/Character/BaseCharacter.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: dalbi090211의 캐릭터/ASC/스탯 기반, Youngwoo의 전투·집라인, horimi_의 성장·스킬 기반과 함께 쓰는 공통 클래스. 본인은 공통 이동·상태 연계·스태미나 패시브·HUD/체력바·리스폰 바인딩 등을 구현한 범위만 발췌.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 1-1] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
// Fill out your copyright notice in the Description page of Project Settings.

// [원문 7-7] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "AbilitySystemBlueprintLibrary.h"

// [원문 10-12] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "UI/OverheadHealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"

// [원문 15-16] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

// [원문 25-25] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "../../MovementComponent/BaseMovementComponent.h"

// [원문 34-34] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(ACharacter::CharacterMovementComponentName))

// [원문 44-50] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	OverheadHealthBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadHealthBarComp"));
	OverheadHealthBarComp->SetupAttachment(GetMesh(), TEXT("head"));
	OverheadHealthBarComp->SetWidgetSpace(EWidgetSpace::World);
	OverheadHealthBarComp->SetDrawSize(FVector2D(400.f, 100.f));
	OverheadHealthBarComp->SetRelativeLocation(FVector(0.f, 0.f, 35.f));
	OverheadHealthBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverheadHealthBarComp->SetTickWhenOffscreen(false);

// [원문 75-76] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
void ABaseCharacter::Jump()
{

// [원문 79-80] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	static const FGameplayTag ParryingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Parrying"), false);

// [원문 86-86] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag HeavyMeleeLungingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.HeavyMeleeLunging"), false);

// [원문 92-114] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UBaseMovementComponent *CMC = CachedBaseMovement;

	// W키(앞으로 가는 입력)가 눌려있는지 확인
	FVector ForwardInput = GetCharacterMovement()->GetLastInputVector();
	bool bIsPressingForward = FVector::DotProduct(ForwardInput.GetSafeNormal(), GetActorForwardVector()) > 0.5f;

	if (CMC && bIsPressingForward)
	{
		FVector TargetLoc;
		float WallHeight;

		// 앞에 넘을 수 있는 벽이 있다면?
		if (CMC->CheckVaulting(TargetLoc, WallHeight))
		{
			// 클라이언트는 즉시 방향 동기화 후 Vault 어빌리티 실행
			if (IsLocallyControlled())
			{
				ServerSetMovementInput(ForwardInput.GetSafeNormal());
			}

			FGameplayEventData Payload;
			Payload.EventMagnitude = WallHeight; // 벽 높이 전달


// [원문 116-140] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			LocationData->TargetLocation.LiteralTransform = FTransform(TargetLoc);
			LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;

			Payload.TargetData.Add(LocationData);

			static const FGameplayTag VaultTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Vault"), false);
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, VaultTag, Payload);
			return;
		}
	}

	static const FGameplayTag HasDoubleJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasDoubleJumped"), false);
	static const FGameplayTag AirDashStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.AirDash"), false);
	static const FGameplayTag JumpQueuedTag = FGameplayTag::RequestGameplayTag(FName("Event.Input.JumpQueued"), false);
	static const FGameplayTag RollingTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Rolling"), false);
	static const FGameplayTag CanJumpTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.CanJump"), false);
	static const FGameplayTag RollAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.GroundRoll"), false);
	static const FGameplayTag RollJumpAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.RollJump"), false);
	static const FGameplayTag SlideStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Slide"), false);
	static const FGameplayTag SlideAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide"), false);
	static const FGameplayTag WallClingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.WallCling"), false);
	static const FGameplayTag HasWallJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasWallJumped"), false);
	static const FGameplayTag WallJumpAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump"), false);
	static const FGameplayTag DoubleJumpAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.DoubleJump"), false);


// [원문 142-142] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (ASC->HasMatchingGameplayTag(AirDashStateTag))

// [원문 146-150] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		{

			return;
		}


// [원문 152-152] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		{

// [원문 155-155] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 158-158] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 160-160] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (ASC->HasMatchingGameplayTag(RollingTag))

// [원문 163-164] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (ASC->HasMatchingGameplayTag(CanJumpTag))
		{

// [원문 171-172] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(RollJumpAbilityTag));
		}

// [원문 175-175] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 177-177] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (ASC->HasMatchingGameplayTag(SlideStateTag))

// [원문 180-180] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		FGameplayTagContainer SlideTagsToCancel(SlideAbilityTag);

// [원문 189-191] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	bool bIsWallClinging = ASC->HasMatchingGameplayTag(WallClingStateTag);
	bool bHasWallJumped = ASC->HasMatchingGameplayTag(HasWallJumpedTag);
	bool bHasDoubleJumped = ASC->HasMatchingGameplayTag(HasDoubleJumpedTag);

// [원문 197-197] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		{

// [원문 200-201] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		}


// [원문 205-205] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WallJumpAbilityTag));

// [원문 211-211] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(DoubleJumpAbilityTag));

// [원문 217-217] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WallJumpAbilityTag));

// [원문 221-221] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 227-237] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		{
			if (IsLocallyControlled())
			{
				FVector InputDir = GetCharacterMovement()->GetLastInputVector().GetSafeNormal();
				ServerSetMovementInput(InputDir);
			}

			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(DoubleJumpAbilityTag));
			return;
		}
	}

// [원문 239-241] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	
}


// [원문 278-278] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag BasicAttackTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.BasicAttack")), false);

// [원문 299-299] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ReloadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Reload")), false);

// [원문 320-320] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 322-322] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	{

// [원문 349-349] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ReloadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Reload")), false);

// [원문 417-417] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag LightMeleeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.MeleeAttack.Light")), false);

// [원문 445-445] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag HeavyMeleeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.MeleeAttack.Heavy")), false);

// [원문 481-481] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Parry")), false);

// [원문 510-510] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	CachedBaseMovement = Cast<UBaseMovementComponent>(GetCharacterMovement());

// [원문 518-529] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (ASC && CachedBaseMovement)
	{
		static const FGameplayTag DashTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.AirDash"), false);
		static const FGameplayTag WallJumpTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump"), false);
		static const FGameplayTag VaultingTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Vaulting"), false);
		
		bool bIsDashing = ASC->HasMatchingGameplayTag(DashTag);
		bool bIsWallJumping = ASC->HasMatchingGameplayTag(WallJumpTag);
		bool bIsVaulting = ASC->HasMatchingGameplayTag(VaultingTag);
		CachedBaseMovement->bDisableWallCling = (bIsDashing || bIsWallJumping || bIsVaulting);
	}


// [원문 532-534] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		static const FGameplayTag VaultingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Vaulting"), false);
		if (ASC && ASC->HasMatchingGameplayTag(VaultingStateTag))
		{

// [원문 536-537] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		}


// [원문 543-547] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (FMath::Abs(AimYaw - DeltaRot.Yaw) > 1.0f)
		{
			AimYaw = DeltaRot.Yaw;
			Server_SetAimYaw(AimYaw);
		}

// [원문 549-567] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	if (OverheadHealthBarComp && !OverheadHealthBarComp->bHiddenInGame)
	{
		// 혹시 모를 Null 포인터 체크 (멀티플레이어 환경 방어)
		if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
		{
			if (APlayerCameraManager* CameraManager = LocalPC->PlayerCameraManager)
			{
				FVector CameraLoc = CameraManager->GetCameraLocation();
				FVector WidgetLoc = OverheadHealthBarComp->GetComponentLocation();

				// 위젯 -> 카메라를 향하는 방향 벡터 (마주보기)
				FVector DirectionToCamera = (CameraLoc - WidgetLoc).GetSafeNormal();

				// 회전 적용
				OverheadHealthBarComp->SetWorldRotation(DirectionToCamera.Rotation());
			}
		}
	}

// [원문 628-629] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UE_LOG(LogTemp, Warning, TEXT("테이블 있음"));


// [원문 633-633] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UE_LOG(LogTemp, Warning, TEXT("어트리 있음"));

// [원문 638-638] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UE_LOG(LogTemp, Warning, TEXT("행 있음"));

// [원문 731-731] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	DOREPLIFETIME(ABaseCharacter, ReplicatedMovementInput);

// [원문 746-746] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		SetupOverheadHealthBarUI();

// [원문 758-758] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			SetupOverheadHealthBarUI();

// [원문 765-765] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	ADLPlayerState* PS = GetPlayerState<ADLPlayerState>();

// [원문 801-809] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (UBaseMovementComponent* CMC = CachedBaseMovement)
	{
		CMC->OnWallClingStateChanged.RemoveDynamic(this, &ABaseCharacter::HandleWallClingStateChanged);
		CMC->OnWallClingStateChanged.AddDynamic(this, &ABaseCharacter::HandleWallClingStateChanged);

		CMC->OnSlideStateChanged.RemoveDynamic(this, &ABaseCharacter::HandleSlideStateChanged);
		CMC->OnSlideStateChanged.AddDynamic(this, &ABaseCharacter::HandleSlideStateChanged);
	}


// [원문 815-815] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			for (const auto& EachAbility : InitialAbilities)

// [원문 818-834] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

			ApplyInitialLevelRewards();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(this, this);

			for (const auto& EachEffect : InitialPassiveEffects)
			{
				if (EachEffect)
				{
					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EachEffect, 1.0f, EffectContext);
					if (SpecHandle.IsValid())
					{
						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}

// [원문 836-843] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		else
		{
			// 리스폰 시 체력 만땅 채우기
			ASC->SetNumericAttributeBase(
				UACharacterAttributeSet::GetHealthAttribute(),
				ASC->GetNumericAttribute(UACharacterAttributeSet::GetMaxHealthAttribute())
			);
		}

// [원문 851-873] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (IsLocallyControlled())
		{
			if (ABasePlayerController* BC = Cast<ABasePlayerController>(GetController()))
			{
				BC->OnLocallyPossessed(this);
			}
		}
	}


	// 부활 시 캐릭터 스킬 슬롯 핸들 주소 자동 복구 로직!
	if (ASC)
	{
		for (FAbilitySlot& Slot : AbilitySlots)
		{
			if (Slot.AbilityClass)
			{
				if (FGameplayAbilitySpec* ExistingSpec = ASC->FindAbilitySpecFromClass(Slot.AbilityClass))
				{
					Slot.Handle = ExistingSpec->Handle; // 핸들 복구 성공!
				}
			}
		}

// [원문 875-884] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	// 포인트 변화 델리게이트 바인딩
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UACharacterAttributeSet::GetAbilityPointsAttribute())
			.AddUObject(this, &ABaseCharacter::OnAbilityPointsChangedNative);

		ASC->GetGameplayAttributeValueChangeDelegate(UACharacterAttributeSet::GetAbilityUnlockTokensAttribute())
			.AddUObject(this, &ABaseCharacter::OnAbilityUnlockTokensChangedNative);
	}

// [원문 897-914] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	// 로컬 플레이어 자신이거나 데디케이티드 서버면 3D 체력바 갱신 패스
	if (IsLocallyControlled() || GetNetMode() == NM_DedicatedServer) return;

	if (UOverheadHealthBarWidget* HealthWidget = Cast<UOverheadHealthBarWidget>(OverheadHealthBarComp->GetWidget()))
	{
		// 최대 체력이 변했으면 칸 수(Setup) 재배치
		if (!FMath::IsNearlyEqual(MaxHealth, LastKnownMaxHealth))
		{
			LastKnownMaxHealth = MaxHealth;
			HealthWidget->SetupHealthBar(New, MaxHealth, IsEnemyToLocalPlayer());
		}
		else
		{
			// 단순 데미지/회복이면 퍼센트만 갱신
			HealthWidget->UpdateHealthValue(New);
		}
	}

// [원문 930-962] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
{
	Super::Landed(Hit);

	if (ASC)
	{
		if (Hit.ImpactNormal.Z < 0.7f)
		{
			return;
		}
		
		static const FGameplayTag HasWallJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasWallJumped"), false);
		static const FGameplayTag HasAirDashedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasAirDashed"), false);
		static const FGameplayTag HasDoubleJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasDoubleJumped"), false);
		static const FGameplayTag JumpQueuedTag = FGameplayTag::RequestGameplayTag(FName("Event.Input.JumpQueued"), false);
		static const FGameplayTag HasVaultedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasVaulted"), false);
		static const FGameplayTag WallClingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.WallCling"), false);
		static const FGameplayTag WallClingAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling"), false);
		static const FGameplayTag WallJumpAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump"), false);

		ASC->SetLooseGameplayTagCount(HasWallJumpedTag, 0);
		ASC->SetLooseGameplayTagCount(HasAirDashedTag, 0);
		ASC->SetLooseGameplayTagCount(HasDoubleJumpedTag, 0);
		ASC->SetLooseGameplayTagCount(JumpQueuedTag, 0);
		ASC->SetLooseGameplayTagCount(HasVaultedTag, 0);
		ASC->SetLooseGameplayTagCount(WallClingStateTag, 0);

		FGameplayTagContainer TagsToCancel;
		TagsToCancel.AddTag(WallClingAbilityTag);
		TagsToCancel.AddTag(WallJumpAbilityTag);
		ASC->CancelAbilities(&TagsToCancel);
	}
}


// [원문 964-967] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
{
	ReplicatedMovementInput = NewInput;
}


// [원문 997-1002] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(OcclusionCheckTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(HealthBarRetryTimerHandle);
	}


// [원문 1012-1055] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

float ABaseCharacter::GetMontageEventTimeByTag(UAnimMontage* Montage, FGameplayTag EventTag)
{
	if (!Montage || !EventTag.IsValid())
		return 0.0f;

	// 몽타주 안에 있는 모든 노티파이를 순회
	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		if (UAnimNotify* Notify = NotifyEvent.Notify)
		{
			// 리플렉션: 이 노티파이가 가진 모든 변수(Property)를 검사
			for (TFieldIterator<FStructProperty> PropIt(Notify->GetClass()); PropIt; ++PropIt)
			{
				// 변수의 타입이 FGameplayTag 인지 확인
				if (PropIt->Struct == FGameplayTag::StaticStruct())
				{
					// 실제 태그 값을 읽어옴
					FGameplayTag* FoundTag = PropIt->ContainerPtrToValuePtr<FGameplayTag>(Notify);

					// 우리가 찾는 태그(Event.Reload.Complete)와 일치하면!
					if (FoundTag && *FoundTag == EventTag)
					{
						return NotifyEvent.GetTriggerTime(); // 노티파이가 박힌 정확한 시간을 반환
					}
				}
			}
		}
	}

	return 0.0f; // 못 찾으면 0 반환
}

void ABaseCharacter::OnAbilityPointsChangedNative(const FOnAttributeChangeData& Data)
{
	// UI(블루프린트)에게 "포인트 변했어! 새 값은 이거야!"라고 방송을 내보냅니다.
	OnAbilityPointsChanged.Broadcast(Data.NewValue);
}

void ABaseCharacter::OnAbilityUnlockTokensChangedNative(const FOnAttributeChangeData& Data)
{
	OnAbilityUnlockTokensChanged.Broadcast(Data.NewValue);
}


// [원문 1176-1176] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Debuff.Stun")), false);

// [원문 1233-1259] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
void ABaseCharacter::SetUpgradeMode(bool bActive)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (bActive)
	{
		PC->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		if (SkillContainerWidget)
		{
			// TakeWidget()을 사용하면 Slate 위젯을 안전하게 가져와 포커스를 줄 수 있습니다.
			InputMode.SetWidgetToFocus(SkillContainerWidget->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	OnUpgradeModeToggled.Broadcast(bActive);
}


// [원문 1298-1417] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
}


void ABaseCharacter::SetupOverheadHealthBarUI()
{
	if (IsLocallyControlled() || GetNetMode() == NM_DedicatedServer)
	{
		OverheadHealthBarComp->SetHiddenInGame(true);
		return;
	}

	ADLPlayerState* PS = GetPlayerState<ADLPlayerState>();

	// 데이터 복제 확인
	if (!ASC || !HealthComponent || !PS || PS->GetTeamIdx() == INDEX_NONE)
	{
		if (GetWorld() && !GetWorldTimerManager().IsTimerActive(HealthBarRetryTimerHandle))
		{
			GetWorldTimerManager().SetTimer(HealthBarRetryTimerHandle, this, &ThisClass::SetupOverheadHealthBarUI, 0.1f, false);
		}
		return;
	}

	// ★ [추가 및 수정] 위젯 컴포넌트 내부의 3D 위젯이 렌더링을 위해 생성되었는지 확인
	UOverheadHealthBarWidget* HealthWidget = Cast<UOverheadHealthBarWidget>(OverheadHealthBarComp->GetWidget());
	if (!HealthWidget)
	{
		// 위젯이 아직 메모리에 안 올라왔다면 0.1초 뒤에 다시 시도하도록 타이머 예약!
		if (GetWorld() && !GetWorldTimerManager().IsTimerActive(HealthBarRetryTimerHandle))
		{
			GetWorldTimerManager().SetTimer(HealthBarRetryTimerHandle, this, &ThisClass::SetupOverheadHealthBarUI, 0.1f, false);
		}
		return;
	}

	// 모든 셋업에 성공했으므로 재시도 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthBarRetryTimerHandle);
	}

	OverheadHealthBarComp->SetHiddenInGame(false);

	const float MaxH = ASC->GetNumericAttribute(UACharacterAttributeSet::GetMaxHealthAttribute());
	const float CurH = ASC->GetNumericAttribute(UACharacterAttributeSet::GetHealthAttribute());

	LastKnownMaxHealth = MaxH;
	HealthWidget->SetupHealthBar(CurH, MaxH, IsEnemyToLocalPlayer());

	if (!GetWorldTimerManager().IsTimerActive(OcclusionCheckTimerHandle))
	{
		GetWorldTimerManager().SetTimer(OcclusionCheckTimerHandle, this, &ThisClass::CheckHealthBarOcclusion, 0.2f, true);
	}
}

void ABaseCharacter::CheckHealthBarOcclusion()
{
	if (!IsValid(this) || !GetWorld())
	{
		return;
	}

	// 2. [방어막 2] 위젯 컴포넌트가 확실히 존재하는지 검사
	if (!IsValid(OverheadHealthBarComp))
	{
		return;
	}

	// 3. [방어막 3] 메쉬가 확실히 존재하는지 검사 (GetMesh가 null인 경우 완벽 방어)
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh == nullptr)
	{
		return;
	}

	// 4. [방어막 4] 메쉬가 최근에 화면에 그려졌는지 검사 (Frustum Culling)
	if (!CharacterMesh->WasRecentlyRendered(0.2f))
	{
		return;
	}

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->PlayerCameraManager) return;

	const FVector CameraLoc = LocalPC->PlayerCameraManager->GetCameraLocation();
	const FVector TargetLoc = OverheadHealthBarComp->GetComponentLocation();

	if (FVector::DistSquared(CameraLoc, TargetLoc) > (MaxHealthBarDrawDistance * MaxHealthBarDrawDistance))
	{
		OverheadHealthBarComp->SetHiddenInGame(true);
		return;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (APawn* LocalPawn = LocalPC->GetPawn())
	{
		QueryParams.AddIgnoredActor(LocalPawn);
	}

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLoc, TargetLoc, ECC_WorldStatic, QueryParams);
	OverheadHealthBarComp->SetHiddenInGame(bHit);
}

bool ABaseCharacter::IsEnemyToLocalPlayer() const
{
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC) return true;

	ADLPlayerState* LocalPS = LocalPC->GetPlayerState<ADLPlayerState>();
	ADLPlayerState* TargetPS = GetPlayerState<ADLPlayerState>();

	if (LocalPS && TargetPS)
	{
		return LocalPS->GetTeamIdx() != TargetPS->GetTeamIdx();
	}
	return true;
}


// [원문 1433-1467] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

void ABaseCharacter::HandleWallClingStateChanged(bool bIsClinging)
{
	if (!ASC) return;
	static const FGameplayTag WallClingStateTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.WallCling"), false);
	static const FGameplayTag WallClingAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling"), false);

	if (bIsClinging)
	{
		if (!ASC->HasMatchingGameplayTag(WallClingStateTag))
		{
			ASC->SetLooseGameplayTagCount(WallClingStateTag, 1);
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WallClingAbilityTag));
		}
	}
	else
	{
		ASC->SetLooseGameplayTagCount(WallClingStateTag, 0);
		ASC->RemoveLooseGameplayTag(WallClingStateTag);
		FGameplayTagContainer TagsToCancel(WallClingAbilityTag);
		ASC->CancelAbilities(&TagsToCancel);
	}
}

void ABaseCharacter::HandleSlideStateChanged(bool bIsSliding)
{
	if (!ASC) return;
	if (!bIsSliding)
	{
		static const FGameplayTag SlideAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide"), false);
		FGameplayTagContainer TagsToCancel(SlideAbilityTag);
		ASC->CancelAbilities(&TagsToCancel);
	}
}


