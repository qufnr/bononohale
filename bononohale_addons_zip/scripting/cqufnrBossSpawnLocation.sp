#include <loringlib>
#include <qufnrtools>
#include <bononohale>


float g_flSpawnLocation[3];
float g_flSpawnAngles[3];

Handle g_hHudSync = null;

#define PTC_FX_SPAWN_LOCATION	"boss_spawn"
#define PTC_FX_SPAWN_EXPLODE	"boss_spawn_explode"

public void OnPluginStart () {
	RegAdminCmd ( "sm_bossspawn", onCommandSetBossSpawn, ADMFLAG_ROOT, "보스 스폰 위치를 설정합니다." );
	LoadTranslations ( "playeroptioncontroller.phrases.txt" );
}

public void OnMapStart () {

	loringlib_PrecacheParticle ( "particles/qufnr/new/fx_boss_spawn.pcf", true );
	loringlib_PrecacheParticleName ( PTC_FX_SPAWN_LOCATION );
	loringlib_PrecacheParticleName ( PTC_FX_SPAWN_EXPLODE );

	g_hHudSync = CreateHudSynchronizer ();

	g_flSpawnAngles = NULL_VECTOR;
	g_flSpawnLocation = NULL_VECTOR;
	
	char szPath[256];
	BuildPath ( Path_SM, szPath, sizeof szPath, "data/bossspawn.txt" );
	
	KeyValues kv = new KeyValues ( "data" );
	if ( kv.ImportFromFile ( szPath ) ) {
		kv.GotoFirstSubKey ();
		char szLevelname[2][128];
		GetCurrentMap ( szLevelname[0], sizeof szLevelname[] );
		do { 
			kv.GetSectionName ( szLevelname[1], sizeof szLevelname[] );
			if ( StrEqual ( szLevelname[0], szLevelname[1] ) ) {
				kv.GetVector ( "spawn", g_flSpawnLocation );
				kv.GetVector ( "angles", g_flSpawnAngles );
				
				break;
			}
		} while ( kv.GotoNextKey () );
	}
	else {
		if ( kv.JumpToKey ( "Map name", true ) ) {
			kv.SetString ( "spawn", "Spawn Location vectors." );
			kv.SetString ( "angles", "Spawn Angle vectors." );
			
			kv.Rewind ();
			
			kv.ExportToFile ( szPath );
		}
	}
	
	delete kv;
	
	
}

public void OnMapEnd () {
	for ( int i = 1; i <= MaxClients; i ++ )
		ClearSyncHud ( i, g_hHudSync );
}

public Action onCommandSetBossSpawn ( int client, int args ) {
	if ( client > 0 ) {
		if ( IsPlayerAlive ( client ) ) {
			if ( GetEntityFlags ( client ) & FL_ONGROUND ) {
				char szPath[256];
				BuildPath ( Path_SM, szPath, sizeof szPath, "data/bossspawn.txt" );
				
				KeyValues kv = new KeyValues ( "data" );
				if ( kv.ImportFromFile ( szPath ) ) {
					char szLevelname[128];
					GetCurrentMap ( szLevelname, sizeof szLevelname );
					
					if ( kv.JumpToKey ( szLevelname, true ) ) {
						GetClientAbsOrigin ( client, g_flSpawnLocation );
						GetClientAbsAngles ( client, g_flSpawnAngles );
						kv.SetVector ( "spawn", g_flSpawnLocation );
						kv.SetVector ( "angles", g_flSpawnAngles );
						
						kv.Rewind ();
						kv.ExportToFile ( szPath );
						
						PrintToChat ( client, "[SM] 현재 위치로 설정 했습니다." );
					}
				}
				
				delete kv;
			}
			else {
				PrintToChat ( client, "[SM] 공중에는 설정할 수 없습니다." );
				return Plugin_Handled;
			}
		}
		else {
			PrintToChat ( client, "[SM] 살아있는 상태에서 다시 시도하세요." );
			return Plugin_Handled;
		}
	}

	return Plugin_Handled;
}

