
void SOUND_OnClearMusicTimer () {
	if ( g_hHaleMusicTimer != null ) {
		KillTimer ( g_hHaleMusicTimer );
		g_hHaleMusicTimer = null;
	}
}

void SOUND_OnPlayHaleMusic ( int haleIndex ) {
	if ( haleIndex > -1 ) {

		int iRandom = GetRandomInt ( 1, MAX_HALE_MUSIC ) - 1;
		strcopy ( g_strMusic, sizeof g_strMusic, g_szHaleMusic[haleIndex][iRandom] );
		if ( g_strMusic[0] == EOS ) {
			strcopy ( g_strMusic, sizeof g_strMusic, g_szHaleMusic[haleIndex][0] );
			if ( g_strMusic[0] == EOS )
				return;
		}
		
		
		Handle hMusicHandle = OpenSoundFile ( g_strMusic );
		
		if ( hMusicHandle != null && g_hHaleMusicTimer == null ) {
		
			for ( int c = 1; c <= MaxClients; c ++ ) {
				if ( loringlib_IsValidClient ( c ) && !IsFakeClient ( c ) &&
				g_POptions[c].musicToggle )
					EmitSoundToClient ( c, g_strMusic, SOUND_FROM_PLAYER, SNDCHAN_AUTO, SNDLEVEL_NONE );
			}
			
			DataPack data;
			g_hHaleMusicTimer = CreateDataTimer ( 0.1, SOUND_HaleMusicRepeat, data, TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
			data.WriteFloat ( GetGameTime () + ( GetSoundLengthFloat ( hMusicHandle ) + 1.0 ) );
			data.WriteString ( g_strMusic );
			
			PrintToServer ( "Play Music: %s Length %.2f", g_strMusic, GetSoundLengthFloat ( hMusicHandle ) );
		}
		
		delete hMusicHandle;
	}
}

public Action SOUND_HaleMusicRepeat ( Handle hndl, DataPack data ) {
	data.Reset ();
	
	
	float flLength = data.ReadFloat ();
	char szMusic[256];
	data.ReadString ( szMusic, sizeof szMusic );
	
	if ( g_GameStatus.gameEnd || !g_GameStatus.gameStart ) {
		
		for ( int target = 1; target <= MaxClients; target ++ )
			if ( loringlib_IsValidClient ( target ) )
				StopSound ( target, SNDCHAN_AUTO, szMusic );
		
		g_hHaleMusicTimer = null;
		return Plugin_Stop;
	}
	
	if ( GetGameTime () > flLength ) {
		
		for ( int c = 1; c <= MaxClients; c ++ ) {
			if ( loringlib_IsValidClient ( c ) && !IsFakeClient ( c ) &&
			g_POptions[c].musicToggle )
				EmitSoundToClient ( c, szMusic, SOUND_FROM_PLAYER, SNDCHAN_AUTO, SNDLEVEL_NONE );
		}
			
		static Handle hMusicHandle;
		hMusicHandle = OpenSoundFile ( szMusic );
		
		DataPack data2;
		g_hHaleMusicTimer = CreateDataTimer ( 0.1, SOUND_HaleMusicRepeat, data2, TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
		data2.WriteFloat ( GetGameTime () + GetSoundLengthFloat ( hMusicHandle ) + 1.0 );
		data2.WriteString ( szMusic );
		
		return Plugin_Stop;
	}
	
	return Plugin_Continue;
}

void SOUND_OnPlayHaleSound ( int player, int sndType ) {
	if ( loringlib_IsValidClient ( player ) &&
		g_PGameData[player].hale ) {
		
		int iHaleIndex = g_PGameData[player].haleIndex;
		
		char szSnd[256];
		int iRandom = GetRandomInt ( 1, MAX_HALE_SOUNDS ) - 1;
		
		strcopy ( szSnd, sizeof szSnd, g_szHaleSoundData[iHaleIndex][sndType][iRandom] );
		if ( szSnd[0] == EOS ) {
			strcopy ( szSnd, sizeof szSnd, g_szHaleSoundData[iHaleIndex][sndType][0] );
			if ( szSnd[0] == EOS )
				return;
		}
		
		EmitSoundToAll ( szSnd, player, SNDCHAN_AUTO, SNDLEVEL_NORMAL );
		EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER );
		
	//	PrintToChatAll ( "Sound Play: %s", szSnd );
	}
}

void SOUND_OnPlayHalePainSound ( int player, int dmg_health ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) &&
		g_PGameData[player].hale ) {
		if ( dmg_health > 1000 )
			SOUND_OnPlayHaleSound ( player, HALE_SND_INDEX_PAIN );
	}
}

void SOUND_OnPlayHaleVictorySound ( int winner ) {
	if ( winner == HALE_TEAM ) {
		int iHaleClient = -1;
		for ( int hale = 1; hale <= MaxClients; hale ++ ) {
			if ( loringlib_IsValidClient__PlayGame ( hale ) && g_PGameData[hale].hale ) {
				iHaleClient = hale;
				break;
			}
		}
		
		if ( iHaleClient != -1 )
			SOUND_OnPlayHaleSound ( iHaleClient, HALE_SND_INDEX_VICTORY );
	}
}

void SOUND_OnPlayHaleCountdownIntroSound ( int haleIndex ) {
	if ( haleIndex > g_iHaleData || haleIndex <= -1 )
		return;
	
	char szSnd[256];
	int iRandom = GetRandomInt ( 1, MAX_HALE_SOUNDS ) - 1;
	
	strcopy ( szSnd, sizeof szSnd, g_szHaleSoundData[haleIndex][HALE_SND_INDEX_INTRO][iRandom] );
	if ( szSnd[0] == EOS ) {
		strcopy ( szSnd, sizeof szSnd, g_szHaleSoundData[haleIndex][HALE_SND_INDEX_INTRO][0] );
		if ( szSnd[0] == EOS )
			return;
	}
	
	EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER );
}