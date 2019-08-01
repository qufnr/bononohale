
static float flModifyNextAttack[MAXPLAYERS + 1];

void WEAPON_ReadWeaponData () {
	g_iWeaponData = 0;
	g_bWeaponLoaded = false;
	
	char szBuild[256];
	BuildPath ( Path_SM, szBuild, sizeof szBuild, "data/bononohale/weapon.txt" );
	KeyValues kv = new KeyValues ( "weapon" );
	
	if ( kv.ImportFromFile ( szBuild ) ) {
		kv.GotoFirstSubKey ();
		
		do {
			kv.GetSectionName ( g_WeaponData[g_iWeaponData].classname, sizeof g_WeaponData[].classname );
			
			g_WeaponData[g_iWeaponData].damageMultiple = kv.GetFloat ( "damage_multiple" );
			g_WeaponData[g_iWeaponData].knockbackScale = kv.GetFloat ( "knockback" );
			g_WeaponData[g_iWeaponData].fireRate = kv.GetFloat ( "fire_rate" );
			g_WeaponData[g_iWeaponData].reloadSpd = kv.GetFloat ( "reload_speed" );
			
			g_iWeaponData ++;
			
			if ( !g_bWeaponLoaded )
				g_bWeaponLoaded = true;
		}
		
		while ( kv.GotoNextKey () );
	}
	
	delete kv;
}

void WEAPON_ConnectOnWeaponHooking ( int player, bool hook = true ) {
	flModifyNextAttack[player] = 0.0;
	
	if ( hook ) {
		SDKHook ( player, SDKHook_PostThink, WEAPON_OnWeaponFirerateModify );
		SDKHook ( player, SDKHook_PostThinkPost, WEAPON_OnWeaponFirerateModifyPost );
		return;
	}
	

	SDKUnhook ( player, SDKHook_PostThink, WEAPON_OnWeaponFirerateModify );
	SDKUnhook ( player, SDKHook_PostThinkPost, WEAPON_OnWeaponFirerateModifyPost );
}

void WEAPON_EntCreateOnWeaponReloadHook ( int entity ) {
	if ( HasEntProp ( entity, Prop_Send, "m_reloadState" ) )
		SDKHook ( entity, SDKHook_ReloadPost, WEAPON_OnWeaponReloadPost );
}

public void WEAPON_OnWeaponReloadPost ( int weapon, bool successful ) {
	int iOwner = loringlib_GetWeaponOwner ( weapon );
	if ( loringlib_IsValidClient ( iOwner ) ) {
		if ( GetEntProp ( weapon, Prop_Send, "m_reloadState" ) != 2 )
			return;
		char szWeapon[32];
		GetEdictClassname ( weapon, szWeapon, sizeof szWeapon );
		int iIndexOfWeapon = GetEntProp ( weapon, Prop_Send, "m_iItemDefinitionIndex" );
		switch ( iIndexOfWeapon ) {
			case 23:	strcopy ( szWeapon, sizeof szWeapon, "weapon_mp5sd" );
			case 60:	strcopy ( szWeapon, sizeof szWeapon, "weapon_m4a1_silencer" );
			case 61: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_usp_silencer" );
			case 63: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_cz75a" );
		}
		
		int iIndex = -1;
		for ( int i = 0; i < g_iWeaponData; i ++ ) {
			if ( StrEqual ( szWeapon, g_WeaponData[i].classname ) ) {
				iIndex = i;
				break;
			}
		}
		
		if ( iIndex > -1 ) {
			float flIncrease = 1.0 / g_WeaponData[iIndex].reloadSpd;
		
			float flOldIdleTime = GetEntPropFloat ( weapon, Prop_Send, "m_flTimeWeaponIdle" );
			float flGameTime = GetGameTime ();
			float flNewIdleTime = ( flOldIdleTime - flGameTime ) * flIncrease + flGameTime;
			
			SetEntPropFloat ( weapon, Prop_Send, "m_flTimeWeaponIdle", flNewIdleTime );
		}
	}
}

