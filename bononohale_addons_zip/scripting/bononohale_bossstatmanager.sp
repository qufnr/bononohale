#include <loringlib>
#include <qufnrtools>
#include <bononohale>


public Plugin myinfo = {
	name = "bononohale boss stat manager"
	, author = "qufnr"
	, description = ""
	, url = ""
	, version = "1.0"
};

enum struct BossStats {
	float healthMultiple;
}

BossStats g_BossStats;

public void OnPluginStart () {
	RegAdminCmd ( "sm_bossstat", onCommandSetBossStats, ADMFLAG_ROOT, "Set boss stats." );
}

public void OnMapStart () {
	char szMap[2][256];
	GetCurrentMap ( szMap[0], sizeof szMap[] );
	char szPath[256];
	BuildPath ( Path_SM, szPath, sizeof szPath, "data/bononohalebossmanager.txt" );
	KeyValues kv = new KeyValues ( "boss_stat_manager" );
	
	if ( kv.ImportFromFile ( szPath ) ) {
		kv.GotoFirstSubKey ();
		do {
			kv.GetSectionName ( szMap[1], sizeof szMap[] );
			if ( StrEqual ( szMap[0], szMap[1], false) ) {
				g_BossStats.healthMultiple = kv.GetFloat ( "health_multiple" );
			}
		} while ( kv.GotoNextKey () );
	} else {
		if ( kv.JumpToKey ( "Map name", true ) ) {
			kv.SetString ( "health_multiple", "health multiple value for bosses." );
			
			kv.Rewind ();
			kv.ExportToFile ( szPath );
		}
	}
	
	delete kv;
}

public void NONOHALE_OnHumanToHale ( int client, int haleIndex ) {
	if ( g_BossStats.healthMultiple <= 0 )
		return;
	loringlib_SetEntityMaxHealth ( client, RoundFloat ( float ( loringlib_GetEntityMaxHealth ( client ) ) * g_BossStats.healthMultiple ) );
	loringlib_SetEntityHealth ( client, loringlib_GetEntityMaxHealth ( client ) );
}

public Action onCommandSetBossStats ( int client, int args ) {
	if ( client > 0 ) {
		if ( args < 2 ) {
			ReplyToCommand ( client, " \x04[Boss Stat] \x01사용법: sm_bosstat <type> <value>" );
			return Plugin_Handled;
		}
		
		char szPath[256];
		BuildPath ( Path_SM, szPath, sizeof szPath, "data/bononohalebossmanager.txt" );
		KeyValues kv = new KeyValues ( "boss_stat_manager" );
		if ( !kv.ImportFromFile ( szPath ) ) {
			ReplyToCommand ( client, " \x04[Boss Stat] \x01파일을 찾을 수 없음." );
			delete kv;
			return Plugin_Handled;
		}
		
		
		char szMap[256];
		GetCurrentMap ( szMap, sizeof szMap );
		if ( kv.JumpToKey ( szMap, true ) ) {
			char szType[32];
			GetCmdArg ( 1, szType, sizeof szType );
			
			char szValue[16];
			any aData;
			GetCmdArg ( 2, szValue, sizeof szValue );
			
			if ( StrEqual ( szType, "healthmul", false ) ) {
				aData = StringToFloat ( szValue );
				
				if ( aData > 0 ) {
					
					g_BossStats.healthMultiple = aData;
					kv.SetFloat ( "health_multiple", aData );
					
					kv.Rewind ();
					kv.ExportToFile ( szPath );
					
					PrintToChat ( client, " \x04[Boss Stat] \x01health_multiple을 %.2f로 설정했습니다.", aData );
				}
			}
			else {
				PrintToChat ( client, " \x04[Boss Stat] \x01알 수 없는 타입." );
			}
		}
		
		delete kv;
	}
	
	return Plugin_Handled;
}