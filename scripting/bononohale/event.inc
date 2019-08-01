void EVENT_RegisterEvents () {
	HookEvent ( "round_start", EVENT_OnRoundStart );
	HookEvent ( "round_end", EVENT_OnRoundEnd );
	HookEvent ( "player_spawn", EVENT_OnPlayerSpawn );
	HookEvent ( "player_hurt", EVENT_OnPlayerHurt );
	HookEvent ( "player_death", EVENT_OnPlayerDeath );
	HookEvent ( "player_death", EVENT_OnPlayerDeathPre, EventHookMode_Pre );
}

public void EVENT_OnRoundStart ( Event ev, const char[] name, bool dontBroadcast ) {
	
	GAME_FindStart ();
	BASESKILL_ResetVariables ();
}

public void EVENT_OnRoundEnd ( Event ev, const char[] name, bool dontBroadcast ) {
	int iWinner = ev.GetInt ( "winner" );
	
	MISC_RoundEndOnDisplayRankBoard ();
	
	SOUND_OnPlayHaleVictorySound ( iWinner );
	
	GAME_RoundEndOnSetStatus ();
	GAME_KillFindTimer ();
}

public void EVENT_OnPlayerSpawn ( Event ev, const char[] name, bool dontBroadcast ) {
	int player = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	
	PLAYER_SpawnOnSetData ( player );
}

public void EVENT_OnPlayerHurt ( Event ev, const char[] name, bool dontBroadcast ) {
	int victim = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	int attacker = GetClientOfUserId ( ev.GetInt ( "attacker" ) );
	
	int iDamage = ev.GetInt ( "dmg_health" );
	
	INFECT_HurtOnHaleDamage ( victim, attacker, iDamage );
	INFECT_HurtOnHaleNoTagging ( victim );
	SOUND_OnPlayHalePainSound ( victim, iDamage );
}

public void EVENT_OnPlayerDeath ( Event ev, const char[] name, bool dontBroadcast ) {
	int victim = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	int attacker = GetClientOfUserId ( ev.GetInt ( "attacker" ) );
	
	INFECT_HaleUnhook ( victim );
	SOUND_OnPlayHaleSound ( attacker, HALE_SND_INDEX_KILL );
	
	PLAYER_OnLastSurviveNotice ();
}

public Action EVENT_OnPlayerDeathPre ( Event ev, const char[] name, bool dontBroadcast ) {
	int victim = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	SOUND_OnPlayHaleSound ( victim, HALE_SND_INDEX_DEATH );
	
	return Plugin_Continue;
}