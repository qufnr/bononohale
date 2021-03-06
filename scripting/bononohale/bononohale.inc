

public APLRes AskPluginLoad2 ( Handle myself, bool late, char[] error, int err_max ) {
	
	CreateNative ( "NONOHALE_GetCountdown", Native_GetCountdown );
	
	CreateNative ( "NONOHALE_IsGameStart", Native_IsGameStart );
	CreateNative ( "NONOHALE_IsGameEnd", Native_IsGameEnd );
	CreateNative ( "NONOHALE_IsGamePlayerWaiting", Native_IsGamePlayerWaiting );
	CreateNative ( "NONOHALE_IsGameFindHale", Native_IsGameFindHale );
	CreateNative ( "NONOHALE_IsGameWarmup", Native_IsGameWarmup );
	
	CreateNative ( "NONOHALE_IsClientHale", Native_IsClientHale );
	CreateNative ( "NONOHALE_GetHaleIndex", Native_GetHaleIndex );
	CreateNative ( "NONOHALE_GetHaleDamages", Native_GetHaleDamages );
	CreateNative ( "NONOHALE_GetClientHaleIndex", Native_GetClientHaleIndex );
	CreateNative ( "NONOHALE_IsClientStun", Native_IsClientStun );
	CreateNative ( "NONOHALE_GetPreFindHaleClient", Native_GetPreFindHaleClient );
	CreateNative ( "NONOHALE_GetChooseHaleIndex", Native_GetChooseHaleIndex );
	CreateNative ( "NONOHALE_GetClientSoundToggle", Native_GetClientSoundToggle );
	
	CreateNative ( "NONOHALE_GetDamages", Native_GetDamages );
	CreateNative ( "NONOHALE_SetDamages", Native_SetDamages );
	CreateNative ( "NONOHALE_AddDamages", Native_AddDamages );
	
	CreateNative ( "NONOHALE_GetClientQueuePoints", Native_GetClientQueuePoints );
	CreateNative ( "NONOHALE_SetClientQueuePoints", Native_SetClientQueuePoints );
	
	CreateNative ( "NONOHALE_GetPlayMusic", Native_GetPlayMusic );
	CreateNative ( "NONOHALE_StopPlayMusic", Native_StopPlayMusic );
	
	CreateNative ( "NONOHALE_SetPlayerStun", Native_SetPlayerStun );
	
	CreateNative ( "NONOHALE_GetHaleName", Native_GetHaleName );
	CreateNative ( "NONOHALE_GetHaleDescription", Native_GetHaleDescription );
	
	g_hForwardHandle[FORWARD_ON_ROUND_START_COUNT_DOWN] = CreateGlobalForward ( "NONOHALE_OnRoundStartCountdown", ET_Ignore, Param_Cell, Param_Cell );
	g_hForwardHandle[FORWARD_ON_ROUND_START_COUNT_DOWN_LAST_TICK] = CreateGlobalForward ( "NONOHALE_OnRoundStartCountdownLastTick", ET_Ignore, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HUMAN_TO_HALE] = CreateGlobalForward ( "NONOHALE_OnHumanToHale", ET_Ignore, Param_Cell, Param_Cell );
	
	g_hForwardHandle[FORWARD_ON_HALE_JUMP_READY] = CreateGlobalForward ( "NONOHALE_OnHaleJumpReady", ET_Ignore, Param_Cell, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HALE_JUMP] = CreateGlobalForward ( "NONOHALE_OnHaleJump", ET_Ignore, Param_Cell, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HALE_RAGE] = CreateGlobalForward ( "NONOHALE_OnHaleRage", ET_Hook, Param_Cell, Param_Cell, Param_FloatByRef, Param_FloatByRef );
	g_hForwardHandle[FORWARD_ON_HALE_RAGE_POST] = CreateGlobalForward ( "NONOHALE_OnHaleRagePost", ET_Ignore, Param_Cell, Param_Cell, Param_Float, Param_Float );
	g_hForwardHandle[FORWARD_ON_HALE_RAGE_END] = CreateGlobalForward ( "NONOHALE_OnHaleRageEnd", ET_Ignore, Param_Cell, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HALE_WEIGHT_DOWN] = CreateGlobalForward ( "NONOHALE_OnHaleWeightDown", ET_Ignore, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HALE_WEIGHT_DOWN_LANDING] = CreateGlobalForward ( "NONOHALE_OnHaleWeightDownLanding", ET_Ignore, Param_Cell );
	g_hForwardHandle[FORWARD_ON_CLIENT_STOMP] = CreateGlobalForward ( "NONOHALE_OnClientStomp", ET_Ignore, Param_Cell, Param_Cell );
	g_hForwardHandle[FORWARD_ON_CLIENT_STUN] = CreateGlobalForward ( "NONOHALE_OnClientStun", ET_Hook, Param_Cell, Param_Cell, Param_FloatByRef, Param_CellByRef );
	g_hForwardHandle[FORWARD_ON_PRE_SET_HALE_CLIENT] = CreateGlobalForward ( "NONOHALE_OnPreSetHaleClient", ET_Ignore, Param_Cell );
	g_hForwardHandle[FORWARD_ON_HALE_RAGE_CHARGE] = CreateGlobalForward ( "NONOHALE_OnHaleRageCharge", ET_Hook, Param_Cell, Param_Cell, Param_CellByRef );
	
	return APLRes_Success;
}

