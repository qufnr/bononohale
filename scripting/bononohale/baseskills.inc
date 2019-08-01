
#define		BASESKILL_JUMP_READY	1.0
#define		BASESKILL_JUMP_SCALE	850.0
#define		BASESKILL_GOOMBA_JUMPPOWER	300.0

bool g_bBaseSkillJumpHold = false;
bool g_bBaseSkillWeightDown = false;
float g_flJumpHoldTick = 0.0;
float g_flJumpHoldStartTick = 0.0;

static float flDynamicJumpCooldown = 0.0;
static float flWeightDownCooldown = 0.0;

static int iWeightDownAction[MAXPLAYERS + 1] = { -1, ... };	//	-1 None, 1 Weightdown Start, 2 Weightdown Landing

bool g_bGoombaFrameEnd[MAXPLAYERS + 1] = { false, ... };
float g_flGoombaFrameEndVel[MAXPLAYERS + 1][3];

void BASESKILL_PrecacheAll () {
	loringlib_PrecacheParticle ( HALE_STUN_FX_PATH, true );
	loringlib_PrecacheParticleName ( HALE_STUN_FX_SCARE );
	loringlib_PrecacheParticleName ( HALE_STUN_FX_STUN );
}

void BASESKILL_ResetVariables () {
	g_bBaseSkillJumpHold = false;
	g_bBaseSkillWeightDown = false;
	g_flJumpHoldTick = 0.0;
	g_flJumpHoldStartTick = 0.0;	
	flDynamicJumpCooldown = 0.0;
	flWeightDownCooldown = 0.0;
	for ( int i = 0; i < sizeof iWeightDownAction; i ++ )
		iWeightDownAction[i] = -1;
}

void BASESKILL_OnHaleBlockSecondaryAttack ( int player, int key ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) && g_PGameData[player].hale ) {
		if ( key & IN_ATTACK2 ) {
			int iActive = loringlib_GetActiveWeapon ( player );
			if ( iActive != -1 ) {
				int iKnife = GetPlayerWeaponSlot ( player, CS_SLOT_KNIFE );
				if ( iKnife != -1 ) {
					if ( iKnife == iActive ) {
						SetEntPropFloat ( iActive, Prop_Send, "m_flNextSecondaryAttack", GetGameTime () + 9999.9 );
					}
				}
			}
		}
	}
}

void BASESKILL_OnHaleWeightDown ( int player, int key ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) &&
		!IsFakeClient ( player ) &&
		g_PGameData[player].hale ) {
		
		if ( g_PGameData[player].weightDownReady ) {
			if ( !( GetEntityFlags ( player ) & FL_ONGROUND ) ) {
				float flAngles[3];
				GetClientEyeAngles ( player, flAngles );
				
				if ( flAngles[0] > 60.0 && ( key & IN_DUCK ) ) {
					if ( !g_bBaseSkillWeightDown ) {
						iWeightDownAction[player] = 1;
						
						g_bBaseSkillWeightDown = true;
						g_PGameData[player].weightDownReady = false;
						float flVel[3];
						GetEntPropVector ( player, Prop_Data, "m_vecAbsVelocity", flVel );
						flVel[2] = -1000.0;
						
						TeleportEntity ( player, NULL_VECTOR, NULL_VECTOR, flVel );
						
						float flGravity = GetEntityGravity ( player );
						SetEntityGravity ( player, 6.0 );
						DataPack data;
						flWeightDownCooldown = g_HaleData[g_PGameData[player].haleIndex].weightDownCooldown
						CreateTimer ( 0.1, BASESKILL_WeightDownCooldown, GetClientUserId ( player ), TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
						CreateDataTimer ( 1.0, BASESKILL_GravityCat, data, TIMER_FLAG_NO_MAPCHANGE );
						data.WriteCell ( GetClientUserId ( player ) );
						data.WriteFloat ( flGravity );
						
						Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_WEIGHT_DOWN] );
						Call_PushCell ( player );
						Call_Finish ();
					}
				}
			}
		}
	}
}

/**
 * Ground ent changed
 */
void BASESKILL_WeightDownLanding ( int player ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) ) {
		if ( g_PGameData[player].hale && iWeightDownAction[player] == 1 ) {
			iWeightDownAction[player] = 2;
			
			Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_WEIGHT_DOWN_LANDING] );
			Call_PushCell ( player );
			Call_Finish ();
		}
	}
}

