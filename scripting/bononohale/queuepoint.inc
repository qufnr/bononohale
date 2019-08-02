
int g_iBotQueuePoints = 0;

/**
 * 라운드 종료 때 계산을 시작합니다.
 */
void QUEUEPOINT_CalcQueuePoints () {
	int iAddPoints[MAXPLAYERS + 1];
	
	for ( int cl = 1; cl <= MaxClients; cl ++ ) {
		if ( loringlib_IsValidClient ( cl ) ) {
			if ( g_PGameData[cl].hale ) {
				if ( IsFakeClient ( cl ) ) {
					g_iBotQueuePoints = 0;
				}
				else {
					iAddPoints[cl] = -getClientQueuePoints ( cl );
				}
			}
			
			else if ( !IsFakeClient ( cl ) &&
				( GetClientTeam ( cl ) > CS_TEAM_SPECTATOR ) ) {
				iAddPoints[cl] = 10;
			}
			
			setClientQueuePoints ( cl, getClientQueuePoints ( cl ) + iAddPoints[cl] );
		}
	}
}

stock int getClientWithMostQueuePoints () {
	int iWinner;
	
	for ( int cl = 1; cl <= MaxClients; cl ++ ) {
		if ( loringlib_IsValidClient ( cl ) &&
			getClientQueuePoints ( cl ) >= getClientQueuePoints ( iWinner ) )
			if ( GetClientTeam ( cl ) > CS_TEAM_SPECTATOR )
				iWinner = cl;
	}
	
	return iWinner;
}

stock int getClientQueuePoints ( int cl ) {
	if ( !loringlib_IsValidClient ( cl ) )
		return 0;
	
	if ( IsFakeClient ( cl ) )
		return g_iBotQueuePoints;
	
	return g_PGameData[cl].queuePoints;
}

stock int setClientQueuePoints ( int cl, int points ) {
	if ( loringlib_IsValidClient ( cl ) && !IsFakeClient ( cl ) ) {
		g_PGameData[cl].queuePoints = points;
	}
}