
#define		SND_FX_PLAYER_LAST_SURVIVE	"qufnr/new/countdown/announcer_am_lastmanalive0%d.mp3"

public void OnClientPostAdminCheck ( int player ) {
	
	SDKHook ( player, SDKHook_PostThinkPost, PLAYER_OnPostThinkPost );
	SDKHook ( player, SDKHook_StartTouch, BASESKILL_OnStartTouch );
	SDKHook ( player, SDKHook_OnTakeDamage, PLAYER_OnTakeDamage );
	SDKHook ( player, SDKHook_GroundEntChangedPost, PLAYER_OnGroundChanged );
	WEAPON_ConnectOnWeaponHooking ( player );
	
	//	Reset player data
	g_PGameData[player].hale = false;
	g_PGameData[player].haleIndex = -1;
	g_PGameData[player].haleRage = false;
	g_PGameData[player].haleRageDmgs = 0;
	
	g_PGameData[player].jumpReady = true;
	g_PGameData[player].weightDownReady = true;
	
	g_PGameData[player].queuePoints = 0;
	
	g_PGameData[player].stun = false;
	
	PLAYER_ReadOptionData ( player );
}

public void OnClientDisconnect ( int player ) {
	INFECT_HaleDisconnectOnGameEnd ( player );
	
	SDKUnhook ( player, SDKHook_PostThinkPost, PLAYER_OnPostThinkPost );
	SDKUnhook ( player, SDKHook_OnTakeDamage, PLAYER_OnTakeDamage );
	SDKUnhook ( player, SDKHook_GroundEntChangedPost, PLAYER_OnGroundChanged );
	WEAPON_ConnectOnWeaponHooking ( player, false );
	
	PLAYER_WriteOptionData ( player );
}

void PLAYER_ReadOptionData ( int player ) {
	if ( !IsFakeClient ( player ) ) {
		char szPath[256];
		char szSteam[64];
		GetClientAuthId ( player, AuthId_SteamID64, szSteam, sizeof szSteam );
		BuildPath ( Path_SM, szPath, sizeof szPath, "data/bononohale/players/%s.txt", szSteam );
		
		KeyValues kv = new KeyValues ( "options" );
		if ( kv.ImportFromFile ( szPath ) ) {
			g_POptions[player].musicToggle = kv.GetNum ( "togglemusic" );
			g_POptions[player].displayDamages = kv.GetNum ( "displaydamages" );
			g_POptions[player].displayBossHealth = kv.GetNum ( "displaybosshealth" );
		} else {	//	Set default values
			kv.SetNum ( "togglemusic", 1 );
			kv.SetNum ( "displaydamages", 1 );
			kv.SetNum ( "displaybosshealth", 1 );
			
			g_POptions[player].musicToggle = 1;
			g_POptions[player].displayDamages = 1;
			g_POptions[player].displayBossHealth = 1;
			
			kv.ExportToFile ( szPath );
		}
		
		delete kv;
	}
}

void PLAYER_WriteOptionData ( int player ) {
	if ( !IsFakeClient ( player ) ) {
		char szPath[256];
		char szSteam[64];
		GetClientAuthId ( player, AuthId_SteamID64, szSteam, sizeof szSteam );
		BuildPath ( Path_SM, szPath, sizeof szPath, "data/bononohale/players/%s.txt", szSteam );
		
		KeyValues kv = new KeyValues ( "options" );
		if ( kv.ImportFromFile ( szPath ) ) {
			kv.SetNum ( "togglemusic", g_POptions[player].musicToggle );
			kv.SetNum ( "displaydamages", g_POptions[player].displayDamages );
			
			kv.ExportToFile ( szPath );
		}
		
		delete kv;
	}
}

public void PLAYER_OnGroundChanged ( int player ) {
	BASESKILL_WeightDownLanding ( player );
}

public void PLAYER_OnPostThinkPost ( int player ) {
	BASESKILL_FrameGoombaJump ( player );
	BASESKILL_OnHaleWeightDown ( player, GetClientButtons ( player ) );
	BASESKILL_OnHaleJump ( player, GetClientButtons ( player ) );
}