public Action BASESKILL_WeightDownCooldown ( Handle hndl, any userid ) {
	int p = GetClientOfUserId ( userid );
	
	flWeightDownCooldown -= 0.1;
	
	if ( flWeightDownCooldown <= 0.0 ) {
		if ( loringlib_IsValidClient ( p ) )
			g_PGameData[p].weightDownReady = true;
		
		return Plugin_Stop;
	}
	return Plugin_Continue;
}


public Action BASESKILL_GravityCat ( Handle hndl, DataPack data ) {
	data.Reset ();
	
	g_bBaseSkillWeightDown = false;
	
	int p = GetClientOfUserId ( data.ReadCell () );
	
	
	if ( loringlib_IsValidClient__PlayGame ( p ) ) {
		SetEntityGravity ( p, data.ReadFloat () );
	}
	
	return Plugin_Stop;
}

void BASESKILL_OnHaleJump ( int player, int key ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) &&
		!IsFakeClient ( player ) &&
		g_PGameData[player].hale &&
		g_PGameData[player].jumpReady ) {
		float flVel[3];
		GetEntPropVector ( player, Prop_Data, "m_vecAbsVelocity", flVel );
		
		float flAngles[3];
		GetClientEyeAngles ( player, flAngles );
		
		if ( ( ( key & IN_DUCK ) && ( flAngles[0] < -45.0 ) ) || ( key & IN_ATTACK2 ) ) {
			if ( !g_bBaseSkillJumpHold ) {
				g_bBaseSkillJumpHold = true;
				
				g_flJumpHoldTick = GetGameTime ();
				g_flJumpHoldStartTick = GetGameTime ();
				
				qufnrTools_DisplayProgressBarTime ( player, RoundFloat ( BASESKILL_JUMP_READY ) );
			}
			
			
			if ( ( g_flJumpHoldTick - g_flJumpHoldStartTick ) < BASESKILL_JUMP_READY ) {
				g_flJumpHoldTick = GetGameTime ();
			}
			
		//	PrintCenterText ( player, "Hold Time: %.2f", g_flJumpHoldTick - g_flJumpHoldStartTick );
			
			
		}
		else {
			if ( g_bBaseSkillJumpHold ) {
				g_bBaseSkillJumpHold = false;
				qufnrTools_DisplayProgressBarTime ( player, 0 );
			
				if ( flAngles[0] < -45.0 &&
					( g_flJumpHoldTick - g_flJumpHoldStartTick ) > 0.3 ) {
					g_PGameData[player].jumpReady = false;
					BASESKILL_NewHaleJump ( player, g_flJumpHoldTick - g_flJumpHoldStartTick, g_HaleData[g_PGameData[player].haleIndex].jumpVector );
				
					Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_JUMP] );
					Call_PushCell ( player );
					Call_PushCell ( g_PGameData[player].haleIndex );
					Call_Finish ();
				}
			}
		}
	}
}

void BASESKILL_OnHaleRageEnable ( int player ) {
	if ( g_PGameData[player].hale ) {
		int iHaleIndex = g_PGameData[player].haleIndex;
		
		if ( g_HaleData[iHaleIndex].rageRange > 0 && g_HaleData[iHaleIndex].rageRange <= g_PGameData[player].haleRageDmgs ) {

			if ( !g_PGameData[player].haleRage ) {
				
				float flDist = g_HaleData[iHaleIndex].distOfRage;
				float flDuration = g_HaleData[iHaleIndex].durationOfRage;
				
				Action action = Plugin_Continue;
				Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_RAGE] );
				Call_PushCell ( player );
				Call_PushCell ( g_PGameData[player].haleIndex );
				Call_PushFloatRef ( flDist );
				Call_PushFloatRef ( flDuration );
				Call_Finish ( action );
				
				if ( action == Plugin_Handled || action == Plugin_Stop ||
					flDuration <= 0 )
					return;
			
				SOUND_OnPlayHaleSound ( player, HALE_SND_INDEX_RAGE );
				g_PGameData[player].haleRage = true;
				
				if ( flDist > 0 )
					BASESKILL_RangePlayerStun ( player, flDist );
				
				CreateTimer ( flDuration, BASESKILL_RageToggle, GetClientSerial ( player ), TIMER_FLAG_NO_MAPCHANGE );
			}
		}
	}
}

