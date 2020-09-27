#include <loringlib>
#include <qufnrtools>
#include <bononohale>

public Plugin myinfo = {
	name = "Boss William Birkin"
	, author = "Bloody World"
	, description = "bononohale on William Birkin"
	, version = "1.0"
	, url = "https://steamcommunity.com/id/clshtpzkdl"
};

#define MAIN_BOSS_INDEX		4

int evolution = 0; // 보스 진화단계
float evolution_limit = 0.0; // 보스 누적 데미지
int boss = 0; // 보스 인덱스
float sec_dmg = 0.0; // 보스 순간 데미지
int boss_percent[3] = 0; // 보스 퍼센트 체력  
int tpercent = 0; // 10퍼센트 체력

Handle g_hBossInfo = null;

#define SND_FX_EVOLUTION_FIRST	"qufnr/new/hale_class/william_birkin/evol1.mp3"
#define SND_FX_EVOLUTION_SECOND	"qufnr/new/hale_class/william_birkin/evol2.mp3"
#define SND_FX_GROUNDSLAM	"qufnr/new/hale_class/william_birkin/groundslam%d.mp3"	//	1 2

#define SFX_EVOL1	"qufnr/new/hale_class/william_birkin/sfx_1evol.mp3"
#define SFX_EVOL2	"qufnr/new/hale_class/william_birkin/sfx_2evol.mp3"
#define SFX_COMBO	"qufnr/new/hale_class/william_birkin/sfx_combo%d.mp3"	//	1 3
#define SFX_SLAM	"qufnr/new/hale_class/william_birkin/sfx_slam%d.mp3"	//	1 2

#define BGM_PHASE2	"qufnr/new/hale_class/william_birkin/bgm_phase2.mp3"
#define BGM_PHASE3	"qufnr/new/hale_class/william_birkin/bgm_phase3.mp3"

#define PTC_FX_EVOL_FIRST	"william_evol1"
#define PTC_FX_EVOL_SECOND	"william_evol2"
#define PTC_FX_GROUNDSLAM	"william_groundslam"
#define PTC_FX_COMBO	"william_combo"
#define PTC_FX_COMBO2	"william_combo2"

public void OnPluginStart () {
	HookEvent("round_end", RoundEnd);
	HookEvent("round_start", RoundStart);
	HookEvent("weapon_fire", Weapon_Fire);
}

public void OnMapStart () {

	loringlib_PrecacheParticle ( "particles/qufnr/new/boss/fx_william_birkin.pcf", true );
	loringlib_PrecacheParticleName ( PTC_FX_EVOL_FIRST );
	loringlib_PrecacheParticleName ( PTC_FX_EVOL_SECOND );
	loringlib_PrecacheParticleName ( PTC_FX_GROUNDSLAM );
	loringlib_PrecacheParticleName ( PTC_FX_COMBO );
	loringlib_PrecacheParticleName ( PTC_FX_COMBO2 );
	
	PrecacheSound ( BGM_PHASE2, true );
	PrecacheSound ( BGM_PHASE3, true );
	
	PrecacheModel("models/player/custom_player/kodua/g/stage1.mdl", true);
	PrecacheModel("models/player/custom_player/kodua/g/stage2.mdl", true);
	PrecacheModel("models/player/custom_player/kodua/g/stage3.mdl", true);
	
	PrecacheModel ( "models/player/custom_player/kodua/wb1/wb1_arms.mdl", true );
	PrecacheModel ( "models/player/custom_player/kodua/wb2/wb2_arms.mdl", true );
	PrecacheModel ( "models/player/custom_player/kodua/wb3/wb3_arms.mdl", true );
}

public OnClientPutInServer(id)
{
	SDKHook(id, SDKHook_OnTakeDamage, OnTakeDamage);
	SDKHook(id, SDKHook_PreThink, OnPreThink);
}

public OnClientDisconnect(id)
{
	SDKUnhook(id, SDKHook_OnTakeDamage, OnTakeDamage);
	SDKUnhook(id, SDKHook_PreThink, OnPreThink);
}

public Action RoundStart(Event event, const char[] name, bool dontBroadcast)
{
	reset();
}

