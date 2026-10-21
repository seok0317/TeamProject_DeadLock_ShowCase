// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/GameCore/GameRule/Nexus.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameCore/Component/HealthComponent.h"
#include "GameCore/Struct/ACharacterAttributeSet.h"
#include "DL_AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BrainComponent.h"
#include "GameCore/Struct/DLCollisionTypes.h"
#include "Net/UnrealNetwork.h"

ANexus::ANexus(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	// Phase 1 전용 피격 스켈레탈 메쉬 컴포넌트 생성
	Phase1HitMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Phase1HitMesh"));
	Phase1HitMesh->SetupAttachment(RootComponent);
	Phase1HitMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 피격 판정 허용

	// UI 띄우기용 스피어 생성
	UITriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("UITriggerSphere"));
	UITriggerSphere->SetupAttachment(RootComponent);
	UITriggerSphere->SetSphereRadius(1500.f); 
	UITriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
	
	// 애니메이션용 SkeletalMeshComponent 기본 설정 (1페이즈에선 충돌 없음)
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// 넥서스는 기본적으로 고정형
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void ANexus::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetTeam(NexusTeamIdx); // 명시적으로 지정한 넥서스 팀 번호를 AICharacter 베이스 팀 로직에 등록
	}

	// 넥서스 전용 델리게이트 설정
	if (HealthComponent)
	{
		// AAICharacter에서 등록된 사망 콜백을 제거하고 넥서스 전용 추가
		HealthComponent->OnDeathStarted.RemoveDynamic(this, &ANexus::OnMinionDeath);
		HealthComponent->OnDeathStarted.AddDynamic(this, &ANexus::OnNexusDeath);

		// 체력 절반 체크용 델리게이트 연결
		HealthComponent->OnHealthChanged.AddDynamic(this, &ANexus::OnNexusHealthChanged);
	}

	// 오버랩 델리게이트 바인딩
	UITriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANexus::OnUITriggerBeginOverlap);
	UITriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANexus::OnUITriggerEndOverlap);

	// 1페이즈 진입 처리
	CurrentPhase = ENexusPhase::Phase1;
	
	// 1페이즈일 때는 AI 행동 끄기 (공격 안함)
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic(TEXT("Phase1"));
		}
	}
}

void ANexus::SetInvincible(bool bInvincible)
{
	bIsInvincible = bInvincible;
}

void ANexus::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANexus, bIsInvincible);
}

float ANexus::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 무적 상태라면 데미지 0 반환
	if (bIsInvincible)
	{
		return 0.f;
	}

	// 팀 식별 (아군 오사/팀킬 방지)
	// 1. 공격 컨트롤러(EventInstigator)를 통해 공격자 폰을 가져와서 팀 검사
	ABaseCharacter* AttackerChar = nullptr;
	if (EventInstigator)
	{
		AttackerChar = Cast<ABaseCharacter>(EventInstigator->GetPawn());
	}
	
	// 2. 컨트롤러가 안 넘어왔다면, 데미지 유발자(투사체 등)의 주인을 통해 검사
	if (!AttackerChar && DamageCauser)
	{
		AttackerChar = Cast<ABaseCharacter>(DamageCauser->GetInstigator());
	}

	// 3. 공격자가 확인되었고, 넥서스와 팀이 같다면 데미지 무시
	if (AttackerChar && AttackerChar->GetTeamIdx() == this->GetTeamIdx())
	{
		return 0.f;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ANexus::ApplyTeamCollision(int32 InTeamIdx)
{
	Super::ApplyTeamCollision(InTeamIdx);

	if (InTeamIdx < 0)
	{
		return;
	}

	int TargetCollisionIdx = InTeamIdx % 2;
	static const ECollisionChannel Channels[] = { ECC_Team1, ECC_Team2, ECC_Monster };

	// BaseCharacter에서는 기본 Mesh에 팀 콜리전을 적용하지만,
	// 넥서스는 1페이즈에서 별도의 Phase1HitMesh를 피격용으로 사용하므로 여기에 팀 콜리전을 적용합니다.
	if (Phase1HitMesh)
	{
		Phase1HitMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Phase1HitMesh->SetCollisionObjectType(Channels[TargetCollisionIdx]);
		Phase1HitMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Phase1HitMesh->SetCollisionResponseToChannel(ECC_HitObject, ECR_Block);
		Phase1HitMesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Block);
	}

	// 유저 BP 설계: 넥서스는 페이즈와 상관없이 항상 Phase1HitMesh로 피격 판정을 받음.
	// 따라서 Super가 강제로 켜버린 메인 메쉬(GetMesh) 충돌은 모든 페이즈에서 무조건 꺼줍니다.
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
}