public void NONOHALE_OnPreSetHaleClient ( int haleClient ) {

	if ( !validData () || loringlib_GetPlayGameClients () <= 1 )
		return;
	
	SetEntProp ( haleClient, Prop_Data, "m_takedamage", DAMAGE_NO, 1 );
	
	loringlib_ShowFadeUserMessageEx ( haleClient, 300, 11000, FFADE_AUTO, { 0, 0, 0, 255 } );
	
	TeleportEntity ( haleClient, g_flSpawnLocation, g_flSpawnAngles, NULL_VECTOR );
	CreateTimer ( 0.2, setClientInvisible, haleClient, TIMER_FLAG_NO_MAPCHANGE );
	SetEntityMoveType ( haleClient, MOVETYPE_NONE );
}

public Action setClientInvisible ( Handle hndl, any haleClient ) {
	if ( loringlib_IsValidClient__PlayGame ( haleClient ) ) {
		loringlib_SetEntityRenderColor ( haleClient, 255, 255, 255, 0 );
		
		SetHudTextParams ( -1.0, -1.0, 15.0, 255, 255, 255, 255, 2, 0.2, 0.005, 0.5 );
		SetGlobalTransTarget ( haleClient );
		char szBuffer[1024], szDescription[512], szName[32];
		Format ( szBuffer, sizeof szBuffer, "%t", "hudtext_you_choose_boss" );
		if ( NONOHALE_GetChooseHaleIndex () > -1 ) {
			NONOHALE_GetHaleName ( NONOHALE_GetChooseHaleIndex (), szName, sizeof szName );
			NONOHALE_GetHaleDescription ( NONOHALE_GetChooseHaleIndex (), szDescription, sizeof szDescription );
			if ( szDescription[0] != EOS &&
				!StrEqual ( szDescription, "No description", false ) ) {
				Format ( szBuffer, sizeof szBuffer, "%s\n\n%t", szBuffer, "hudtext_boss_description", szName, szDescription );
				ReplaceString ( szBuffer, sizeof szBuffer, "{new}", "\n" );
			}
		}
		ShowSyncHudText ( haleClient, g_hHudSync, szBuffer );
		
		loringlib_CreateParticleEx ( haleClient, 0, g_flSpawnLocation, NULL_VECTOR, PTC_FX_SPAWN_LOCATION, false, float ( NONOHALE_GetCountdown () + 1 ) );
			
		loringlib_KillWeaponSlot ( haleClient );
	//	GivePlayerItem ( haleClient, "weapon_knife" );
	}
	
	return Plugin_Stop;
}

public void NONOHALE_OnHumanToHale ( int hale ) {
	if ( !validData () )
		return;
	
	float flOrigin[3], flTargetOrigin[3];
	GetClientAbsOrigin ( hale, flOrigin );
	
	for ( int c = 1; c <= MaxClients; c ++ ) {
		if ( loringlib_IsValidClient__PlayGame ( c ) && hale != c ) {
			GetClientAbsOrigin ( c, flTargetOrigin );
			if ( GetVectorDistance ( flOrigin, flTargetOrigin ) <= 300.0 ) {
				loringlib_KnockbackToClient ( hale, c, 800.0, true );
				loringlib_MakeDamage ( hale, c, 346, "weapon_knife", DMG_SHOCK );
			}
		}
	}
	
	SetEntProp ( hale, Prop_Data, "m_takedamage", DAMAGE_YES, 1 );
	
//	loringlib_ShowFadeUserMessageEx ( hale, 100, 50, FFADE_OUT, { 0, 0, 0, 0 } );
	loringlib_SetEntityRenderColor ( hale, 255, 255, 255, 255 );
	SetEntityMoveType ( hale, MOVETYPE_WALK );
	
	loringlib_CreateParticleEx ( hale, 0, flOrigin, NULL_VECTOR, PTC_FX_SPAWN_EXPLODE, false, 10.0 );
}


stock bool validData () {
	return !( g_flSpawnLocation[0] == 0.0 && g_flSpawnLocation[1] == 0.0 && g_flSpawnLocation[2] == 0.0 );
}