public Action RoundEnd(Event event, const char[] name, bool dontBroadcast)
{
	reset();
	for ( int cl = 1; cl < MaxClients; cl ++ ) {
		if ( loringlib_IsValidClient ( cl ) ) {
			StopSound ( cl, SNDCHAN_AUTO, BGM_PHASE2 );
			StopSound ( cl, SNDCHAN_AUTO, BGM_PHASE3 );
		}
	}
}

public Weapon_Fire(Handle:event, const String:name[], bool:dontBroadcast)
{
	new id = GetClientOfUserId(GetEventInt(event, "userid"));
	char weapons[64]; 
	GetClientWeapon(id, weapons, sizeof(weapons)); 
	if(StrContains(weapons, "knife", false) != -1 && evolution == 3 && id == boss)
	{
		for(int i = 1; i <= MaxClients; i++)
		{
			if(IsClientInGame(i) && IsPlayerAlive(i) && boss != i)
			{
				if(IsTargetInSightRange(id, i, 60.0, 200.0, true))
				{
					makedamage(id, i, 60);
				}
			}
		}	
		
		float flPos[3], flAng[3];
		GetClientEyePosition ( id, flPos );
		GetClientEyeAngles ( id, flAng );
		loringlib_AddInFrontOf ( flPos, flAng, 50.0, flPos );
		loringlib_CreateParticleEx ( id, 0, flPos, flAng, GetRandomInt ( 0, 1 ) ? PTC_FX_COMBO : PTC_FX_COMBO2 );
	
		char szSnd[256];
		Format ( szSnd, sizeof szSnd, SFX_COMBO, GetRandomInt ( 1, 3 ) );
		if ( !IsSoundPrecached ( szSnd ) )
			PrecacheSound ( szSnd );
		EmitSoundToAll ( szSnd, id );
	}
} 

