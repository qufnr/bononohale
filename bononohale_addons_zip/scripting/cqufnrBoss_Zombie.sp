#include <loringlib>
#include <qufnrtools>
#include <bononohale>
#include <fpvm_interface>

public Plugin myinfo = {
	name = "Boss Zombie"
	, author = "qufnr"
	, description = "bononohale on Zombie Boss"
	, version = "1.0"
	, url = "https://steamcommunity.com/id/qufnrp"
};

#define MAIN_BOSS_INDEX		0
#define ZOMBIE_HAND_VIEWMODEL		"models/player/custom_player/zombie/normal_m_01/hand/eminem/hand_normal_m_01.mdl"

public void OnPluginStart () {
	HookEvent ( "player_spawn", onPlayerSpawn );
}

public void NONOHALE_OnHumanToHale ( int client, int hale ) {
	if ( hale == MAIN_BOSS_INDEX ) {
		FPVMI_AddViewModelToClient ( client, "weapon_knife", PrecacheModel ( ZOMBIE_HAND_VIEWMODEL ) );
		loringlib_KillWeaponSlot ( client );
		CreateTimer ( 0.1, ChangeViewmodelPost, client, TIMER_FLAG_NO_MAPCHANGE );
	}
}

public Action ChangeViewmodelPost ( Handle hndl, any client ) {
	if ( loringlib_IsValidClient__PlayGame ( client ) ) {
		GivePlayerItem ( client, "weapon_knife" );
	}
}

public void onPlayerSpawn ( Event ev, const char[] name, bool dontBroadcast ) {
	int client = GetClientOfUserId ( ev.GetInt ( "userid" ) );
	
	if ( loringlib_IsValidClient__PlayGame ( client ) ) {
		FPVMI_RemoveViewModelToClient ( client, "weapon_knife" );
		GivePlayerItem ( client, "weapon_knife" );
	}
}