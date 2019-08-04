# bononohale (a.k.a Counter-Strike: Freak Offensive)
팀 포트리스2 모드인 Freak Fotress 2(이하 FF2)를 기반으로 만든 CS:GO의 모드입니다.
기본 FF2의 틀만 짜여진 모드 이므로 점차 업데이트 하겠습니다,

## Admin Commands  
명령어 구분: () - 필수, [] - 선택

명령 타겟 필터: ***@boss, @!boss, @hale, @!hale***

> sm_setstun (#userid|name) [time]  
**특정 타겟을 스턴 합니다.**  

> sm_setboss (index)  
**해당 라운드 보스를 설정합니다. (라운드 시작하고 보스가 선택되기 전에 설정해야 합니다.)**

> sm_setrage (#userid|name) (value)  
**특정 타겟의 분노를 설정합니다. (타겟이 보스여야만 적용 됩니다.)**


## Commands
명령어 구분: () - 필수, [] - 선택

> sm_bnnoption  
**해당 모드에 관련된 클라이언트 설정을 컨트롤 합니다.**

> sm_queuepoint ***or*** sm_qp [username|rank]  
**인수가 없으면 자신의 Queue Points를 봅니다, username을 입력하면 해당 유저의 Queue Points를 확인합니다. rank는 Queue Points가 많은 순서 TOP5를 확인합니다.**  

> sm_resetqp  
**자신의 Queue Points를 초기화 합니다.**


## SourceMod Bononohale API
```c
/**
 * 보스 선택 시간을 반환합니다.
 *
 * @return 보스 선택 시간
 */
native int NONOHALE_GetCountdown ();

/**
 * 게임이 시작됬는지 채크합니다.
 *
 * @return 게임이 시작됬을 경우 true를 반환합니다.
 */
native bool NONOHALE_IsGameStart ();

/**
 * 게임이 종료됬는지 채크합니다.
 *
 * @return 게임이 종료됬을 경우 true를 반환합니다.
 */
native bool NONOHALE_IsGameEnd ();

/**
 * 플레이어를 기다리는 중인지 채크합니다.
 *
 * @return 플레이어를 기다리는 중일 경우 true를 반환합니다.
 */
native bool NONOHALE_IsGamePlayerWaiting ();

/**
 * 헤일을 찾는 중인지 채크합니다.
 *
 * @return 헤일을 찾고 있을 경우 true를 반환합니다.
 */
native bool NONOHALE_IsGameFindHale ();

/**
 * 클라이언트가 헤일인지 채크합니다.
 *
 * @param client		클라이언트 인덱스
 * @return 클라이언트가 헤일일 경우 true를 반환합니다.
 */
native int NONOHALE_IsClientHale ( int client );

/**
 * 헤일 클라이언트 값을 반환합니다.
 *
 * @return 헤일인 클라이언트 값
 */
native int NONOHALE_GetHaleIndex ();

/**
 * 헤일의 피해량 값을 반환합니다.
 *
 * @return 피해량 값
 */
native int NONOHALE_GetHaleDamages ();

/**
 * 클라이언트가 스턴 상태인지 채크합니다.
 *
 * @param client
 * @return 스턴일 경우 true를 반환합니다.
 */
native bool NONOHALE_IsClientStun ( int client );

/**
 * 해당 클라이언트의 헤일 인덱스 값을 반환합니다.
 *
 * @param client
 * @return 헤일 인덱스 값, 클라이언트가 헤일이 아닐 경우 -1을 반환합니다.
 */
native int NONOHALE_GetClientHaleIndex ( int client );

/**
 * 게임 시작 전에 선택된 헤일 클라이언트를 반환합니다.
 *
 * @return iHaleClient
 */
native int NONOHALE_GetPreFindHaleClient ();

/**
 * 클라이언트의 피해량 값을 반환합니다.
 * @note 클라이언트가 헤일일 경우 0을 반환합니다.
 *
 * @param client
 */
native int NONOHALE_GetDamages ( int client );

/**
 * 클라이언트의 피해량 값을 설정합니다.
 *
 * @param client
 * @param setValue
 */
native void NONOHALE_SetDamages ( int client, int setValue );

/**
 * 클라이언트의 피해량을 더합니다.
 *
 * @param client
 * @param addValue
 */
native void NONOHALE_AddDamages ( int client, int addValue );

/**
 * 클라이언트의 큐 포인트를 반환합니다.
 *
 * @param client
 * @return queue points
 */
native int NONOHALE_GetClientQueuePoints ( int client );

/**
 * 클라이언트의 큐 포인트를 설정합니다.
 *
 * @param client
 * @param value
 */
native void NONOHALE_SetClientQueuePoints ( int client, int value );

/**
 * 재생중인 음악을 구합니다.
 *
 * @param music
 * @param maxlen
 */
native void NONOHALE_GetPlayMusic ( const char[] music, int maxlen );

/**
 * 헤일을 찾을 때 틱(초)마다 호출합니다.
 *
 * @param ticks		카운트 수
 * @param firstCreated		타이머가 처음 만들어진 시점일 경우 true
 */
forward void NONOHALE_OnRoundStartCountdown ( int ticks, bool firstCreated );

/**
 * 헤일을 찾을 때 마지막 틱에 호출합니다.
 *
 * @param haleClientIndex	헤일로 선택된 클라이언트 인덱스
 */
forward void NONOHALE_OnRoundStartCountdownLastTick ( int haleClientIndex );

/**
 * 선택된 클라이언트가 헤일이 될 때 호출합니다.
 *
 * @param client	클라이언트 인덱스
 * @param haleIndex	헤일 인덱스
 */
forward void NONOHALE_OnHumanToHale ( int client, int haleIndex );

/**
 * 헤일이 점프가 준비 되었을 때 호출합니다.
 *
 * @param client	클라이언트 인덱스
 * @param haleIndex	헤일 인덱스
 */
forward void NONOHALE_OnHaleJumpReady ( int client, int haleIndex );

/**
 * 헤일이 점프 했을 때 호출합니다.
 *
 * @param client	클라이언트 인덱스
 * @param haleIndex	헤일 인덱스
 */
forward void NONOHALE_OnHaleJump ( int client, int haleIndex );

/**
 * 헤일이 내려찍기 (Weight Down)했을 때 호출합니다.
 *
 * @param client		클라이언트 인덱스
 */
forward void NONOHALE_OnHaleWeightDown ( int client );

/**
 * 내려찍기 후 착지했을 때 호출합니다.
 *
 * @param client		클라이언트 인덱스
 * @param origin		착지 위치
 */
forward void NONOHALE_OnHaleWeightDownLanding ( int client );

/**
 * 스톰프를 했을 때 호출합니다.
 *
 * @param victim		피해자
 * @param attacker		공격자
 */
forward void NONOHALE_OnClientStomp ( int victim, int attacker );

/**
 * 헤일이 분노를 사용할 때 호출합니다.
 *
 * @param client
 * @param haleIndex
 */
forward Action NONOHALE_OnHaleRage ( int client, int haleIndex, float& distance, float& duration );

/**
 * 헤일이 분노를 사용하고 종료 되었을 때 호출합니다.
 *
 * @param client
 * @param haleIndex
 */
forward void NONOHALE_OnHaleRageEnd ( int client, int haleIndex );

/**
 * 스턴에 걸렸을 때 호출합니다.
 *
 * @param victim
 * @param attacker
 * @param stunTime		스턴 시간
 * @param stunDamage	스턴 피해량
 */
forward Action NONOHALE_OnClientStun ( int victim, int attacker, float& stunTime, float& stunDamage );

/**
 * 라운드 시작 사전에 헤일을 선택할 때 호출 합니다.
 *
 * @param haleClient
 */
forward void NONOHALE_OnPreSetHaleClient ( int haleClient );

/**
 * 헤일이 분노 게이지를 채울 때 호출합니다.
 *
 * @param client
 * @param haleIndex
 * @param damage
 */
forward Action NONOHALE_OnHaleRageCharge ( int client, int haleIndex, int& damage );
```