public Action BASESKILL_RageToggle ( Handle hndl, any serial ) {
	int p = GetClientFromSerial ( serial );
	
	if ( loringlib_IsValidClient ( p ) && g_PGameData[p].hale ) {
		if ( g_PGameData[p].haleRage ) {
			g_PGameData[p].haleRage = false;
			g_PGameData[p].haleRageDmgs = 0;

			Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_RAGE_END] );
			Call_PushCell ( p );
			Call_PushCell ( g_PGameData[p].haleIndex );
			Call_Finish ();
		}
	}
	
	return Plugin_Stop;
}

stock bool BASESKILL_RangePlayerStun ( int player, float flDist = 50.0, float stunTime = 5.0, int damage = 0 ) {
	
	bool bResult = false;
	
	if ( IsPlayerAlive ( player ) ) {
		float flPos[3], flTarPos[3];
		GetClientEyePosition ( player, flPos );
		for ( int p = 1; p <= MaxClients; p ++ ) {
			if ( loringlib_IsValidClient__PlayGame ( p ) ) {
				if ( GetClientTeam ( p ) != GetClientTeam ( player ) ) {
					GetClientEyePosition ( p, flTarPos );
					
					if ( GetVectorDistance ( flPos, flTarPos ) <= flDist ) {
						if ( !bResult )
							bResult = true;
					
						BASESKILL_SetPlayerStun ( p, player, stunTime, damage );
					}
				}
			}
		}
	}
	
	return bResult;
}

stock void BASESKILL_SetPlayerStun ( int victim, int attacker, float& stunTime, int& damage ) {
	if ( loringlib_IsValidClient__PlayGame ( victim ) && loringlib_IsValidClient ( attacker ) ) {
		if ( !g_PGameData[victim].hale && !g_PGameData[victim].stun ) {
			
			Action action = Plugin_Continue;
			Call_StartForward ( g_hForwardHandle[FORWARD_ON_CLIENT_STUN] );
			Call_PushCell ( victim );
			Call_PushCell ( attacker );
			Call_PushFloatRef ( stunTime );
			Call_PushCellRef ( damage );
			Call_Finish ( action );
			
			if ( action == Plugin_Handled || action == Plugin_Stop )
				return;
			
			g_PGameData[victim].stun = true;
			
			ClientCommand ( victim, "thirdperson" );
			
			if ( damage > 0 )
				loringlib_MakeDamage ( victim, attacker, damage, "prop_exploding_barrel", DMG_SHOCK );
			
			float flSpeed = loringlib_GetEntityLaggedmovement ( victim );
			float flGravity = GetEntityGravity ( victim );
			
			loringlib_SetEntityLaggedmovement ( victim, 0.5 );
			SetEntityGravity ( victim, 5.0 );
			
			float flEye[3];
			GetClientEyePosition ( victim, flEye );
			flEye[2] += 10.0;
			loringlib_CreateParticleEx ( victim, 0, flEye, NULL_VECTOR, HALE_STUN_FX_SCARE, true, stunTime );
			
			DataPack data;
			CreateDataTimer ( stunTime, BASESKILL_SetPlayerStunToggle, data, TIMER_FLAG_NO_MAPCHANGE );
			data.WriteCell ( GetClientSerial ( victim ) );
			data.WriteFloat ( flSpeed );
			data.WriteFloat ( flGravity );
		
		}
	}
}

public Action BASESKILL_SetPlayerStunToggle ( Handle hndl, DataPack data ) {
	data.Reset ();
	
	int p = GetClientFromSerial ( data.ReadCell () );
	
	if ( !loringlib_IsValidClient ( p ) ) {
//		delete data;
		return Plugin_Stop;
	}
	
	float flSpeed = data.ReadFloat ();
	float flGravity = data.ReadFloat ();
	
	g_PGameData[p].stun = false;
	ClientCommand ( p, "firstperson" );
	
	loringlib_SetEntityLaggedmovement ( p, flSpeed );
	SetEntityGravity ( p, flGravity );
	
//	delete data;
	return Plugin_Stop;
}

stock void BASESKILL_OldHaleJump ( int player, float holdTime, float multiplier ) {
	float flCharge = holdTime * 100.0;
	if ( flCharge > 100.0 )
		flCharge = 100.0;
	
	float flVel[3];
	GetEntPropVector ( player, Prop_Data, "m_vecVelocity", flVel );
	
	flVel[0] *= ( 1 + Sine ( flCharge * FLOAT_PI / 50 ) );
	flVel[1] *= ( 1 + Sine ( flCharge * FLOAT_PI / 50 ) );
	flVel[2] = multiplier + flCharge * 13.0;
	
//	PrintToChat ( player, "flCharge %f", flCharge );
	
	TeleportEntity ( player, NULL_VECTOR, NULL_VECTOR, flVel );
	
	SOUND_OnPlayHaleSound ( player, HALE_SND_INDEX_JUMP );
	
	CreateTimer ( g_HaleData[g_PGameData[player].haleIndex].jumpCooldown, BASESKILL_HaleJumpCooldown, GetClientSerial ( player ), TIMER_FLAG_NO_MAPCHANGE );
}

