void INFECT_HumanToHale ( int player, int iHaleIndex ) {
	SDKHook ( player, SDKHook_WeaponCanUse, INFECT_OnHaleBlockUseWeapon );
	
	loringlib_RemoveWeaponSlot ( player, CS_SLOT_PRIMARY );
	loringlib_RemoveWeaponSlot ( player, CS_SLOT_SECONDARY );
	
	GivePlayerItem ( player, "weapon_knife" );
	FakeClientCommand ( player, "use weapon_knife" );
	
	g_PGameData[player].damages = 0;
	g_PGameData[player].hale = true;
	
	g_PGameData[player].haleRage = false;
	g_PGameData[player].haleRageDmgs = 0;
	
	g_PGameData[player].haleIndex = iHaleIndex;
	
	if ( StrEqual ( g_HaleData[iHaleIndex].model, "null", false ) )
		CS_UpdateClientModel ( player );
	else
		SetEntityModel ( player, g_HaleData[iHaleIndex].model );
	
	if ( g_HaleData[iHaleIndex].armsModel[0] != EOS )
		loringlib_SetEntityArmsModel ( player, g_HaleData[iHaleIndex].armsModel );
	loringlib_SetEntityMaxHealth ( player, g_HaleData[iHaleIndex].health + ( ( loringlib_GetPlayGameClients () - 1 ) * g_HaleData[iHaleIndex].healthMath ) );
	loringlib_SetEntityHealth ( player, loringlib_GetEntityMaxHealth ( player ) );
	loringlib_SetEntityLaggedmovement ( player, g_HaleData[iHaleIndex].speed );
	loringlib_SetEntityHelmet ( player, 0 );
	loringlib_SetEntityArmor ( player, g_HaleData[iHaleIndex].armor );
	SetEntityGravity ( player, g_HaleData[iHaleIndex].gravity );
	
	CS_SwitchTeam ( player, HALE_TEAM );
	
	if ( g_ConfigData.startOnRespawnHale ) {
		float flOrigin[3];
		for ( int i = 0; i < sizeof flOrigin; i ++ )
			flOrigin[i] = g_PGameData[player].spawnLocation[i];
		TeleportEntity ( player, flOrigin, NULL_VECTOR, NULL_VECTOR );
	}
	
//	SOUND_OnPlayHaleSound ( player, HALE_SND_INDEX_INTRO );	//	Intro voice
	
	Call_StartForward ( g_hForwardHandle[FORWARD_ON_HUMAN_TO_HALE] );
	Call_PushCell ( player );
	Call_PushCell ( iHaleIndex );
	Call_Finish ();
}

public Action INFECT_OnHaleBlockUseWeapon ( int player, int weapon ) {
	if ( loringlib_IsValidClient ( player ) && g_PGameData[player].hale ) {
		if ( weapon != -1 ) {
			char szWeapon[32];
			GetEdictClassname ( weapon, szWeapon, sizeof szWeapon );
			if ( StrContains ( szWeapon, "knife" ) != -1 )
				return Plugin_Continue;
		}
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

void INFECT_HurtOnHaleNoTagging ( int player ) {
	if ( loringlib_IsValidClient ( player ) && g_PGameData[player].hale ) {
		SetEntPropFloat ( player, Prop_Send, "m_flStamina", 0.0 );
		SetEntPropFloat ( player, Prop_Send, "m_flVelocityModifier", 1.0 );
	}
}

void INFECT_HaleUnhook ( int player ) {
	if ( loringlib_IsValidClient ( player ) ) {
		SDKUnhook ( player, SDKHook_WeaponCanUse, INFECT_OnHaleBlockUseWeapon );
		if ( g_PGameData[player].hale )
			g_PGameData[player].hale = false;
	}
}

void INFECT_HurtOnHaleDamage ( int victim, int attacker, int damage ) {
	if ( loringlib_IsValidClient ( victim ) && loringlib_IsValidClient ( attacker ) ) {
		if ( GetClientTeam ( victim ) != GetClientTeam ( attacker ) ) {
			if ( g_PGameData[victim].hale ) {
				g_PGameData[attacker].attackDamages += damage;
				g_PGameData[victim].damages += damage;
				
				if ( !g_PGameData[victim].haleRage ) {
					
					Action action = Plugin_Continue;
					Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_RAGE_CHARGE] );
					Call_PushCell ( victim );
					Call_PushCell ( g_PGameData[victim].haleIndex );
					Call_PushCellRef ( damage );
					Call_Finish ( action );
					
					if ( action == Plugin_Handled || action == Plugin_Stop )
						return;
				
					g_PGameData[victim].haleRageDmgs += damage;
					int iHaleIndex = g_PGameData[victim].haleIndex;
					if ( g_HaleData[iHaleIndex].rageRange < g_PGameData[victim].haleRageDmgs )
						g_PGameData[victim].haleRageDmgs = g_HaleData[iHaleIndex].rageRange;
					
					if ( g_HaleData[iHaleIndex].rageRange <= g_PGameData[victim].haleRageDmgs )
						if ( !IsFakeClient ( victim ) )
							qufnrTools_PrintCenterText ( victim, "%t", "hinttext_hale_rage" );
				}
			}
		}
	}
}

void INFECT_HaleDisconnectOnGameEnd ( int player ) {
	if ( g_PGameData[player].hale && g_PGameData[player].haleIndex > -1 ) {
		if ( GetTeamClientCount ( HALE_TEAM ) <= 1 ) {
			CS_TerminateRound ( 5.0, CSRoundEnd_Draw, true );
		}
	}
}