public int Native_GetCountdown ( Handle plugin, int params ) {
	return g_ConfigData.findTime;
}

public int Native_IsGameStart ( Handle plugin, int params ) {
	return g_GameStatus.gameStart;
}

public int Native_IsGameEnd ( Handle plugin, int params ) {
	return g_GameStatus.gameEnd;
}

public int Native_IsGamePlayerWaiting ( Handle plugin, int params ) {
	return g_GameStatus.playerWaiting;
}

public int Native_IsGameFindHale ( Handle plugin, int params ) {
	return g_GameStatus.gameFindHale;
}

public int Native_IsGameWarmup ( Handle plugin, int params ) {
	return g_GameStatus.gameWarmup;
}

public int Native_IsClientHale ( Handle plugin, int params ) {
	return g_PGameData[GetNativeCell ( 1 )].hale;
}

public int Native_GetHaleIndex ( Handle plugin, int params ) {
	int iIndex = -1;
	
	for ( int p = 1; p <= MaxClients; p ++ ) {
		if ( loringlib_IsValidClient ( p ) && g_PGameData[p].hale ) {
			iIndex = p;
			break;
		}
	}
	
	return iIndex;
}

public int Native_GetHaleDamages ( Handle plugin, int params ) {
	int iDamages = 0;
	
	for ( int p = 1; p <= MaxClients; p ++ ) {
		if ( loringlib_IsValidClient ( p ) && g_PGameData[p].hale ) {
			iDamages = g_PGameData[p].damages;
			break;
		}
	}
	
	return iDamages;
}

public int Native_GetClientHaleIndex ( Handle plugin, int params ) {
	return g_PGameData[GetNativeCell ( 1 )].haleIndex;
}

public int Native_IsClientStun ( Handle plugin, int params ) {
	return g_PGameData[GetNativeCell ( 1 )].stun;
}

public int Native_GetPreFindHaleClient ( Handle plugin, int params ) {
	return g_iHaleClient;
}

public int Native_GetDamages ( Handle plugin, int params ) {
	return g_PGameData[GetNativeCell ( 1 )].attackDamages;
}

public int Native_SetDamages ( Handle plugin, int params ) {
	g_PGameData[GetNativeCell ( 1 )].attackDamages = GetNativeCell ( 2 );
	return 1;
}

public int Native_AddDamages ( Handle plugin, int params ) {
	if ( !g_PGameData[GetNativeCell ( 1 )].hale ) {
		g_PGameData[GetNativeCell ( 1 )].attackDamages += GetNativeCell ( 2 );
	}
	
	return 1;
}

public int Native_GetClientQueuePoints ( Handle plugin, int params ) {
	return getClientQueuePoints ( GetNativeCell ( 1 ) );
}

public int Native_SetClientQueuePoints ( Handle plugin, int params ) {
	setClientQueuePoints ( GetNativeCell ( 1 ), GetNativeCell ( 2 ) );
	return 1;
}

public int Native_GetPlayMusic ( Handle plugin, int params ) {
	SetNativeString ( 1, g_strMusic, GetNativeCell ( 2 ) );
	return 1;
}

public int Native_StopPlayMusic ( Handle plugin, int params ) {
	if ( g_strMusic[0] != EOS ) {
		if ( g_hHaleMusicTimer != null ) {
			for ( int cl = 1; cl < MaxClients; cl ++ ) {
				if ( loringlib_IsValidClient ( cl ) ) {
					StopSound ( cl, SNDCHAN_AUTO, g_strMusic );
				}
			}
			
			strcopy ( g_strMusic, sizeof g_strMusic, "" );
			KillTimer ( g_hHaleMusicTimer );
			g_hHaleMusicTimer = null;
			
			return 1;
		}
	}
	
	return 0;
}

public int Native_SetPlayerStun ( Handle plugin, int params ) {
	BASESKILL_SetPlayerStun ( GetNativeCell ( 1 ), GetNativeCell ( 2 ), GetNativeCell ( 3 ), GetNativeCell ( 4 ) );
	return 1;
}

public int Native_GetHaleName ( Handle plugin, int params ) {
	return SetNativeString ( 2, g_HaleData[GetNativeCell ( 1 )].name, GetNativeCell ( 3 ) );
}

public int Native_GetHaleDescription ( Handle plugin, int params ) {
	return SetNativeString ( 2, g_HaleData[GetNativeCell ( 1 )].description, GetNativeCell ( 3 ) );
}

public int Native_GetChooseHaleIndex ( Handle plugin, int params ) {
	return g_iChooseHaleIndex;
}

public int Native_GetClientSoundToggle ( Handle plugin, int params ) {
	return g_POptions[GetNativeCell ( 1 )].musicToggle;
}