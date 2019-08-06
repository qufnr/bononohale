#define CVAR_VERSION_FLAGS	FCVAR_PLUGIN | FCVAR_SPONLY | FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_DONTRECORD
#define CVAR_DEFAULT_FLAGS	FCVAR_PLUGIN

ConVar g_cvarMinPlayers;
ConVar g_cvarHaleFindTime;
ConVar g_cvarIgnoreFreezeTime;
ConVar g_cvarGoombaHumanDamage;
ConVar g_cvarGoombaHaleDamage;
ConVar g_cvarGoombaSound;
ConVar g_cvarDisplayBossHealth;
ConVar g_cvarStartOnRespawnHale;
ConVar g_cvarDisplayRankOfDamage;

void CONFIG_ReadFile () {
	AutoExecConfig_SetFile ( "bononohale" );
	AutoExecConfig_CreateConVar ( "bononohale_version", PLUGIN_VERSION, "Version", CVAR_VERSION_FLAGS );
	
	g_cvarMinPlayers = AutoExecConfig_CreateConVar ( "bononohale_gamestart_minimum_players", "2", "게임 시작 전에 필요한 플레이어 수를 설정합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarHaleFindTime = AutoExecConfig_CreateConVar ( "bononohale_hale_find_time", "20", "헤일을 선택하는 시간을 설정합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarIgnoreFreezeTime = AutoExecConfig_CreateConVar ( "bononohale_ignore_freeze_time", "0", "프리즈 타임을 무시하고 헤일을 선택합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarGoombaHumanDamage = AutoExecConfig_CreateConVar ( "bononohale_goomba_human_damage", "500.0", "인간 밟기 피해량", CVAR_DEFAULT_FLAGS );
	g_cvarGoombaHaleDamage = AutoExecConfig_CreateConVar ( "bononohale_goomba_hale_damage", "1000.0", "헤일 밟기 피해량", CVAR_DEFAULT_FLAGS )
	g_cvarGoombaSound = AutoExecConfig_CreateConVar ( "bononohale_goomba_sound", "1", "밟기 사운드를 활성화합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarDisplayBossHealth = AutoExecConfig_CreateConVar ( "bononohale_display_boss_health", "1", "헤일 체력을 표시합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarStartOnRespawnHale = AutoExecConfig_CreateConVar ( "bononohale_start_respawn_hale", "0", "라운드가 시작 하면 헤일이 스폰으로 이동합니다.", CVAR_DEFAULT_FLAGS );
	g_cvarDisplayRankOfDamage = AutoExecConfig_CreateConVar ( "bononohale_display_rank_of_damage", "1", "라운드 종료 시 피해량 순위를 표시합니다.", CVAR_DEFAULT_FLAGS );
	
	g_cvarMinPlayers.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarHaleFindTime.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarIgnoreFreezeTime.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarGoombaHumanDamage.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarGoombaHaleDamage.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarGoombaSound.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarDisplayBossHealth.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarStartOnRespawnHale.AddChangeHook ( CONFIG_OnCvarChanged );
	g_cvarDisplayRankOfDamage.AddChangeHook ( CONFIG_OnCvarChanged );

	AutoExecConfig_ExecuteFile ();
	
	PrintToServer ( "======================================" );
	PrintToServer ( "bononohale Config loading..." );
	PrintToServer ( "======================================" );
}

public void CONFIG_OnCvarChanged ( ConVar cvar, const char[] oldValue, const char[] newValue ) {
	OnConfigsExecuted ();
}

public void OnConfigsExecuted () {
	g_ConfigData.minPlayers = GetConVarInt ( g_cvarMinPlayers );
	g_ConfigData.findTime = GetConVarInt ( g_cvarHaleFindTime );
	g_ConfigData.ignoreFreezetime = GetConVarInt ( g_cvarIgnoreFreezeTime );
	g_ConfigData.goombaHumanDamage = GetConVarFloat ( g_cvarGoombaHumanDamage );
	g_ConfigData.goombaHaleDamage = GetConVarFloat ( g_cvarGoombaHaleDamage );
	g_ConfigData.goombaSound = GetConVarBool ( g_cvarGoombaSound );
	g_ConfigData.displayBossHealth = GetConVarBool ( g_cvarDisplayBossHealth );
	g_ConfigData.startOnRespawnHale = GetConVarBool ( g_cvarStartOnRespawnHale );
	g_ConfigData.displayRankOfDamage = GetConVarBool ( g_cvarDisplayRankOfDamage );
	g_ConfigData.roundEndDelay = FindConVar ( "mp_round_restart_delay" ).FloatValue;
	
	PrintToServer ( "======================================" );
	PrintToServer ( "bononohale Config loaded." );
	PrintToServer ( "======================================" );
}

void CONFIG_ReadHaleData () {
	g_iHaleData = 0;

	char szBuild[256];
	BuildPath ( Path_SM, szBuild, sizeof szBuild, "data/bononohale/hale.txt" );
	
	KeyValues kv = new KeyValues ( "data" );
	
	if ( kv.ImportFromFile ( szBuild ) ) {
		kv.GotoFirstSubKey ();
		
		do {
			kv.GetSectionName ( g_HaleData[g_iHaleData].name, sizeof g_HaleData[].name );
			kv.GetString ( "playermodel", g_HaleData[g_iHaleData].model, sizeof g_HaleData[].model );
			if ( g_HaleData[g_iHaleData].model[0] != EOS )
				if ( !StrEqual ( g_HaleData[g_iHaleData].model, "null", false ) )
					if ( IsModelPrecached ( g_HaleData[g_iHaleData].model ) )
						PrecacheModel ( g_HaleData[g_iHaleData].model, true );
						
			kv.GetString ( "armsmodel", g_HaleData[g_iHaleData].armsModel, sizeof g_HaleData[].armsModel );
			if ( g_HaleData[g_iHaleData].armsModel[0] != EOS )
				if ( IsModelPrecached ( g_HaleData[g_iHaleData].armsModel ) )
					PrecacheModel ( g_HaleData[g_iHaleData].armsModel, true );
			
			kv.GetString ( "description", g_HaleData[g_iHaleData].description, sizeof g_HaleData[].description );
			
			g_HaleData[g_iHaleData].rageRange = kv.GetNum ( "ragerange" );
			
			g_HaleData[g_iHaleData].health = kv.GetNum ( "health" );
			g_HaleData[g_iHaleData].healthMath = kv.GetNum ( "healthmath" );
			g_HaleData[g_iHaleData].armor = kv.GetNum ( "armor" );
			
			g_HaleData[g_iHaleData].speed = kv.GetFloat ( "speed" );
			g_HaleData[g_iHaleData].gravity = kv.GetFloat ( "gravity" );
			
			g_HaleData[g_iHaleData].jumpVector = kv.GetFloat ( "jumpvec" );
			g_HaleData[g_iHaleData].jumpCooldown = kv.GetFloat ( "jumpcooldown" );
			g_HaleData[g_iHaleData].weightDownCooldown = kv.GetFloat ( "weightdowncooldown" );
			
			g_HaleData[g_iHaleData].knockbackImmunityScale = kv.GetFloat ( "knockbackImmunityScale" );
			
			g_HaleData[g_iHaleData].damageMultiple = kv.GetFloat ( "damage_multiple" );
			g_HaleData[g_iHaleData].attackSpeed = kv.GetFloat ( "attack_speed" );
			
			g_HaleData[g_iHaleData].distOfRage = kv.GetFloat ( "distance_of_rage" );
			if ( kv.GetFloat ( "distance_of_rage" ) < 0 )
				g_HaleData[g_iHaleData].distOfRage = 0.0;
			g_HaleData[g_iHaleData].durationOfRage = kv.GetFloat ( "duration_of_rage" );
			if ( kv.GetFloat ( "duration_of_rage" ) <= 0 )
				g_HaleData[g_iHaleData].durationOfRage = 5.0;
			
			//	Voice Sounds
			char szKeyName[32];
			for ( int i = 0; i < MAX_SND_TYPE; i ++ ) {
				for ( int x = 0; x < MAX_HALE_SOUNDS; x ++ ) {
					switch ( i ) {
						case HALE_SND_INDEX_DEATH:		Format ( szKeyName, sizeof szKeyName, "snd_death_%d", x + 1 );
						case HALE_SND_INDEX_INTRO:		Format ( szKeyName, sizeof szKeyName, "snd_intro_%d", x + 1 );
						case HALE_SND_INDEX_JUMP:		Format ( szKeyName, sizeof szKeyName, "snd_jump_%d", x + 1 );
						case HALE_SND_INDEX_KILL:		Format ( szKeyName, sizeof szKeyName, "snd_kill_%d", x + 1 );
						case HALE_SND_INDEX_PAIN:		Format ( szKeyName, sizeof szKeyName, "snd_pain_%d", x + 1 );
						case HALE_SND_INDEX_RAGE:		Format ( szKeyName, sizeof szKeyName, "snd_rage_%d", x + 1 );
						case HALE_SND_INDEX_VICTORY:	Format ( szKeyName, sizeof szKeyName, "snd_victory_%d", x + 1 );
					}
					
					kv.GetString ( szKeyName, g_szHaleSoundData[g_iHaleData][i][x], sizeof g_szHaleSoundData[][][] );
					if ( g_szHaleSoundData[g_iHaleData][i][x][0] != EOS )
						if ( !IsSoundPrecached ( g_szHaleSoundData[g_iHaleData][i][x] ) )
							PrecacheSound ( g_szHaleSoundData[g_iHaleData][i][x], true );
				}
			}
			
			//	Music Sounds
			for ( int i = 0; i < MAX_HALE_MUSIC; i ++ ) {
				Format ( szKeyName, sizeof szKeyName, "music_%d", i + 1 );
				
				kv.GetString ( szKeyName, g_szHaleMusic[g_iHaleData][i], sizeof g_szHaleMusic[][] );
				if ( g_szHaleMusic[g_iHaleData][i][0] != EOS )
					if ( !IsSoundPrecached ( g_szHaleMusic[g_iHaleData][i] ) )
						PrecacheSound ( g_szHaleMusic[g_iHaleData][i], true );
			}
			
			g_iHaleData ++;
		}
		
		while ( kv.GotoNextKey () );
	}
	
	delete kv;
	
	if ( g_iHaleData < 1 ) {
		SetFailState ( "Hale data Not found. (File: %s)", szBuild );
		return;
	}
}