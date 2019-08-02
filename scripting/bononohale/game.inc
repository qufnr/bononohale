

void GAME_RoundEndOnSetStatus () {
	GAME_SetDefaultStatus ();
	
	g_GameStatus.gameEnd = true;
}

public Action GAME_OnGlobalTimerThink ( Handle hndl ) {
	
	GAME_DisplayHaleDamage ();
	
	for ( int player = 1; player <= MaxClients; player ++ ) {
		if ( loringlib_IsValidClient ( player ) )  {
			BASESKILL_CooldownHudText ( player );
		}
	}
	
	return Plugin_Continue;
}

void GAME_SetDefaultStatus () {
	g_GameStatus.gameStart = false;
	g_GameStatus.gameEnd = false;
	g_GameStatus.gameFindHale = false;
	g_GameStatus.playerWaiting = false;
}

void GAME_KillFindTimer () {
	if ( g_hFindTimer != null ) {
		KillTimer ( g_hFindTimer );
		g_hFindTimer = null;
	}
}

void GAME_FindStart () {
	GAME_SetDefaultStatus ();
	
	g_GameStatus.gameFindHale = true;
	g_iChooseHaleIndex = -1;
	g_iHaleClient = GAME_GetRandomPlayer ();
	
	if ( loringlib_IsValidClient ( g_iHaleClient ) ) {
		if ( !IsFakeClient ( g_iHaleClient ) )
			qufnrTools_PrintToChat ( g_iHaleClient, "%t", "system_you_hale" );
		
		Call_StartForward ( g_hForwardHandle[FORWARD_ON_PRE_SET_HALE_CLIENT] );
		Call_PushCell ( g_iHaleClient );
		Call_Finish ();
	}
	
	for ( int player = 1; player <= MaxClients; player ++ ) {
		if ( loringlib_IsValidClient__PlayGame ( player ) )
			if ( GetClientTeam ( player ) != HUMAN_TEAM )
				CS_SwitchTeam ( player, HUMAN_TEAM );
	}
	
	if ( !g_ConfigData.ignoreFreezetime ) {
		ConVar hFreezeTime = FindConVar ( "mp_freezetime" );
		if ( GetConVarInt ( hFreezeTime ) > 0 ) {
			CreateTimer ( GetConVarFloat ( hFreezeTime ), GAME_FindStartPost, _, TIMER_FLAG_NO_MAPCHANGE );
			return;
		}
	}
	
	CreateTimer ( 0.0, GAME_FindStartPost, _, TIMER_FLAG_NO_MAPCHANGE );
}

