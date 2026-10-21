# Blueprint·에셋 기여의 근거와 공개 범위

`.uasset/.umap`의 변경은 원작 리소스 제작이나 BP 그래프 전체의 작성자를 입증하지 않습니다. 본인 PR·변경 이력과 C++ 호출부를 함께 확인했고, 바이너리 파일 자체는 이 저장소에 포함하지 않았습니다.

| 업무 | 확인한 이력·연결 | 추가 확인할 자료 |
|---|---|---|
| MainHUD·탄약·체력·레벨·소울·스킬 UI | #52, #57, #65; BaseCharacter의 상태 getter·델리게이트·초기화 연결 | WBP_MainHUD/SkillContainer 생성·바인딩·재시도 해제 그래프 |
| 스킬 해금·강화·입력 충돌 | #52; SkillIcon·UI 모드·쿨타임 조회 | WBP_SkillSlot/UpgradeColumn/TierButton과 Handled 반환 |
| 활력 표시·재생·소모 | #52; 패시브 GE 적용·Base Value 상한 보정 | Stamina 머티리얼·GE 주기·이동 GA 비용 연결 |
| 월드 체력바 | #57; C++ 구간 계산·피아·거리·가림 판정 | WBP_OverheadHealthBar/HealthBlock의 실제 슬롯 수·배치·경계값 |
| 인벤토리 쿨타임 | #57; 슬롯 위젯·쿨타임 이벤트 | WBP_InventorySlot의 카운트다운·사용 직후 갱신 |
| Lash 캐릭터·로프·Cue | #57/#62/#64; Lash GA와 다중 대상 EffectContext | BP_Lash·GCN·Niagara 연결과 실제 플레이 |
| 자동 파쿠르·몽타주 | #44/#46/#68; Vault 판정·커브 이동 | GA_Vault의 기본값 오버라이드·섹션/커브·취소 동작 |
| 넥서스·결과 UI | #70; Nexus의 단계·파괴 이벤트, AoE GA | BP_NexusBase/WBP_GameResult의 팀별 결과·복귀·BT 반복 설정 |
| 맵·라이팅·리소스 통합 | 본인 맵·설정 변경 이력 | 전체 맵 원 제작이나 외부 에셋 소유로 해석하지 않음 |

BP 그래프를 확인하지 못한 부분은 C++로 임의 재작성하지 않았습니다. 원본 프로젝트의 모든 본인 변경 에셋·설정 경로는 로컬 조사 문서에 보존되어 있습니다.
