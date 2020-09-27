#include <loringlib>
#include <qufnrtools>
#include <bononohale>

public Plugin myinfo = {
	name = "Boss Fleshpound"
	, author = "qufnr & nubi"
	, description = "bononohale on Fleshpound Boss"
	, version = "1.2"
	, url = "https://steamcommunity.com/id/qufnrp"
};

#define MAIN_BOSS_INDEX		1	//	Fleshpound

#define DEFDATA_SPEED		0
#define DEFDATA_GRAVITY		1

#define RAGE_SPEED		1.5
#define RAGE_GRAVITY	0.82
#define RAGE_DURATION	10.0

#define SND_FX_ATTACK	"qufnr/new/hale_class/fleshpound/rage_attack%d.mp3"	//	1 2

#define PTC_FX_ATTACK	"fleshpound_pound"
#define PTC_FX_RAGE_ENABLE	"fleshpound_ult"

float g_flHaleDefData[2] = { 1.0, ... };
int g_iLastVictim = -1;
int g_iRageParticleEnt = -1;
bool g_bEnableRage = false;
Handle g_hRageDurationHandle = null;

public void OnMapStart () {
	loringlib_PrecacheParticle ( "particles/qufnr/new/boss/fx_fleshpound.pcf", true );
	loringlib_PrecacheParticleName ( PTC_FX_ATTACK );
	loringlib_PrecacheParticleName ( PTC_FX_RAGE_ENABLE );
}

public void OnClientDisconnect ( int client ) {
	SDKUnhook ( client, SDKHook_OnTakeDamage, Fleshpound_OnGivenDamage );
}

public void OnMapEnd () {
	if ( g_hRageDurationHandle != null ) {
		KillTimer ( g_hRageDurationHandle );
		g_hRageDurationHandle = null;
	}
}

void AllUnhook () {
	for ( int i = 1; i <= MaxClients; i ++ )
		if ( loringlib_IsValidClient ( i ) )
			SDKUnhook ( i, SDKHook_OnTakeDamage, Fleshpound_OnGivenDamage );
}

public Action NONOHALE_OnHaleRageCharge ( int client, int haleIndex, int& damage ) {
	if ( haleIndex == MAIN_BOSS_INDEX && g_bEnableRage )
		return Plugin_Handled;
	return Plugin_Continue;
}

public void NONOHALE_OnRoundStartCountdownLastTick ( int haleClientIndex ) {
	g_bEnableRage = false;
	g_iLastVictim = -1;
}

public Action NONOHALE_OnHaleRage ( int client, int haleIndex ) {
	if ( g_bEnableRage )
		return Plugin_Handled;
	
	if ( haleIndex == MAIN_BOSS_INDEX ) {
		
		for ( int humans = 1; humans <= MaxClients; humans ++ )
			if ( loringlib_IsValidClient__PlayGame ( humans ) &&
				GetClientTeam ( humans ) == NONOHALE_TEAM_HUMAN )
					SDKHook ( humans, SDKHook_OnTakeDamage, Fleshpound_OnGivenDamage );
		g_iLastVictim = -1;
		g_bEnableRage = true;
		
		g_flHaleDefData[DEFDATA_SPEED] = loringlib_GetEntityLaggedmovement ( client );
		g_flHaleDefData[DEFDATA_GRAVITY] = GetEntityGravity ( client );
		
		loringlib_SetEntityLaggedmovement ( client, RAGE_SPEED );
		SetEntityGravity ( client, RAGE_GRAVITY );
		
		loringlib_ShowShakeMessage ( client, 35.0, 2.0 );
		
		qufnrTools_DisplayProgressBarTime ( client, RoundFloat ( RAGE_DURATION ) );
		g_hRageDurationHandle = CreateTimer ( RAGE_DURATION, Fleshpound_RageDurationEnd, GetClientUserId ( client ), TIMER_FLAG_NO_MAPCHANGE );
		
		float flOrigin[3];
		GetClientAbsOrigin ( client, flOrigin );
		flOrigin[2] += 2.0;
		g_iRageParticleEnt = loringlib_CreateParticleEx ( client, 0, flOrigin, NULL_VECTOR, PTC_FX_RAGE_ENABLE, true, RAGE_DURATION );
	}
	
	return Plugin_Continue;
}

public Action Fleshpound_OnGivenDamage ( int victim, int& attacker, int& inflictor, float& damage, int& damagetype, int& weapon, float damageForce[3], float damagePosition[3], int damagecustom ) {
	if ( loringlib_IsValidClient ( victim ) && loringlib_IsValidClient ( attacker ) &&
		GetClientTeam ( victim ) != GetClientTeam ( attacker ) ) {
		if ( NONOHALE_IsClientHale ( attacker ) && NONOHALE_GetClientHaleIndex ( attacker ) == MAIN_BOSS_INDEX &&
			g_iLastVictim == -1 ) {
			if ( weapon > -1 ) {
				char szWeapon[32];
				qufnrTools_GetWeaponClassname ( weapon, szWeapon, sizeof szWeapon );
				
				if ( StrContains ( szWeapon, "knife" ) != -1 ) {
					g_iLastVictim = victim;
					
					loringlib_CreateParticleEx ( attacker, 0, damagePosition, NULL_VECTOR, PTC_FX_ATTACK );
					
					char szSnd[256];
					Format ( szSnd, sizeof szSnd, SND_FX_ATTACK, GetRandomInt ( 1, 2 ) );
					if ( !IsSoundPrecached ( szSnd ) )
						PrecacheSound ( szSnd );
					EmitSoundToAll ( szSnd, _, _, _, _, _, _, _, damagePosition );
					
					if ( IsValidEdict ( g_iRageParticleEnt ) ) {
						AcceptEntityInput ( g_iRageParticleEnt, "kill" );
						g_iRageParticleEnt = -1;
					}
					
					Fleshpound_RageDurationEnd ( null, GetClientUserId ( attacker ) );
					
					damage *= 346.0;
					return Plugin_Changed;
				}
			}
		}
		else {
			AllUnhook ();
			return Plugin_Continue;
		}
	}
	
	return Plugin_Continue;
}

public Action Fleshpound_RageDurationEnd ( Handle hndl, any userid ) {
	int client = GetClientOfUserId ( userid );
	
	if ( loringlib_IsValidClient ( client ) ) {
		qufnrTools_KillProgressBarTime ( client );
		
		g_bEnableRage = false;
		g_iLastVictim = -1;
		g_iRageParticleEnt = -1;
		
		AllUnhook ();
		
		loringlib_SetEntityLaggedmovement ( client, g_flHaleDefData[DEFDATA_SPEED] );
		if ( g_flHaleDefData[DEFDATA_GRAVITY] <= 2.0 )
			SetEntityGravity ( client, g_flHaleDefData[DEFDATA_GRAVITY] );
		else
			SetEntityGravity ( client, 0.95 );
		
		g_flHaleDefData[DEFDATA_SPEED] = 1.0;
		g_flHaleDefData[DEFDATA_GRAVITY] = 1.0;
	}
	
	if ( hndl == null )
		KillTimer ( g_hRageDurationHandle );
	
	g_hRageDurationHandle = null;
	return Plugin_Stop;
}