stock void BASESKILL_NewHaleJump ( int player, float holdTime, float multiplier ) {
	float flCharge = holdTime * 100.0;
	if ( flCharge > 100.0 )
		flCharge = 100.0;
	
	float flEyeAngles[3], flVelocity[3];
	GetClientEyeAngles ( player, flEyeAngles );
	GetEntPropVector ( player, Prop_Data, "m_vecVelocity", flVelocity );
	
	flVelocity[0] += Cosine ( DegToRad ( flEyeAngles[0] ) ) * Cosine ( DegToRad ( flEyeAngles[1] ) ) * 500 * multiplier;
	flVelocity[1] += Cosine ( DegToRad ( flEyeAngles[0] ) ) * Sine ( DegToRad ( flEyeAngles[1] ) ) * 500 * multiplier;
	flVelocity[2] = ( 750 + 175.0 * flCharge / 70 ) * multiplier;
	
	TeleportEntity ( player, NULL_VECTOR, NULL_VECTOR, flVelocity );
	
	SOUND_OnPlayHaleSound ( player, HALE_SND_INDEX_JUMP );
	flDynamicJumpCooldown = g_HaleData[g_PGameData[player].haleIndex].jumpCooldown;
	CreateTimer ( 0.1, BASESKILL_HaleJumpCooldown, GetClientSerial ( player ), TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT );
}

public Action BASESKILL_HaleJumpCooldown ( Handle hndl, any serial ) {
	int p = GetClientFromSerial ( serial );
	
	flDynamicJumpCooldown -= 0.1;
	
	if ( flDynamicJumpCooldown <= 0.0 ) {
		if ( loringlib_IsValidClient ( p ) )
			g_PGameData[p].jumpReady = true;
		return Plugin_Stop;
	}
	
	return Plugin_Continue;
}

void BASESKILL_CooldownHudText ( int p ) {
	if ( loringlib_IsValidClient ( p ) && !IsFakeClient ( p ) && g_PGameData[p].hale ) {
		char szInfo[256];
		
		SetGlobalTransTarget ( p );
		Format ( szInfo, sizeof szInfo, "%t\n", "hudtext_hale_baseskill_title" );
		if ( g_PGameData[p].jumpReady )
			Format ( szInfo, sizeof szInfo, "%s　%t\n", szInfo, "hudtext_hale_jump_ready" );
		else
			Format ( szInfo, sizeof szInfo, "%s　%t\n", szInfo, "hudtext_hale_jump_cooldown", flDynamicJumpCooldown );
		if ( g_PGameData[p].weightDownReady )
			Format ( szInfo, sizeof szInfo, "%s　%t\n", szInfo, "hudtext_hale_weightdown_ready" );
		else
			Format ( szInfo, sizeof szInfo, "%s　%t\n", szInfo, "hudtext_hale_weightdown_cooldown", flWeightDownCooldown );
		if ( g_PGameData[p].haleRageDmgs >= g_HaleData[g_PGameData[p].haleIndex].rageRange &&
			!g_PGameData[p].haleRage )
			Format ( szInfo, sizeof szInfo, "%s　%t\n", szInfo, "hudtext_hale_rage" );
		
		int iRGBA[4];
		qufnrTools_GetCSGOHudColor ( p, iRGBA );
		SetHudTextParams ( 0.015, 0.16, 0.2, iRGBA[0], iRGBA[1], iRGBA[2], iRGBA[3], 0, 0.0, 0.0, 0.0 );
		ShowSyncHudText ( p, g_hHudSyncHndl[HUDSYNC_HALE_BASESKILL], szInfo );
	}
}

/**
 * 굼바
 */
