#include <loringlib>
#include <qufnrtools>
#include <bononohale>

public Plugin myinfo = {
	name = "Boss Hellknight"
	, author = "qufnr & nubi"
	, description = "bononohale on Hellknight Boss"
	, version = "1.0"
	, url = "https://steamcommunity.com/id/qufnrp"
};

#define MAIN_BOSS_INDEX		2

#define WEIGHT_DOWN_KNOCKBACK_DIST	400.0
#define WEIGHT_DOWN_KNOCKBACK_SCALE	10.0
#define WEIGHT_DOWN_DAMAGE		GetRandomInt ( 15, 20 )

#define RAGE_READY_DURATION		2.0

#define BULLET_PROJECTILE_SPEED	2000.0
#define BULLET_EXPLODE_SCALE	500
#define BULLET_EXPLODE_RADIUS	450
#define BULLET_MODEL	"models/props/de_dust/hr_dust/dust_garbage_container/dust_trash_bag.mdl"

#define PTC_FX_BULLET			"hellknight_bullet2"
#define PTC_FX_BULLET_EXPLODE	"hellknight_bullet2_explode"
#define PTC_FX_READY			"hellknight_ready"
#define PTC_FX_BULLET_SHOOT		"hellknight_shoot"
#define PTC_FX_SLAM				"hellknight_slam"

#define SND_FX_BULLET_EXPLODE	"qufnr/new/hale_class/hellknight/explode%d.mp3"	//	1 2
#define SND_FX_SLAM				"qufnr/new/hale_class/hellknight/slam%d.mp3"	//	1 2
#define SND_FX_BULLET_SHOOT		"qufnr/new/hale_class/hellknight/shoot1.mp3"

public void OnMapStart () {
	PrecacheModel ( BULLET_MODEL, true );
	
	loringlib_PrecacheParticle ( "particles/qufnr/new/boss/fx_hellknight.pcf", true );
	loringlib_PrecacheParticleName ( PTC_FX_BULLET );
	loringlib_PrecacheParticleName ( PTC_FX_BULLET_EXPLODE );
	loringlib_PrecacheParticleName ( PTC_FX_READY );
	loringlib_PrecacheParticleName ( PTC_FX_BULLET_SHOOT );
	loringlib_PrecacheParticleName ( PTC_FX_SLAM );
}

public Action NONOHALE_OnHaleRage ( int client, int haleIndex, float& distance, float& duration ) {
	if ( NONOHALE_GetClientHaleIndex ( client ) == MAIN_BOSS_INDEX ) {
		SetEntityMoveType ( client, MOVETYPE_NONE );
		
		SetEntProp ( client, Prop_Data, "m_takedamage", DAMAGE_NO, 1 );
		
		float flAbsOrigin[3];
		GetClientAbsOrigin ( client, flAbsOrigin );
		loringlib_CreateParticleEx ( client, 0, flAbsOrigin, NULL_VECTOR, PTC_FX_READY, true, RAGE_READY_DURATION );
		
		CreateTimer ( RAGE_READY_DURATION, HellKnight_OnRage, GetClientUserId ( client ), TIMER_FLAG_NO_MAPCHANGE );
	}
	
	return Plugin_Continue;
}

public Action HellKnight_OnRage ( Handle hndl, any userid ) {
	int client = GetClientOfUserId ( userid );
	
	if ( loringlib_IsValidClient__PlayGame ( client ) ) {
	
		float flNull[3] = { 0.0, 0.0, 0.0 };
		TeleportEntity ( client, NULL_VECTOR, NULL_VECTOR, flNull );
		SetEntityMoveType ( client, MOVETYPE_WALK );
		
		SetEntProp ( client, Prop_Data, "m_takedamage", DAMAGE_YES, 1 );

		if ( NONOHALE_GetClientHaleIndex ( client ) == MAIN_BOSS_INDEX ) {
			int entity = CreateEntityByName ( "decoy_projectile" );
			
			if ( entity != -1 ) {
				
				loringlib_SetEntityOwner2 ( entity, client );
				loringlib_SetEntityThrowner ( entity, client );
				
				float flEyeAng[3], flEyePos[3];
				float flVel[3], flPlayerVel[3];
				
				GetClientEyeAngles ( client, flEyeAng );
				GetClientEyePosition ( client, flEyePos );
				
				GetAngleVectors ( flEyeAng, flVel, NULL_VECTOR, NULL_VECTOR );
				ScaleVector ( flVel, BULLET_PROJECTILE_SPEED );
				
				GetEntPropVector ( client, Prop_Data, "m_vecVelocity", flPlayerVel );
				AddVectors ( flVel, flPlayerVel, flVel );
				
				float flSpin[3] = { 750.0, 0.0, 0.0 };
				SetEntPropVector ( entity, Prop_Data, "m_vecAngVelocity", flSpin );
				
				SDKHook ( entity, SDKHook_SpawnPost, hnf9gfu3g4j1v8df284v4jd1 );
			
				DispatchSpawn ( entity );
				TeleportEntity ( entity, flEyePos, flEyeAng, flVel );
				
				loringlib_CreateParticleEx ( entity, 0, flEyePos, flEyeAng, PTC_FX_BULLET_SHOOT );
				
				char szSnd[256];
				Format ( szSnd, sizeof szSnd, SND_FX_BULLET_SHOOT );
				if ( !IsSoundPrecached ( szSnd ) )
					PrecacheSound ( szSnd );
				EmitSoundToAll ( szSnd, SOUND_FROM_WORLD, SNDCHAN_AUTO, SNDLEVEL_NORMAL, _, _, _, _, flEyePos );
				
				float flEntPos[3];
				loringlib_GetEntityOriginEx ( entity, flEntPos );
				loringlib_CreateParticleEx ( entity, 0, flEntPos, NULL_VECTOR, PTC_FX_BULLET, true, 99.9 );
				
				SetEntProp ( entity, Prop_Send, "m_usSolidFlags", FSOLID_NOT_SOLID | FSOLID_TRIGGER );
				SetEntProp ( entity, Prop_Data, "m_nSolidType", SOLID_VPHYSICS );
				SetEntProp ( entity, Prop_Send, "m_CollisionGroup", COLLISION_GROUP_DEBRIS );
			
				SDKHook ( entity, SDKHook_EndTouchPost, HellKnight_OnTouchFireball );
			}
		}
	}
	
	return Plugin_Stop;
}

