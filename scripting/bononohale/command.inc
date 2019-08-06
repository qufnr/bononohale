void COMMAND_RegisterCommands () {
	RegConsoleCmd ( "sm_bnnoption", COMMAND_OnCommandOptions, "Control of the options." );
	RegConsoleCmd ( "sm_qp", COMMAND_OnCommandQueuePoint, "Print queue point to client." );
	RegConsoleCmd ( "sm_queuepoint", COMMAND_OnCommandQueuePoint, "Print queue point to client." );
	RegConsoleCmd ( "sm_resetqp", COMMAND_OnCommandQueuePointReset, "Reset queue point for client." );
	RegAdminCmd ( "sm_setstun", COMMAND_OnCommandSetStun, ADMFLAG_ROOT, "Set client to stun." );
	RegAdminCmd ( "sm_setboss", COMMAND_OnCommandSetBoss, ADMFLAG_ROOT, "Set boss on this round." );
	RegAdminCmd ( "sm_setrage", COMMAND_OnCommandSetRage, ADMFLAG_ROOT, "Set boss rage charge." );
	RegAdminCmd ( "sm_setqp", COMMAND_OnCommandSetQueuePoint, ADMFLAG_ROOT, "Set queue point for client." );
	
	AddCommandListener ( COMMAND_OnPressDropKey, "drop" );
}

void COMMAND_AddMultiTargetTilter () {
	AddMultiTargetFilter ( "@boss", COMMAND_HaleTargetFilter, "The current Boss or Bosses.", false );
	AddMultiTargetFilter ( "@hale", COMMAND_HaleTargetFilter, "The current Boss or Bosses.", false );
	AddMultiTargetFilter ( "@!boss", COMMAND_HaleTargetFilter, "All non-Boss players.", false );
	AddMultiTargetFilter ( "@!hale", COMMAND_HaleTargetFilter, "All non-Boss players.", false );
}

public bool COMMAND_HaleTargetFilter ( const char[] pattern, Handle clients ) {
	bool bNon = StrContains ( pattern, "!", false ) != -1;
	
	for ( int i = MaxClients; i ; i-- ) {
		if ( loringlib_IsValidClient ( i ) &&
			FindValueInArray ( clients, i ) == -1 ) {
			if ( g_PGameData[i].hale ) {
				if ( !bNon )
					PushArrayCell ( clients, i );
			}else if ( bNon )
				PushArrayCell ( clients, i );
		}
	}
	
	return true;
}

public Action COMMAND_OnPressDropKey ( int player, const char[] cmd, int argc ) {
	if ( player > 0 && !IsFakeClient ( player ) ) {
		if ( g_PGameData[player].hale ) {
			BASESKILL_OnHaleRageEnable ( player );
			return Plugin_Handled;
		}
	}
	
	return Plugin_Continue;
}

public Action COMMAND_OnCommandSetStun ( int p, int args ) {
	if ( p > 0 ) {
		if ( args < 1 ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setstun_usage" );
			return Plugin_Handled;
		}
		
		char szName[32];
		GetCmdArg ( 1, szName, sizeof szName );
		int target = FindTarget ( p, szName, false );
		
		if ( !loringlib_IsValidClient ( target ) ) {
			qufnrTools_ReplyToCommand ( p, "%t", "invalid_client_index" );
			return Plugin_Handled;
		}
		
		if ( g_PGameData[target].hale ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setstun_target_is_hale" );
			return Plugin_Handled;
		}
		
		if ( g_PGameData[target].stun ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setstun_target_is_stun" );
			return Plugin_Handled;
		}
		
		char szTime[16], szDamage[16];
		GetCmdArg ( 2, szTime, sizeof szTime );
		GetCmdArg ( 3, szDamage, sizeof szDamage );
		
		float flTime = StringToFloat ( szTime );
		if ( flTime <= 0.0 )
			flTime = 5.0;
		
		int iDamage = StringToInt ( szDamage );
		if ( iDamage < 0 )
			iDamage = 0;
		
		BASESKILL_SetPlayerStun ( target, p, flTime, iDamage );
		
		qufnrTools_ReplyToCommand ( p, "%t", "command_setstun_success", target );
		qufnrTools_PrintToChat ( target, "%t", "command_setstun_success_target" );
	}
	
	return Plugin_Handled;
}

