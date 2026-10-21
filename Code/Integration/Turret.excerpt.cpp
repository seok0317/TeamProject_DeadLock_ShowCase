// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/GameCore/GameRule/Turret.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: dalbi090211의 포탑 클래스에 고유 사망 콜백과 마지막 최종 포탑 확인→같은 팀 넥서스 무적 해제 처리를 추가. 포탑 AI·빔 공격 본체 제외.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 5-7] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "Kismet/GameplayStatics.h"
#include "GameCore/GameRule/Nexus.h"
#include "GameCore/Component/HealthComponent.h"

// [원문 16-16] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	Super::BeginPlay();

// [원문 24-31] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		SetTeam(TurretTeamIdx);
	}

	if (HealthComponent)
	{
		// 부모(AAICharacter)의 즉사 로직(OnMinionDeath)을 제거하고 타워 고유 사망 로직으로 덮어씌움
		HealthComponent->OnDeathStarted.RemoveDynamic(this, &ATurret::OnMinionDeath);
		HealthComponent->OnDeathStarted.AddDynamic(this, &ATurret::OnTurretDeath);

// [원문 34-87] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

void ATurret::OnTurretDeath(AActor* DeadActor, AActor* InInstigator)
{
	if (DeadActor != this) return;

	UE_LOG(LogTemp, Warning, TEXT("[Turret] OnTurretDeath called for %s, Team: %d, bIsFinalTower: %d"), *GetName(), TurretTeamIdx, bIsFinalTower);

	if (bIsFinalTower && HasAuthority())
	{
		bool bOtherFinalTowersAlive = false;
		TArray<AActor*> FoundTurrets;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurret::StaticClass(), FoundTurrets);
		
		for (AActor* Actor : FoundTurrets)
		{
			if (ATurret* OtherTurret = Cast<ATurret>(Actor))
			{
				if (OtherTurret != this && OtherTurret->TurretTeamIdx == this->TurretTeamIdx && OtherTurret->bIsFinalTower)
				{
					if (OtherTurret->GetHealthComponent() && !OtherTurret->GetHealthComponent()->IsDead())
					{
						bOtherFinalTowersAlive = true;
						break;
					}
				}
			}
		}

		if (!bOtherFinalTowersAlive)
		{
			TArray<AActor*> FoundNexus;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANexus::StaticClass(), FoundNexus);
			for (AActor* Actor : FoundNexus)
			{
				if (ANexus* Nexus = Cast<ANexus>(Actor))
				{
					if (Nexus->GetTeamIdx() == this->TurretTeamIdx)
					{
						UE_LOG(LogTemp, Warning, TEXT("[Turret] ALL Final Towers dead! Unlocking Nexus: %s"), *Nexus->GetName());
						Nexus->SetInvincible(false);
						break;
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Turret] Other Final Towers are still alive. Nexus remains invincible."));
		}
	}

	// 넥서스 연산이 모두 끝난 뒤, 자연스럽게 사라지도록 3초 뒤에 자동 파괴(가비지 컬렉션)
	SetLifeSpan(3.0f);
}