public void NONOHALE_OnRoundStartCountdownLastTick ( int haleClientIndex ) {
	if ( NONOHALE_GetClientHaleIndex ( haleClientIndex ) == MAIN_BOSS_INDEX ) {
		if ( g_hBossInfo == null )
			g_hBossInfo = CreateTimer(10.0, Server_Info2, _, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
	}
}

public void NONOHALE_OnHumanToHale ( int client, int haleIndex ) {
	if ( haleIndex == MAIN_BOSS_INDEX ) {
		reset();
		
		boss = client;
		
		tpercent = loringlib_GetEntityMaxHealth ( client ) / 10;
		boss_percent[0] = tpercent * 10;
		boss_percent[1] = tpercent * 7;
		boss_percent[2] = tpercent * 5;
		
		DrSkill01 ( client );
	}
	else
		boss = 0;
}

public Action NONOHALE_OnHaleRage ( int client, int haleIndex, float& distance, float& duration ) {
	if ( haleIndex == MAIN_BOSS_INDEX ) {
		
		bool bChanged = false;
		
		if( evolution == 2 ) {
			if ( GetEntityFlags ( client ) & FL_ONGROUND ) {
				Skill02_Attack ( client );
				bChanged = true;
			}
			else {
				PrintToChat ( client, " \x04[Boss] \x01공중에서는 사용할 수 없습니다." );
				return Plugin_Handled;
			}
			distance = 500.0;
		}
		else if( evolution == 3 ){
			bChanged = true;
			distance = 750.0;
		}
		return bChanged ? Plugin_Changed : Plugin_Continue;
	}
	
	return Plugin_Continue;
}

public Action:OnTakeDamage(client, &attacker, &inflictor, &Float:dmg, &damagetype)
{
	if( (0 < client <= MaxClients) &&
		(0 < attacker <= MaxClients) )
	{
		if(GetClientTeam(client) != GetClientTeam(attacker))
		{
			if(client == boss)
			{	
				evolution_limit += dmg;
				sec_dmg += dmg;
				
				if(evolution == 1 && GetClientHealth(boss) <= boss_percent[1])
					DrSkill02(boss);
				if(evolution == 2 && GetClientHealth(boss) <= boss_percent[2] && tpercent * 2 <= sec_dmg)
					DrSkill03(boss);
				if(evolution == 3) dmg = 15 * (dmg / 20.0);
			}
			
			if(damagetype == DMG_SLASH)
			{
				if(evolution != 3 && attacker == boss)
				{
					if(evolution == 1) dmg = 70.0;
					if(evolution == 2) dmg = 50.0;
				}	
				else if(evolution == 3)
				{
					dmg = 0.0;
				}			
			}
		}
	}
	return Plugin_Changed;
}

public Action OnPreThink(int client)
{
	int weapons = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if(weapons > 0)
	{
		char classname[MAX_NAME_LENGTH];
		GetEdictClassname(weapons, classname, sizeof(classname));
		if(StrContains(classname, "knife") && boss == client)
		{	
			if(evolution == 3)
			{
				SetEntPropFloat(weapons, Prop_Send, "m_flNextPrimaryAttack", GetEntPropFloat(weapons, Prop_Send, "m_flNextPrimaryAttack") - ((1.01 - 1.0) / 2));  			
			}
		}	
	}
	return Plugin_Continue;
}

public Action Server_Info2(Handle timer)
{

	if ( NONOHALE_IsGameEnd () || !NONOHALE_IsGameStart () ) {
		g_hBossInfo = null;
		return Plugin_Stop;
	}

	if(boss) 
	{
		if(sec_dmg) sec_dmg = 0.0;
	}
	
	return Plugin_Continue;
}

public Skill02_Attack(id)
{
	float origin[3], iorigin[3];
	GetClientEyePosition(id, origin);
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && IsPlayerAlive(i) && boss != i)
		{
			if(IsTargetInSightRange(id, i, 90.0, 300.0, false))
			{
				makedamage(id, i, 1000);
			}
			
			GetClientEyePosition(i, iorigin);
			if(GetVectorDistance(origin, iorigin, false) <= 500.0)
			{
				loringlib_ShowShakeMessage(i, 300.0, 1.0);
			}
		}
	}
	
	char szSnd[256];
	Format ( szSnd, sizeof szSnd, SND_FX_GROUNDSLAM, GetRandomInt ( 1 , 2 ) );
	if ( !IsSoundPrecached ( szSnd ) )
		PrecacheSound ( szSnd );
	EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER, SNDCHAN_WEAPON );	
	
	Format ( szSnd, sizeof szSnd, SFX_SLAM, GetRandomInt ( 1, 2 ) );
	if ( !IsSoundPrecached ( szSnd ) )
		PrecacheSound ( szSnd );
	EmitSoundToAll ( szSnd, id );
	
	float flPos[3], flAng[3];
	GetClientAbsOrigin ( id,  flPos );
	GetClientAbsAngles ( id, flAng );
	loringlib_CreateParticleEx ( id, 0, flPos, flAng, PTC_FX_GROUNDSLAM );
}

public reset()
{
	evolution = 0;
	evolution_limit = 0.0;
	boss = 0;
}

public DrSkill01(id)
{
	if(evolution != 1) evolution = 1;
	SetEntityModel(id, "models/player/custom_player/kodua/g/stage1.mdl");
	SetEntityHealth(id, boss_percent[0]);
	loringlib_SetEntityArmsModel ( id, "" );
	loringlib_SetEntityArmsModel ( id, "models/player/custom_player/kodua/wb1/wb1_arms.mdl" );
}

public DrSkill02(id)
{
	if(evolution != 2) evolution = 2;
	SetEntityModel(id, "models/player/custom_player/kodua/g/stage2.mdl");
	if ( !IsSoundPrecached ( SND_FX_EVOLUTION_FIRST ) )
		PrecacheSound ( SND_FX_EVOLUTION_FIRST );
	EmitSoundToAll ( SND_FX_EVOLUTION_FIRST, SOUND_FROM_PLAYER, SNDCHAN_WEAPON );
	SetEntityHealth(id, boss_percent[1]);
	loringlib_SetEntityArmsModel ( id, "" );
	loringlib_SetEntityArmsModel ( id, "models/player/custom_player/kodua/wb2/wb2_arms.mdl" );
	
	if ( !IsSoundPrecached ( SFX_EVOL1 ) )
		PrecacheSound ( SFX_EVOL1 );
	EmitSoundToAll ( SFX_EVOL1, id );
	
	if ( NONOHALE_StopPlayMusic () ) {
		EmitSoundToAll ( BGM_PHASE2, SOUND_FROM_PLAYER, SNDCHAN_AUTO );
	}
	
	float flOrigin[3];
	GetClientAbsOrigin ( id, flOrigin );
	loringlib_CreateParticleEx ( id, 0, flOrigin, NULL_VECTOR, PTC_FX_EVOL_FIRST, true );
}