public Action GAME_FindStartPost ( Handle hndl ) {

	g_iFindTime = g_ConfigData.findTime;
	
	if ( loringlib_GetPlayGameClients () < g_ConfigData.minPlayers )
		g_GameStatus.playerWaiting = true;
	
	if ( g_hFindTimer == null ) {
		g_hFindTimer = CreateTimer ( 1.0, GAME_FindCountDown, _, TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
		
		if ( g_hFindTimer != null ) {
			Call_StartForward ( g_hForwardHandle[FORWARD_ON_ROUND_START_COUNT_DOWN] );
			Call_PushCell ( g_iFindTime );
			Call_PushCell ( 1 );
			Call_Finish ();
		}
	}
	
	return Plugin_Stop;
}

public Action GAME_FindCountDown ( Handle hndl ) {
	
	SetHudTextParams ( -1.0, 0.25, 1.1, 255, 255, 255, 255, 0, 0.0, 0.0, 0.0 );
	
	if ( g_GameStatus.playerWaiting ) {
		
		loringlib_ShowSyncHudTextToAll ( g_hHudSyncHndl[HUDSYNC_FIND_TEXT], "%t", "hudtext_wait_player" );
		
		if ( loringlib_GetPlayGameClients () >= g_ConfigData.minPlayers ) {
			g_GameStatus.gameFindHale = false;
			g_GameStatus.playerWaiting = false;
			CS_TerminateRound ( g_ConfigData.roundEndDelay, CSRoundEnd_GameStart, true );
			
			SetHudTextParams ( -1.0, 0.25, 2.5, 255, 255, 255, 255, 0, 0.0, 0.0, 0.0 );
			loringlib_ShowSyncHudTextToAll ( g_hHudSyncHndl[HUDSYNC_FIND_TEXT], "%t", "hudtext_gamestart" );
			
			g_hFindTimer = null;
			return Plugin_Stop;
		}
		
		return Plugin_Continue;
	}
	
	if ( loringlib_GetPlayGameClients () <= 1 ) {
		g_GameStatus.gameFindHale = false;
		CS_TerminateRound ( g_ConfigData.roundEndDelay, CSRoundEnd_Draw, true );
		
		g_hFindTimer = null;
		return Plugin_Stop;
	}
	
	if ( !loringlib_IsValidClient ( g_iHaleClient ) || !IsPlayerAlive ( g_iHaleClient ) ) {
		g_iHaleClient = -1;
		
		CS_TerminateRound ( g_ConfigData.roundEndDelay, CSRoundEnd_Draw, true );
		
		g_hFindTimer = null;
		return Plugin_Stop;
	}
	
	loringlib_ShowSyncHudTextToAll ( g_hHudSyncHndl[HUDSYNC_FIND_TEXT], "%t", "hudtext_find_hale", g_iFindTime );
	g_iFindTime --;
	
	if ( g_iChooseHaleIndex == -1 ) {
		if ( 7 == g_iFindTime ) {
			g_iChooseHaleIndex = GetRandomInt ( 1, g_iHaleData ) - 1;
			SOUND_OnPlayHaleCountdownIntroSound ( g_iChooseHaleIndex );
			PrintToServer ( "g_iChooseHaleIndex: %d", g_iChooseHaleIndex );
		}
	}
	
	Call_StartForward ( g_hForwardHandle[FORWARD_ON_ROUND_START_COUNT_DOWN] );
	Call_PushCell ( g_iFindTime );
	Call_PushCell ( 0 );
	Call_Finish ();
	
	if ( g_iFindTime <= 0 ) {
		g_hFindTimer = CreateTimer ( 1.5, GAME_FindCountDownEnd, _, TIMER_FLAG_NO_MAPCHANGE );
		return Plugin_Stop;
	}
	
	return Plugin_Continue;
}

public Action GAME_FindCountDownEnd ( Handle hndl ) {
	g_GameStatus.gameFindHale = false;
	g_GameStatus.gameStart = true;
	
	INFECT_HumanToHale ( g_iHaleClient, g_iChooseHaleIndex );
	
	if ( !loringlib_IsValidClient__PlayGame ( g_iHaleClient ) ) {
		GAME_SetDefaultStatus ();
		
		CS_TerminateRound ( g_ConfigData.roundEndDelay, CSRoundEnd_Draw, true );
		
		g_hFindTimer = null;
		return Plugin_Stop;
	}
	
	SOUND_OnPlayHaleMusic ( g_iChooseHaleIndex );
	
	for ( int players = 1; players <= MaxClients; players ++ )
		if ( loringlib_IsValidClient__PlayGame ( players ) )
			if ( g_iHaleClient != players )
				CS_SwitchTeam ( players, HUMAN_TEAM );
	
	SetHudTextParams ( -1.0, 0.25, 3.0, 255, 255, 255, 255, 0, 0.0, 0.0, 0.0 );
	loringlib_ShowSyncHudTextToAll ( g_hHudSyncHndl[HUDSYNC_FIND_TEXT], "%t", "hudtext_who_are_hale", g_iHaleClient, g_HaleData[g_iChooseHaleIndex].name );
	
	Call_StartForward ( g_hForwardHandle[FORWARD_ON_ROUND_START_COUNT_DOWN_LAST_TICK] );
	Call_PushCell ( g_iHaleClient );
	Call_Finish ();
	
	g_hFindTimer = null;
	return Plugin_Stop;
}

void GAME_DisplayHaleDamage () {
	char szDamage[16];
	char szHaleRageData[2][16], szHaleHealthData[2][16];
	char szHudFormat[256];
	char szHudHaleHealth[256];
	
	int iSpectatorData[2];
	
	for ( int p = 1; p <= MaxClients; p ++ ) {
		if ( loringlib_IsValidClient ( p ) && !IsFakeClient ( p ) ) {
			SetGlobalTransTarget ( p );
			
			if ( g_POptions[p].displayDamages ) {
				if ( !g_PGameData[p].hale || !IsPlayerAlive ( p ) ) {
					if ( !IsPlayerAlive ( p ) &&
						( loringlib_GetClientObserverMode ( p ) == CS_SPECMODE_FIRSTPERSON ||
						loringlib_GetClientObserverMode ( p ) == CS_SPECMODE_THIRDPERSON ) &&
						loringlib_IsValidClient__PlayGame ( loringlib_GetClientObserverTarget ( p ) ) ) {
						iSpectatorData[0] = loringlib_GetClientObserverTarget ( p );
						iSpectatorData[1] = g_PGameData[iSpectatorData[0]].attackDamages;
					}
					else {
						iSpectatorData[0] = p;
						iSpectatorData[1] = g_PGameData[p].attackDamages;
					}
					
					
					if ( iSpectatorData[0] != p && !IsFakeClient ( iSpectatorData[0] ) ) {
						loringlib_NumberFormat ( g_PGameData[iSpectatorData[0]].attackDamages, szDamage, sizeof szDamage );
						Format ( szHudFormat, sizeof szHudFormat, "%t", "hudtext_spec_haleinfo", iSpectatorData[0], szDamage );
						loringlib_NumberFormat ( g_PGameData[p].attackDamages, szDamage, sizeof szDamage );
						Format ( szHudFormat, sizeof szHudFormat, "%s\n%t", szHudFormat, "hudtext_haleinfo", szDamage );
					}
					else {
						loringlib_NumberFormat ( g_PGameData[p].attackDamages, szDamage, sizeof szDamage );
						Format ( szHudFormat, sizeof szHudFormat, "%t", "hudtext_haleinfo", szDamage );
					}
				}
				
				if ( g_PGameData[p].hale && g_PGameData[p].haleIndex > -1 ) {
					loringlib_NumberFormat ( g_PGameData[p].haleRageDmgs, szHaleRageData[0], sizeof szHaleRageData[] );
					loringlib_NumberFormat ( g_HaleData[g_PGameData[p].haleIndex].rageRange, szHaleRageData[1], sizeof szHaleRageData[] );
					Format ( szHudFormat, sizeof szHudFormat, "%t", "hudtext_haleinfo_by_hale", szHaleRageData[0], szHaleRageData[1], loringlib_GetPercentage ( g_PGameData[p].haleRageDmgs, g_HaleData[g_PGameData[p].haleIndex].rageRange ) );
				}
				
				qufnrTools_FormatEscapeSequence ( szHudFormat, sizeof szHudFormat );
				SetHudTextParams ( -1.0, 0.866, 0.2, 255, 113, 113, 255, 0, 0.0, 0.0, 0.0 );
				ShowSyncHudText ( p, g_hHudSyncHndl[HUDSYNC_HALE_DAMAGE], szHudFormat );
			}
			
			if ( g_GameStatus.gameStart ) {
			
				if ( g_ConfigData.displayBossHealth ) {
					if ( g_POptions[p].displayBossHealth ) {
						if ( loringlib_IsValidClient ( g_iHaleClient ) ) {
							if ( g_iHaleClient == p )
								continue;
						
							if ( IsPlayerAlive ( g_iHaleClient ) ) {
								loringlib_NumberFormat ( loringlib_GetEntityHealth ( g_iHaleClient ), szHaleHealthData[0], sizeof szHaleHealthData[] );
								loringlib_NumberFormat ( loringlib_GetEntityMaxHealth ( g_iHaleClient ), szHaleHealthData[1], sizeof szHaleHealthData[] );
								Format ( szHudHaleHealth, sizeof szHudHaleHealth, "%t", "hudtext_hale_health", g_HaleData[g_iChooseHaleIndex].name, szHaleHealthData[0], szHaleHealthData[1] );
							} else
								Format ( szHudHaleHealth, sizeof szHudHaleHealth, "%t", "hudtext_hale_death" );
							qufnrTools_FormatEscapeSequence ( szHudHaleHealth, sizeof szHudHaleHealth );
							SetHudTextParams ( -1.0, 0.07, 0.15, 0, 255, 0, 255, 0, 0.0, 0.0, 0.0 );
							ShowSyncHudText ( p, g_hHudSyncHndl[HUDSYNC_HALE_HEALTH], szHudHaleHealth );
						}
					}
				}
			}
		}
	}
}

stock int GAME_GetRandomPlayer () {
	if ( GetClientCount () <= 1 )
		return -1;

	int player = -1;
	
	while ( true ) {
	//	player = loringlib_GetRandomAlive ( GetRandomInt ( 1, loringlib_GetPlayGameClients () ) );

		player = getClientWithMostQueuePoints ();

		if ( !loringlib_IsValidClient__PlayGame ( player ) )
			continue;
		
	//	INFECT_HumanToHale ( player, g_iChooseHaleIndex );
		break;
	}
	
	return player;
}

public Action CS_OnTerminateRound ( float& delay, CSRoundEndReason& reason ) {
	if ( g_GameStatus.playerWaiting && loringlib_GetPlayGameClients () == 1 )
		return Plugin_Handled;
	
	return Plugin_Continue;
}