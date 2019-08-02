
void MISC_CreateGlobalTimer () {
	if ( g_hGlobalRepeatTimer == null )
		CreateTimer ( GLOBAL_TIMER_TICK, GAME_OnGlobalTimerThink, _, TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
	
}

void MISC_KillGlobalTimer () {
	if ( g_hGlobalRepeatTimer != null ) {
		KillTimer ( g_hGlobalRepeatTimer );
		g_hGlobalRepeatTimer = null;
	}
}

void MISC_CreateHudSyncHndl () {
	for ( int i = 0; i < sizeof g_hHudSyncHndl; i ++ )
		g_hHudSyncHndl[i] = CreateHudSynchronizer ();
}

void MISC_KillHudSyncHndl () {
	for ( int i = 0; i < sizeof g_hHudSyncHndl; i ++ )
		if ( g_hHudSyncHndl[i] != null )
			for ( int p = 1; p <= MaxClients; p ++ )
				if ( loringlib_IsValidClient ( p ) )
					ClearSyncHud ( p, g_hHudSyncHndl[i] );
}

/**
 * 랭크 표시
 * https://kimggur2gd.tistory.com/45
 */
void MISC_RoundEndOnDisplayRankBoard () {
	
	if ( g_ConfigData.displayRankOfDamage ) {
		int iArray[MAXPLAYERS + 1][2];
		int iTemp[1][2];	//	변수 옮길 값
		
		for ( int i = 0; i < sizeof iArray; i ++ ) {
			iArray[i][0] = -1;
			iArray[i][1] = 0;
		}
		
		for ( int i = 1; i <= MaxClients; i ++ ) {
			if ( loringlib_IsValidClient ( i ) ) {
				iArray[i][0] = i;
				iArray[i][1] = g_PGameData[i].attackDamages;
			}
		}

		for ( int y = MAXPLAYERS; y > 0; y -- ) {
			for ( int z = 0; z < y; z ++ ) {
				if ( iArray[z][1] < iArray[z + 1][1] ) {
					//	점수 복사
					iTemp[0][1] = iArray[z][1];
					iArray[z][1] = iArray[z + 1][1];
					iArray[z + 1][1] = iTemp[0][1];
				
					//	이름 복사
					iTemp[0][0] = iArray[z][0];
					iArray[z][0] = iArray[z + 1][0];
					iArray[z + 1][0] = iTemp[0][0];
				}
			}
		}
		
		//	Display to clients.
		SetHudTextParams ( -1.0, 0.36, g_ConfigData.roundEndDelay, 242, 203, 97, 255, 0, 0.05, 0.05, 0.1 );
		char szBoard[256];
		char szRankData[2][32];
		for ( int cl = 1; cl <= MaxClients; cl ++ ) {
			if ( loringlib_IsValidClient ( cl ) && !IsFakeClient ( cl ) ) {
				SetGlobalTransTarget ( cl );
				Format ( szBoard, sizeof szBoard, "%t\n", "hudtext_board_title" );
				
				for ( int i = 0; i < 3; i ++ ) {
					if ( loringlib_IsValidClient ( iArray[i][0] ) )
						GetClientName ( iArray[i][0], szRankData[0], sizeof szRankData[] );
					else
						Format ( szRankData[0], sizeof szRankData[], "%t", "hudtext_unknown_player" );
					
					if ( iArray[i][0] <= 0 )
						Format ( szRankData[1], sizeof szRankData[], "-" );
					else
						loringlib_NumberFormat ( iArray[i][1], szRankData[1], sizeof szRankData[] );
					
					Format ( szBoard, sizeof szBoard, "%s%t\n", szBoard, "hudtext_rank_format", i + 1, szRankData[0], szRankData[1] );
				}
				
				ShowSyncHudText ( cl, g_hHudSyncHndl[HUDSYNC_HALE_DMGRANK], szBoard );
			}
		}
	}
}