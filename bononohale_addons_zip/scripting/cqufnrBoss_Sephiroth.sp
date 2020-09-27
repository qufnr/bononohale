#include <loringlib>
#include <qufnrtools>
#include <bononohale>

public Plugin myinfo = {
	name = "Boss Sephiroth"
	, author = "qufnr & nubi"
	, description = "bononohale on Sephiroth Boss"
	, version = "1.0"
	, url = "https://steamcommunity.com/id/qufnrp"
};

#define MAIN_BOSS_INDEX		3

#define ATTACK_DISTANCE		180.0
#define ATTACK_DAMAGE		GetRandomFloat ( 30.0, 40.0 )

#define SLASH_DURATION		5.0
#define SLASH_THROW_SPEED	1250.0
#define SLASH_DAMAGE		500

#define PTC_FX_ATTACK		"sephiroth_primary"
#define PTC_FX_ATTACK2		"sephiroth_primary2"
#define PTC_FX_SLASH		"sephiroth_strike"
#define PTC_FX_SLASH_START	"sephiroth_strikecharge"

#define SLASH_COLLISION_MODEL	"models/props/de_venice/clock_tower_trim/clock_tower_trim_small_128.mdl"

#define SND_FX_RAGE_KILL	"qufnr/new/hale_class/sephiroth/ragekill%d.mp3"	//	1 2
#define SND_FX_LAST_ONE		"qufnr/new/hale_class/sephiroth/lastone1.mp3"
#define SND_FX_SLASH_IMPACT	"qufnr/new/hale_class/sephiroth/impact%d.mp3"	//	1 3

/**
 * RADIUS, DISTANCE, DAMAGE SCALE
 */
#define RADIUS_DAMAGE_CALC(%1,%2,%3)		Sine ( ( ( %1 - %2 ) / %1 ) * ( FLOAT_PI / 2 ) ) * %3

bool g_bRageEnable = false;

public void OnPluginStart () {
	HookEvent ( "weapon_fire", on_weapon_fire );
	HookEvent ( "round_start", on_round_start );
	HookEvent ( "player_death", on_player_death );
}

public void OnMapStart () {
	loringlib_PrecacheParticle ( "particles/qufnr/new/boss/fx_sephiroth.pcf", true );
	loringlib_PrecacheParticleName ( PTC_FX_ATTACK );
	loringlib_PrecacheParticleName ( PTC_FX_ATTACK2 );
	loringlib_PrecacheParticleName ( PTC_FX_SLASH );
	loringlib_PrecacheParticleName ( PTC_FX_SLASH_START );
	
	PrecacheModel ( SLASH_COLLISION_MODEL, true );
}

public void on_player_death ( Event ev, const char[] name, bool dontBroadcast ) {
	int client = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	int attacker = GetClientOfUserId ( ev.GetInt ( "attacker" ) );
	char szWeapon[32];
	ev.GetString ( "weapon", szWeapon, sizeof szWeapon );
	
	if ( loringlib_IsValidClient ( client ) && loringlib_IsValidClient ( attacker ) ) {
		if ( NONOHALE_IsClientHale ( attacker ) && NONOHALE_GetClientHaleIndex ( attacker ) == MAIN_BOSS_INDEX ) {
			if ( StrContains ( szWeapon, "knife", false ) != -1 ) {
				if ( g_bRageEnable ) {
					char szSnd[256];
					Format ( szSnd, sizeof szSnd, SND_FX_RAGE_KILL, GetRandomInt ( 1, 2 ) );
					if ( !IsSoundPrecached ( szSnd ) )
						PrecacheSound ( szSnd );
					EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER, SNDCHAN_WEAPON );
				}
			}
		}
	}
	
	if ( NONOHALE_GetHaleIndex () == MAIN_BOSS_INDEX ) {
		if ( loringlib_GetTeamCount ( NONOHALE_TEAM_HUMAN, true ) == 1 ) {
			if ( !IsSoundPrecached ( SND_FX_LAST_ONE ) )
				PrecacheSound ( SND_FX_LAST_ONE );
			EmitSoundToAll ( SND_FX_LAST_ONE, SOUND_FROM_PLAYER, SNDCHAN_AUTO );
		}
	}
}

public void on_round_start ( Event ev, const char[] name, bool dontBroadcast ) {
	g_bRageEnable = false;
}

public void on_weapon_fire ( Event ev, const char[] name, bool dontBroadcast ) {
	int client = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	char szWeapon[32];
	ev.GetString ( "weapon", szWeapon, sizeof szWeapon );
	if ( loringlib_IsValidClient ( client ) ) {
		if ( NONOHALE_IsClientHale ( client ) && NONOHALE_GetClientHaleIndex ( client ) == MAIN_BOSS_INDEX ) {
			
			if ( StrContains ( szWeapon, "knife", false ) != -1 ) {
				
				if ( g_bRageEnable )
					Sephiroth_ThrowSlash ( client );
			
				float flAbsOrigin[3], flTargetOrigin[3], flEyeAngles[3];
				float flDist;
				
				GetClientEyeAngles ( client, flEyeAngles );
				GetClientAbsOrigin ( client, flAbsOrigin );
				loringlib_AddInFrontOf ( flAbsOrigin, flEyeAngles, 50.0, flAbsOrigin );
				
				for ( int cl = 1; cl <= MaxClients; cl ++ ) {
					if ( loringlib_IsValidClient__PlayGame ( cl ) &&
						cl != client &&
						GetClientTeam ( cl ) == NONOHALE_TEAM_HUMAN ) {
						GetClientAbsOrigin ( cl, flTargetOrigin );
						flDist = GetVectorDistance ( flAbsOrigin, flTargetOrigin );
						if ( loringlib_IsTargetInSightRange ( client, cl, 180.0, ATTACK_DISTANCE ) ) {
							loringlib_MakeDamage ( client, cl, RoundFloat ( RADIUS_DAMAGE_CALC ( ATTACK_DISTANCE, flDist, ATTACK_DAMAGE ) ), "prop_exploding_barrel", DMG_SHOCK );
						}
					}
				}
				
				float flEyePos[3];
				GetClientEyePosition ( client, flEyePos );
				loringlib_AddInFrontOf ( flEyePos, flEyeAngles, 50.0, flEyePos );
				
				loringlib_CreateParticleEx ( client, 0, flEyePos, flEyeAngles, GetRandomInt ( 0, 1 ) ? PTC_FX_ATTACK : PTC_FX_ATTACK2 );
			}
		}
	}
}

