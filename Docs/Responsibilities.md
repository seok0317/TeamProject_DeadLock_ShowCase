# 전체 담당 업무와 구현 상태

기존 조사 48개 항목을 현재 Git과 대조해 재사용한 업무 인덱스입니다. 각 항목은 신규 독립 시스템 1개를 뜻하지 않습니다. BP 표현·설정은 바이너리 내부와 런타임을 새로 검증하지 않았습니다. 원본 PR은 비공개 저장소 접근 권한이 필요합니다.

## A01. 기본 조작과 3인칭 카메라

- **실제 구현**: Enhanced Input으로 이동·시점·점프·대시 액션을 바인딩하고, 컨트롤러 Yaw 기준 전후·좌우 이동을 계산했다. SpringArm 거리·우측 어깨 오프셋·CameraLag와 회전 설정을 조정했다.
- **담당 범위·기여 형태**: 기존 캐릭터 뼈대를 확장한 직접 구현. 카메라 컴포넌트 자체의 최초 생성과 이후 다른 팀원의 조준·포털 기능까지 본인 구현으로 포함하지 않는다.
- **기술·관련 클래스**: APlayerCharacter, UEnhancedInputComponent, UEnhancedInputLocalPlayerSubsystem, USpringArmComponent
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 카메라 기준 이동과 실제 이동기 방향 계산을 연결한 공통 조작 기반.
- **전시 코드**: [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp)
- **이력**: [`5ac8fafa4`](https://github.com/2021Cyan/TeamProject2/commit/5ac8fafa4eaebbe217628e060c3baf3a98faf412) · [`251c6be67`](https://github.com/2021Cyan/TeamProject2/commit/251c6be67fd7bbda352ce7adbf4a8cc80f035a9e) · [`6067991e6`](https://github.com/2021Cyan/TeamProject2/commit/6067991e627efd9ab7ea84a55fb277a996fc27d5) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6).

## A02. 공통 커스텀 이동 컴포넌트 도입

- **실제 구현**: FObjectInitializer::SetDefaultSubobjectClass로 기본 CharacterMovement를 UBaseMovementComponent로 교체하고, 이동 물리·벽 감지·슬라이드·파쿠르 검사 기능을 공통 컴포넌트에 구성했다.
- **담당 범위·기여 형태**: UBaseMovementComponent 신규 구현 및 팀 공통 BaseCharacter 통합. Viscous 전용 이동 컴포넌트의 구체형 동작은 팀원 작업이다.
- **기술·관련 클래스**: UBaseMovementComponent, ABaseCharacter, FObjectInitializer
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 캐릭터별 중복 구현을 줄이고 공통 이동 판정과 튜닝 지점을 제공.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`5ac8fafa4`](https://github.com/2021Cyan/TeamProject2/commit/5ac8fafa4eaebbe217628e060c3baf3a98faf412) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6).

## A03. 공중 대시와 관성·방향별 애니메이션

- **실제 구현**: 입력 방향을 캐릭터 전방·우측 벡터와 내적해 몽타주 4방향 섹션을 선택한다. 준비 구간에서 감속한 뒤 기존 수평 관성과 대시 벡터를 합산하고 최대 대시 속도를 제한해 Root Motion Force로 이동한다.
- **담당 범위·기여 형태**: 공중 대시 C++·GA·몽타주 설정 직접 구현. 공중 재사용 제한과 종료 재진입 방지 포함.
- **기술·관련 클래스**: UAirDashAbility, LocalPredicted, UAbilityTask_ApplyRootMotionConstantForce, bIsEnding
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 기존 속도와 입력을 합성하는 조작감 개선, 종료 콜백 중복에 의한 태그 누적 방지. 지연별 러버밴딩 개선 수치는 없음.
- **전시 코드**: [Code/Movement/AirDashAbility.excerpt.cpp](../Code/Movement/AirDashAbility.excerpt.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`5ac8fafa4`](https://github.com/2021Cyan/TeamProject2/commit/5ac8fafa4eaebbe217628e060c3baf3a98faf412) · [`251c6be67`](https://github.com/2021Cyan/TeamProject2/commit/251c6be67fd7bbda352ce7adbf4a8cc80f035a9e) · [`6067991e6`](https://github.com/2021Cyan/TeamProject2/commit/6067991e627efd9ab7ea84a55fb277a996fc27d5) · [`cf35cc62e`](https://github.com/2021Cyan/TeamProject2/commit/cf35cc62ed17e3b4e2e49234491da38284a906a7) · [`15a213c40`](https://github.com/2021Cyan/TeamProject2/commit/15a213c407e1705e5f9ccfbdd858eea022a46c86) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #39](https://github.com/2021Cyan/TeamProject2/pull/39) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6).

## A04. 지상 구르기와 카메라 연출

- **실제 구현**: 지상 대시 입력을 구르기로 분기하고 입력 방향으로 캐릭터를 회전시킨다. 몽타주와 Root Motion Force를 재생하며, 진행 방향에 따라 짧게 CameraLag를 조정하고 종료·취소 시 원래 설정을 복원한다. 후속 수정에서 중력이 반영되는 이동을 적용했다.
- **담당 범위·기여 형태**: UGroundRollAbility 신규 구현, 캐릭터 입력과 몽타주 통합. 무적 회피 기능은 확인되지 않아 주장하지 않는다.
- **기술·관련 클래스**: UGroundRollAbility, Root Motion Constant Force, SpringArm, TimerManager
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 이동·몸통 회전·카메라 상태를 함께 전환하고 취소 경로에서도 복원.
- **전시 코드**: [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp) · [Code/Movement/GroundRollAbility.excerpt.cpp](../Code/Movement/GroundRollAbility.excerpt.cpp)
- **이력**: [`1c68301a4`](https://github.com/2021Cyan/TeamProject2/commit/1c68301a42b4a037dcf380f07a12f5ffb4aec2fd) · [`21de6b96d`](https://github.com/2021Cyan/TeamProject2/commit/21de6b96d260c9eb094f324b7b7f1846098febc8) · [`7f20ec590`](https://github.com/2021Cyan/TeamProject2/commit/7f20ec59068440f8d5b197b37df7c9d338ac9f67) · [PR #39](https://github.com/2021Cyan/TeamProject2/pull/39) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7).

## A05. 구르기 점프와 입력 허용 구간

- **실제 구현**: 구르기 중 타이머로 State.Movement.CanJump 태그를 열고 닫는다. 해당 구간의 점프 입력은 구르기를 취소하고 전방 부스트와 수직 속도를 가진 RollJump를 실행한다.
- **담당 범위·기여 형태**: 구르기 점프 C++ 직접 구현. 추가 스태미나 비용과 게이지 색상 연동은 GA·GE·UMG 설정까지 별도 검증해야 한다.
- **기술·관련 클래스**: URollJumpAbility, UGroundRollAbility, LaunchCharacter, GameplayTag
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 짧은 입력 타이밍을 태그로 표현해 이동기와 UI가 같은 상태를 참조.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Movement/GroundRollAbility.excerpt.cpp](../Code/Movement/GroundRollAbility.excerpt.cpp) · [Code/Movement/RollJumpAbility.cpp](../Code/Movement/RollJumpAbility.cpp)
- **이력**: [`f769f460c`](https://github.com/2021Cyan/TeamProject2/commit/f769f460c783ae3045b907f28c2e43ce8fb0f3cf) · [`f30f73fda`](https://github.com/2021Cyan/TeamProject2/commit/f30f73fda7d836b5374dd781b9ab2db747078976) · [`249fc259f`](https://github.com/2021Cyan/TeamProject2/commit/249fc259fce810baaf7a9101fa360da732b721e4) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7).

## A06. 이단 점프

- **실제 구현**: 현재 수평 속도에 입력 방향 힘을 합성하고 상한을 적용한 뒤 수직 점프력을 부여한다. 사용 태그로 공중 중복 실행을 제한하고 착지 시 회수한다.
- **담당 범위·기여 형태**: UDoubleJumpAbility와 공통 점프 분기 직접 구현. 후속 버그 수정과 자동 파쿠르 감시 포함.
- **기술·관련 클래스**: UDoubleJumpAbility, LaunchCharacter, GameplayTag, TimerManager
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 입력·관성·사용 횟수와 후속 벽넘기 동작을 함께 제어.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Movement/DoubleJumpAbility.cpp](../Code/Movement/DoubleJumpAbility.cpp)
- **이력**: [`f769f460c`](https://github.com/2021Cyan/TeamProject2/commit/f769f460c783ae3045b907f28c2e43ce8fb0f3cf) · [`15098b31b`](https://github.com/2021Cyan/TeamProject2/commit/15098b31b3dc8cdaa6042a6bb1ebc24ad459857b) · [`7f20ec590`](https://github.com/2021Cyan/TeamProject2/commit/7f20ec59068440f8d5b197b37df7c9d338ac9f67) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7) · [Issue #9](https://github.com/2021Cyan/TeamProject2/issues/9) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## A07. 벽 감지와 매달림 상태·몽타주

- **실제 구현**: 낙하 중 전후좌우 LineTrace와 벽 노멀의 Z값으로 수직벽을 감지한다. 벽넘기 가능한 지형은 제외하고, 벽 방향에 맞춰 정면·좌·우 매달림 몽타주를 선택한다. 다른 이동기 중에는 감지를 제한한다.
- **담당 범위·기여 형태**: 벽 감지·WallCling 어빌리티·상태 연동 직접 구현. 자유 등반이나 별도 벽 달리기 전체 시스템으로 확대 표현하지 않는다.
- **기술·관련 클래스**: UBaseMovementComponent::UpdateWallCling, UWallClingAbility, FVector::DotProduct
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 벽 노멀과 캐릭터 방향으로 애니메이션을 분기하고 이동 상태 간 충돌을 제어.
- **전시 코드**: [Code/Movement/WallClingAbility.cpp](../Code/Movement/WallClingAbility.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`f30f73fda`](https://github.com/2021Cyan/TeamProject2/commit/f30f73fda7d836b5374dd781b9ab2db747078976) · [`eb6b91bdb`](https://github.com/2021Cyan/TeamProject2/commit/eb6b91bdb29c19fcc62806ac4c664a8613882ae2) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## A08. 벽 점프와 무한 상승 제한

- **실제 구현**: 벽 노멀 방향 밀어내기 힘과 입력 방향 관성을 합성한다. 첫 벽 점프는 수직 힘을 주고, 후속 벽 점프는 수직 성분을 0으로 제한한다. 벽 상태 해제 및 대시 몽타주 방향 선택을 연결했다.
- **담당 범위·기여 형태**: UWallJumpAbility 직접 구현·개선. 강한 벽 점프→이단 점프→약한 벽 점프 우선순위는 공통 Jump에서 처리.
- **기술·관련 클래스**: UWallJumpAbility, LocalPredicted, LaunchCharacter, HasWallJumped
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 벽 반발·조향·사용 이력에 따른 상승 제한을 결합.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Movement/WallJumpAbility.cpp](../Code/Movement/WallJumpAbility.cpp)
- **이력**: [`15098b31b`](https://github.com/2021Cyan/TeamProject2/commit/15098b31b3dc8cdaa6042a6bb1ebc24ad459857b) · [`f30f73fda`](https://github.com/2021Cyan/TeamProject2/commit/f30f73fda7d836b5374dd781b9ab2db747078976) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## A09. 이동기 우선순위·점프 예약·착지 초기화

- **실제 구현**: 공통 Jump에서 파쿠르, 대시 중 점프 예약, 구르기 점프, 슬라이드 점프, 벽 점프, 이단 점프를 상태별로 분기한다. 대시 종료 시 예약 점프를 실행하고, 유효한 바닥 착지 시 공중 이동 태그를 초기화한다.
- **담당 범위·기여 형태**: BaseCharacter 공용 코드에 직접 기여. 패링·강공격 본체는 타 팀원 구현이며 관련 입력 차단·태그 캐싱은 통합 작업으로 구분한다.
- **기술·관련 클래스**: ABaseCharacter::Jump/Landed, ReplicatedMovementInput, ServerSetMovementInput, GAS 태그
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 각 이동기를 별도 클래스에 두면서 전이 우선순위를 공통 진입점에서 관리.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`f769f460c`](https://github.com/2021Cyan/TeamProject2/commit/f769f460c783ae3045b907f28c2e43ce8fb0f3cf) · [`15098b31b`](https://github.com/2021Cyan/TeamProject2/commit/15098b31b3dc8cdaa6042a6bb1ebc24ad459857b) · [`f30f73fda`](https://github.com/2021Cyan/TeamProject2/commit/f30f73fda7d836b5374dd781b9ab2db747078976) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## A10. 경사 슬라이딩 물리와 조향

- **실제 구현**: 지면 노멀에 중력을 투영해 내리막 방향을 계산한다. 평지 최소 속도와 경사 하향 조건을 구분하고, 경사 가속·최고 속도 제한·속도별 동적 감속을 적용한다. 슬라이딩 중 기본 걷기 가속 대신 현재 속도를 유지하는 방향 보간으로 조향한다.
- **담당 범위·기여 형태**: CalcVelocity 기반 커스텀 물리 직접 구현. 마찰 변수의 존재만으로 모든 설정이 사용된다고 단정하지 않는다.
- **기술·관련 클래스**: UBaseMovementComponent::CalcVelocity/CanSlide, CurrentFloor, VInterpTo, DotProduct
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 경사면 투영과 감속 모델을 직접 구성해 평지·내리막 이동감을 분리.
- **전시 코드**: [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`846584c22`](https://github.com/2021Cyan/TeamProject2/commit/846584c22c3e2a00304410ffbe2bc9bdf89a2582) · [`eb6b91bdb`](https://github.com/2021Cyan/TeamProject2/commit/eb6b91bdb29c19fcc62806ac4c664a8613882ae2) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #8](https://github.com/2021Cyan/TeamProject2/issues/8).

## A11. 슬라이딩 입력·앉기·기상·점프 연계

- **실제 구현**: 입력 시작 시 조건을 검사해 슬라이드 GA를 실행하고 입력 해제·저속 상태에서 종료한다. bWantsToCrouch로 엔진의 앉기 처리를 연결하고 Start/Loop/Getup 몽타주와 슬라이드 중 점프 전이를 구성했다.
- **담당 범위·기여 형태**: 슬라이드 어빌리티·입력·공통 Jump 통합 직접 구현. 캐릭터별 실제 캡슐 크기 및 몽타주 연결값은 BP 확인 대상.
- **기술·관련 클래스**: USlideAbility, APlayerCharacter, UpdateCharacterStateBeforeMovement, GAS 태그
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 입력·물리·캡슐·애니메이션 종료 조건의 연계.
- **전시 코드**: [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp) · [Code/Movement/SlideAbility.cpp](../Code/Movement/SlideAbility.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`846584c22`](https://github.com/2021Cyan/TeamProject2/commit/846584c22c3e2a00304410ffbe2bc9bdf89a2582) · [`eb6b91bdb`](https://github.com/2021Cyan/TeamProject2/commit/eb6b91bdb29c19fcc62806ac4c664a8613882ae2) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [Issue #8](https://github.com/2021Cyan/TeamProject2/issues/8).

## A12. 슬라이드·벽 상태의 이동 예측 데이터 확장

- **실제 구현**: FSavedMove에 슬라이드와 벽 매달림 상태를 저장한다. 서로 다른 상태의 이동 패킷 병합을 막고, FLAG_Custom_0/1로 압축·복원하며 이동 재생 준비와 서버의 플래그 해독을 구현했다.
- **담당 범위·기여 형태**: 커스텀 CharacterMovement 네트워크 예측 데이터 직접 구현. 실제 지연·패킷 손실별 안정성 측정 자료는 없음.
- **기술·관련 클래스**: FSavedMove_BaseMovement, FNetworkPredictionData_Client_BaseMovement, GetCompressedFlags, PrepMoveFor
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 별도 매 프레임 상태 RPC 대신 CharacterMovement의 저장 이동·압축 플래그 체계에 연결.
- **전시 코드**: [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`846584c22`](https://github.com/2021Cyan/TeamProject2/commit/846584c22c3e2a00304410ffbe2bc9bdf89a2582) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #8](https://github.com/2021Cyan/TeamProject2/issues/8) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## A13. 파쿠르 장애물 판정·착지 위치·높이별 몽타주

- **실제 구현**: 전방 LineTrace로 벽을 찾고 하향 Capsule Sweep으로 윗면과 높이를 검사한다. 캡슐 반경·반높이와 오프셋으로 착지 좌표를 계산하며, 높이 임계 배열에 따라 몽타주 섹션을 선택한다. 판정 수치를 EditDefaultsOnly로 분리했다.
- **담당 범위·기여 형태**: 판정·GA·튜닝 구조 직접 구현. PR의 ‘전후 캡슐 트레이스’, ‘120 이하 생략’ 표현은 현재 기본 C++과 차이가 있다.
- **기술·관련 클래스**: UBaseMovementComponent::CheckVaulting, UVaultAbility, FVaultAnimThreshold
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 캐릭터 크기를 반영한 착지 좌표와 높이 기반 데이터 분기.
- **전시 코드**: [Code/Movement/VaultAbility.cpp](../Code/Movement/VaultAbility.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp) · [Code/Movement/BaseMovementComponent.h](../Code/Movement/BaseMovementComponent.h)
- **이력**: [`42808dd0d`](https://github.com/2021Cyan/TeamProject2/commit/42808dd0d120b7ba652ece250cd09d27b202dce4) · [`7f20ec590`](https://github.com/2021Cyan/TeamProject2/commit/7f20ec59068440f8d5b197b37df7c9d338ac9f67) · [`4dab57ad7`](https://github.com/2021Cyan/TeamProject2/commit/4dab57ad700bb515b0b4dea85568b4664822f9de) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #9](https://github.com/2021Cyan/TeamProject2/issues/9).

## A14. 커브 기반 벽넘기·자동 파쿠르·회전 분리

- **실제 구현**: CurveFloat와 타이머로 먼저 Z 상승, 이후 XY 전진하는 궤적을 보간한다. 선택된 몽타주 섹션 길이를 이동 시간에 반영하고, 이동 중 몸통 회전과 카메라를 분리한다. 이단·벽 점프 중 전방 입력과 장애물이 감지되면 기존 이동기를 취소하고 Vault로 전환한다.
- **담당 범위·기여 형태**: 직접 구현 및 후속 타이밍 개선. 취소·실패 경로까지 무결하다는 주장은 실행 확인 전 보류.
- **기술·관련 클래스**: UVaultAbility::VaultTick, CurveFloat, TimerManager, GameplayEvent TargetData
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 입력 편의성을 위한 자동 전이와 캐릭터·카메라 회전 분리. 서버 동기화 완전 보장은 주장하지 않음.
- **전시 코드**: [Code/Movement/DoubleJumpAbility.cpp](../Code/Movement/DoubleJumpAbility.cpp) · [Code/Movement/VaultAbility.cpp](../Code/Movement/VaultAbility.cpp) · [Code/Movement/WallJumpAbility.cpp](../Code/Movement/WallJumpAbility.cpp)
- **이력**: [`42808dd0d`](https://github.com/2021Cyan/TeamProject2/commit/42808dd0d120b7ba652ece250cd09d27b202dce4) · [`7f20ec590`](https://github.com/2021Cyan/TeamProject2/commit/7f20ec59068440f8d5b197b37df7c9d338ac9f67) · [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #9](https://github.com/2021Cyan/TeamProject2/issues/9).

## A15. 이동 상태와 GAS 분리·반복 조회 축소

- **실제 구현**: 이동 컴포넌트는 벽·슬라이드 물리 상태 변화를 델리게이트로 알리고, BaseCharacter가 GAS 태그와 어빌리티를 제어하도록 역할을 분리했다. CachedBaseMovement를 BeginPlay에 저장하고 반복 Cast를 대체했으며 자주 쓰는 태그를 static const로 변경했다.
- **담당 범위·기여 형태**: 공통 이동 구조 리팩토링 직접 수행. 전체 Tick 제거가 아니라 Tick 내부의 일부 반복 작업 축소.
- **기술·관련 클래스**: OnWallClingStateChanged, OnSlideStateChanged, UPROPERTY 캐시, static const FGameplayTag
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 물리 상태와 게임플레이 정책의 책임 분리. 시간·프레임 개선율은 측정 자료 없음.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp) · [Code/Movement/BaseMovementComponent.cpp](../Code/Movement/BaseMovementComponent.cpp)
- **이력**: [`08a7c2785`](https://github.com/2021Cyan/TeamProject2/commit/08a7c27856b77d63c8fd538bcd4ae071395be2bc) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #8](https://github.com/2021Cyan/TeamProject2/issues/8) · [Issue #9](https://github.com/2021Cyan/TeamProject2/issues/9) · [Issue #10](https://github.com/2021Cyan/TeamProject2/issues/10).

## B01. 크로스헤어·탄약·재장전 표시

- **실제 구현**: 크로스헤어와 장탄수 HUD를 구성하고 재장전 상태에 원형 게이지로 전환하도록 연결했다. 재장전 GA에 ActivationOwnedTags를 추가했으며, 몽타주 Notify의 GameplayTag 프로퍼티를 리플렉션으로 찾아 이벤트 시간을 반환하는 C++ 함수를 작성했다.
- **담당 범위·기여 형태**: HUD·UMG/머티리얼·C++ 연결 기여. 사격과 재장전 어빌리티 본체는 2021Cyan 작업.
- **기술·관련 클래스**: UMG, M_RadialGauge, ABaseCharacter::GetMontageEventTimeByTag, UGA_Reload
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 애니메이션 이벤트와 UI 시간 정보를 연결하는 C++→BP 유틸리티. 최종 게이지 계산 그래프는 캡처 필요.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Integration/GA_Reload.excerpt.cpp](../Code/Integration/GA_Reload.excerpt.cpp)
- **이력**: [`d6af142b8`](https://github.com/2021Cyan/TeamProject2/commit/d6af142b877cc16e23bc4276540a74e78562d759) · [`985ace845`](https://github.com/2021Cyan/TeamProject2/commit/985ace845d159f0baabf04c56036bbe40347e402) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B02. 플레이어 체력·레벨·소울 HUD

- **실제 구현**: 사선형 체력 표시와 레벨·소울 수 표시를 MainHUD의 하위 위젯으로 구성하고 기존 스탯 데이터에 연결했다. 관련 UI 텍스처와 머티리얼을 추가·정리했다.
- **담당 범위·기여 형태**: 화면 표시·데이터 연결 담당. 스탯 초기화·레벨 보상·소울 획득 규칙 자체의 원 구현은 다른 팀원.
- **기술·관련 클래스**: UMG, UACharacterAttributeSet 연동, WBP_HealthBar, WBP_SoulAndLevel
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 게임 상태를 역할별 하위 위젯으로 분리한 HUD. 사망·리스폰 HUD 전체의 본인 구현은 확인되지 않음.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`d6af142b8`](https://github.com/2021Cyan/TeamProject2/commit/d6af142b877cc16e23bc4276540a74e78562d759) · [`985ace845`](https://github.com/2021Cyan/TeamProject2/commit/985ace845d159f0baabf04c56036bbe40347e402) · [`94b576a11`](https://github.com/2021Cyan/TeamProject2/commit/94b576a110378d5bb9f535908a0575f1d9b40f51) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B03. 분할형 스태미나 UI·상태 색상

- **실제 구현**: 최대 스태미나에 따른 동적 분할과 충전·구르기 점프 가능 상태별 색상 전환을 위한 곡선형 머티리얼 및 위젯을 제작했다.
- **담당 범위·기여 형태**: 본인 PR 설명과 머티리얼·WBP 변경 확인. 노란색·하늘색 전환의 실제 파라미터 그래프 및 적용 상태는 캡처 확인 대상.
- **기술·관련 클래스**: UMG, M_StaminaBar, 머티리얼 파라미터, State.Movement.CanJump
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 이동 입력 허용 상태를 자원 UI에 시각적으로 연결.
- **전시 코드**: [Code/Movement/GroundRollAbility.excerpt.cpp](../Code/Movement/GroundRollAbility.excerpt.cpp)
- **이력**: [`249fc259f`](https://github.com/2021Cyan/TeamProject2/commit/249fc259fce810baaf7a9101fa360da732b721e4) · [`9cd43bbdd`](https://github.com/2021Cyan/TeamProject2/commit/9cd43bbdd7c2c2ed47367b4a0aef13a9ab4f3e07) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B04. 스태미나 재생·소모·기본 패시브 적용·초과 보정

- **실제 구현**: InitialPassiveEffects 배열을 순회해 시작 시 GE를 적용하는 공통 경로를 추가했다. 스태미나 재생·비용 GE를 연결하고 PostGameplayEffectExecute에서 Stamina의 BaseValue가 MaxStamina를 넘으면 직접 보정했다.
- **담당 범위·기여 형태**: 패시브 적용 경로와 스태미나 초과 버그 수정 직접 구현. 전체 AttributeSet·스탯 생성 시스템은 다른 팀원 구현.
- **기술·관련 클래스**: GameplayEffect, UACharacterAttributeSet, SetNumericAttributeBase, InitialPassiveEffects
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. GE의 실제 주기·비용값은 BP 디테일 확인 필요.
- **소개할 기술적 특징**: 표시값뿐 아니라 누적된 기본 자원값을 보정해 최대치를 넘는 문제에 대응.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Integration/ACharacterAttributeSet.excerpt.cpp](../Code/Integration/ACharacterAttributeSet.excerpt.cpp)
- **이력**: [`249fc259f`](https://github.com/2021Cyan/TeamProject2/commit/249fc259fce810baaf7a9101fa360da732b721e4) · [`9cd43bbdd`](https://github.com/2021Cyan/TeamProject2/commit/9cd43bbdd7c2c2ed47367b4a0aef13a9ab4f3e07) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #6](https://github.com/2021Cyan/TeamProject2/issues/6) · [Issue #7](https://github.com/2021Cyan/TeamProject2/issues/7) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B05. 스킬 슬롯·해금·강화 UI

- **실제 구현**: SkillIcon 필드를 추가하고 SkillContainer→UpgradeColumn→TierButton 구조로 스킬 UI를 구성했다. 포인트·해금 토큰 변화 델리게이트를 추가하여 버튼 상태, 해금·강화 표시를 갱신하도록 연결하고 초기 레벨 보상 호출을 보완했다.
- **담당 범위·기여 형태**: UI 구조·아이콘 데이터·상태 통지 직접 기여. 스킬 레벨업 비용·서버 승급 규칙·레벨 시스템 원 구현은 horimihorimi.
- **기술·관련 클래스**: FAbilitySlot, FOnAbilityPointsChanged, FOnAbilityUnlockTokensChanged, UMG
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 성장 규칙과 UI를 델리게이트로 연결하고 표시 계층을 모듈화.
- **전시 코드**: [Code/Integration/AbilitySlot.excerpt.h](../Code/Integration/AbilitySlot.excerpt.h) · [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`985ace845`](https://github.com/2021Cyan/TeamProject2/commit/985ace845d159f0baabf04c56036bbe40347e402) · [`695932c9c`](https://github.com/2021Cyan/TeamProject2/commit/695932c9c9407d67bd239dc7adfc7735ba35589b) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B06. 태그 기반 스킬 쿨타임 조회

- **실제 구현**: 지정한 쿨타임 태그를 가진 활성 GE를 질의하고 첫 효과의 전체·남은 시간을 반환하는 BlueprintCallable/Pure 함수를 추가했다. 원형 쿨타임 머티리얼과 스킬 슬롯에서 사용할 수 있게 했다.
- **담당 범위·기여 형태**: UDL_AbilitySystemComponent의 쿨타임 조회 함수 직접 구현. ASC 클래스 전체의 원 작성자는 다른 팀원.
- **기술·관련 클래스**: FGameplayEffectQuery::MakeQuery_MatchAllOwningTags, FActiveGameplayEffect, UDL_AbilitySystemComponent
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: GAS의 실제 활성 효과 시간을 UI가 읽도록 연결. 여러 동일 태그 효과 중 최대 잔여시간을 선택하는 구현은 아님.
- **전시 코드**: [Code/Integration/DL_AbilitySystemComponent.excerpt.cpp](../Code/Integration/DL_AbilitySystemComponent.excerpt.cpp) · [Code/Integration/DL_AbilitySystemComponent.excerpt.h](../Code/Integration/DL_AbilitySystemComponent.excerpt.h)
- **이력**: [`985ace845`](https://github.com/2021Cyan/TeamProject2/commit/985ace845d159f0baabf04c56036bbe40347e402) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B07. 강화 메뉴 입력·포커스·클릭 충돌 처리

- **실제 구현**: Tab 누름·해제로 강화 모드를 전환하고 커서 표시, GameAndUI/GameOnly 입력 모드, 위젯 포커스를 변경한다. 모드 변경 델리게이트를 추가했다. PR에는 UMG 클릭을 Handled로 소비해 UI 클릭 중 발사를 막은 작업도 기록되어 있다.
- **담당 범위·기여 형태**: C++ 입력·포커스 직접 구현, 클릭 소비 BP는 추가 확인. 상점 UI·로비 UI 전체 구현과 구분.
- **기술·관련 클래스**: FInputModeGameAndUI, FInputModeGameOnly, TakeWidget, Enhanced Input
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 게임 조작과 메뉴 상호작용의 입력 경계를 명시적으로 전환.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp)
- **이력**: [`695932c9c`](https://github.com/2021Cyan/TeamProject2/commit/695932c9c9407d67bd239dc7adfc7735ba35589b) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B08. 월드 체력바의 블록 분할·부분 체력 계산

- **실제 구현**: 최대 체력을 100 단위 블록 수로 변환하고 현재 체력을 인덱스·블록 내 비율로 나눠 BP 이벤트에 전달한다. 최대 체력 변경 시 재구성하고 일반 체력 변화 시 값 갱신으로 분기한다.
- **담당 범위·기여 형태**: UOverheadHealthBarWidget C++ 및 WBP 직접 구현. 40칸 사전 풀링은 PR 설명으로만 확인되며 C++만으로 확정할 수 없다.
- **기술·관련 클래스**: UOverheadHealthBarWidget, BlueprintImplementableEvent, CeilToInt/FloorToInt
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. 블록 위젯 재사용과 배치 방식은 BP 확인 필요.
- **소개할 기술적 특징**: C++에서 체력 구간을 계산하고 표현은 BP에 위임. 현재 코드는 주변 블록도 순회하므로 O(1) 단일 칸 갱신 주장은 제외.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/UI/OverheadHealthBarWidget.cpp](../Code/UI/OverheadHealthBarWidget.cpp)
- **이력**: [`fce22f4c1`](https://github.com/2021Cyan/TeamProject2/commit/fce22f4c14522e87e4dcd671c51e2b040c88f2e1) · [`c2d41eb09`](https://github.com/2021Cyan/TeamProject2/commit/c2d41eb0901b7431e51e90d03af123643322b03e) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B09. 체력바 피아 식별·가림·거리·빌보드

- **실제 구현**: 자기 자신과 전용 서버에서는 체력바를 숨긴다. 로컬 팀과 대상 팀 비교값을 위젯에 전달하고, 가시 상태에서 카메라를 향하게 회전한다. 0.2초 주기 트레이스, 거리 제곱 비교, WasRecentlyRendered 조건으로 표시 여부를 조정했으며 체력바 배치 위치도 후속 조정했다.
- **담당 범위·기여 형태**: 공통 BaseCharacter에 직접 구현. 팀 인덱스의 전체 체계는 팀 공통 기능이고, 빨강·회색 최종 색상은 WBP에 의존한다.
- **기술·관련 클래스**: UWidgetComponent, LineTraceSingleByChannel, ECC_WorldStatic, DistSquared, TimerManager
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 매 프레임 가림 검사 대신 주기 검사와 거리·렌더 여부 조건을 사용. 실제 프레임 개선율은 없음.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`fce22f4c1`](https://github.com/2021Cyan/TeamProject2/commit/fce22f4c14522e87e4dcd671c51e2b040c88f2e1) · [`c2d41eb09`](https://github.com/2021Cyan/TeamProject2/commit/c2d41eb0901b7431e51e90d03af123643322b03e) · [`f167e8037`](https://github.com/2021Cyan/TeamProject2/commit/f167e80378a27d3ea6853b5b0b6ca1443ad90f12) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B10. 복제 지연에 대응한 체력바 초기화

- **실제 구현**: ASC·HealthComponent·PlayerState·팀 인덱스·내부 위젯 생성이 준비되지 않으면 0.1초 후 초기화를 재시도한다. 완료 시 타이머를 해제하고 현재 체력으로 초기 표시하며 EndPlay에서 관련 타이머를 정리한다.
- **담당 범위·기여 형태**: 공통 캐릭터의 UI 초기화 보강 직접 구현. PR #65의 별도 스킬 HUD 로드 재시도는 BP 에셋 변경으로 구분.
- **기술·관련 클래스**: OnRep_PlayerState, PossessedBy, TimerManager, HealthBarRetryTimerHandle
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 클라이언트 복제 순서와 위젯 생성 시점이 다를 수 있는 초기화 문제에 대응.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`c2d41eb09`](https://github.com/2021Cyan/TeamProject2/commit/c2d41eb0901b7431e51e90d03af123643322b03e) · [`94b576a11`](https://github.com/2021Cyan/TeamProject2/commit/94b576a110378d5bb9f535908a0575f1d9b40f51) · [`7a808fc3e`](https://github.com/2021Cyan/TeamProject2/commit/7a808fc3e40a03cdbdbeebcbb9d7d51abf1b896a) · [`ed7c9861c`](https://github.com/2021Cyan/TeamProject2/commit/ed7c9861c36fd057bec7ca166306d068be74cc97) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [PR #65](https://github.com/2021Cyan/TeamProject2/pull/65) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## B11. 인벤토리 HUD 슬롯 재사용·정렬

- **실제 구현**: NativeConstruct에서 슬롯 12개를 만들고 2행 6열의 열 우선 순서로 배치한다. 아이템 정의를 미리 조회해 FItemSortData에 보관하고 Tier→Category→Price→ItemId로 정렬한 뒤 기존 슬롯을 갱신하며 남은 슬롯은 비운다.
- **담당 범위·기여 형태**: 인벤토리 표시 C++·UMG 신규 구현. 소유·구매·판매·효과 적용을 담당하는 DLInventoryComponent/ItemSubsystem 본체는 horimihorimi.
- **기술·관련 클래스**: UInventoryHUDWidget, UUniformGridPanel, TArray::Sort, FItemSortData
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 반복 갱신 시 위젯 생성 대신 재사용하고 정렬 비교자 내부의 반복 데이터 조회를 피함. 전체 위젯 수명에서 단 한 번 생성된다는 의미는 아님.
- **전시 코드**: [Code/UI/InventoryHUDWidget.cpp](../Code/UI/InventoryHUDWidget.cpp)
- **이력**: [`064bb59af`](https://github.com/2021Cyan/TeamProject2/commit/064bb59af764f11c2d889536b3e097ae3d42b1ed) · [`7a808fc3e`](https://github.com/2021Cyan/TeamProject2/commit/7a808fc3e40a03cdbdbeebcbb9d7d51abf1b896a) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20) · [Issue #21](https://github.com/2021Cyan/TeamProject2/issues/21) · [Issue #22](https://github.com/2021Cyan/TeamProject2/issues/22).

## B12. 인벤토리 이벤트·빙의 변경·복제 대기 연동

- **실제 구현**: OnPossessedPawnChanged를 구독하고 폰 교체 시 기존 이벤트를 해제한다. PlayerController/PlayerState에서 인벤토리를 찾아 바인딩하며 복제 전에는 0.2초 후 재시도한다. 인벤토리 변경·구매·판매 이벤트에서 HUD를 갱신한다.
- **담당 범위·기여 형태**: 표시 계층의 연결·재연결 직접 구현. PlayerState로 인벤토리를 옮긴 기반 구조는 공동 코드와 구분하며, #57의 초기 연동 미완료 기록과 후속 수정 커밋을 함께 남긴다.
- **기술·관련 클래스**: OnPossessedPawnChanged, AddDynamic/RemoveDynamic, UDLInventoryComponent, PlayerState
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 리스폰·빙의·복제 지연에 따른 표시 연결 끊김을 복구하는 구조. 재시도 타이머 종료 경로는 추가 실행 검증 권장.
- **전시 코드**: [Code/UI/InventoryHUDWidget.cpp](../Code/UI/InventoryHUDWidget.cpp)
- **이력**: [`064bb59af`](https://github.com/2021Cyan/TeamProject2/commit/064bb59af764f11c2d889536b3e097ae3d42b1ed) · [`7a808fc3e`](https://github.com/2021Cyan/TeamProject2/commit/7a808fc3e40a03cdbdbeebcbb9d7d51abf1b896a) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20) · [Issue #22](https://github.com/2021Cyan/TeamProject2/issues/22).

## B13. 아이템 아이콘·트리거/액티브 쿨타임 표시

- **실제 구현**: 아이템 아이콘을 로드하고 RuntimeEffects에서 트리거형 내부 쿨타임 종료 시각을 읽는다. 액티브 아이템은 GrantedAbilityHandle로 GA 인스턴스를 찾아 SkillBase 또는 일반 GAS 경로에서 남은 시간·전체 시간을 조회해 BP에 전달한다.
- **담당 범위·기여 형태**: UInventorySlotWidget 직접 구현. 아이템 효과 자체의 판정·거래 규칙·바인딩 생성은 다른 팀원.
- **기술·관련 클래스**: FItemEffectRuntimeState, FGameplayAbilitySpecHandle, GetCooldownRemaining, BlueprintImplementableEvent
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. 원형 애니메이션과 실제 사용 직후 갱신은 BP 확인 필요.
- **소개할 기술적 특징**: 효과 유형별 서로 다른 쿨타임 정보를 공통 UI 이벤트로 전달.
- **전시 코드**: [Code/UI/InventorySlotWidget.cpp](../Code/UI/InventorySlotWidget.cpp) · [Code/UI/InventorySlotWidget.h](../Code/UI/InventorySlotWidget.h)
- **이력**: [`064bb59af`](https://github.com/2021Cyan/TeamProject2/commit/064bb59af764f11c2d889536b3e097ae3d42b1ed) · [`7a808fc3e`](https://github.com/2021Cyan/TeamProject2/commit/7a808fc3e40a03cdbdbeebcbb9d7d51abf1b896a) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20) · [Issue #22](https://github.com/2021Cyan/TeamProject2/issues/22).

## C01. Lash 캐릭터 구성·공통 전투 기능 연결

- **실제 구현**: Lash BP에 ABP·블렌드스페이스·이동 몽타주·GA를 연결하고 근접 공격·패링 등 기존 공통 기능을 적용했다. 캐릭터 크기와 파쿠르 설정을 후속 조정했다.
- **담당 범위·기여 형태**: 캐릭터 조립·에셋 설정·통합 담당. 총기·근접·패링·에임오프셋 시스템 전체 및 외부 애니메이션 원 제작으로 주장하지 않는다.
- **기술·관련 클래스**: BP_Lash, ABP_Lash, BlendSpace, AnimMontage, InitialAbilities
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 공통 이동·전투 기반 위에 캐릭터별 애니메이션과 전용 스킬을 조합.
- **전시 코드**: [Code/SharedCharacter/PlayerCharacter.excerpt.cpp](../Code/SharedCharacter/PlayerCharacter.excerpt.cpp)
- **이력**: [`6067991e6`](https://github.com/2021Cyan/TeamProject2/commit/6067991e627efd9ab7ea84a55fb277a996fc27d5) · [`cf35cc62e`](https://github.com/2021Cyan/TeamProject2/commit/cf35cc62ed17e3b4e2e49234491da38284a906a7) · [`892a2c906`](https://github.com/2021Cyan/TeamProject2/commit/892a2c906b44e8df6bf72b64716479daf86b3034) · [`3ed2d3581`](https://github.com/2021Cyan/TeamProject2/commit/3ed2d35818c33d81398686700ff0cee68ce71864) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C02. Ground Strike 지상/공중 강하·부채꼴 타격

- **실제 구현**: 지상과 공중 시전을 분기한다. 공중에서는 캐릭터 수평 전방과 하강 벡터로 목표를 트레이스하고 RootMotionMoveToForce로 강하한다. 이동 모드 전환·타임아웃에서 Impact 섹션으로 넘어가며 구체 수집 후 내적 필터로 전방 적에게 피해·추가 GE를 적용한다.
- **담당 범위·기여 형태**: 전용 GA·피해 범위·이펙트 이벤트 직접 구현. 공통 SkillBase·피해 공식은 다른 팀원 기반을 사용.
- **기술·관련 클래스**: UGA_Lash_GroundStrike, MoveToForce, MovementModeChangedDelegate, Sphere Sweep, DotProduct, GameplayCue
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 물리 착지와 애니메이션 타격 시점을 연결하고 수집·방향 필터를 분리. PR의 ‘카메라 전방’과 달리 현재 강하 방향은 캐릭터 전방 기준.
- **전시 코드**: [Code/Lash/GA_Lash_GroundStrike.cpp](../Code/Lash/GA_Lash_GroundStrike.cpp)
- **이력**: [`37858f285`](https://github.com/2021Cyan/TeamProject2/commit/37858f285646187c5a1a23bb29fc7794f7e4bebc) · [`94b576a11`](https://github.com/2021Cyan/TeamProject2/commit/94b576a110378d5bb9f535908a0575f1d9b40f51) · [`718f4d576`](https://github.com/2021Cyan/TeamProject2/commit/718f4d5761df415a677c547aeb21284353811ba6) · [`d6c1cfe90`](https://github.com/2021Cyan/TeamProject2/commit/d6c1cfe90f20d117a02c126684c1d8955c269983) · [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C03. Grapple 도약·공중 이동기 재사용·로프 연결

- **실제 구현**: 기존 Scope가 전달한 대상을 받아 대상 방향과 상향 벡터를 합성한 힘으로 도약한다. 공중 대시·이단 점프·점프 예약 태그를 초기화하고, 종료 시 마지막 Root Motion 속도를 유지한다. EffectContext의 HitResult에 대상을 담아 로프 Cue에 전달한다.
- **담당 범위·기여 형태**: Lash 전용 도약·이동기 연계·Cue 데이터 직접 구현. 범용 Scope 타기팅 자체는 horimihorimi 작업.
- **기술·관련 클래스**: UGA_Lash_Grapple, ConstantForce, MaintainLastRootMotionVelocity, GameplayEffectContext
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 공중 이동 자원 초기화와 이동 관성을 조합하는 연계 스킬. 후딜 캔슬의 최종 BP 태그 설정은 추가 확인.
- **전시 코드**: [Code/Lash/GA_Lash_Grapple.cpp](../Code/Lash/GA_Lash_Grapple.cpp)
- **이력**: [`718f4d576`](https://github.com/2021Cyan/TeamProject2/commit/718f4d5761df415a677c547aeb21284353811ba6) · [`11fb03813`](https://github.com/2021Cyan/TeamProject2/commit/11fb038132c009b5697c366e9ea08c87524f35a9) · [`b96bfea6d`](https://github.com/2021Cyan/TeamProject2/commit/b96bfea6dc75988d73696cd8f4b26c715f26e6c0) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C04. Flog 다중 대상 공격·흡혈·다중 로프 Cue

- **실제 구현**: Scope로 수집한 적들을 공격하고 추가 GE를 적용한다. SpiritPower×계수+기본 피해로 이론 피해량을 계산해 대상별 회복량을 합산한 후 Data.Heal SetByCaller로 자신에게 한 번 적용한다. 약참조 대상 배열을 EffectContext에 담아 다중 로프 Cue를 실행한다.
- **담당 범위·기여 형태**: 전용 GA·회복 합산·Cue 데이터 직접 구현. 공통 피해 계산/팀 판정 함수의 원 작성자는 다른 팀원.
- **기술·관련 클래스**: UGA_Lash_Flog, TWeakObjectPtr, AddActors, SetSetByCallerMagnitude, GameplayCue
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 여러 대상의 연출 데이터와 회복 적용을 집계. 실제 방어 적용 후 피해에 비례한 흡혈이라고 표현하면 코드와 다름.
- **전시 코드**: [Code/Integration/SkillDamageLibrary.excerpt.cpp](../Code/Integration/SkillDamageLibrary.excerpt.cpp) · [Code/Lash/GA_Lash_Flog.cpp](../Code/Lash/GA_Lash_Flog.cpp)
- **이력**: [`718f4d576`](https://github.com/2021Cyan/TeamProject2/commit/718f4d5761df415a677c547aeb21284353811ba6) · [`11fb03813`](https://github.com/2021Cyan/TeamProject2/commit/11fb038132c009b5697c366e9ea08c87524f35a9) · [`b96bfea6d`](https://github.com/2021Cyan/TeamProject2/commit/b96bfea6dc75988d73696cd8f4b26c715f26e6c0) · [`bc67b4167`](https://github.com/2021Cyan/TeamProject2/commit/bc67b4167a89465259187599adbef16fd79799a7) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C05. Death Slam 시선 락온·확정·표시 UI

- **실제 구현**: 지상/공중 스캔 몽타주를 선택하고 0.05초마다 Scope 대상의 응시 시간을 누적한다. 플레이어 대상만 추적하며 머리 소켓에 락온 위젯을 붙이고 진행률을 전달한다. 확정 입력·스캔 제한 시간·대상 없음 취소 경로를 구현했다.
- **담당 범위·기여 형태**: 궁극기 스캔 상태·락온 위젯 C++ 인터페이스 직접 구현. Scope 대상 수집의 기본 프레임워크는 팀원 기능.
- **기술·관련 클래스**: UGA_Lash_DeathSlam, FLashLockonTarget, ULashLockonWidget, UWidgetComponent, WaitInputPress
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. 확정 직후 배열 정리와 로컬 재검증 순서는 추가 재현 필요.
- **소개할 기술적 특징**: 단순 일회성 타기팅에서 응시 누적·확정·취소의 단계형 상호작용으로 확장.
- **전시 코드**: [Code/Lash/GA_Lash_DeathSlam.cpp](../Code/Lash/GA_Lash_DeathSlam.cpp) · [Code/Lash/LashLockonWidget.h](../Code/Lash/LashLockonWidget.h)
- **이력**: [`cfc50ba0a`](https://github.com/2021Cyan/TeamProject2/commit/cfc50ba0a2bdb950f6e315face8bdf088c1c6951) · [`c52346ea0`](https://github.com/2021Cyan/TeamProject2/commit/c52346ea0844481614b56b8d7e50dfb0a53f3bc4) · [`cea838eeb`](https://github.com/2021Cyan/TeamProject2/commit/cea838eeb69bc93eb99bd339eea256aa299dabe0) · [`aacbc68d6`](https://github.com/2021Cyan/TeamProject2/commit/aacbc68d6e4084a1eb6ffecf4b0a94850e388520) · [PR #64](https://github.com/2021Cyan/TeamProject2/pull/64) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C06. Death Slam 띄우기·투하·RPC·상태 복원

- **실제 구현**: 확정 대상을 끌어올리고 Pawn 충돌을 무시하며 스턴 GE·Flying 상태를 적용한다. 로프 끝점과 착지 인디케이터를 갱신하고 로컬 카메라가 계산한 착지 좌표를 Server RPC로 전달한다. 투하 후 착지 또는 시간 제한을 감시해 범위 피해·Cue를 실행하고 타이머·위젯·이펙트·GE·충돌·이동 모드를 정리한다.
- **담당 범위·기여 형태**: 궁극기 후반 상태와 네트워크 수정 직접 구현. 원래 중력값의 완전 복원이나 클라이언트 좌표 검증 완료까지 주장하지 않는다.
- **기술·관련 클래스**: UGA_Lash_DeathSlam, Server_SendSlamTargetLoc, TimerManager, LaunchCharacter, Niagara, FActiveGameplayEffectHandle
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 대상 확정부터 투하까지의 단계 제어와 카메라 의존 좌표의 서버 전달.
- **전시 코드**: [Code/Lash/GA_Lash_DeathSlam.cpp](../Code/Lash/GA_Lash_DeathSlam.cpp)
- **이력**: [`cfc50ba0a`](https://github.com/2021Cyan/TeamProject2/commit/cfc50ba0a2bdb950f6e315face8bdf088c1c6951) · [`c52346ea0`](https://github.com/2021Cyan/TeamProject2/commit/c52346ea0844481614b56b8d7e50dfb0a53f3bc4) · [`cea838eeb`](https://github.com/2021Cyan/TeamProject2/commit/cea838eeb69bc93eb99bd339eea256aa299dabe0) · [`d5cf4cc10`](https://github.com/2021Cyan/TeamProject2/commit/d5cf4cc1041ed686dacf76df7a42884442a80159) · [`aacbc68d6`](https://github.com/2021Cyan/TeamProject2/commit/aacbc68d6e4084a1eb6ffecf4b0a94850e388520) · [`3aa15496b`](https://github.com/2021Cyan/TeamProject2/commit/3aa15496b56aef4de4b2de168ef43e6f5d5a95d8) · [PR #64](https://github.com/2021Cyan/TeamProject2/pull/64) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## C07. 다중 대상 Cue 브리지·Niagara 연결·폐기 GCN

- **실제 구현**: EffectContext의 약참조 액터 배열을 유효한 고유 액터 배열로 꺼내는 Blueprint 유틸리티를 추가했다. GroundStrike 방향·위치 Cue, Grapple/Flog 로프 Cue, 궁극기 빔·폭발 VFX를 연결했다. Flog 로프 C++ GCN을 구현했다가 이후 리팩토링에서 삭제했다.
- **담당 범위·기여 형태**: Cue 연동·유틸리티·삭제된 GCN 직접 구현. 가져온 VFX/텍스처의 원 제작이나 범용 액터 풀 시스템 구현으로 주장하지 않는다.
- **기술·관련 클래스**: USkillDamageLibrary::GetActorsFromEffectContext, FGameplayCueParameters, UNiagaraComponent, UGCN_Lash_FlogRope(삭제)
- **구현·현재 상태**: 브리지와 Cue 연동은 현재 코드 확인. GCN_Lash_FlogRope.cpp/.h는 과거 구현 후 삭제 확인.
- **소개할 기술적 특징**: GAS Context에 담긴 다중 대상 정보를 BP 표현 계층으로 연결. 삭제본은 커밋으로 복원해 설명 가능.
- **전시 코드**: [Code/Integration/SkillDamageLibrary.excerpt.cpp](../Code/Integration/SkillDamageLibrary.excerpt.cpp) · [Code/Integration/SkillDamageLibrary.excerpt.h](../Code/Integration/SkillDamageLibrary.excerpt.h)
- **이력**: [`11fb03813`](https://github.com/2021Cyan/TeamProject2/commit/11fb038132c009b5697c366e9ea08c87524f35a9) · [`b96bfea6d`](https://github.com/2021Cyan/TeamProject2/commit/b96bfea6dc75988d73696cd8f4b26c715f26e6c0) · [`bc67b4167`](https://github.com/2021Cyan/TeamProject2/commit/bc67b4167a89465259187599adbef16fd79799a7) · [`d6c1cfe90`](https://github.com/2021Cyan/TeamProject2/commit/d6c1cfe90f20d117a02c126684c1d8955c269983) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## D01. 넥서스 기반 클래스·피격·접근 UI

- **실제 구현**: AAICharacter를 상속한 ANexus에 Phase1 피격 메시와 UI 감지 구체를 추가하고 일반 이동을 막았다. 에디터 팀 인덱스를 공통 팀 체계에 등록하고 메시의 팀별 충돌 채널을 설정했다. 접근·이탈 시 BP UI 이벤트를 호출한다.
- **담당 범위·기여 형태**: ANexus 신규 구현. 상속받은 AI·Health·팀 시스템 자체는 팀원 기반.
- **기술·관련 클래스**: ANexus, USkeletalMeshComponent, USphereComponent, ApplyTeamCollision, BlueprintImplementableEvent
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. 접근 UI의 표시·애니메이션은 BP 확인 필요.
- **소개할 기술적 특징**: 피격 표현·팀 구분·접근 UI를 하나의 게임 목표 액터에 통합.
- **전시 코드**: [Code/Nexus/Nexus.cpp](../Code/Nexus/Nexus.cpp)
- **이력**: [`825c0d252`](https://github.com/2021Cyan/TeamProject2/commit/825c0d252a9ec7f81b03d81531dfcc322ceef375) · [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [`ffe5cf38c`](https://github.com/2021Cyan/TeamProject2/commit/ffe5cf38c2bf4b1a7283de028ef13ed980c46d36) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## D02. 체력 기준 2단계 넥서스 전환

- **실제 구현**: 초기 Phase1에서는 AI Brain 로직을 정지한다. 체력이 최대치의 50% 이하로 내려가면 체력을 경계값으로 보정하고 Phase2로 한 번 전환하며 AI Brain을 재시작하고 BP 이벤트를 호출한다.
- **담당 범위·기여 형태**: 페이즈·체력 경계·AI 활성 전환 직접 구현. CurrentPhase는 현재 헤더에서 Replicated 속성이 아니므로 상태 복제 완료로 확대하지 않는다.
- **기술·관련 클래스**: ENexusPhase, HealthComponent delegate, SetNumericAttributeBase, BrainComponent
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 체력 경계와 페이즈 중복 전환 방지, AI·애니메이션 표현 훅을 연결.
- **전시 코드**: [Code/Nexus/Nexus.cpp](../Code/Nexus/Nexus.cpp)
- **이력**: [`825c0d252`](https://github.com/2021Cyan/TeamProject2/commit/825c0d252a9ec7f81b03d81531dfcc322ceef375) · [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## D03. 넥서스 광역 공격·충전/폭발 동기화

- **실제 구현**: GA_Nexus_AoE에서 비용 검증 후 충전 Niagara와 공격 몽타주·폭발 이벤트를 연결한다. 구체 범위로 적을 찾고 캡슐 반경을 고려해 피해·추가 GE를 적용한다. 충전 시작·중단·폭발 연출은 Multicast 함수로 전달한다.
- **담당 범위·기여 형태**: AoE GA와 넥서스 연출 RPC 직접 구현. PR의 5초 반복 주기는 C++ 내부 반복 타이머가 아니라 BT/BP 설정 확인이 필요.
- **기술·관련 클래스**: UGA_Nexus_AoE, WaitGameplayEvent, PlayMontageAndWait, SphereOverlapActors, NetMulticast
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음. 반복 간격과 실제 BT·몽타주 이벤트 연결은 추가 확인.
- **소개할 기술적 특징**: 공격 판정과 네트워크 연출을 단계별로 분리하고 전용 서버의 렌더 연출을 건너뜀.
- **전시 코드**: [Code/Nexus/GA_Nexus_AoE.cpp](../Code/Nexus/GA_Nexus_AoE.cpp) · [Code/Nexus/Nexus.cpp](../Code/Nexus/Nexus.cpp)
- **이력**: [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [`ffe5cf38c`](https://github.com/2021Cyan/TeamProject2/commit/ffe5cf38c2bf4b1a7283de028ef13ed980c46d36) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## D04. 넥서스 무적·아군 피해 방지·피해 경로 보강

- **실제 구현**: 초기 bIsInvincible을 복제하고 일반 TakeDamage에서 무적·아군 공격을 차단한다. 공통 ApplyDamage에 넥서스 예외를 추가했으며 이를 거치지 않는 장판 등의 ApplyDamageSpec 경로에도 무적 검사를 추가했다.
- **담당 범위·기여 형태**: 넥서스 조건과 공통 피해 유틸리티의 해당 분기 직접 구현. 전체 피해 계산 함수·방어력 공식은 dalbi090211 기반.
- **기술·관련 클래스**: ANexus, DOREPLIFETIME, TakeDamage, USkillDamageLibrary::ApplyDamage/ApplyDamageSpec
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 같은 게임 규칙을 여러 피해 진입 경로에 적용해 우회 경로를 보완. 프로젝트의 모든 피해 루트가 차단된다는 완전 보장은 보류.
- **전시 코드**: [Code/Integration/SkillDamageLibrary.excerpt.cpp](../Code/Integration/SkillDamageLibrary.excerpt.cpp) · [Code/Nexus/Nexus.cpp](../Code/Nexus/Nexus.cpp)
- **이력**: [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [`85de1ba55`](https://github.com/2021Cyan/TeamProject2/commit/85de1ba55bfb207fa3a2c506d60fd6208b5d08ec) · [`ffe5cf38c`](https://github.com/2021Cyan/TeamProject2/commit/ffe5cf38c2bf4b1a7283de028ef13ed980c46d36) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## D05. 최종 포탑 파괴와 넥서스 무적 해제

- **실제 구현**: 기존 ATurret에 최종 포탑 여부와 사망 핸들러를 추가했다. 서버에서 같은 팀의 다른 최종 포탑 생존 여부를 검사하고 모두 파괴되면 같은 팀 넥서스의 무적을 해제한다. 부모 즉시 삭제 콜백을 끊고 신호 처리 후 수명을 설정한다.
- **담당 범위·기여 형태**: 포탑의 승패 연계 부분 확장 기여. 포탑 AI·기본 전투·스폰 시스템의 원 구현은 dalbi090211.
- **기술·관련 클래스**: ATurret::OnTurretDeath, GetAllActorsOfClass, HealthComponent delegate, SetLifeSpan
- **구현·현재 상태**: C++ 구현과 본인 변경 이력 확인. 현재 실행·빌드 재검증은 하지 않음.
- **소개할 기술적 특징**: 사망 이벤트의 삭제 순서를 조정해 건물 파괴와 최종 목표 해금을 연결.
- **전시 코드**: [Code/Integration/Turret.excerpt.cpp](../Code/Integration/Turret.excerpt.cpp) · [Code/Integration/Turret.excerpt.h](../Code/Integration/Turret.excerpt.h)
- **이력**: [`ffe5cf38c`](https://github.com/2021Cyan/TeamProject2/commit/ffe5cf38c2bf4b1a7283de028ef13ed980c46d36) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #24](https://github.com/2021Cyan/TeamProject2/issues/24) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## D06. 게임 종료·승패 UI·로비 복귀 연결

- **실제 구현**: 넥서스 전용 사망 이벤트로 부모의 즉시 제거를 대체하고 Phase2 사망 시 Reliable Multicast로 BP 종료 이벤트를 전달한다. PR와 BP 변경에는 파괴 연출, 팀 비교 승패 화면, 5초 후 로비 복귀 흐름이 기록되어 있다.
- **담당 범위·기여 형태**: C++ 종료 이벤트와 결과 WBP·넥서스 BP 연결 담당. 실제 팀별 결과 판단·OpenLevel 노드·복귀 시간은 BP 내부 검증 전까지 문서/에셋 근거 수준으로 구분.
- **기술·관련 클래스**: ANexus::OnNexusDeath/MulticastNexusDestroyed, BlueprintImplementableEvent, UMG
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 목표 파괴 이벤트를 모든 클라이언트의 결과 표시와 후속 맵 이동으로 연결.
- **전시 코드**: [Code/Nexus/Nexus.cpp](../Code/Nexus/Nexus.cpp)
- **이력**: [`4685652f7`](https://github.com/2021Cyan/TeamProject2/commit/4685652f7174ef4ab49a88a3f0fa8ea8666988f3) · [`85de1ba55`](https://github.com/2021Cyan/TeamProject2/commit/85de1ba55bfb207fa3a2c506d60fd6208b5d08ec) · [`ffe5cf38c`](https://github.com/2021Cyan/TeamProject2/commit/ffe5cf38c2bf4b1a7283de028ef13ed980c46d36) · [PR #70](https://github.com/2021Cyan/TeamProject2/pull/70) · [Issue #29](https://github.com/2021Cyan/TeamProject2/issues/29).

## E01. 무타깃 스킬의 서버 시전 대기 문제 수정

- **실제 구현**: 당시 SkillBase의 TargetingMode::None에서 서버가 TargetData를 기다리는 문제에 대응해 즉시 시전 우회 로직을 추가했다가 빈 TargetData를 보내는 방식으로 개선했다.
- **담당 범위·기여 형태**: 팀원 SkillBase 내부의 특정 네트워크 분기 수정 기여. SkillBase 전체·Preview·Scope 시스템의 설계/구현을 본인 작업으로 포함하지 않는다.
- **기술·관련 클래스**: UGA_SkillBase::StartTargeting, SendTargetDataToServer, ESkillTargetingMode::None
- **구현·현재 상태**: 과거 구현 확인. 최신 develop에서는 horimihorimi의 #72 리팩토링으로 None 모드를 서버 대기에서 제외하는 구조로 대체됨.
- **소개할 기술적 특징**: 클라이언트·서버 타기팅 생명주기의 불일치를 발견하고 수정한 이력. 최종 구조의 원 작성자와 분리.
- **전시 코드**: [Code/Historical/Integration/GA_SkillBase.excerpt.cpp](../Code/Historical/Integration/GA_SkillBase.excerpt.cpp)
- **이력**: [`985ace845`](https://github.com/2021Cyan/TeamProject2/commit/985ace845d159f0baabf04c56036bbe40347e402) · [`249fc259f`](https://github.com/2021Cyan/TeamProject2/commit/249fc259fce810baaf7a9101fa360da732b721e4) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [Issue #12](https://github.com/2021Cyan/TeamProject2/issues/12) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## E02. HUD 생성 위치·리스폰 시 표시 데이터 복구

- **실제 구현**: 캐릭터별 중복 HUD 생성을 공통 경로로 옮기고 OnLocallyPossessed 이벤트를 추가했다. 리스폰 시 기존 ASC에서 스킬 SpecHandle을 복구하고 체력 기본값을 최대값으로 채우는 보완 코드를 추가했다.
- **담당 범위·기여 형태**: UI 수명·리스폰 연결 부분의 수정 기여. 사망/리스폰 규칙·PlayerState ASC 소유 구조·GameMode 재스폰 본체는 dalbi090211.
- **기술·관련 클래스**: ABaseCharacter::InitAbilitySystem, ABasePlayerController::OnLocallyPossessed, FindAbilitySpecFromClass
- **구현·현재 상태**: 본인 PR·에셋 변경 및 C++ 연동 확인. 블루프린트 그래프의 최종 연결과 실행 결과는 추가 확인.
- **소개할 기술적 특징**: 죽음·빙의 변경으로 Pawn이 교체되어도 UI와 기존 ASC의 스킬 정보를 다시 연결.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp) · [Code/Integration/BasePlayerController.excerpt.h](../Code/Integration/BasePlayerController.excerpt.h)
- **이력**: [`9cd43bbdd`](https://github.com/2021Cyan/TeamProject2/commit/9cd43bbdd7c2c2ed47367b4a0aef13a9ab4f3e07) · [`94b576a11`](https://github.com/2021Cyan/TeamProject2/commit/94b576a110378d5bb9f535908a0575f1d9b40f51) · [`7a808fc3e`](https://github.com/2021Cyan/TeamProject2/commit/7a808fc3e40a03cdbdbeebcbb9d7d51abf1b896a) · [`ed7c9861c`](https://github.com/2021Cyan/TeamProject2/commit/ed7c9861c36fd057bec7ca166306d068be74cc97) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [PR #65](https://github.com/2021Cyan/TeamProject2/pull/65) · [Issue #18](https://github.com/2021Cyan/TeamProject2/issues/18) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20).

## E03. 태그·입력·빌드 의존성·머지 통합

- **실제 구현**: 이동·스킬·Cue 태그를 등록하고 UI와 GAS 모듈 의존성을 조정했다. 공통 Base/PlayerCharacter와 설정 파일의 충돌을 해결하고 중복·정리 작업을 수행했다. UI 머티리얼 경로를 정리하고 입력/슬롯 데이터를 통합했다.
- **담당 범위·기여 형태**: 본인 변경 및 병합 충돌 해결 확인. 병합으로 함께 들어온 다른 팀원 기능은 본인 구현으로 계산하지 않음.
- **기술·관련 클래스**: DefaultGameplayTags.ini, ModuleRules, UMG/Slate/SlateCore, Git combined diff
- **구현·현재 상태**: 변경과 병합 이력 확인. 공동 Issue #33의 담당자 등록만으로 전체 통합 테스트 완료를 확정하지 않음.
- **소개할 기술적 특징**: 분리 개발된 이동·전투·성장·UI 기능을 공통 캐릭터와 설정에 통합한 기여.
- **전시 코드**: [Code/Integration/AbilitySlot.excerpt.h](../Code/Integration/AbilitySlot.excerpt.h)
- **이력**: [`5ac8fafa4`](https://github.com/2021Cyan/TeamProject2/commit/5ac8fafa4eaebbe217628e060c3baf3a98faf412) · [`695932c9c`](https://github.com/2021Cyan/TeamProject2/commit/695932c9c9407d67bd239dc7adfc7735ba35589b) · [`fce22f4c1`](https://github.com/2021Cyan/TeamProject2/commit/fce22f4c14522e87e4dcd671c51e2b040c88f2e1) · [`5eeca4f8f`](https://github.com/2021Cyan/TeamProject2/commit/5eeca4f8f4e122856bfc4e130ab2b1a490f5b7c6) · [`9ba63274e`](https://github.com/2021Cyan/TeamProject2/commit/9ba63274eca7970c6c063033c6ae14147a6da17a) · [`8dae28f31`](https://github.com/2021Cyan/TeamProject2/commit/8dae28f31e5cca447e95127aa32f6e85573353bc) · [`57e699f90`](https://github.com/2021Cyan/TeamProject2/commit/57e699f90a2a5e13f634d5e3e51412a3dd778b0c) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [Issue #20](https://github.com/2021Cyan/TeamProject2/issues/20) · [Issue #33](https://github.com/2021Cyan/TeamProject2/issues/33).

## E04. Doorman 태그 갱신 임시 비활성화·머지 대응

- **실제 구현**: GA_CallBell, GA_LugguageCart, GA_Doorway, GA_HotelGuest의 RefreshAbilityTags에서 SetAssetTags 호출을 임시 주석 처리한 변경을 확인했다. #57에는 해당 임시 조치가 명시되어 있다.
- **담당 범위·기여 형태**: 다른 팀원 캐릭터 파일에 가한 통합 수정. Doorman 스킬 구현으로 포함하지 않으며, 최종 해결이나 개선 효과는 이 변경만으로 단정하지 않는다.
- **기술·관련 클래스**: GameplayAbility AssetTags, RefreshAbilityTags, 병합 충돌 대응
- **구현·현재 상태**: 과거 임시 변경 확인. 현재 경로는 Doorman 하위 폴더로 재구성되었고 태그 구조는 2021Cyan의 후속 작업으로 변경됨.
- **소개할 기술적 특징**: 공동 파일 안의 실제 기여 이력을 누락하지 않기 위한 기록. 대표 기능 요약에 단독 기술로 넣지 않는 편이 정확함.
- **전시 코드**: BP 설정·에셋 이력 또는 정리 작업. 별도 C++ 본체로 대체해 만들지 않음.
- **이력**: [`9ba63274e`](https://github.com/2021Cyan/TeamProject2/commit/9ba63274eca7970c6c063033c6ae14147a6da17a) · [`7ba4c4149`](https://github.com/2021Cyan/TeamProject2/commit/7ba4c4149bbb5bbfafbe6e76e4dbfdafb58c93a8) · [`d6c1cfe90`](https://github.com/2021Cyan/TeamProject2/commit/d6c1cfe90f20d117a02c126684c1d8955c269983) · [PR #57](https://github.com/2021Cyan/TeamProject2/pull/57) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [Issue #25](https://github.com/2021Cyan/TeamProject2/issues/25).

## E05. 캐릭터별 애니메이션 설정·집라인 이벤트 타이밍 조정

- **실제 구현**: 대시 몽타주 설정을 GA에서 캐릭터 속성으로 옮기고 여러 이동 몽타주 참조를 노출했다. #38/#44/#46에 캐릭터별 섹션·InitialAbilities 설정 절차를 작성했다. 집라인 몽타주 이벤트 타이밍 조정 커밋도 확인했다.
- **담당 범위·기여 형태**: 캐릭터 데이터화·세팅 문서·애니메이션 통합 기여. 집라인 스플라인·탑승·이동 시스템 원 구현과 애니메이션 수정 도구는 2021Cyan 작업.
- **기술·관련 클래스**: UPROPERTY(EditAnywhere), AnimMontage section, InitialAbilities, GameplayEvent Notify
- **구현·현재 상태**: 몽타주 참조와 설정 변경 확인. 집라인 이벤트의 정확한 전후 시각은 바이너리 에셋 캡처 필요.
- **소개할 기술적 특징**: 여러 캐릭터가 공통 이동기를 사용하도록 설정 위치와 통합 절차를 정리.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.h](../Code/SharedCharacter/BaseCharacter.excerpt.h)
- **이력**: [`15a213c40`](https://github.com/2021Cyan/TeamProject2/commit/15a213c407e1705e5f9ccfbdd858eea022a46c86) · [`892a2c906`](https://github.com/2021Cyan/TeamProject2/commit/892a2c906b44e8df6bf72b64716479daf86b3034) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #39](https://github.com/2021Cyan/TeamProject2/pull/39) · [PR #44](https://github.com/2021Cyan/TeamProject2/pull/44) · [PR #46](https://github.com/2021Cyan/TeamProject2/pull/46) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #19](https://github.com/2021Cyan/TeamProject2/issues/19) · [Issue #26](https://github.com/2021Cyan/TeamProject2/issues/26).

## E06. 초기 맵 통합·리소스 연결·테스트/폰트 정리

- **실제 구현**: 초기 테스트 맵·입력·마네킹 리소스를 반입하고 이동 테스트맵 및 캐릭터 배치를 수정했다. 이후 맵 밝기·캐릭터 크기를 조정하고 테스트 에셋·불필요한 폰트·임시 파일을 정리했다.
- **담당 범위·기여 형태**: 에셋 반입·설정·정리 기여. 전장 전체 제작, 마네킹/캐릭터 모델·애니메이션·외부 VFX 원 제작, 환경 렌더 최적화로 확대하지 않는다.
- **기술·관련 클래스**: Unreal Editor, .umap/.uasset, BlendSpace, Git 에셋 이력
- **구현·현재 상태**: 바이너리 변경·삭제 이력 확인. 구체 배치·라이팅 설정과 리소스 출처는 에디터/외부 출처 추가 확인.
- **소개할 기술적 특징**: 프로젝트를 함께 실행할 수 있도록 한 통합·정리 작업도 이력에 포함.
- **전시 코드**: BP 설정·에셋 이력 또는 정리 작업. 별도 C++ 본체로 대체해 만들지 않음.
- **이력**: [`1d62f96d5`](https://github.com/2021Cyan/TeamProject2/commit/1d62f96d5cfe07e682a0605370877c3d22e1720a) · [`a4cd049d8`](https://github.com/2021Cyan/TeamProject2/commit/a4cd049d80f70445c913e743ef64ae70ebcbb00f) · [`3fd307375`](https://github.com/2021Cyan/TeamProject2/commit/3fd307375183d48ea8ed58677b1c39be81470cc6) · [`795b038bb`](https://github.com/2021Cyan/TeamProject2/commit/795b038bb8a049b954d0667254f82cd8ab98290e) · [`c1a352d18`](https://github.com/2021Cyan/TeamProject2/commit/c1a352d18b8af4d73e41f480f5380f96372aa45f) · [`3ed2d3581`](https://github.com/2021Cyan/TeamProject2/commit/3ed2d35818c33d81398686700ff0cee68ce71864) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #52](https://github.com/2021Cyan/TeamProject2/pull/52) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68) · [Issue #30](https://github.com/2021Cyan/TeamProject2/issues/30).

## E07. 기존 캐릭터 파일 통합·삭제 구현·코드 정리

- **실제 구현**: 초기 Private/Character 및 Private/Player의 이전 캐릭터 파일을 제거하고 GameCore 구조로 통합했다. FlogRope C++ GCN을 구현한 뒤 폐기했으며 디버그 로그·주석·테스트 임시 코드를 정리했다.
- **담당 범위·기여 형태**: 정리·이행·삭제 작업 확인. 제거한 기존 파일 전체를 본인이 최초 구현했다고 해석하지 않는다.
- **기술·관련 클래스**: Git 삭제 이력, GameCore 공통 캐릭터 구조, GameplayCueNotify_Static
- **구현·현재 상태**: 이력으로 확인. 삭제 파일은 현재 작업 트리에 없음. GCN 생성·삭제 양쪽 커밋을 근거 문서에 보존.
- **소개할 기술적 특징**: 현재 클래스 목록만으로는 빠지는 과거 구현과 구조 이행을 별도로 설명할 수 있음.
- **전시 코드**: [Code/SharedCharacter/BaseCharacter.excerpt.cpp](../Code/SharedCharacter/BaseCharacter.excerpt.cpp)
- **이력**: [`5ac8fafa4`](https://github.com/2021Cyan/TeamProject2/commit/5ac8fafa4eaebbe217628e060c3baf3a98faf412) · [`bc67b4167`](https://github.com/2021Cyan/TeamProject2/commit/bc67b4167a89465259187599adbef16fd79799a7) · [`c5dc09fdf`](https://github.com/2021Cyan/TeamProject2/commit/c5dc09fdf2b7056c90bada2580118a4f792b0b66) · [`03863e944`](https://github.com/2021Cyan/TeamProject2/commit/03863e944dcdee3745eb1dae2cdda814d00a812b) · [`c1a352d18`](https://github.com/2021Cyan/TeamProject2/commit/c1a352d18b8af4d73e41f480f5380f96372aa45f) · [PR #38](https://github.com/2021Cyan/TeamProject2/pull/38) · [PR #62](https://github.com/2021Cyan/TeamProject2/pull/62) · [PR #68](https://github.com/2021Cyan/TeamProject2/pull/68).

