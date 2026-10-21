// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/UI/InventorySlotWidget.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GA_SkillBase.h"

void UInventorySlotWidget::SetItemData(const FItemInstance& Instance, const FItemDefinitionRow& Def)
{
    // 1. 아이콘 세팅
    if (ItemIcon && !Def.Icon.IsNull())
    {
        if (UTexture2D* LoadedTexture = Def.Icon.LoadSynchronous())
        {
            ItemIcon->SetBrushFromTexture(LoadedTexture, false);
            ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
    }

    // 2. 쿨타임 데이터 찾기
    float CurrentTime = GetWorld()->GetTimeSeconds();
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
    bool bFoundCooldown = false;

    for (int32 i = 0; i < Def.Effects.Num(); ++i)
    {
        const FItemEffectDefinition& EffectDef = Def.Effects[i];
        const FItemEffectRuntimeState* RuntimeState = Instance.RuntimeEffects.FindByPredicate(
            [&](const FItemEffectRuntimeState& State) { return State.EffectId == EffectDef.EffectId; });

        if (!RuntimeState) continue;

        // 트리거 효과
        if (EffectDef.EffectType == EItemEffectType::Trigger)
        {
            if (RuntimeState->CooldownEndTime > CurrentTime)
            {
                BP_OnCooldownStarted(RuntimeState->CooldownEndTime, EffectDef.TriggerData.InternalCooldown);
                bFoundCooldown = true;
                break;
            }
        }
        // 액티브(Active) 효과 
        else if (EffectDef.EffectType == EItemEffectType::ActiveAbility && ASC)
        {
            if (RuntimeState->GrantedAbilityHandle.IsValid())
            {
                FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(RuntimeState->GrantedAbilityHandle);
                if (Spec && Spec->GetPrimaryInstance())
                {
                    if (UGA_SkillBase* SkillGA = Cast<UGA_SkillBase>(Spec->GetPrimaryInstance()))
                    {
                        float TimeRemaining = SkillGA->GetCooldownRemaining();
                        if (TimeRemaining > 0.f)
                        {
                            BP_OnCooldownStarted(CurrentTime + TimeRemaining, SkillGA->GetConfiguredCooldownDuration());
                            bFoundCooldown = true;
                            break;
                        }
                    }
                    else
                    {
                        float TimeRemaining = 0.f, TotalDuration = 0.f;
                        Spec->GetPrimaryInstance()->GetCooldownTimeRemainingAndDuration(Spec->Handle, ASC->AbilityActorInfo.Get(), TimeRemaining, TotalDuration);
                        if (TimeRemaining > 0.f)
                        {
                            BP_OnCooldownStarted(CurrentTime + TimeRemaining, TotalDuration);
                            bFoundCooldown = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // 쿨타임이 안 돌고 있다면 끄라고 신호 보냄
    if (!bFoundCooldown)
    {
        BP_OnCooldownStopped();
    }
}

void UInventorySlotWidget::SetEmpty()
{
    if (ItemIcon)
    {
        ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    // 슬롯이 비워졌으니 쿨타임도 강제로 끔
    BP_OnCooldownStopped();
}