public Action COMMAND_OnCommandSetBoss ( int p, int args ) {
	if ( p > 0 ) {
		if ( args < 1 ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setboss_usage" );
			return Plugin_Handled;
		}
		
		char szIndex[16];
		GetCmdArg ( 1, szIndex, sizeof szIndex );
		int iHaleIndex = StringToInt ( szIndex );
		
		if ( iHaleIndex >= g_iHaleData ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setboss_invalid_boss" );
			return Plugin_Handled;
		}
		
		if ( g_iChooseHaleIndex != -1 ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setboss_already_set", g_HaleData[g_iChooseHaleIndex].name );
			return Plugin_Handled;
		}
		
		g_iChooseHaleIndex = iHaleIndex;
		qufnrTools_ReplyToCommand ( p, "%t", "command_setboss_success", g_HaleData[iHaleIndex].name );
	}
	
	return Plugin_Handled;
}

public Action COMMAND_OnCommandSetRage ( int p, int args ) {
	if ( p > 0 ) {
		if ( args < 2 ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setrage_usage" );
			return Plugin_Handled;
		}
		
		char szTarget[32];
		GetCmdArg ( 1, szTarget, sizeof szTarget );
		int iTarget = FindTarget ( p, szTarget, false );
		if ( loringlib_IsValidClient__PlayGame ( iTarget ) ) {
			if ( g_PGameData[iTarget].hale && g_PGameData[iTarget].haleIndex > -1 ) {
				if ( g_PGameData[iTarget].haleRage ) {
					qufnrTools_ReplyToCommand ( p, "%t", "command_setrage_player_are_rage" );
					return Plugin_Handled;
				}
				
				char szValue[16];
				GetCmdArg ( 2, szValue, sizeof szValue );
				int iValue = StringToInt ( szValue );
				if ( iValue > 0 ) {
					int iHaleIndex = g_PGameData[iTarget].haleIndex;
					
					qufnrTools_ReplyToCommand ( p, "%t", "command_setrage_success", iTarget, iValue );
					
					Action action = Plugin_Continue;
					Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_RAGE_CHARGE] );
					Call_PushCell ( iTarget );
					Call_PushCell ( iHaleIndex );
					Call_PushCellRef ( iValue );	//	Set Damage value
					Call_Finish ( action );
					
					if ( action == Plugin_Handled || action == Plugin_Stop ) {
						qufnrTools_ReplyToCommand ( p, "%t", "error_ignore_event" );
						return Plugin_Handled;
					}
					
					qufnrTools_PrintToChat ( iTarget, "%t", "command_setrage_success_target", iValue );
					g_PGameData[iTarget].haleRageDmgs = iValue;
					if ( g_HaleData[iHaleIndex].rageRange < g_PGameData[iTarget].haleRageDmgs )
						g_PGameData[iTarget].haleRageDmgs = g_HaleData[iHaleIndex].rageRange;
					
					if ( g_HaleData[iHaleIndex].rageRange <= g_PGameData[iTarget].haleRageDmgs )
						if ( !IsFakeClient ( iTarget ) )
							qufnrTools_PrintCenterText ( iTarget, "%t", "hinttext_hale_rage" );
					
					return Plugin_Handled;
				}
			}
			else {
				qufnrTools_ReplyToCommand ( p, "%t", "command_setrage_target_not_boss" );
				return Plugin_Handled;
			}
		}
	}
	
	return Plugin_Handled;
}

public Action COMMAND_OnCommandSetQueuePoint ( int p, int args ) {
	if ( p > 0 ) {
		if ( args < 2 ) {
			qufnrTools_ReplyToCommand ( p, "%t", "command_setqp_usage" );
			return Plugin_Handled;
		}
		
		char szName[32];
		GetCmdArg ( 1, szName, sizeof szName );
		int iTarget = FindTarget ( p, szName, false );
		
		if ( loringlib_IsValidClient ( iTarget ) ) {
			char szValue[16];
			GetCmdArg ( 2, szValue, sizeof szValue );
			int iValue = StringToInt ( szValue );
			
			setClientQueuePoints ( iTarget, iValue );
			
			qufnrTools_ReplyToCommand ( p, "%t", "command_setqp_success", iTarget, iValue );
			qufnrTools_PrintToChat ( iTarget, "%t", "command_setqp_success_target", iValue );
			
			return Plugin_Handled;
		}
	}
	
	return Plugin_Handled;
}

public Action COMMAND_OnCommandOptions ( int p, int args ) {
	if ( p > 0 && args < 1 ) {
		Menu menu = new Menu ( menuOptionMenuHandler );
		qufnrTools_SetMenuTitle ( menu, p, "%t:\n　", "menutitle_client_options" );
		
		qufnrTools_AddMenuItem ( menu, p, _, "", "%t: %t", "menuitem_music_toggle", g_POptions[p].musicToggle ? "toggle_on" : "toggle_off" );
		qufnrTools_AddMenuItem ( menu, p, _, "", "%t: %t", "menuitem_displaydmgs_toggle", g_POptions[p].displayDamages ? "toggle_on" : "toggle_off" );
		qufnrTools_AddMenuItem ( menu, p, _, "", "%t: %t", "menuitem_displaybosshealth_toggle", g_POptions[p].displayBossHealth ? "toggle_on" : "toggle_off" );
		
		menu.Display ( p, MENU_TIME_FOREVER );
	}
	
	return Plugin_Handled;
}