public Action NONOHALE_OnHaleRage ( int client, int haleIndex, float& distance, float& duration ) {
	if ( haleIndex == MAIN_BOSS_INDEX ) {
		if ( g_bRageEnable )
			return Plugin_Handled;
		if ( !g_bRageEnable ) {
			float flPos[3];
			GetClientAbsOrigin ( client, flPos );
			flPos[2] += 35.0;
			loringlib_CreateParticleEx ( client, 0, flPos, NULL_VECTOR, PTC_FX_SLASH_START, true, 5.0 );
		
			g_bRageEnable = true;
			qufnrTools_DisplayProgressBarTime ( client, RoundFloat ( SLASH_DURATION ) );
			CreateTimer ( SLASH_DURATION, Sephiroth_OnToggleRage, GetClientUserId ( client ), TIMER_FLAG_NO_MAPCHANGE );
		}
	}
	
	return Plugin_Continue;
}

public Action Sephiroth_OnToggleRage ( Handle hndl, any userid ) {
	int client = GetClientOfUserId ( userid );
	
	if ( loringlib_IsValidClient ( client ) )
		qufnrTools_DisplayProgressBarTime ( client, 0 );
	
	g_bRageEnable = false;
	return Plugin_Stop;
}

void Sephiroth_ThrowSlash ( int client ) {
	int entity = CreateEntityByName ( "decoy_projectile" );
	
	SDKHook ( entity, SDKHook_SpawnPost, Sephiroth_OnSpawnSlashEnt );
	
	DispatchSpawn ( entity );
	
	SetVariantString ( "OnUser1 !self:kill::5.0:-1" );
	AcceptEntityInput ( entity, "AddOutput" );
	AcceptEntityInput ( entity, "FireUser1" );
	
	float flAbsAngles[3], flOrigin[3];
	float flVelocity[3];
	GetClientEyePosition ( client, flOrigin );
	GetClientEyeAngles ( client, flAbsAngles );
	
	GetAngleVectors ( flAbsAngles, flVelocity, NULL_VECTOR, NULL_VECTOR );
	ScaleVector ( flVelocity, SLASH_THROW_SPEED );
	
	char szSnd[256];
	Format ( szSnd, sizeof szSnd, SND_FX_SLASH_IMPACT, GetRandomInt ( 1, 3 ) );
	if ( !IsSoundPrecached ( szSnd ) )
		PrecacheModel ( szSnd );
	EmitSoundToAll ( szSnd, SOUND_FROM_WORLD, SNDCHAN_WEAPON, SNDLEVEL_NORMAL, _, _, _, _, flOrigin );
	
	TeleportEntity ( entity, flOrigin, flAbsAngles, flVelocity );
	
	SetEntProp ( entity, Prop_Send, "m_usSolidFlags", FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetEntProp ( entity, Prop_Data, "m_nSolidType", SOLID_VPHYSICS );
	SetEntProp ( entity, Prop_Send, "m_CollisionGroup", COLLISION_GROUP_DEBRIS );
	
	loringlib_SetEntityOwner2 ( entity, client );
	
	float flEndPos[3], flEffAng[3];
	loringlib_GetEntityOriginEx ( entity, flEndPos );
	GetClientEyeAngles ( client, flEffAng )
	loringlib_CreateParticleEx ( entity, 0, flEndPos, flEffAng, PTC_FX_SLASH, true, 5.0 );
	
	SDKHook ( entity, SDKHook_EndTouchPost, Sephiroth_OnSlashTouchPost );
}

public void Sephiroth_OnSpawnSlashEnt ( int entity ) {
	SetEntityModel ( entity, SLASH_COLLISION_MODEL );
	SetEntityMoveType ( entity, MOVETYPE_FLY );
	loringlib_SetEntityRenderColor ( entity, 0, 0, 0, 0 );
}

public void Sephiroth_OnSlashTouchPost ( int entity, int toucher ) {
	if ( IsValidEntity ( entity ) ) {
		int iAttacker = loringlib_GetEntityOwner2 ( entity );
		if ( !loringlib_IsValidClient ( iAttacker ) )
			iAttacker = 0;
		
		if ( toucher > 0 ) {
			if ( loringlib_IsValidClient__PlayGame ( toucher ) &&
				GetClientTeam ( toucher ) == NONOHALE_TEAM_HUMAN ) {
				loringlib_MakeDamage ( iAttacker, toucher, SLASH_DAMAGE, "prop_exploding_barrel", DMG_SHOCK );
			}
		}
		
		//	World Touching
		if ( toucher <= 0 )
			AcceptEntityInput ( entity, "kill" );
	}
}