void ANexus::TransitionToPhase2()
{
	if (CurrentPhase == ENexusPhase::Phase2)
	{
		return;
	}

	CurrentPhase = ENexusPhase::Phase2;

	// 2페이즈 진입 시 AI 컨트롤러 다시 활성화 (공격 시작)
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->RestartLogic();
		}
	}

	// 블루프린트로 이벤트 전달 (폭발 이펙트 스폰, 골격 메쉬 교체, 애니메이션 재생, 체력 리셋 등)
	OnPhaseChanged(CurrentPhase);
}

void ANexus::OnNexusDeath(AActor* DeadActor, AActor* InInstigator)
{
	if (DeadActor != this)
	{
		return;
	}

	if (CurrentPhase == ENexusPhase::Phase2)
	{
		// 서버에서 멀티캐스트를 발송하여 모든 클라이언트가 OnNexusDestroyedEvent를 실행하게 함
		MulticastNexusDestroyed();
	}
	
	// [수정됨] 부모 (AICharacter)의 사망 로직(즉시 Destroy)을 호출하지 않습니다!
	// 넥서스는 즉시 삭제되지 않으므로, 블루프린트의 OnNexusDestroyedEvent에서
	// 이펙트 연출(Delay 등)을 모두 끝마친 뒤 마지막에 직접 'Destroy Actor' 노드를 달아서 삭제해주시면 됩니다.
}

void ANexus::MulticastNexusDestroyed_Implementation()
{
	// 이 함수는 모든 클라이언트(화면)에서 각각 실행됩니다.
	OnNexusDestroyedEvent();
}

void ANexus::OnNexusHealthChanged(float OldValue, float NewValue, float MaxHealth)
{
	// 1페이즈에서 체력이 절반(50%) 이하로 떨어지면 2페이즈로 전환
	if (CurrentPhase == ENexusPhase::Phase1 && NewValue <= (MaxHealth * 0.5f))
	{
		// 한 번에 강력한 공격을 맞아 50% 미만으로 뚫고 내려갔다면, 체력을 정확히 50%로 강제 복구 (오버 데미지 방어)
		if (NewValue < (MaxHealth * 0.5f))
		{
			if (UDL_AbilitySystemComponent* DL_ASC = Cast<UDL_AbilitySystemComponent>(GetAbilitySystemComponent()))
			{
				DL_ASC->SetNumericAttributeBase(UACharacterAttributeSet::GetHealthAttribute(), MaxHealth * 0.5f);
			}
		}

		TransitionToPhase2();
	}
}

void ANexus::MulticastPlayAoECharging_Implementation(UNiagaraSystem* EffectSystem, FVector Scale, FVector Offset)
{
	// 데디케이티드 서버가 아니라면 (즉, 화면을 그리는 클라이언트나 리슨서버라면) 스폰
	if (EffectSystem && GetNetMode() != NM_DedicatedServer)
	{
		if (ActiveChargingNiagara)
		{
			ActiveChargingNiagara->DestroyComponent();
		}
		
		ActiveChargingNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
			EffectSystem,
			GetRootComponent(),
			NAME_None,
			Offset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		if (ActiveChargingNiagara)
		{
			ActiveChargingNiagara->SetRelativeScale3D(Scale);
		}
	}
}

void ANexus::MulticastStopAoECharging_Implementation()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (ActiveChargingNiagara)
		{
			ActiveChargingNiagara->DestroyComponent();
			ActiveChargingNiagara = nullptr;
		}
	}
}

void ANexus::MulticastPlayAoEExplosion_Implementation(UNiagaraSystem* EffectSystem, FVector Scale, FVector Offset)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (ActiveChargingNiagara)
		{
			ActiveChargingNiagara->DestroyComponent();
			ActiveChargingNiagara = nullptr;
		}

		if (EffectSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectSystem, GetActorLocation() + Offset, FRotator::ZeroRotator, Scale);
		}
	}
}

void ANexus::OnUITriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		// 플레이어라면 위젯 업데이트 발송 (블루프린트에서 구체적인 캐스팅 검사 및 HUD 갱신 구현)
		UpdateNexusUI(OtherActor, true);
	}
}

void ANexus::OnUITriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
	{
		UpdateNexusUI(OtherActor, false);
	}
}