public int menuOptionMenuHandler ( Menu menu, MenuAction action, int param1, int param2 ) {
	switch ( action ) {
		case MenuAction_End: delete menu;
		case MenuAction_Select: {
			switch ( param2 ) {
				case 0: {
					g_POptions[param1].musicToggle = !g_POptions[param1].musicToggle;
					if ( !g_POptions[param1].musicToggle ) {
						if ( g_strMusic[0] != EOS )
							StopSound ( param1, SNDCHAN_AUTO, g_strMusic );
					}
				} case 1:
					g_POptions[param1].displayDamages = !g_POptions[param1].displayDamages;
				case 2:
					g_POptions[param1].displayBossHealth = !g_POptions[param1].displayBossHealth;
			}
			
			COMMAND_OnCommandOptions ( param1, 0 );
		}
	}
}

public Action COMMAND_OnCommandQueuePoint ( int player, int args ) {
	if ( player > 0 ) {
		if ( args < 1 ) {
			qufnrTools_ReplyToCommand ( player, "%t", "command_display_queuepoint", getClientQueuePoints ( player ) );
			return Plugin_Handled;
		}
		
		char szName[2][32];
		GetCmdArg ( 1, szName[0], sizeof szName[] );
		
		if ( StrEqual ( szName[0], "rank", false ) ) {
			int iPlayerArray[MAXPLAYERS + 1][2];
			int iTemp[1][2];
			
			for ( int i = 1; i <= MaxClients; i ++ ) {
				if ( loringlib_IsValidClient ( i ) ) {
					iPlayerArray[i][0] = i;
					iPlayerArray[i][1] = getClientQueuePoints ( i );
				}
				else {
					iPlayerArray[i][0] = -1;
					iPlayerArray[i][1] = -1;
				}
			}
			
			for ( int y = MaxClients + 1; y > 0; y -- ) {
				for ( int z = 0; z < y; z ++ ) {
					if ( iPlayerArray[z][1] < iPlayerArray[z + 1][1] ) {
						iTemp[0][1] = iPlayerArray[z][1];
						iPlayerArray[z][1] = iPlayerArray[z + 1][1];
						iPlayerArray[z + 1][1] = iTemp[0][1];
					
						iTemp[0][0] = iPlayerArray[z][0];
						iPlayerArray[z][0] = iPlayerArray[z + 1][0];
						iPlayerArray[z + 1][0] = iTemp[0][0];
					}
				}
			}
			
			qufnrTools_ReplyToCommand ( player, "%t", "command_queuepoint_board" );
			for ( int i = 0; i < 5; i ++ ) {
				if ( loringlib_IsValidClient ( iPlayerArray[i][0] ) )
					qufnrTools_ReplyToCommand ( player, "%t", "command_queuepoint_board_format", i + 1, iPlayerArray[i][0], iPlayerArray[i][1] );
				else
					qufnrTools_ReplyToCommand ( player, "%t", "command_queuepoint_board_format_null", i + 1 );
			}
			
			return Plugin_Handled;
		}
		
		int iTarget = -1;
		for ( int cl = 1; cl <= MaxClients; cl ++ ) {
			if ( loringlib_IsValidClient ( cl ) ) {
				GetClientName ( cl, szName[1], sizeof szName[] );
				if ( strcmp ( szName[0], szName[1], false ) != -1 ) {
					iTarget = cl;
				}
			}
		}
		
		if ( loringlib_IsValidClient ( iTarget ) ) {
			qufnrTools_ReplyToCommand ( player, "%t", "command_display_target_queuepoint", iTarget, getClientQueuePoints ( iTarget ) );
			return Plugin_Handled;
		}
		
		else {
			qufnrTools_ReplyToCommand ( player, "%t", "invalid_client_index" );
			return Plugin_Handled;
		}
	}
	
	return Plugin_Handled;
}

public Action COMMAND_OnCommandQueuePointReset ( int player, int args ) {
	if ( player > 0 ) {
		setClientQueuePoints ( player, 0 );
		qufnrTools_PrintToChat ( player, "%t", "command_queuepoint_reset" );
	}
	
	return Plugin_Handled;
}