public DrSkill03(id)
{
	if(evolution != 3) evolution = 3;
	SetEntityModel(id, "models/player/custom_player/kodua/g/stage3.mdl");
	loringlib_SetEntityLaggedmovement ( id, 1.5 );
	
	if ( !IsSoundPrecached ( SND_FX_EVOLUTION_SECOND ) )
		PrecacheSound ( SND_FX_EVOLUTION_SECOND );
	EmitSoundToAll ( SND_FX_EVOLUTION_SECOND, SOUND_FROM_PLAYER, SNDCHAN_WEAPON );
	SetEntityHealth(id, boss_percent[2]);
	loringlib_SetEntityArmsModel ( id, "" );
	loringlib_SetEntityArmsModel ( id, "models/player/custom_player/kodua/wb3/wb3_arms.mdl" );
	
	if ( !IsSoundPrecached ( SFX_EVOL2 ) )
		PrecacheSound ( SFX_EVOL2 );
	EmitSoundToAll ( SFX_EVOL2, id );
	
	for ( int cl = 1; cl < MaxClients; cl ++ )
		if ( loringlib_IsValidClient ( cl ) )
			StopSound ( cl, SNDCHAN_AUTO, BGM_PHASE2 );
	EmitSoundToAll ( BGM_PHASE3, SOUND_FROM_PLAYER, SNDCHAN_AUTO );
	
	float flOrigin[3];
	GetClientAbsOrigin ( id, flOrigin );
	loringlib_CreateParticleEx ( id, 0, flOrigin, NULL_VECTOR, PTC_FX_EVOL_SECOND, true );
}

public DrSkill04(id)
{
	makedamage(id, id, 1000);
}

bool makedamage(int client, int target, int damage)
{
	new pointhurt = CreateEntityByName("point_hurt");
	if(pointhurt != -1)
	{
		if(target != -1)
		{
			decl String:targetname[64];
			Format(targetname, 128, "%f%f", GetEngineTime(), GetRandomFloat());
			DispatchKeyValue(target,"TargetName", targetname);
			DispatchKeyValue(pointhurt,"DamageTarget", targetname);
		}
		else DispatchKeyValue(pointhurt,"DamageTarget", "");

		decl String:number[64];
		IntToString(damage, number, 64);
		DispatchKeyValue(pointhurt,"Damage", number);
		DispatchSpawn(pointhurt);
		if(MaxClients >= client > 0 && IsClientInGame(client))
		{
			AcceptEntityInput(pointhurt, "Hurt", client);
		}
		AcceptEntityInput(pointhurt, "Kill");
		return true;
	}
	else return false;
}

stock bool:IsTargetInSightRange(client, target, Float:angle, Float:distance, bool udcheck)
{    
	if(GetClientTeam(client) == GetClientTeam(target)) return false;
	decl Float:clientpos[3], Float:targetpos[3], Float:anglevector[3], Float:targetvector[3], Float:resultangle, Float:resultdistance;

	if(udcheck == true) GetClientEyeAngles(client, anglevector);
	else GetClientAbsAngles(client, anglevector);	
	GetAngleVectors(anglevector, anglevector, NULL_VECTOR, NULL_VECTOR);
	NormalizeVector(anglevector, anglevector);

	GetClientAbsOrigin(client, clientpos);
	GetClientAbsOrigin(target, targetpos);

	MakeVectorFromPoints(clientpos, targetpos, targetvector);
	NormalizeVector(targetvector, targetvector);

	resultangle = RadToDeg(ArcCosine(GetVectorDotProduct(targetvector, anglevector)));
	
	if(resultangle <= angle/2)  
	{
		if(distance > 0)
		{
			resultdistance = GetVectorDistance(clientpos, targetpos);
			if(distance >= resultdistance)
				return true;
			else
				return false;
		}
		else
			return true;
	}
	else
		return false;
}