public Action BASESKILL_OnStartTouch ( int entity, int other ) {
	if ( loringlib_IsValidClient ( other ) &&
		loringlib_IsValidClient__PlayGame ( entity ) ) {
		
		if ( ( GetEntityMoveType ( entity ) & MOVETYPE_LADDER ) ||
			loringlib_GetEntityVerticalSpeed ( entity ) < 250.0 )
			return Plugin_Continue;
		
		float flOrigin[3];
		float flOtherOrigin[3];
		GetClientAbsOrigin ( entity, flOrigin );
		GetClientAbsOrigin ( other, flOtherOrigin );
		
		float flHeightDiff = flOrigin[2] - flOtherOrigin[2];
		
		if ( ( flHeightDiff > 70.0 ) ||
			( ( GetEntityFlags ( other ) & FL_DUCKING ) && flHeightDiff > 45.0 ) ) {
			float flVel[3];
			GetEntPropVector ( entity, Prop_Data, "m_vecAbsVelocity", flVel );
			
			if ( GetClientTeam ( entity ) != GetClientTeam ( other ) &&
				GetEntProp ( other, Prop_Data, "m_takedamage", 1 ) != 0 ) {
				
				//	Jump
				float flAngles[3];
				GetClientEyeAngles ( entity, flAngles );
				flAngles[0] = DegToRad ( flAngles[0] );
				flAngles[1] = DegToRad ( flAngles[1] );
				
				flVel[0] = BASESKILL_GOOMBA_JUMPPOWER * Cosine ( flAngles[0] ) * Cosine ( flAngles[1] );
				flVel[1] = BASESKILL_GOOMBA_JUMPPOWER * Cosine ( flAngles[0] ) * Sine ( flAngles[1] );
				flVel[2] = BASESKILL_GOOMBA_JUMPPOWER + 100.0;
				
				g_bGoombaFrameEnd[entity] = true;
				g_flGoombaFrameEndVel[entity] = flVel;
			//	TeleportEntity ( entity, NULL_VECTOR, NULL_VECTOR, flVel );
				
			//	SDKHooks_TakeDamage ( other, entity, entity, g_PGameData[entity].hale ? g_ConfigData.goombaHaleDamage : g_ConfigData.goombaHumanDamage, DMG_PREVENT_PHYSICS_FORCE | DMG_CRUSH | DMG_ALWAYSGIB );
				loringlib_MakeDamage ( entity, other, g_PGameData[entity].hale ? RoundFloat ( g_ConfigData.goombaHaleDamage ) : RoundFloat ( g_ConfigData.goombaHumanDamage ), "prop_exploding_barrel", DMG_GENERIC );
			
				Call_StartForward ( g_hForwardHandle[FORWARD_ON_CLIENT_STOMP] );
				Call_PushCell ( other );
				Call_PushCell ( entity );
				Call_Finish ();
			}
		}
	}
	
	return Plugin_Continue;
}

void BASESKILL_FrameGoombaJump ( int player ) {
	if ( loringlib_IsValidClient__PlayGame ( player ) ) {
		if ( g_bGoombaFrameEnd[player] ) {
			TeleportEntity ( player, NULL_VECTOR, NULL_VECTOR, g_flGoombaFrameEndVel[player] );
			
			if ( g_ConfigData.goombaSound ) {
				if ( !IsSoundPrecached ( HALE_GOOMBA_SND ) )
					PrecacheSound ( HALE_GOOMBA_SND, true );
				
				float flOrigin[3];
				GetClientAbsOrigin ( player, flOrigin );
				EmitSoundToAll ( HALE_GOOMBA_SND, SOUND_FROM_WORLD, _, _, _, _, _, _, flOrigin );
			}
		}
		
		g_bGoombaFrameEnd[player] = false;
	}
}

stock void BASESKILL_HaleJump ( int player, float scale ) {

	float flAngles[3];
	GetClientEyeAngles ( player, flAngles );
	if ( flAngles[0] >= -40.0 )
		return;
	
	float flVec[3], flEyePos[3], flEyeEndPos[3];
	GetClientEyePosition ( player, flEyePos );
	loringlib_GetEndPosition ( player, flEyeEndPos );
	
	MakeVectorFromPoints ( flEyePos, flEyeEndPos, flVec );
	NormalizeVector ( flVec, flVec );
	
	ScaleVector ( flVec, scale );
	TeleportEntity ( player, NULL_VECTOR, NULL_VECTOR, flVec );
	
	qufnrTools_PrintHintText ( player, "%t", "hinttext_skill_activate" );
	
	Call_StartForward ( g_hForwardHandle[FORWARD_ON_HALE_JUMP] );
	Call_PushCell ( player );
	Call_PushCell ( g_PGameData[player].haleIndex );
	Call_Finish ();
}