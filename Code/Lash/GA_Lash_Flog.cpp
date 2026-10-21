// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: B — 본인 구현 + 팀원 비기능 수정
// 원본: Source/TeamProject_DL/Lash/GA_Lash_Flog.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: Flog 공격·이론 피해 기반 회복·Cue 연결은 본인. Youngwoo의 fef04fe01 병합 수정은 Character.h include와 마지막 줄바꿈이며, 두 번째 부모와의 diff로 확인.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GA_Lash_Flog.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Common/SkillDamageLibrary.h"
#include "TeamUtils.h"
#include "GameFramework/Character.h"
#include "GameCore/Struct/ACharacterAttributeSet.h"

UGA_Lash_Flog::UGA_Lash_Flog()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	TargetingMode = ESkillTargetingMode::Scope;
	ConfirmMode = ESkillConfirmMode::Auto;
	AutoConfirmDelay = 0.01f;
}

void UGA_Lash_Flog::OnCastStarted_Implementation()
{
	Super::OnCastStarted_Implementation();

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	//차단 태그 제거 함수를 호출
	GetWorld()->GetTimerManager().SetTimer(
		AbilityEndTimerHandle,
		this,
		&UGA_Lash_Flog::TimerUnlockControls,
		SkillLockDuration,
		false
	);
}

void UGA_Lash_Flog::TimerUnlockControls()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		static const FGameplayTag SkillActiveTag = FGameplayTag::RequestGameplayTag(FName("State.Skill.Active"), false);
		ASC->RemoveLooseGameplayTag(SkillActiveTag);
	}
}

void UGA_Lash_Flog::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Lash_Flog::OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload)
{
	if (!EventTag.MatchesTag(SkillEventParentTag)) return;

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!AvatarChar || !MyASC || CachedTargetData.Num() == 0) return;

	// CachedTargetData에서 타겟 액터 배열 추출
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(CachedTargetData, 0);
	if (TargetActors.Num() == 0) return;

	
	// =========================================================================
	FGameplayCueParameters CueParams;
	CueParams.Instigator = AvatarChar; // 빔의 시작점이 될 시전자(나 자신) 지정

	// AddActors 함수 파라미터 규격에 맞게 TWeakObjectPtr 배열로 변환
	TArray<TWeakObjectPtr<AActor>> WeakActors;
	for (AActor* Actor : TargetActors)
	{
		if (Actor)
		{
			WeakActors.Add(Actor);
		}
	}

	// 1-2. EffectContext를 생성하고 액터 배열을 얹어줌
	FGameplayEffectContextHandle CueContext = MyASC->MakeEffectContext();
	CueContext.AddActors(WeakActors);
	CueParams.EffectContext = CueContext;

	// 1-3. 내 ASC에 큐를 발송함 (이 RPC를 타고 클라이언트의 GCN_Lash_FlogRope가 실행됨)
	static const FGameplayTag FlogRopeCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Lash.FlogRope"), false);
	MyASC->ExecuteGameplayCue(FlogRopeCueTag, CueParams);

	if (HasAuthority(&CurrentActivationInfo))
	{
		float TotalHealAmount = 0.0f;

		for (AActor* TargetActor : TargetActors)
		{
			if (!TargetActor || !SkillTeamUtils::IsHostile(AvatarChar, TargetActor)) continue;

			// 2-1. 데미지 적용
			USkillDamageLibrary::ApplyDamage(AvatarChar, TargetActor, SpiritCoefficient, BaseDamage, TAG_Data_Damage_Spirit);

			// 2-2. 적(TargetActor)의 ASC를 가져와서 FlogEffect(채찍 효과) 적용
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC && FlogEffectClass)
			{
				FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
				Context.AddInstigator(AvatarChar, AvatarChar);

				TargetASC->ApplyGameplayEffectToSelf(FlogEffectClass->GetDefaultObject<UGameplayEffect>(), GetAbilityLevel(), Context);
			}

			// 2-3. 힐량 누적 계산
			float SpiritPower = MyASC->GetNumericAttribute(UACharacterAttributeSet::GetSpiritPowerAttribute());
			float TheoreticalDamage = (SpiritPower * SpiritCoefficient) + BaseDamage;
			TotalHealAmount += (TheoreticalDamage * HealMultiplier);
		}

		// 2-4. 누적된 힐량을 나 자신에게 일괄 적용
		if (TotalHealAmount > 0.0f && HealEffectClass)
		{
			FGameplayEffectContextHandle HealContext = MyASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(HealEffectClass, GetAbilityLevel(), HealContext);

			if (SpecHandle.IsValid())
			{
				static const FGameplayTag HealTag = FGameplayTag::RequestGameplayTag(FName("Data.Heal"), false);
				SpecHandle.Data->SetSetByCallerMagnitude(HealTag, TotalHealAmount);
				MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