public void hnf9gfu3g4j1v8df284v4jd1 ( int entity ) {
	SetEntityModel ( entity, BULLET_MODEL );
	SetEntityMoveType ( entity, MOVETYPE_FLY );
	loringlib_SetEntityRenderColor ( entity, 0, 0, 0, 0 );
}

public void HellKnight_OnTouchFireball ( int entity, int touch ) {
	if ( entity > 0 && IsValidEntity ( entity ) ) {
		if ( touch == 0 || ( loringlib_IsValidClient__PlayGame ( touch ) && GetClientTeam ( touch ) == NONOHALE_TEAM_HUMAN ) ) {
			float flEntOrigin[3];
			loringlib_GetEntityOriginEx ( entity, flEntOrigin );
			
			int iOwner = loringlib_GetEntityThrowner ( entity );
			if ( !loringlib_IsValidClient ( iOwner ) ) {
				iOwner = 0;
			}
			
			loringlib_MakeExplosion2 ( iOwner, _, flEntOrigin, "prop_exploding_barrel", BULLET_EXPLODE_SCALE, BULLET_EXPLODE_RADIUS, SF_ENVEXPLOSION_NOPARTICLES | SF_ENVEXPLOSION_NOSMOKE | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NOFIREBALL, true );
			loringlib_CreateParticleEx ( iOwner, 0, flEntOrigin, NULL_VECTOR, PTC_FX_BULLET_EXPLODE, false, 5.0 );
			
			char szSnd[256];
			Format ( szSnd, sizeof szSnd, SND_FX_BULLET_EXPLODE, GetRandomInt ( 1, 2 ) );
			if ( !IsSoundPrecached ( szSnd ) )
				PrecacheSound ( szSnd );
			EmitSoundToAll ( szSnd, SOUND_FROM_PLAYER, SNDCHAN_AUTO, SNDLEVEL_NORMAL );
			
			AcceptEntityInput ( entity, "kill" );
		}
	}
}

public void NONOHALE_OnHaleWeightDownLanding ( int client ) {
	if ( NONOHALE_GetClientHaleIndex ( client ) == MAIN_BOSS_INDEX ) { 
		float flOrigin[3], flAngles[3];
		GetClientAbsOrigin ( client, flOrigin );
		GetClientAbsAngles ( client, flAngles );
		loringlib_ShowShakeMessage ( client, 20.0, 1.5 );
		
		loringlib_CreateParticleEx ( client, 0, flOrigin, NULL_VECTOR, PTC_FX_SLAM );
		
		char szSnd[256];
		Format ( szSnd, sizeof szSnd, SND_FX_SLAM, GetRandomInt ( 1, 2 ) );
		if ( !IsSoundPrecached ( szSnd ) )
			PrecacheSound ( szSnd );
		EmitSoundToAll ( szSnd, SOUND_FROM_WORLD, SNDCHAN_AUTO, SNDLEVEL_NORMAL, _, _, _, _, flOrigin );
		
		float flVel[3];
		float flTargetOrigin[3];
		for ( int target = 1; target <= MaxClients; target ++ ) {
			if ( loringlib_IsValidClient__PlayGame ( target ) &&
				GetClientTeam ( client ) != GetClientTeam ( target ) ) {
				GetClientAbsOrigin ( target, flTargetOrigin );
				
				if ( GetVectorDistance ( flOrigin, flTargetOrigin ) <= WEIGHT_DOWN_KNOCKBACK_DIST ) {
					GetEntPropVector ( target, Prop_Data, "m_vecAbsVelocity", flVel );
					flVel[0] = 0.0;
					flVel[1] = 0.0;
					flVel[2] = 360.0;
					TeleportEntity ( target, NULL_VECTOR, NULL_VECTOR, flVel );
					
					loringlib_MakeDamage ( client, target, WEIGHT_DOWN_DAMAGE, "prop_exploding_barrel", DMG_SHOCK );
					loringlib_ShowShakeMessage ( target, 30.0, 1.5 );
				}
			}
		}
	}
}

stock void SetKnockback ( int client, int target, float scale = 100.0 ) {
	float tempVectors[3];
	float clientEyePosition[3], targetEyePosition[3];
	float velocity[3], resultVelocity[3];
	
	GetClientEyePosition ( client, clientEyePosition );
	GetClientEyePosition ( target, targetEyePosition );
	
	MakeVectorFromPoints ( clientEyePosition, targetEyePosition, tempVectors );
	NormalizeVector ( tempVectors, tempVectors );
	
	ScaleVector ( tempVectors, scale );
	GetEntPropVector ( target, Prop_Data, "m_vecVelocity", velocity );
	AddVectors ( velocity, tempVectors, resultVelocity );
	resultVelocity[2] = 60.0;
	
	TeleportEntity ( target, NULL_VECTOR, NULL_VECTOR, resultVelocity );
}