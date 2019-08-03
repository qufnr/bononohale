#include <loringlib>
#include <qufnrTools>

#include <autoexecconfig>
#include <soundlib>

#define PLUGIN_VERSION	"1.1.2"

public Plugin myinfo = {
	name = "CS:GO VS. Saxston Hale Mode"
	, author = "qufnr"
	, description = ""
	, version = PLUGIN_VERSION
	, url = "https://steamcommunity.com/id/qufnrp"
};

int g_iFindTime = 0;
Handle g_hFindTimer = null;

#define		GLOBAL_TIMER_TICK	0.1

#define		HALE_TEAM	CS_TEAM_T
#define		HUMAN_TEAM	CS_TEAM_CT

#define		HALE_GOOMBA_SND		"qufnr/new/fx_stomp.mp3"

#define		HALE_STUN_FX_PATH		"particles/qufnr/stun_type_status.pcf"
#define		HALE_STUN_FX_SCARE		"status_scare"
#define		HALE_STUN_FX_STUN		"status_stun"

/**
 * Game status Variables
 */
enum struct GameStatus {
	bool gameStart;
	bool gameEnd;
	bool gameFindHale;
	bool playerWaiting;
}
GameStatus g_GameStatus;

/**
 * Player game data Variables
 */
enum struct PGameData {
	float spawnLocation[3];

	bool hale;
	int haleIndex;
	bool haleRage;
	int haleRageDmgs;
	
	int damages;			//	보스가 입은 피해량
	int attackDamages;		//	보스에게 입힌 피해량
	
	int queuePoints;		//	큐 포인트
	
	bool jumpReady;
	bool weightDownReady;
	
	bool stun;
	
}
PGameData g_PGameData[MAXPLAYERS + 1];

enum struct PlayerOptions {
	int musicToggle;
	int displayDamages;
	int displayBossHealth;
}
PlayerOptions g_POptions[MAXPLAYERS + 1];

/**
 * Config data Variables
 */
enum struct ConfigData {
	int minPlayers;		//	게임 시작 시 필요한 인원 수
	int findTime;		//	헤일 찾는 시간
	int ignoreFreezetime;	//	카운트다운 프리즈타임 무시
	
	float goombaHumanDamage;
	float goombaHaleDamage;
	bool goombaSound;
	
	bool displayBossHealth;
	bool startOnRespawnHale;
	bool displayRankOfDamage;
	
	float roundEndDelay;
}
ConfigData g_ConfigData;

/**
 * Hale Info data Variables
 */
int g_iHaleData = 0;
enum struct HaleData {
	char name[64];
	char model[256];
	char armsModel[256];
	int health;
	int healthMath;
	int armor;
	int rageRange;
	float speed;
	float gravity;
	float jumpVector;
	float jumpCooldown;
	float weightDownCooldown;
	float knockbackImmunityScale;
	
	float distOfRage;
	float durationOfRage;
	
	float damageMultiple;
	float attackSpeed;
}
#define MAX_HALE_SOUNDS	4
#define MAX_HALE_MUSIC	8
#define MAX_SND_TYPE	7
#define HALE_SND_INDEX_DEATH	0
#define HALE_SND_INDEX_INTRO	1
#define HALE_SND_INDEX_JUMP		2
#define HALE_SND_INDEX_KILL		3
#define HALE_SND_INDEX_PAIN		4
#define HALE_SND_INDEX_RAGE		5
#define HALE_SND_INDEX_VICTORY	6
#define MAX_HALE_DATA	512
HaleData g_HaleData[MAX_HALE_DATA];
char g_szHaleSoundData[MAX_HALE_DATA][MAX_SND_TYPE][MAX_HALE_SOUNDS][256];
char g_szHaleMusic[MAX_HALE_DATA][MAX_HALE_MUSIC][256];

/**
 * Weapon Variables
 */
int g_iWeaponData = 0;
enum struct WeaponData {
	char classname[32];
	float damageMultiple;
	float knockbackScale;
	float fireRate;
	float reloadSpd;
}
#define MAX_CSGO_WEAPONS	46
WeaponData g_WeaponData[MAX_CSGO_WEAPONS];
bool g_bWeaponLoaded = false;

Handle g_hGlobalRepeatTimer = null;

Handle g_hHudSyncHndl[8] = { null, ... };
#define HUDSYNC_FIND_TEXT		0
#define HUDSYNC_HALE_DAMAGE		1
#define HUDSYNC_HALE_BASESKILL	2
#define HUDSYNC_HALE_HEALTH		3
#define HUDSYNC_HALE_DMGRANK	4

Handle g_hForwardHandle[13] = { null, ... };
#define FORWARD_ON_ROUND_START_COUNT_DOWN	0
#define FORWARD_ON_HUMAN_TO_HALE	1
#define FORWARD_ON_HALE_JUMP_READY	2
#define FORWARD_ON_HALE_JUMP		3
#define FORWARD_ON_HALE_RAGE		4
#define FORWARD_ON_HALE_RAGE_END	5
#define FORWARD_ON_HALE_WEIGHT_DOWN	6
#define FORWARD_ON_CLIENT_STOMP		7
#define FORWARD_ON_CLIENT_STUN		8
#define FORWARD_ON_ROUND_START_COUNT_DOWN_LAST_TICK	9
#define FORWARD_ON_PRE_SET_HALE_CLIENT	10
#define FORWARD_ON_HALE_RAGE_CHARGE	11
#define FORWARD_ON_HALE_WEIGHT_DOWN_LANDING	12

int g_iChooseHaleIndex = -1;
int g_iHaleClient = -1;

#include "bononohale/bononohale.inc"	//	API

/**
 * Plugin include
 */
#include "bononohale/config.inc"
#include "bononohale/command.inc"
#include "bononohale/event.inc"
#include "bononohale/game.inc"
#include "bononohale/weapon.inc"
#include "bononohale/player.inc"
#include "bononohale/infect.inc"
#include "bononohale/misc.inc"
#include "bononohale/sound.inc"
#include "bononohale/baseskills.inc"
#include "bononohale/queuepoint.inc"

public void OnPluginStart () {
	LoadTranslations ( "common.phrases.txt" );
	LoadTranslations ( "bononohale.phrases.txt" );
	
	CONFIG_ReadFile ();
	
	COMMAND_RegisterCommands ();
	COMMAND_AddMultiTargetTilter ();
	EVENT_RegisterEvents ();
}

public void OnServerLoad () {
	CONFIG_ReadHaleData ();
	WEAPON_ReadWeaponData ();
	
	GAME_SetDefaultStatus ();
	
	MISC_CreateGlobalTimer ();
	MISC_CreateHudSyncHndl ();
	
	BASESKILL_PrecacheAll ();
}

public void OnMapEnd () {

	GAME_KillFindTimer ();
	
	MISC_KillGlobalTimer ();
	MISC_KillHudSyncHndl ();
	
	SOUND_OnClearMusicTimer ();
}

public void OnEntityCreated ( int entity, const char[] classname ) {
	WEAPON_EntCreateOnWeaponReloadHook ( entity );
}