public Action PLAYER_OnTakeDamage ( int victim, int& attacker, int& inflictor, float& damage, int& damagetype, int& weapon, float damageForce[3], float damagePosition[3], int damagecustom ) {
	if ( loringlib_IsValidClient ( victim ) && 
		g_PGameData[victim].hale &&
		( damagetype & DMG_FALL ) )
		return Plugin_Handled;
	
	if ( loringlib_IsValidClient ( victim ) && loringlib_IsValidClient ( attacker ) ) {
		if ( GetClientTeam ( victim ) != GetClientTeam ( attacker ) ) {
			if ( g_PGameData[attacker].hale ) {
				damage = WEAPON_OnHaleKnifeDamage ( victim, attacker, damagetype, weapon, damage );
				return Plugin_Changed;
			}
		
			if ( g_PGameData[victim].hale ) {
				damage = WEAPON_OnDamageWeaponModify ( victim, attacker, weapon, damage, damagePosition );
				return Plugin_Changed;
			//	PrintHintText ( attacker, "Victim Tagging: %.2f", GetEntPropFloat ( victim, Prop_Send, "m_flVelocityModifier" ) );
			}
		}
	}
	
	return Plugin_Continue;
}

public Action OnPlayerRunCmd ( int player, int& keys, int& impulse, float vel[3], float angles[3], int& weapon, int& subtype, int& cmdnum, int& tickcount, int& seed, int mouse[2] ) {
	
	bool bChange = false;
	if ( loringlib_IsValidClient ( player ) ) {
		static bool bClientReload[MAXPLAYERS + 1];
	
		BASESKILL_OnHaleBlockSecondaryAttack ( player, keys );
		
		int iWeapon = loringlib_GetActiveWeapon ( player );
		
		//	Stun
		if ( IsPlayerAlive ( player ) && g_PGameData[player].stun ) {
			
			if ( iWeapon > 0 && IsValidEdict ( iWeapon ) &&
				HasEntProp ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" ) )
				SetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack", 99999999.9 );
			
			if ( ( keys & IN_ATTACK2 ) )
				keys &= ~IN_ATTACK2;
			
			bChange = true;
		}
		else {
			if ( iWeapon > 0 && IsValidEdict ( iWeapon ) &&
				HasEntProp ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" ) )
				if ( GetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" ) >= 99999999.9 )
					SetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack", GetGameTime () );
		}
		
		//	Reload
		if ( iWeapon != -1 ) {
			bool bReload = loringlib_IsWeaponReloading ( iWeapon );
			if ( !bReload &&
				HasEntProp ( iWeapon, Prop_Send, "m_reloadState" ) &&
				GetEntProp ( iWeapon, Prop_Send, "m_reloadState" ) > 0 )
				bReload = true;
			
			if ( bReload && !bClientReload[player] )
				WEAPON_ChangeWeaponReloadSpeed ( player );
			
			bClientReload[player] = bReload;
		}
	}
	
	return bChange ? Plugin_Changed : Plugin_Continue;
}

void PLAYER_SpawnOnSetData ( int player ) {
	g_PGameData[player].hale = false;
	g_PGameData[player].haleIndex = -1;
	g_PGameData[player].haleRage = false;
	g_PGameData[player].haleRageDmgs = 0;
	
	g_PGameData[player].jumpReady = true;
	
	g_PGameData[player].damages = 0;
	g_PGameData[player].attackDamages = 0;
	
	g_PGameData[player].stun = false;
	
	loringlib_SetEntityArmsModel ( player, "" );
	
	CreateTimer ( 0.1, PLAYER_SetSpawnLocation, GetClientUserId ( player ), TIMER_FLAG_NO_MAPCHANGE );
}

public Action PLAYER_SetSpawnLocation ( Handle hndl, any userid ) {
	int p = GetClientOfUserId ( userid );
	
	if ( loringlib_IsValidClient__PlayGame ( p ) ) {
		float flOrigin[3];
		GetClientAbsOrigin ( p, flOrigin );
		for ( int i = 0; i < sizeof flOrigin; i ++ )
			g_PGameData[p].spawnLocation[i] = flOrigin[i];
	}
}

void PLAYER_OnLastSurviveNotice () {
	if ( loringlib_GetTeamCount ( HUMAN_TEAM, true ) == 1 && loringlib_GetTeamCount ( HALE_TEAM, true ) > 0 ) {
		char szSnd[256];
		Format ( szSnd, sizeof szSnd, SND_FX_PLAYER_LAST_SURVIVE, GetRandomInt ( 1, 4 ) );
		if ( !IsSoundPrecached ( szSnd ) )
			PrecacheSound ( szSnd, true );
		
		EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER, SNDLEVEL_NONE, SNDCHAN_AUTO );
	}
}