void WEAPON_ChangeWeaponReloadSpeed ( int player ) {
	if ( loringlib_IsValidClient ( player ) ) {
		int iWeapon = loringlib_GetActiveWeapon ( player );
		if ( iWeapon != -1 ) {
			char szWeapon[32];
			GetEdictClassname ( iWeapon, szWeapon, sizeof szWeapon );
			int iIndexOfWeapon = GetEntProp ( iWeapon, Prop_Send, "m_iItemDefinitionIndex" );
			switch ( iIndexOfWeapon ) {
				case 23:	strcopy ( szWeapon, sizeof szWeapon, "weapon_mp5sd" );
				case 60:	strcopy ( szWeapon, sizeof szWeapon, "weapon_m4a1_silencer" );
				case 61: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_usp_silencer" );
				case 63: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_cz75a" );
			}
			
			int iIndex = -1;
			for ( int i = 0; i < g_iWeaponData; i ++ ) {
				if ( StrEqual ( szWeapon, g_WeaponData[i].classname ) ) {
					iIndex = i;
					break;
				}
			}
			
			if ( iIndex > -1 ) {
				bool bShotgun = HasEntProp ( iWeapon, Prop_Send, "m_reloadState" );
				if ( bShotgun ) {
					int iReloadState = GetEntProp ( iWeapon, Prop_Send, "m_reloadState" );
					if ( iReloadState == 0 )
						return;
				}
				
				float flOldNextAttack = GetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" );
				float flGameTime = GetGameTime ();
				
				float flIncrease = 1.0 / g_WeaponData[iIndex].reloadSpd;
				SetEntPropFloat ( iWeapon, Prop_Send, "m_flPlaybackRate", 1.0 / flIncrease );
				
				int iViewModel = GetEntPropEnt ( player, Prop_Send, "m_hViewModel" );
				if ( iViewModel != INVALID_ENT_REFERENCE )
					SetEntPropFloat ( iViewModel, Prop_Send, "m_flPlaybackRate", 1.0 / flIncrease );
				
				float flNewNextAttack = ( flOldNextAttack - flGameTime ) * flIncrease;
				
				DataPack data;
				if ( bShotgun ) {
					CreateDataTimer ( 0.01, WEAPON_CheckOfShotgunEnd, data, TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
					data.WriteCell ( EntIndexToEntRef ( iWeapon ) );
					data.WriteCell ( GetClientUserId ( player ) );
				}
				
				else {
					CreateDataTimer ( flNewNextAttack, WEAPON_ResetPlaybackRate, data, TIMER_FLAG_NO_MAPCHANGE );
					data.WriteCell ( EntIndexToEntRef ( iWeapon ) );
					data.WriteCell ( GetClientUserId ( player ) );
					
				}
				
				flNewNextAttack += flGameTime;
				
				SetEntPropFloat ( iWeapon, Prop_Send, "m_flTimeWeaponIdle", flNewNextAttack );
				SetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack", flNewNextAttack );
				SetEntPropFloat ( player, Prop_Send, "m_flNextAttack", flNewNextAttack );
			}
		}
	}
}

public Action WEAPON_ResetPlaybackRate ( Handle hndl, DataPack data ) {
	data.Reset ();
	
	int iWeapon = EntRefToEntIndex ( data.ReadCell () );
	int iPlayer = GetClientOfUserId ( data.ReadCell () );
	
	if ( iWeapon != INVALID_ENT_REFERENCE )
		SetEntPropFloat ( iWeapon, Prop_Send, "m_flPlaybackRate", 1.0 );
	
	if ( loringlib_IsValidClient ( iPlayer ) )
		WEAPON_ResetPlayerViewModel ( iPlayer );
	
	return Plugin_Stop;
}

public Action WEAPON_CheckOfShotgunEnd ( Handle hndl, DataPack data ) {
	data.Reset ();
	
	int iWeapon = EntRefToEntIndex ( data.ReadCell () );
	int iPlayer = GetClientOfUserId ( data.ReadCell () );
	
	if ( iWeapon != INVALID_ENT_REFERENCE ) {
		if ( loringlib_IsValidClient ( iPlayer ) ) {
			WEAPON_ResetPlayerViewModel ( iPlayer );
			return Plugin_Stop;
		}
	}
	
	int weaponOwner = loringlib_GetWeaponOwner ( iWeapon );
	if ( weaponOwner <= 0 ) {
		if ( loringlib_IsValidClient ( iPlayer ) )
			WEAPON_ResetPlayerViewModel ( iPlayer );
			
		SetEntPropFloat ( iWeapon, Prop_Send, "m_flPlaybackRate", 1.0 );
		return Plugin_Stop;
	}
	
	int reloadState = GetEntProp ( iWeapon, Prop_Send, "m_reloadState" );
	
	if ( reloadState > 0 )
		return Plugin_Continue;
	
	SetEntPropFloat ( iWeapon, Prop_Send, "m_flPlaybackRate", 1.0 );
	
	if ( loringlib_IsValidClient ( iPlayer ) )
		WEAPON_ResetPlayerViewModel ( iPlayer );
		
	return Plugin_Stop;
}

float WEAPON_OnDamageWeaponModify ( int victim, int attacker, int weapon, float damage, float damagePosition[3] ) {
	if ( g_bWeaponLoaded ) {
		if ( loringlib_IsValidClient ( victim ) && loringlib_IsValidClient ( attacker ) ) {
			if ( g_PGameData[victim].hale &&
				g_PGameData[victim].haleIndex > -1 &&
				GetClientTeam ( victim ) != GetClientTeam ( attacker ) ) {
				if ( weapon != -1 ) {
					char szClassname[32];
					GetEdictClassname ( weapon, szClassname, sizeof szClassname );
					
					int iIndex = -1;
					
					for ( int i = 0; i < g_iWeaponData; i ++ ) {
						if ( StrEqual ( g_WeaponData[i].classname, szClassname ) ) {
							iIndex = i;
							break;
						}
					}
					
					if ( iIndex > -1 ) {
						float flDamage = damage;
						if ( g_WeaponData[iIndex].damageMultiple > 0.0 )
							flDamage *= g_WeaponData[iIndex].damageMultiple;
						
						if ( g_WeaponData[iIndex].knockbackScale > 0.0 ) {
							float flKnockbackResult = flDamage * g_WeaponData[iIndex].knockbackScale / g_HaleData[g_PGameData[victim].haleIndex].knockbackImmunityScale;
							
							if ( ( GetEntityFlags ( victim ) & FL_DUCKING | FL_ONGROUND ) )
								flKnockbackResult /= 1.5;
							
							float flAttackerOrigin[3];
							GetClientEyePosition ( attacker, flAttackerOrigin );
							
							float flDist = GetVectorDistance ( flAttackerOrigin, damagePosition );
							
	
							if ( flDist <= 400.0 )
								loringlib_KnockbackToClient ( attacker, victim, flKnockbackResult, true );
							else if ( flDist <= 800.0 )
								loringlib_KnockbackToClient ( attacker, victim, flKnockbackResult / 1.5, true );
							else if ( flDist <= 1000.0 )
								loringlib_KnockbackToClient ( attacker, victim, flKnockbackResult / 2.0, true );
							else if ( flDist > 1000.0 )
								loringlib_KnockbackToClient ( attacker, victim, flKnockbackResult / 2.5, true );
						
							
						}
						
						return flDamage;
					}
				}
			}
		}
	}
	
	return damage;
}

float WEAPON_OnHaleKnifeDamage ( int victim, int attacker, int damageType, int weapon, float damage ) {
	if ( loringlib_IsValidClient ( victim ) && loringlib_IsValidClient ( attacker ) ) {
		if ( g_PGameData[attacker].hale && GetClientTeam ( victim ) == HUMAN_TEAM ) {
			if ( weapon > -1 ) {
				char szClassname[32];
				qufnrTools_GetWeaponClassname ( weapon, szClassname, sizeof szClassname );
				if ( ( damageType & DMG_SLASH ) &&
					StrContains ( szClassname, "knife" ) != -1 ) {
					if ( g_HaleData[g_PGameData[attacker].haleIndex].damageMultiple < 0.0 )
						return damage;
					
					return damage * g_HaleData[g_PGameData[attacker].haleIndex].damageMultiple;
				}
			}
		}
	}
	
	return damage;
}

public void WEAPON_OnWeaponFirerateModify ( int player ) {

	if ( g_bWeaponLoaded && loringlib_IsValidClient ( player ) ) {
		int input = GetClientButtons ( player );
	
		if ( ( input & IN_ATTACK ) ) {
			int iWeapon = loringlib_GetActiveWeapon ( player );
			if ( iWeapon != -1 ) {
				char szWeapon[32];
				GetEdictClassname ( iWeapon, szWeapon, sizeof szWeapon );
				
				//	헤일 공격 속도 조정
				if ( StrContains ( szWeapon, "knife", false ) != -1 &&
					g_PGameData[player].hale ) {
					if ( GetGameTime () < GetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" ) )
						return;
					
					flModifyNextAttack[player] = 1.0 / g_HaleData[g_PGameData[player].haleIndex].attackSpeed;
					if ( flModifyNextAttack[player] == 1.0 )
						flModifyNextAttack[player] = 0.0;
						
					return;
				}
				
				int iIndexOfWeapon = GetEntProp ( iWeapon, Prop_Send, "m_iItemDefinitionIndex" );
				switch ( iIndexOfWeapon ) {
					case 23:	strcopy ( szWeapon, sizeof szWeapon, "weapon_mp5sd" );
					case 60:	strcopy ( szWeapon, sizeof szWeapon, "weapon_m4a1_silencer" );
					case 61: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_usp_silencer" );
					case 63: 	strcopy ( szWeapon, sizeof szWeapon, "weapon_cz75a" );
				}
				
				if ( GetGameTime () < GetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" ) ||
					GetEntProp ( iWeapon, Prop_Send, "m_iClip1" ) <= 0 ||
					GetEntProp ( iWeapon, Prop_Send, "m_iPrimaryAmmoType" ) < 0 )
					return;
				
				int iIndex = -1;
				for ( int i = 0; i < g_iWeaponData; i ++ ) {
					if ( StrEqual ( szWeapon, g_WeaponData[i].classname ) ) {
						iIndex = i;
						break;
					}
				}
				
				if ( iIndex > -1 ) {
					flModifyNextAttack[player] = 1.0 / g_WeaponData[iIndex].fireRate;
					if ( flModifyNextAttack[player] == 1.0 )
						flModifyNextAttack[player] = 0.0;
					
				//	PrintCenterText ( player, "Next Attack: %.2f", flModifyNextAttack[player] );
					
					return;
				}
				
				flModifyNextAttack[player] = 0.0;
			}
		}
	}
}

public void WEAPON_OnWeaponFirerateModifyPost ( int player ) {
	if ( g_bWeaponLoaded && loringlib_IsValidClient ( player ) ) {
		if ( flModifyNextAttack[player] > 0.0 ) {
			int iWeapon = loringlib_GetActiveWeapon ( player );
			if ( iWeapon != -1 ) {
				char szWeapon[32];
				GetEdictClassname ( iWeapon, szWeapon, sizeof szWeapon );
				
				float flNextPrimaryAttack = GetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack" );
				flNextPrimaryAttack -= GetGameTime ();
				flNextPrimaryAttack *= flModifyNextAttack[player];
				flNextPrimaryAttack += GetGameTime ();
				SetEntPropFloat ( iWeapon, Prop_Send, "m_flNextPrimaryAttack", flNextPrimaryAttack );
				flModifyNextAttack[player] = 0.0;
			}
		}
	}
}

stock void WEAPON_ResetPlayerViewModel ( int player ) {
	int iViewModel = GetEntPropEnt ( player, Prop_Send, "m_hViewModel" );
	if ( iViewModel != INVALID_ENT_REFERENCE )
		SetEntPropFloat ( iViewModel, Prop_Send, "m_flPlaybackRate", 1.0 );
}