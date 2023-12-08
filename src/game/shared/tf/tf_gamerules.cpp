//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "tf_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "tf_weaponbase.h"
#include "time.h"
#include "viewport_panel_names.h"
#include "tf_inventory.h"
#include "particle_parse.h"
#ifdef CLIENT_DLL
	#include <game/client/iviewport.h>
	#include "c_tf_player.h"
	#include "c_tf_objective_resource.h"
#else
	#include "basemultiplayerplayer.h"
	#include "voice_gamemgr.h"
	#include "items.h"
	#include "team.h"
	#include "tf_bot_temp.h"
	#include "tf_player.h"
	#include "tf_team.h"
	#include "player_resource.h"
	#include "entity_tfstart.h"
	#include "filesystem.h"
	#include "tf_obj.h"
	#include "tf_objective_resource.h"
	#include "tf_player_resource.h"
	#include "playerclass_info_parse.h"
	#include "team_control_point_master.h"
	#include "coordsize.h"
	#include "entity_healthkit.h"
	#include "tf_gamestats.h"
	#include "entity_capture_flag.h"
	#include "tf_player_resource.h"
	#include "tf_obj_sentrygun.h"
	#include "tier0/icommandline.h"
	#include "activitylist.h"
	#include "hltvdirector.h"
	#include "team_train_watcher.h"
	#include "vote_controller.h"
	#include "tf_voteissues.h"
	#include "eventqueue.h"
	#include "AI_ResponseSystem.h"
	#include "achievements_tf.h"
	#include "ai_basenpc.h"
	#include "ai_dynamiclink.h"
	#include "antlion_maker.h"
	#include "globalstate.h"
	#include "tf_weaponbase_grenadeproj.h"
	#include "tf_weapon_physcannon.h"
	#include "tf_projectile_arrow.h"
	#include "lfe_mapadd.h"
	#include "props.h"		// For props flags used in making the portal weight box
	#include "datacache/imdlcache.h"	// For precaching box model
	#include "lfe_population_manager.h"
	#include "ai_navigator.h"
	#include "ai_network.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ITEM_RESPAWN_TIME	10.0f

ConVar	sk_plr_health_drop_time( "sk_plr_health_drop_time", "30", FCVAR_REPLICATED );
ConVar	sk_plr_grenade_drop_time( "sk_plr_grenade_drop_time", "30", FCVAR_REPLICATED );

static int g_TauntCamAchievements[] = 
{
	0,		// TF_CLASS_UNDEFINED

	0,		// TF_CLASS_SCOUT,	
	0,		// TF_CLASS_SNIPER,
	0,		// TF_CLASS_SOLDIER,
	0,		// TF_CLASS_DEMOMAN,
	0,		// TF_CLASS_MEDIC,
	0,		// TF_CLASS_HEAVYWEAPONS,
	0,		// TF_CLASS_PYRO,
	0,		// TF_CLASS_SPY,
	0,		// TF_CLASS_ENGINEER,

	0,		// TF_CLASS_CIVILIAN,
	0,		// TF_CLASS_COMBINE,
	0,		// TF_CLASS_ZOMBIEFAST,
	0,		// TF_CLASS_ANTLION,

	0,		// TF_CLASS_COUNT_ALL,
};

extern ConVar mp_capstyle;
extern ConVar mp_chattime;
extern ConVar tf_arena_max_streak;
extern ConVar lfe_player_lives;

#ifdef CLIENT_DLL
extern ConVar tf_particles_disable_weather;
#endif

ConVar tf_caplinear( "tf_caplinear", "1", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "If set to 1, teams must capture control points linearly." );
ConVar tf_stalematechangeclasstime( "tf_stalematechangeclasstime", "20", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Amount of time that players are allowed to change class in stalemates." );
ConVar tf_birthday( "tf_birthday", "0", FCVAR_NOTIFY | FCVAR_REPLICATED );

ConVar alyx_darkness_force( "alyx_darkness_force", "0", FCVAR_CHEAT | FCVAR_REPLICATED );

// TF2C's cvars.
ConVar lfe_allow_thirdperson( "lfe_allow_thirdperson", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow players to switch to third person mode." );

#ifdef GAME_DLL
void CC_MPGameMode( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	/*ConVarRef var( pConVar );

	if ( var.GetInt() != var.GetInt() )
	{*/
		mp_restartgame_immediate.SetValue( 1 );

		if ( TFGameRules() )
			TFGameRules()->Reactivate();
	//}
}

ConVar mp_gamemode( "mp_gamemode", "1", FCVAR_REPLICATED, "Current game mode. \n0 = None \n1 = Co-Op\n2 = Versus \n3 = Blu Co-Op \n4 = Horde \n5 = Free", true, 0, true, 6, CC_MPGameMode );

void CC_AllowTruce( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	if ( TFGameRules() )
		TFGameRules()->SetTruce( var.GetBool() );
}
ConVar lfe_allow_truce( "lfe_allow_truce", "0", FCVAR_REPLICATED, "Determines if RED and BLU can damage each other.", CC_AllowTruce );

// TF overrides the default value of this convar
ConVar mp_waitingforplayers_time( "mp_waitingforplayers_time", "30", FCVAR_GAMEDLL | FCVAR_DEVELOPMENTONLY, "WaitingForPlayers time length in seconds" );

ConVar tf_gamemode_arena( "tf_gamemode_arena", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_cp( "tf_gamemode_cp", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_ctf( "tf_gamemode_ctf", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_sd( "tf_gamemode_sd", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_payload( "tf_gamemode_payload", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_mvm( "tf_gamemode_mvm", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_rd( "tf_gamemode_rd", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_beta_content( "tf_beta_content", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_pd( "tf_gamemode_pd", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_tc( "tf_gamemode_tc", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_passtime( "tf_gamemode_passtime", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar tf_gamemode_misc( "tf_gamemode_misc", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );

ConVar tf_gravetalk( "tf_gravetalk", "1", FCVAR_NOTIFY, "Teammates can always chat with each other whether alive or dead." );
ConVar tf_ctf_bonus_time( "tf_ctf_bonus_time", "10", FCVAR_NOTIFY, "Length of team crit time for CTF capture." );

// TF2's Tournament mode cvars
ConVar tf_tournament_classlimit_scout( "tf_tournament_classlimit_scout", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Scouts.\n" );
ConVar tf_tournament_classlimit_sniper( "tf_tournament_classlimit_sniper", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Snipers.\n" );
ConVar tf_tournament_classlimit_soldier( "tf_tournament_classlimit_soldier", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Soldiers.\n" );
ConVar tf_tournament_classlimit_demoman( "tf_tournament_classlimit_demoman", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Demomen.\n" );
ConVar tf_tournament_classlimit_medic( "tf_tournament_classlimit_medic", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Medics.\n" );
ConVar tf_tournament_classlimit_heavy( "tf_tournament_classlimit_heavy", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Heavies.\n" );
ConVar tf_tournament_classlimit_pyro( "tf_tournament_classlimit_pyro", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Pyros.\n" );
ConVar tf_tournament_classlimit_spy( "tf_tournament_classlimit_spy", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Spies.\n" );
ConVar tf_tournament_classlimit_engineer( "tf_tournament_classlimit_engineer", "-1", FCVAR_NOTIFY, "Tournament mode per-team class limit for Engineers.\n" );
ConVar tf_tournament_classchange_allowed( "tf_tournament_classchange_allowed", "1", FCVAR_NOTIFY, "Allow players to change class while the game is active?.\n" );
ConVar tf_tournament_classchange_ready_allowed( "tf_tournament_classchange_ready_allowed", "1", FCVAR_NOTIFY, "Allow players to change class after they are READY?.\n" );

ConVar tf_classlimit( "tf_classlimit", "0", FCVAR_NOTIFY, "Limit on how many players can be any class (i.e. tf_class_limit 2 would limit 2 players per class).\n" );

ConVar lfe_gamemode_coop( "lfe_gamemode_coop", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar lfe_gamemode_versus( "lfe_gamemode_versus", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar lfe_gamemode_blucoop( "lfe_gamemode_blucoop", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar lfe_gamemode_horde( "lfe_gamemode_horde", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar lfe_gamemode_free( "lfe_gamemode_free", "0" , FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar sv_dynamicnpcs_debug("sv_dynamicnpcs_debug", "0", FCVAR_REPLICATED, "Dynamic NPC System Debug.");

void CC_SVDifficulty( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	if ( TFGameRules() )
		TFGameRules()->SetSkillLevel( var.GetInt() );
}

ConVar sv_difficulty( "sv_difficulty", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE, "Set difficulty level.\n1 = Original \n2 = Medium\n3 = Hard \n4 = Unusual \n666 = Haunted" 
#ifdef GAME_DLL
, CC_SVDifficulty
#endif
);

// LFE's non-tournament mode cvars
ConVar lfe_classlimit_scout( "lfe_classlimit_scout", "-1", FCVAR_NOTIFY, "Per-team class limit for Scouts.\n" );
ConVar lfe_classlimit_sniper( "lfe_classlimit_sniper", "-1", FCVAR_NOTIFY, "Per-team class limit for Snipers.\n" );
ConVar lfe_classlimit_soldier( "lfe_classlimit_soldier", "-1", FCVAR_NOTIFY, "Per-team class limit for Soldiers.\n" );
ConVar lfe_classlimit_demoman( "lfe_classlimit_demoman", "-1", FCVAR_NOTIFY, "Per-team class limit for Demomen.\n" );
ConVar lfe_classlimit_medic( "lfe_classlimit_medic", "-1", FCVAR_NOTIFY, "Per-team class limit for Medics.\n" );
ConVar lfe_classlimit_heavy( "lfe_classlimit_heavy", "-1", FCVAR_NOTIFY, "Per-team class limit for Heavies.\n" );
ConVar lfe_classlimit_pyro( "lfe_classlimit_pyro", "-1", FCVAR_NOTIFY, "Per-team class limit for Pyros.\n" );
ConVar lfe_classlimit_spy( "lfe_classlimit_spy", "-1", FCVAR_NOTIFY, "Per-team class limit for Spies.\n" );
ConVar lfe_classlimit_engineer( "lfe_classlimit_engineer", "-1", FCVAR_NOTIFY, "Per-team class limit for Engineers.\n" );

ConVar lfe_allow_revive_marker( "lfe_allow_revive_marker", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow Revive Marker to drop on death." );
ConVar lfe_allow_ignite_prop( "lfe_allow_ignite_prop", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow flamethrower to ignite on prop_physics." );

ConVar lfe_npc_boss_addhealth_per_player( "lfe_npc_boss_addhealth_per_player", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow boss npc to have more health based on player count." );
ConVar lfe_npc_boss_min_player_count( "lfe_npc_boss_min_player_count", "2", FCVAR_NOTIFY | FCVAR_REPLICATED );

extern ConVar tf_halloween_bot_min_player_count;

ConVar tf_halloween_boss_spawn_interval( "tf_halloween_boss_spawn_interval", "240", FCVAR_CHEAT, "Average interval between boss spawns, in seconds" );
ConVar tf_halloween_boss_spawn_interval_variation( "tf_halloween_boss_spawn_interval_variation", "60", FCVAR_CHEAT, "Variation of spawn interval +/-" );

static bool isBossForceSpawning = false;
CON_COMMAND_F( tf_halloween_force_boss_spawn, "For testing.", FCVAR_CHEAT )
{
	isBossForceSpawning = true;
}
#endif
ConVar lfe_mapadd_enable( "lfe_mapadd_enable", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enables mapadd." );
ConVar sv_hl1_ff( "sv_hl1_ff", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable Scientist's Nightmare." );

void CC_ForceRestart( IConVar *pConVar, const char *oldValue, float flOldValue )
{
#ifdef GAME_DLL
	mp_restartgame_immediate.SetValue( 1 );

	if ( TFGameRules() )
		TFGameRules()->Reactivate();
#endif
}

// LF:E's cvars.
ConVar lfe_use_hl2_player_hull( "lfe_use_hl2_player_hull", "1", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar lfe_episodic_flashlight( "lfe_episodic_flashlight", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Toggle episodic flashlight drain." );
ConVar lfe_force_legacy( "lfe_force_legacy", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Force original lambda fortress style mode.", CC_ForceRestart );

ConVar lfe_coop_spawn_search( "lfe_coop_spawn_search", "1", FCVAR_REPLICATED, "Allow spawn searching instead of using checkpoints." );
ConVar lfe_coop_spawn_search_instant( "lfe_coop_spawn_search_instant", "0", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Allow player to stop searching for suitable spot and respawn instantly." );

ConVar lfe_vs_min_red_players( "lfe_vs_min_red_players", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Minumum amount of players on RED required to join BLU in Versus mode." );

ConVar lfe_forced_holiday( "lfe_forced_holiday", "0", FCVAR_NOTIFY | FCVAR_REPLICATED,
"Enables holiday mode ( 0: None\n1: TF's Birthday\n2: Halloween\n3: Christmas\n4: Community Update\n5: End of the Line\n6: Valentine's Day\n7: Meet the Pyro\n8: Full Moon\n9: Full Moon or Halloween\n10: Full Moon or Valentine\n11: April Fools\n12: LF's Birthday\n13: New Years\n"
, true, 0, true, 13 );

ConVar sv_dynamicnpcs( "sv_dynamicnpcs", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE, "Enable The Dynamic NPC System." );
ConVar sv_hl2_beta( "sv_hl2_beta", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable Half-Life 2 Beta content." );

ConVar mapedit_desiredpart("mapedit_desiredpart", "0", FCVAR_DEVELOPMENTONLY);
ConVar mapedit_desiredmap("mapedit_desiredmap", "0", FCVAR_DEVELOPMENTONLY);

ConVar lfe_mapadd_file( "lfe_mapadd_file", "", FCVAR_REPLICATED );

ConVar lfe_weapon_progression( "lfe_weapon_progression", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Weapon Progression." );

ConVar lfe_tfc_content_override( "lfe_tfc_content_override", "0", FCVAR_REPLICATED, "0 - No override\n 1 - Force disable\n 2 - Force enable", true, 0, true, 2, CC_ForceRestart );
ConVar lfe_player_bots( "lfe_player_bots", "0", FCVAR_REPLICATED, "Make all teams use robot playermodels." );
ConVar lfe_allow_antirush( "lfe_allow_antirush", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow anti-rushes.", CC_ForceRestart );

#ifdef GAME_DLL
void ValidateCapturesPerRound( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	if ( var.GetInt() <= 0 )
	{
		// reset the flag captures being played in the current round
		int nTeamCount = TFTeamMgr()->GetTeamCount();
		for ( int iTeam = FIRST_GAME_TEAM; iTeam < nTeamCount; ++iTeam )
		{
			CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
			if ( !pTeam )
				continue;

			pTeam->SetFlagCaptures( 0 );
		}
	}
}
#endif

ConVar tf_flag_caps_per_round( "tf_flag_caps_per_round", "3", FCVAR_REPLICATED, "Number of flag captures per round on CTF maps. Set to 0 to disable.", true, 0, true, 500
#ifdef GAME_DLL
							  , ValidateCapturesPerRound
#endif
							  );

#ifdef GAME_DLL
void cc_tf_medieval_changed( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	TFGameRules()->SetPlayingMedieval( var.GetBool() );
}

ConVar tf_medieval( "tf_medieval", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable Medieval Mode.", cc_tf_medieval_changed );

void cc_powerup_mode( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );

	mp_restartgame_immediate.SetValue( 1 );
	if ( TFGameRules() )
		TFGameRules()->SetPowerupMode( var.GetBool() );
}

ConVar tf_powerup_mode( "tf_powerup_mode", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable/disable powerup mode.", cc_powerup_mode );
#endif


void cc_grapplinghook_enable( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	if ( GetTFInventory() )
	{
		ConVarRef var( pConVar );
		if ( var.GetBool() )
		{
			GetTFInventory()->AddItem( "TF_WEAPON_GRAPPLINGHOOK" );
		}
		else
		{
			GetTFInventory()->RemoveItem( "TF_WEAPON_GRAPPLINGHOOK" );
		}
	}
}

ConVar tf_grapplinghook_enable( "tf_grapplinghook_enable", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "", cc_grapplinghook_enable );

void cc_spells_enabled( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	if ( GetTFInventory() )
	{
		ConVarRef var( pConVar );
		if ( var.GetBool() )
		{
			GetTFInventory()->AddItem( "TF_WEAPON_SPELLBOOK" );
		}
		else
		{
			GetTFInventory()->RemoveItem( "TF_WEAPON_SPELLBOOK" );
		}
	}
}

ConVar tf_spells_enabled( "tf_spells_enabled", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "", cc_spells_enabled );

// This sets what percentage of players are required in the changelevel trigger before map change takes effect. Currently it's set to 100% (all players required).
ConVar	lfe_transition_players_percent( "lfe_transition_players_percent", "50", FCVAR_NOTIFY | FCVAR_REPLICATED,
	"How many players in percent are needed for a level transition?" );

#define PORTAL_WEIGHT_BOX_MODEL_NAME "models/props/metal_box.mdl"
#define PORTAL_METAL_SPHERE_MODEL_NAME "models/props/sphere.mdl"
#define HL2_OILDRUM_EXPLOSIVE_MODEL_NAME "models/props_c17/oildrum001_explosive.mdl"

#ifdef GAME_DLL
// Create the box used for portal puzzles, named 'box'. Used for easy debugging of portal puzzles.
void CC_Create_PortalWeightBox( void )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Try to create entity
	CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName("prop_physics") );
	if (entity)
	{
		entity->PrecacheModel( PORTAL_WEIGHT_BOX_MODEL_NAME );
		entity->SetModel( PORTAL_WEIGHT_BOX_MODEL_NAME );
		entity->SetName( MAKE_STRING("oildrum") );
		entity->AddSpawnFlags( SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
		entity->Precache();
		DispatchSpawn(entity);

		// Now attempt to drop into the world
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors( &forward );
		Ray_t ray; ray.Init( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			tr.endpos.z += 12;
			entity->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( entity, MASK_SOLID );
		}
	}
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand ent_create_portal_weight_box("ent_create_portal_weight_box", CC_Create_PortalWeightBox, "Creates a weight box used in portal puzzles at the location the player is looking.", FCVAR_GAMEDLL | FCVAR_CHEAT);

// Create a very reflective bouncy metal sphere
void CC_Create_PortalMetalSphere( void )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Try to create entity
	CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName("prop_physics") );
	if (entity)
	{
		entity->PrecacheModel( PORTAL_METAL_SPHERE_MODEL_NAME );
		entity->SetModel( PORTAL_METAL_SPHERE_MODEL_NAME );
		entity->SetName( MAKE_STRING("sphere") );
		entity->AddSpawnFlags( SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
		entity->Precache();
		DispatchSpawn(entity);

		// Now attempt to drop into the world
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors( &forward );
		Ray_t ray; ray.Init( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			tr.endpos.z += 12;
			entity->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( entity, MASK_SOLID );
		}
	}
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand ent_create_portal_metal_sphere("ent_create_portal_metal_sphere", CC_Create_PortalMetalSphere, "Creates a reflective metal sphere where the player is looking.", FCVAR_GAMEDLL | FCVAR_CHEAT);

void CC_Create_OildrumExplosive( void )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Try to create entity
	CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName("prop_physics") );
	if (entity)
	{
		entity->PrecacheModel( HL2_OILDRUM_EXPLOSIVE_MODEL_NAME );
		entity->SetModel( HL2_OILDRUM_EXPLOSIVE_MODEL_NAME );
		entity->SetName( MAKE_STRING("box") );
		entity->AddSpawnFlags( SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
		entity->Precache();
		DispatchSpawn(entity);

		// Now attempt to drop into the world
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors( &forward );
		Ray_t ray; ray.Init( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			tr.endpos.z += 12;
			entity->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( entity, MASK_SOLID );
		}
	}
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand prop_physics_create_oildrum( "prop_physics_create_oildrum", CC_Create_OildrumExplosive, "Creates an explosive oildrum.", FCVAR_GAMEDLL | FCVAR_CHEAT );
#endif // GAME_DLL

/**
 * Player hull & eye position for standing, ducking, etc.  This version has a taller
 * player height, but goldsrc-compatible collision bounds.
 */
static CViewVectors g_HLViewVectors(
	Vector( 0, 0, 64 ),			//VEC_VIEW (m_vView)
								
	Vector(-16, -16, 0 ),		//VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),		//VEC_HULL_MAX (m_vHullMax)
													
	Vector(-16, -16, 0 ),		//VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),		//VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),			//VEC_DUCK_VIEW		(m_vDuckView)
													
	Vector(-10, -10, -10 ),		//VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),		//VEC_OBS_HULL_MAX	(m_vObsHullMax)
													
	Vector( 0, 0, 14 )			//VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)
);	

static CViewVectors g_TFViewVectors(
	Vector( 0, 0, 72 ),		//VEC_VIEW (m_vView) eye position
							
	Vector(-24, -24, 0 ),	//VEC_HULL_MIN (m_vHullMin) hull min
	Vector( 24,  24, 82 ),	//VEC_HULL_MAX (m_vHullMax) hull max
												
	Vector(-24, -24, 0 ),	//VEC_DUCK_HULL_MIN (m_vDuckHullMin) duck hull min
	Vector( 24,  24, 55 ),	//VEC_DUCK_HULL_MAX	(m_vDuckHullMax) duck hull max
	Vector( 0, 0, 45 ),		//VEC_DUCK_VIEW		(m_vDuckView) duck view
												
	Vector( -10, -10, -10 ),	//VEC_OBS_HULL_MIN	(m_vObsHullMin) observer hull min
	Vector(  10,  10,  10 ),	//VEC_OBS_HULL_MAX	(m_vObsHullMax) observer hull max
												
	Vector( 0, 0, 14 )		//VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight) dead view height
);						

Vector g_TFClassViewVectors[TF_CLASS_COUNT_ALL] =
{
	Vector( 0, 0, 72 ),		// TF_CLASS_UNDEFINED

	Vector( 0, 0, 65 ),		// TF_CLASS_SCOUT,			// TF_FIRST_NORMAL_CLASS
	Vector( 0, 0, 75 ),		// TF_CLASS_SNIPER,
	Vector( 0, 0, 68 ),		// TF_CLASS_SOLDIER,
	Vector( 0, 0, 68 ),		// TF_CLASS_DEMOMAN,
	Vector( 0, 0, 75 ),		// TF_CLASS_MEDIC,
	Vector( 0, 0, 75 ),		// TF_CLASS_HEAVYWEAPONS,
	Vector( 0, 0, 68 ),		// TF_CLASS_PYRO,
	Vector( 0, 0, 75 ),		// TF_CLASS_SPY,
	Vector( 0, 0, 68 ),		// TF_CLASS_ENGINEER,
	Vector( 0, 0, 65 ),		// TF_CLASS_CIVILIAN,
	Vector( 0, 0, 65 ),		// TF_CLASS_COMBINE,
	Vector( 0, 0, 65 ),		// TF_CLASS_ZOMBIEFAST,
	Vector( 0, 0, 55 ),		// TF_CLASS_ANTLION,
};

const CViewVectors *CTFGameRules::GetViewVectors() const
{
	if ( lfe_use_hl2_player_hull.GetBool() )
		return &g_HLViewVectors;

	return &g_TFViewVectors;
}

REGISTER_GAMERULES_CLASS( CTFGameRules );

BEGIN_NETWORK_TABLE_NOBASE( CTFGameRules, DT_TFGameRules )
#ifdef CLIENT_DLL

	RecvPropInt( RECVINFO( m_nGameType ) ),
	RecvPropInt( RECVINFO( m_nServerGameType ) ),
	RecvPropString( RECVINFO( m_pszTeamGoalStringRed ) ),
	RecvPropString( RECVINFO( m_pszTeamGoalStringBlue ) ),
	RecvPropTime( RECVINFO( m_flCapturePointEnableTime ) ),
	RecvPropInt( RECVINFO( m_nHudType ) ),
	RecvPropBool( RECVINFO( m_bPlayingKoth ) ),
	RecvPropBool( RECVINFO( m_bPlayingMedieval ) ),
	RecvPropBool( RECVINFO( m_bPlayingHybrid_CTF_CP ) ),
	RecvPropBool( RECVINFO( m_bPlayingSpecialDeliveryMode ) ),
	RecvPropBool( RECVINFO( m_bPlayingRobotDestructionMode ) ),
	RecvPropBool( RECVINFO( m_bPlayingMannVsMachine ) ),
	RecvPropBool( RECVINFO( m_bCompetitiveMode ) ),
	RecvPropBool( RECVINFO( m_bPowerupMode ) ),
	RecvPropEHandle( RECVINFO( m_hRedKothTimer ) ), 
	RecvPropEHandle( RECVINFO( m_hBlueKothTimer ) ),
	RecvPropInt( RECVINFO( m_nMapHolidayType ) ),
	RecvPropInt( RECVINFO( m_halloweenScenario ) ),
	RecvPropEHandle( RECVINFO( m_itHandle ) ),
	RecvPropBool( RECVINFO( m_bIsInTraining ) ),
	RecvPropBool( RECVINFO( m_bIsInItemTestingMode ) ),
	RecvPropBool( RECVINFO( m_bMapHasMatchSummaryStage ) ),
	RecvPropBool( RECVINFO( m_bTruceActive ) ),
	RecvPropBool( RECVINFO( m_hLevelTransitionTimer ) ),
	RecvPropString( RECVINFO( m_pszCustomUpgradesFile ) ),

#else

	SendPropInt( SENDINFO( m_nGameType ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nServerGameType ), 4, SPROP_UNSIGNED ),
	SendPropString( SENDINFO( m_pszTeamGoalStringRed ) ),
	SendPropString( SENDINFO( m_pszTeamGoalStringBlue ) ),
	SendPropTime( SENDINFO( m_flCapturePointEnableTime ) ),
	SendPropInt( SENDINFO( m_nHudType ) ),
	SendPropBool( SENDINFO( m_bPlayingKoth ) ),
	SendPropBool( SENDINFO( m_bPlayingMedieval ) ),
	SendPropBool( SENDINFO( m_bPlayingHybrid_CTF_CP ) ),
	SendPropBool( SENDINFO( m_bPlayingSpecialDeliveryMode ) ),
	SendPropBool( SENDINFO( m_bPlayingRobotDestructionMode ) ),
	SendPropBool( SENDINFO( m_bPlayingMannVsMachine ) ),
	SendPropBool( SENDINFO( m_bCompetitiveMode ) ),
	SendPropBool( SENDINFO( m_bPowerupMode ) ),
	SendPropEHandle( SENDINFO( m_hRedKothTimer ) ), 
	SendPropEHandle( SENDINFO( m_hBlueKothTimer ) ),
	SendPropInt( SENDINFO( m_nMapHolidayType ) ),
	SendPropEHandle( SENDINFO( m_itHandle ) ),
	SendPropInt( SENDINFO( m_halloweenScenario ) ),
	SendPropBool( SENDINFO( m_bIsInTraining ) ),
	SendPropBool( SENDINFO( m_bIsInItemTestingMode ) ),
	SendPropBool( SENDINFO( m_bTruceActive ) ),
	SendPropBool( SENDINFO( m_bMapHasMatchSummaryStage ) ),
	SendPropBool( SENDINFO( m_hLevelTransitionTimer ) ),
	SendPropString( SENDINFO( m_pszCustomUpgradesFile ) ),

#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_gamerules, CTFGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( TFGameRulesProxy, DT_TFGameRulesProxy )

#ifdef CLIENT_DLL
	void RecvProxy_TFGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CTFGameRules *pRules = TFGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CTFGameRulesProxy, DT_TFGameRulesProxy )
		RecvPropDataTable( "tf_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_TFGameRules ), RecvProxy_TFGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_TFGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CTFGameRules *pRules = TFGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CTFGameRulesProxy, DT_TFGameRulesProxy )
		SendPropDataTable( "tf_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_TFGameRules ), SendProxy_TFGameRules )
	END_SEND_TABLE()
#endif

ConVar  physcannon_mega_enabled("physcannon_mega_enabled", "0", FCVAR_REPLICATED);

#ifdef GAME_DLL
BEGIN_DATADESC( CTFGameRulesProxy )

	DEFINE_KEYFIELD( m_iHud_Type, FIELD_INTEGER, "hud_type"),
	//DEFINE_KEYFIELD( m_bCTF_Overtime, FIELD_BOOLEAN, "ctf_overtime" ),
	DEFINE_KEYFIELD( m_bForceDynamicNPC, FIELD_BOOLEAN, "force_dynamicnpcs" ),

	// Inputs.
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetRedTeamRespawnWaveTime", InputSetRedTeamRespawnWaveTime ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBlueTeamRespawnWaveTime", InputSetBlueTeamRespawnWaveTime ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "AddRedTeamRespawnWaveTime", InputAddRedTeamRespawnWaveTime ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "AddBlueTeamRespawnWaveTime", InputAddBlueTeamRespawnWaveTime ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetRedTeamGoalString", InputSetRedTeamGoalString ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetBlueTeamGoalString", InputSetBlueTeamGoalString ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetRedTeamRole", InputSetRedTeamRole ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetBlueTeamRole", InputSetBlueTeamRole ),
	//DEFINE_INPUTFUNC( FIELD_STRING, "SetRequiredObserverTarget", InputSetRequiredObserverTarget),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddRedTeamScore", InputAddRedTeamScore ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddBlueTeamScore", InputAddBlueTeamScore ),

	DEFINE_INPUTFUNC( FIELD_VOID, "SetRedKothClockActive", InputSetRedKothClockActive ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetBlueKothClockActive", InputSetBlueKothClockActive ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetCTFCaptureBonusTime", InputSetCTFCaptureBonusTime ),

	DEFINE_INPUTFUNC( FIELD_STRING, "PlayVORed", InputPlayVORed ),
	DEFINE_INPUTFUNC( FIELD_STRING, "PlayVOBlue", InputPlayVOBlue ),
	DEFINE_INPUTFUNC( FIELD_STRING, "PlayVO", InputPlayVO ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimeScale", InputSetTimeScale ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SpawnBoss", InputSpawnBoss ),

	// Outputs
	DEFINE_OUTPUT( m_IsHL2BetaEnabled, "IsHL2BetaEnabled" ),
	DEFINE_OUTPUT( m_IsDynamicNPCEnabled, "IsDynamicNPCEnabled" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetRedTeamRespawnWaveTime( inputdata_t &inputdata )
{
	TFGameRules()->SetTeamRespawnWaveTime( TF_TEAM_RED, inputdata.value.Float() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetBlueTeamRespawnWaveTime( inputdata_t &inputdata )
{
	TFGameRules()->SetTeamRespawnWaveTime( TF_TEAM_BLUE, inputdata.value.Float() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputAddRedTeamRespawnWaveTime( inputdata_t &inputdata )
{
	TFGameRules()->AddTeamRespawnWaveTime( TF_TEAM_RED, inputdata.value.Float() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputAddBlueTeamRespawnWaveTime( inputdata_t &inputdata )
{
	TFGameRules()->AddTeamRespawnWaveTime( TF_TEAM_BLUE, inputdata.value.Float() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetRedTeamGoalString( inputdata_t &inputdata )
{
	TFGameRules()->SetTeamGoalString( TF_TEAM_RED, inputdata.value.String() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetBlueTeamGoalString( inputdata_t &inputdata )
{
	TFGameRules()->SetTeamGoalString( TF_TEAM_BLUE, inputdata.value.String() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetRedTeamRole( inputdata_t &inputdata )
{
	CTFTeam *pTeam = TFTeamMgr()->GetTeam( TF_TEAM_RED );
	if ( pTeam )
	{
		pTeam->SetRole( inputdata.value.Int() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetBlueTeamRole( inputdata_t &inputdata )
{
	CTFTeam *pTeam = TFTeamMgr()->GetTeam( TF_TEAM_BLUE );
	if ( pTeam )
	{
		pTeam->SetRole( inputdata.value.Int() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputAddRedTeamScore( inputdata_t &inputdata )
{
	CTFTeam *pTeam = TFTeamMgr()->GetTeam( TF_TEAM_RED );
	if ( pTeam )
	{
		pTeam->AddScore( inputdata.value.Int() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputAddBlueTeamScore( inputdata_t &inputdata )
{
	CTFTeam *pTeam = TFTeamMgr()->GetTeam( TF_TEAM_BLUE );
	if ( pTeam )
	{
		pTeam->AddScore( inputdata.value.Int() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetRedKothClockActive(inputdata_t &inputdata)
{
	if ( TFGameRules() && TFGameRules()->GetRedKothRoundTimer() )
	{
		TFGameRules()->GetRedKothRoundTimer()->InputEnable( inputdata );

		if ( TFGameRules()->GetBlueKothRoundTimer() )
			TFGameRules()->GetBlueKothRoundTimer()->InputDisable( inputdata );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetBlueKothClockActive( inputdata_t &inputdata )
{
	if ( TFGameRules() && TFGameRules()->GetBlueKothRoundTimer() )
	{
		TFGameRules()->GetBlueKothRoundTimer()->InputEnable( inputdata );

		if ( TFGameRules()->GetRedKothRoundTimer() )
			TFGameRules()->GetRedKothRoundTimer()->InputDisable( inputdata );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetCTFCaptureBonusTime( inputdata_t &inputdata )
{
	if ( TFGameRules() )
	{
		TFGameRules()->m_flCTFBonusTime = inputdata.value.Float();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputPlayVO( inputdata_t &inputdata )
{
	if ( TFGameRules() )
	{
		TFGameRules()->BroadcastSound( 255, inputdata.value.String() );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputPlayVORed( inputdata_t &inputdata )
{
	if ( TFGameRules() )
	{
		TFGameRules()->BroadcastSound( TF_TEAM_RED, inputdata.value.String() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputPlayVOBlue( inputdata_t &inputdata )
{
	if ( TFGameRules() )
	{
		TFGameRules()->BroadcastSound( TF_TEAM_BLUE, inputdata.value.String() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSetTimeScale( inputdata_t &inputdata )
{
	ConVarRef host_timescale( "host_timescale" );
	host_timescale.SetValue( inputdata.value.Float() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::InputSpawnBoss( inputdata_t &inputdata )
{
	CBaseEntity *pBossSpawnpoint = gEntList.FindEntityByClassname( NULL, "info_director_boss" );
	variant_t sVariant;
	if ( pBossSpawnpoint )
		pBossSpawnpoint->AcceptInput( "SpawnBoss", NULL, NULL, sVariant, NULL );

	TFGameRules()->SetDirectorAnger( 20 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::FireGameEvent( IGameEvent *event )
{
	const char *eventName = event->GetName();
	if ( !Q_strcmp( eventName, "round_start" ) || !Q_strcmp( eventName, "teamplay_round_start" ) )
	{
		ConVarRef hl2_beta( "sv_hl2_beta" );
		if ( hl2_beta.GetInt() == 1 )
			m_IsHL2BetaEnabled.FireOutput( this, this );

		ConVarRef dynamic_npcs( "sv_dynamicnpcs" );
		if ( dynamic_npcs.GetInt() == 1 )
			m_IsDynamicNPCEnabled.FireOutput( this, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRulesProxy::Activate()
{
	TFGameRules()->Activate();

	TFGameRules()->SetHudType( m_iHud_Type );

	ConVarRef hl2_beta( "sv_hl2_beta" );
	if ( hl2_beta.GetInt() == 1 )
		m_IsHL2BetaEnabled.FireOutput( this, this );

	ConVarRef dynamic_npcs( "sv_dynamicnpcs" );
	if ( dynamic_npcs.GetInt() == 1 )
		m_IsDynamicNPCEnabled.FireOutput( this, this );

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Co-Op Logic ( RED TEAM )
//-----------------------------------------------------------------------------
class CTFLogicCoOp : public CPointEntity
{
public:
	DECLARE_CLASS(CTFLogicCoOp, CPointEntity);
	DECLARE_DATADESC();
	void	Spawn(void);
	bool	m_bIsEpisodic;
	bool	m_bVehicleSpawningAllowed;
	void	InputEnableVehicles(inputdata_t &inputdata);
	void	InputDisableVehicles(inputdata_t &inputdata);
	void	InputToggleVehicles(inputdata_t &inputdata);
	void	InputSetVehicleType(inputdata_t &inputdata);
	int		m_iVehicleType;
};

BEGIN_DATADESC(CTFLogicCoOp)
	DEFINE_KEYFIELD(m_bIsEpisodic, FIELD_INTEGER, "is_episodic"),
	DEFINE_KEYFIELD(m_bVehicleSpawningAllowed, FIELD_BOOLEAN, "vehicle_spawning_allowed"),
	DEFINE_KEYFIELD(m_iVehicleType, FIELD_INTEGER, "vehicle_type"),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableVehicles", InputEnableVehicles),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableVehicles", InputDisableVehicles),
	DEFINE_INPUTFUNC(FIELD_VOID, "ToggleVehicles", InputToggleVehicles),
	DEFINE_INPUTFUNC(FIELD_VOID, "SetVehicleType", InputSetVehicleType),
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_logic_coop, CTFLogicCoOp );

void CTFLogicCoOp::Spawn( void )
{
	BaseClass::Spawn();

	if ( m_bIsEpisodic )
	{
		hl2_episodic.SetValue( 1 );
	}
}

void CTFLogicCoOp::InputEnableVehicles(inputdata_t &inputdata)
{
	m_bVehicleSpawningAllowed = true;
}

void CTFLogicCoOp::InputDisableVehicles(inputdata_t &inputdata)
{
	m_bVehicleSpawningAllowed = false;
}

void CTFLogicCoOp::InputToggleVehicles(inputdata_t &inputdata)
{
	if (!m_bVehicleSpawningAllowed)
	{
		m_bVehicleSpawningAllowed = true;
	}
	else
	{
		m_bVehicleSpawningAllowed = false;
	}
}

void CTFLogicCoOp::InputSetVehicleType(inputdata_t &inputdata)
{
	m_iVehicleType = inputdata.value.Int();
}

//-----------------------------------------------------------------------------
// Co-Op Logic ( BLU TEAM )
//-----------------------------------------------------------------------------
class CTFLogicBluCoOp : public CPointEntity
{
public:
	DECLARE_CLASS( CTFLogicCoOp, CPointEntity );
	DECLARE_DATADESC();
	void	Spawn( void );
	bool	m_bIsEpisodic;
};

BEGIN_DATADESC( CTFLogicBluCoOp )
	DEFINE_KEYFIELD( m_bIsEpisodic, FIELD_INTEGER, "is_episodic" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_logic_blucoop, CTFLogicBluCoOp );

void CTFLogicBluCoOp::Spawn( void )
{
	BaseClass::Spawn();

	if ( m_bIsEpisodic )
	{
		hl2_episodic.SetValue( 1 );
	}
}

//-----------------------------------------------------------------------------
// Versus Logic
//-----------------------------------------------------------------------------
class CTFLogicVersus : public CPointEntity
{
public:
	DECLARE_CLASS(CTFLogicVersus, CPointEntity);
	DECLARE_DATADESC();
	void	Spawn( void );
	bool	m_bIsEpisodic;
};

BEGIN_DATADESC( CTFLogicVersus )
	DEFINE_KEYFIELD( m_bIsEpisodic, FIELD_INTEGER, "is_episodic" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS(lfe_logic_versus, CTFLogicVersus);

void CTFLogicVersus::Spawn( void )
{
	BaseClass::Spawn();
	
	if ( m_bIsEpisodic )
	{
		hl2_episodic.SetValue( 1 );
	}
}

//-----------------------------------------------------------------------------
// Infection Logic
//-----------------------------------------------------------------------------
class CTFLogicInfection : public CPointEntity
{
public:
	DECLARE_CLASS(CTFLogicInfection, CPointEntity);
	DECLARE_DATADESC();
	void	Spawn( void );
	bool	m_bIsEpisodic;
};

BEGIN_DATADESC( CTFLogicInfection )
	DEFINE_KEYFIELD( m_bIsEpisodic, FIELD_INTEGER, "is_episodic" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS(lfe_logic_infection, CTFLogicInfection);

void CTFLogicInfection::Spawn( void )
{
	BaseClass::Spawn();
	
	if ( m_bIsEpisodic )
	{
		hl2_episodic.SetValue( 1 );
	}
}

//-----------------------------------------------------------------------------
// HL1 Jetpack Long Jump
//-----------------------------------------------------------------------------
class CTFLongJump : public CPointEntity
{
public:
	DECLARE_CLASS(CTFLongJump, CPointEntity);
	void	Spawn(void);
};

LINK_ENTITY_TO_CLASS(lfe_longjump, CTFLongJump);

void CTFLongJump::Spawn(void)
{
	TFGameRules()->m_bLongJump = true;
	BaseClass::Spawn();
}

class CTFLogicVehicleSpawner : public CPointEntity
{
public:
	DECLARE_CLASS(CTFLogicVehicleSpawner, CPointEntity);
	DECLARE_DATADESC();
	CTFLogicVehicleSpawner();
	~CTFLogicVehicleSpawner();

	void	Spawn(void);
	void	SpawnVehicle(void);
	void	Think(void);

	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);
	void	InputForceSpawn(inputdata_t &inputdata);
	void	InputRemoveVehicles(inputdata_t &inputdata);
	void	InputEnableGun(inputdata_t &inputdata);
	void	InputDisableGun(inputdata_t &inputdata);
private:
	bool m_bEnabled;
	//string_t sVehicle;
	int iVehicleNum;
	int iSpawnedVehicles;
	int iDelay;
	bool m_bGunEnabled;
};

BEGIN_DATADESC( CTFLogicVehicleSpawner )
	DEFINE_INPUTFUNC(FIELD_VOID, "ForceSpawn", InputForceSpawn),
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_KEYFIELD(iVehicleNum, FIELD_INTEGER, "vehicle_to_spawn"),
	DEFINE_KEYFIELD(iDelay, FIELD_INTEGER, "delay"),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableGun", InputEnableGun),
	DEFINE_INPUTFUNC(FIELD_VOID, "RemoveVehicles", InputRemoveVehicles),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableGun", InputDisableGun),
END_DATADESC()

LINK_ENTITY_TO_CLASS(lfe_vehiclespawner, CTFLogicVehicleSpawner);

CTFLogicVehicleSpawner::CTFLogicVehicleSpawner()
{
	m_bEnabled = true;
	iSpawnedVehicles = 0;
	iDelay = 10.0;
}

CTFLogicVehicleSpawner::~CTFLogicVehicleSpawner()
{
}

void CTFLogicVehicleSpawner::Spawn(void)
{
	BaseClass::Spawn();
	Think();
}

void CTFLogicVehicleSpawner::InputDisable(inputdata_t &inputdata)
{
	m_bEnabled = false;
}

void CTFLogicVehicleSpawner::InputEnable(inputdata_t &inputdata)
{
	m_bEnabled = true;
}

void CTFLogicVehicleSpawner::InputEnableGun(inputdata_t &inputdata)
{
	m_bGunEnabled = true;
}

void CTFLogicVehicleSpawner::InputDisableGun(inputdata_t &inputdata)
{
	m_bGunEnabled = false;
}

void CTFLogicVehicleSpawner::InputForceSpawn(inputdata_t &inputdata)
{
	SpawnVehicle();
}

void CTFLogicVehicleSpawner::Think(void)
{
	SpawnVehicle();
	SetContextThink(&CTFLogicVehicleSpawner::Think, gpGlobals->curtime + iDelay, "ThinkContextVehicle");
}

void CTFLogicVehicleSpawner::InputRemoveVehicles(inputdata_t &inputdata)
{
	//this feature should and will be rewritten.
	CBaseEntity *pAirboats = gEntList.FindEntityByName(NULL, "airboatfromspawner");
	CBaseEntity *pAirboatsProtected = gEntList.FindEntityByName(NULL, "airboatfromspawner_protected");
	if (pAirboats)
	{
		pAirboats->Remove();
	}
	if (pAirboatsProtected)
	{
		pAirboatsProtected->Remove();
	}
	CBaseEntity *pJeeps = gEntList.FindEntityByName(NULL, "jeepfromspawner");
	CBaseEntity *pJeepsProtected = gEntList.FindEntityByName(NULL, "jeepfromspawner_protected");
	if (pJeeps)
	{
		pJeeps->Remove();
	}
	if (pJeepsProtected)
	{
		pJeepsProtected->Remove();
	}
	iSpawnedVehicles = 0;
}

void CTFLogicVehicleSpawner::SpawnVehicle(void)
{
	variant_t sVariant;
#ifdef GAME_DLL
	bool bCanSpawn;
	bCanSpawn = false;
	int iTeamPlayers = GetGlobalTFTeam(TF_TEAM_RED)->GetNumPlayers();
	if (m_bEnabled && iSpawnedVehicles < iTeamPlayers)
	{
		//CBaseEntity *pAirboats = gEntList.FindEntityByClassname(NULL, "prop_vehicle_airboat");
		/*
		CBaseEntity *pAirboats = gEntList.FindEntityByName(NULL, "airboatfromspawner");
		CBaseEntity *pJeeps2 = gEntList.FindEntityByName(NULL, "jeepfromspawner");
		if (pAirboats)
		{
			//if (pAirboats->GetOwnerEntity() == NULL && pAirboats->GetKeyValue("targetname", "airboatfromspawner", NULL))
			pAirboats->Remove();
			if (iSpawnedVehicles != 0)
			{
				iSpawnedVehicles = iSpawnedVehicles - 1;
			}
		}
		if (pJeeps2)
		{
			pJeeps2->Remove();
			if (iSpawnedVehicles != 0)
			{
				iSpawnedVehicles = iSpawnedVehicles - 1;
			}
		}
		*/
		CBaseEntity *pNearestVehicleSpawned = gEntList.FindEntityByClassnameNearest("prop_vehicle_jeep", GetAbsOrigin(), 1024);
		CBaseEntity *pNearestPly = ToTFPlayer(gEntList.FindEntityByClassnameNearest("player", GetAbsOrigin(), 64));
		if (iVehicleNum == 1)
		{
			//pNearestVehicleSpawned = gEntList.FindEntityByClassnameNearest("prop_vehicle_airboat", GetAbsOrigin(), 999999);
			pNearestVehicleSpawned = gEntList.FindEntityByClassnameNearest("prop_vehicle_airboat", GetAbsOrigin(), 1024);
		}
		else if (iVehicleNum == 2 || iVehicleNum == 3)
		{
			//pNearestVehicleSpawned = gEntList.FindEntityByClassnameNearest("prop_vehicle_jeep", GetAbsOrigin(), 999999);
			pNearestVehicleSpawned = gEntList.FindEntityByClassnameNearest("prop_vehicle_jeep", GetAbsOrigin(), 1024);
		}
		//float DistSpawnerToNearestPly = GetAbsOrigin().DistTo(pNearestPly->GetAbsOrigin());
		//if (pNearestVehicleSpawned && DistVehToSpawner > 1024)
		if (!pNearestVehicleSpawned && !pNearestPly)
		{
			bCanSpawn = true;
		}
		else
		{
			bCanSpawn = false;
		}
		if (iVehicleNum == 1 && bCanSpawn)
		{
			CBaseEntity *pBoat = CreateEntityByName( "prop_vehicle_airboat", -1 );
			if (pBoat)
			{
				QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
				Vector vecForward;
				AngleVectors(EyeAngles(), &vecForward);
				Vector vecOrigin = GetAbsOrigin() + vecForward * 1 + Vector(0, 0, 64);
				pBoat->KeyValue( "model", "models/airboat.mdl" );
				pBoat->KeyValue( "solid", "6" );
				pBoat->KeyValue( "targetname", "airboatfromspawner" );
				pBoat->KeyValue( "vehiclescript", "scripts/vehicles/airboat.txt" );
				pBoat->SetAbsOrigin( vecOrigin );
				pBoat->SetAbsAngles( vecAngles );
				DispatchSpawn( pBoat );
				pBoat->Activate();
				pBoat->Teleport( &vecOrigin, &vecAngles, NULL );
				pBoat->AcceptInput("FromSpawnerBoat", NULL, NULL, sVariant, NULL);
				if ( m_bGunEnabled )
				{
					pBoat->AcceptInput( "EnableGunSpawnerBoat", NULL, NULL, sVariant, NULL );
				}
			}
		}
		else if (iVehicleNum == 2 && bCanSpawn)
		{
			CBaseEntity *pJeep = CreateEntityByName( "prop_vehicle_jeep", -1 );
			if ( pJeep )
			{
				QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
				Vector vecForward;
				AngleVectors(EyeAngles(), &vecForward);
				Vector vecOrigin =GetAbsOrigin() + vecForward * 1 + Vector(0, 0, 64);
				pJeep->KeyValue( "model", "models/buggy.mdl" );
				pJeep->KeyValue( "solid", "6" );
				pJeep->KeyValue( "targetname", "jeepfromspawner" );
				pJeep->KeyValue( "vehiclescript", "scripts/vehicles/jeep_test.txt") ;
				pJeep->SetAbsOrigin( vecOrigin );
				pJeep->SetAbsAngles( vecAngles );
				DispatchSpawn( pJeep );
				pJeep->Activate();
				pJeep->Teleport(&vecOrigin, &vecAngles, NULL);
				pJeep->AcceptInput( "FromSpawner", NULL, NULL, sVariant, NULL );
				if ( m_bGunEnabled )
				{
					pJeep->AcceptInput( "EnableGunSpawner", NULL, NULL, sVariant, NULL );
				}
			}
		}
		else if ( iVehicleNum == 3 && bCanSpawn )
		{
			CBaseEntity *pJalopy = CreateEntityByName( "prop_vehicle_jeep", -1 );
			if (pJalopy)
			{
				QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
				Vector vecForward;
				AngleVectors(EyeAngles(), &vecForward);
				Vector vecOrigin = GetAbsOrigin() + vecForward * 1 + Vector(0, 0, 64);
				pJalopy->KeyValue( "model", "models/vehicle.mdl" );
				pJalopy->KeyValue( "solid", "6" );
				pJalopy->KeyValue( "targetname", "jeepfromspawner" );
				pJalopy->KeyValue( "vehiclescript", "scripts/vehicles/jalopy.txt" );
				pJalopy->SetAbsOrigin( vecOrigin );
				pJalopy->SetAbsAngles( vecAngles );
				DispatchSpawn( pJalopy );
				pJalopy->Activate();
				pJalopy->Teleport( &vecOrigin, &vecAngles, NULL );
				pJalopy->AcceptInput( "FromSpawner", NULL, NULL, sVariant, NULL );
			}
		}
		if (bCanSpawn)
		{
			iSpawnedVehicles = iSpawnedVehicles + 1;
		}
	}
#endif
}

class CTFLogicPlayerTeleports : public CPointEntity
{
public:
	DECLARE_CLASS( CTFLogicPlayerTeleports, CPointEntity );
	DECLARE_DATADESC();
	CTFLogicPlayerTeleports();
	~CTFLogicPlayerTeleports();

	void		Spawn(void);

	void		InputTeleportPlayers(inputdata_t &inputdata);
private:
	int			iTeamToTeleport;
	string_t	m_iszLinkTrigger;
};

BEGIN_DATADESC( CTFLogicPlayerTeleports )
	DEFINE_KEYFIELD( iTeamToTeleport, FIELD_INTEGER, "team_to_teleport" ),
	DEFINE_KEYFIELD( m_iszLinkTrigger,	FIELD_STRING,	"link_trigger" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "TeleportPlayers", InputTeleportPlayers ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_logic_playerteleport, CTFLogicPlayerTeleports );

CTFLogicPlayerTeleports::CTFLogicPlayerTeleports()
{
	iTeamToTeleport = 0;
}

CTFLogicPlayerTeleports::~CTFLogicPlayerTeleports()
{
}

void CTFLogicPlayerTeleports::Spawn( void )
{
	BaseClass::Spawn();
}

void CTFLogicPlayerTeleports::InputTeleportPlayers( inputdata_t &inputdata )
{
	QAngle vecAngles = GetAbsAngles();
	Vector vecOrigin = GetAbsOrigin();
	Vector velocity = vec3_origin;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pTeamPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pTeamPlayer )
			continue;

		if ( !pTeamPlayer->IsAlive() )
			continue;

		if ( m_iszLinkTrigger != NULL_STRING )
		{
			CBaseTrigger *pLinkTrigger = dynamic_cast<CBaseTrigger*>( gEntList.FindEntityByName( NULL, m_iszLinkTrigger ) );
			if ( pLinkTrigger && pLinkTrigger->IsTouching( pTeamPlayer ) )
				continue;
		}

		if ( iTeamToTeleport == 0 )
		{
			pTeamPlayer->Teleport( &vecOrigin, &vecAngles, &velocity );
		}
		else if ( iTeamToTeleport == 1 )
		{
			if ( pTeamPlayer->GetTeamNumber() == TF_TEAM_RED )
				pTeamPlayer->Teleport( &vecOrigin, &vecAngles, &velocity );
		}
		else if ( iTeamToTeleport == 2 )
		{
			if ( pTeamPlayer->GetTeamNumber() == TF_TEAM_BLUE )
				pTeamPlayer->Teleport( &vecOrigin, &vecAngles, &velocity );
		}
	}
}

LINK_ENTITY_TO_CLASS( lfe_vehicle_block, CTFVehicleBlock);

BEGIN_DATADESC( CTFVehicleBlock )
	DEFINE_KEYFIELD( m_bAllowAirboat, FIELD_INTEGER, "allow_airboat" ),
	DEFINE_KEYFIELD( m_bAllowJeep, FIELD_INTEGER, "allow_jeep" ),
	DEFINE_KEYFIELD( m_bAllowJalopy, FIELD_INTEGER, "allow_jalopy" ),
END_DATADESC()

void CTFVehicleBlock::Spawn( void )
{
	BaseClass::Spawn();
}

CTFVehicleBlock::CTFVehicleBlock()
{
	m_bAllowAirboat = true;
	m_bAllowJeep = true;
	m_bAllowJalopy = true;
}

CTFVehicleBlock::~CTFVehicleBlock()
{
}

//------------------------------------------------------------------------------
// Purpose : Logic entity that fired on computer's date
//------------------------------------------------------------------------------
class CTFLogicDate : public CPointEntity
{
public:
	DECLARE_CLASS( CTFLogicDate, CPointEntity );
	CTFLogicDate();
	~CTFLogicDate();

	void Activate(void);
	void DateThink(void);

	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_DATADESC();

private:

	// fired for specified types of map loads
	COutputEvent m_OnDate;

	int iDay;
	int iMonth;
	int iYear;
	int iHour;
	int iMinute;
	int iSecond;
};

LINK_ENTITY_TO_CLASS(lfe_logic_date, CTFLogicDate);

BEGIN_DATADESC( CTFLogicDate )

	DEFINE_KEYFIELD(iDay, FIELD_INTEGER, "day"),
	DEFINE_KEYFIELD(iMonth, FIELD_INTEGER, "month"),
	DEFINE_KEYFIELD(iYear, FIELD_INTEGER, "year"),
	DEFINE_KEYFIELD(iHour, FIELD_INTEGER, "hour"),
	DEFINE_KEYFIELD(iMinute, FIELD_INTEGER, "minute"),
	DEFINE_KEYFIELD(iMinute, FIELD_INTEGER, "second"),

	DEFINE_THINKFUNC( DateThink ),

	// Outputs
	DEFINE_OUTPUT(m_OnDate, "OnDate" ),

END_DATADESC()

CTFLogicDate::CTFLogicDate()
{
	iDay = 0;
	iMonth = 0;
	iYear = 0;
	iHour = 0;
	iMinute = 0;
	iSecond = 0;
}

CTFLogicDate::~CTFLogicDate()
{
}

//------------------------------------------------------------------------------
// Purpose : Fire my outputs here if I fire on map reload
//------------------------------------------------------------------------------
void CTFLogicDate::Activate( void )
{
	BaseClass::Activate();

	SetThink( &CTFLogicDate::DateThink );
	SetNextThink( gpGlobals->curtime );
}

/*
		tm_sec;		// seconds after the minute - [0,59]
		tm_min;		// minutes after the hour - [0,59]
		tm_hour;	// hours since midnight - [0,23]
		tm_mday;	// day of the month - [1,31]
		tm_mon;		// months since January - [0,11]
		tm_year;	// years since 1900
		tm_wday;	// days since Sunday - [0,6] 
		tm_yday;	// days since January 1 - [0,365]
		tm_isdst;	// daylight savings time flag
*/
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFLogicDate::DateThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.1 );

	time_t ltime = time(0);
	const time_t *ptime = &ltime;
	struct tm *today = localtime( ptime );
	if ( today )
	{
		if ( today->tm_mon <= iMonth && today->tm_mday <= iDay && today->tm_year <= iYear && today->tm_hour <= iHour && today->tm_min <= iMinute && today->tm_sec <= iSecond )
		{
			m_OnDate.FireOutput( NULL, this );
		}
	}
}

//------------------------------------------------------------------------------
// Purpose : Engineer building manager
//------------------------------------------------------------------------------
class CTFBuildingManager : public CPointEntity
{
public:
	DECLARE_CLASS( CTFBuildingManager, CPointEntity );
	DECLARE_DATADESC();
	CTFBuildingManager();
	~CTFBuildingManager();

	void	Spawn( void );

	void	InputDetonateAllBuilding( inputdata_t &inputdata );
	void	InputDetonateAllSentry( inputdata_t &inputdata );
	void	InputDetonateAllDispenser( inputdata_t &inputdata );
	void	InputDetonateAllTeleporter( inputdata_t &inputdata );
	void	InputDetonateAllTeleporterEntrance( inputdata_t &inputdata );
	void	InputDetonateAllTeleporterExit( inputdata_t &inputdata );
	void	InputRemoveAllBuilding( inputdata_t &inputdata );
	void	InputRemoveAllSentry( inputdata_t &inputdata );
	void	InputRemoveAllDispenser( inputdata_t &inputdata );
	void	InputRemoveAllTeleporterEntrance( inputdata_t &inputdata );
	void	InputRemoveAllTeleporterExit( inputdata_t &inputdata );
};

BEGIN_DATADESC( CTFBuildingManager )
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateAllBuilding", InputDetonateAllBuilding ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateAllSentry", InputDetonateAllSentry ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateAllDispenser", InputDetonateAllDispenser ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateAllTeleporterEntrance", InputDetonateAllTeleporterEntrance ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DetonateAllTeleporterExit", InputDetonateAllTeleporterExit ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveAllBuilding", InputRemoveAllBuilding ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveAllSentry", InputRemoveAllSentry ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveAllDispenser", InputRemoveAllDispenser ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveAllTeleporterEntrance", InputRemoveAllTeleporterEntrance ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveAllTeleporterExit", InputRemoveAllTeleporterExit ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_building_manager, CTFBuildingManager );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBuildingManager::CTFBuildingManager()
{
}


CTFBuildingManager::~CTFBuildingManager()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBuildingManager::Spawn(void)
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBuildingManager::InputDetonateAllBuilding( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer )
		{
			pPlayer->RemoveAllObjects( false );
		}
	}
}

void CTFBuildingManager::InputDetonateAllSentry( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_SENTRYGUN )
			{
				pObject->DetonateObject();
			}		
		}
	}
}

void CTFBuildingManager::InputDetonateAllDispenser( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_DISPENSER )
			{
				pObject->DetonateObject();
			}		
		}
	}
}

void CTFBuildingManager::InputDetonateAllTeleporterEntrance( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_TELEPORTER && pObject->GetObjectMode() == TELEPORTER_TYPE_ENTRANCE )
			{
				pObject->DetonateObject();
			}		
		}
	}
}

void CTFBuildingManager::InputDetonateAllTeleporterExit( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_TELEPORTER && pObject->GetObjectMode() == TELEPORTER_TYPE_EXIT )
			{
				pObject->DetonateObject();
			}		
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBuildingManager::InputRemoveAllBuilding( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer )
		{
			pPlayer->RemoveAllObjects( true );
		}
	}
}

void CTFBuildingManager::InputRemoveAllSentry( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_SENTRYGUN )
			{
				UTIL_Remove( pObject );
			}		
		}
	}
}

void CTFBuildingManager::InputRemoveAllDispenser( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_DISPENSER )
			{
				UTIL_Remove( pObject );
			}		
		}
	}
}

void CTFBuildingManager::InputRemoveAllTeleporterEntrance( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_TELEPORTER && pObject->GetObjectMode() == TELEPORTER_TYPE_ENTRANCE )
			{
				UTIL_Remove( pObject );
			}		
		}
	}
}

void CTFBuildingManager::InputRemoveAllTeleporterExit( inputdata_t &inputdata )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		for ( int iObject = pPlayer->GetObjectCount()-1; iObject >= 0; iObject-- )
		{
			CBaseObject *pObject = pPlayer->GetObject(iObject);
			Assert( pObject );

			if ( pObject && pObject->GetType() == OBJ_TELEPORTER && pObject->GetObjectMode() == TELEPORTER_TYPE_EXIT )
			{
				UTIL_Remove( pObject );
			}		
		}
	}
}


//-----------------------------------------------------------------------------
// Weapon Progression
//-----------------------------------------------------------------------------

BEGIN_DATADESC(CLFWeaponManager)
	DEFINE_KEYFIELD( m_nWeaponProgress, FIELD_INTEGER, "wpnprogress" ),

	DEFINE_KEYFIELD( m_bGravitygunProgress, FIELD_BOOLEAN, "physcannon_progress" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetWeaponProgress", InputSetWeaponProgress ),

	DEFINE_THINKFUNC( AllowedWeaponThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( lfe_weapon_progress, CLFWeaponManager );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CLFWeaponManager::CLFWeaponManager()
{
	m_nWeaponProgress = 0;
	m_bGravitygunProgress = false;
}

CLFWeaponManager::~CLFWeaponManager()
{
}
void CLFWeaponManager::Spawn( void )
{
	BaseClass::Spawn();

	SetThink( &CLFWeaponManager::AllowedWeaponThink );
	SetNextThink( gpGlobals->curtime );
}

extern ConVar lfe_force_random_weapons;
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFWeaponManager::AllowedWeaponThink( void )
{
	SetNextThink( gpGlobals->curtime );

	if ( lfe_weapon_progression.GetBool() )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CTFPlayer *pTeamPlayer = ToTFPlayer( UTIL_PlayerByIndex(i) );
			if ( !pTeamPlayer )
				continue;

			if ( !pTeamPlayer->IsAlive() )
				continue;

			if ( !pTeamPlayer->GetActiveTFWeapon() )
				continue;

			if ( FClassnameIs(pTeamPlayer->GetActiveTFWeapon(), "tf_weapon_physcannon"))
				continue;

			if ( pTeamPlayer->GetActiveTFWeapon() == pTeamPlayer->Weapon_GetSlot( LOADOUT_POSITION_ACTION ) )
				continue;

			if (m_bGravitygunProgress && !pTeamPlayer->Weapon_OwnsThisID(TF_WEAPON_PHYSCANNON) && TFGameRules()->State_Get() != GR_STATE_RND_RUNNING)
			{
				if (m_bGravitygunProgress)
				{
					pTeamPlayer->GiveNamedItem("tf_weapon_physcannon");
				}
			}
			if ( m_nWeaponProgress == 1 )
			{
				if ( !pTeamPlayer->GetActiveTFWeapon()->IsMeleeWeapon() )
				{
					int iGun = pTeamPlayer->GetActiveTFWeapon()->GetWeaponID();
					pTeamPlayer->SwitchToNextBestWeapon(pTeamPlayer->GetActiveTFWeapon());
					pTeamPlayer->TFWeaponRemove(iGun);
				}
			}
			else if ( m_nWeaponProgress >= 2 )
			{
				CTFWeaponBase *pPrimary = pTeamPlayer->Weapon_GetWeaponByType( TF_WPN_TYPE_PRIMARY );
				if ( !pPrimary )
				{
					TFPlayerClassData_t *pData = pTeamPlayer->GetPlayerClass()->GetData();
					if ( lfe_force_legacy.GetBool() || TFGameRules()->IsTFCAllowed() )
						pTeamPlayer->ManageRegularWeaponsLegacy( pData );
					else if ( lfe_force_random_weapons.GetBool() )
						pTeamPlayer->ManageRandomWeapons( pData );
					else
						pTeamPlayer->ManageRegularWeapons( pData );

					SetThink( NULL );
					SetNextThink( TICK_NEVER_THINK );
				}
			}
			else
			{
				int iGun = pTeamPlayer->GetActiveTFWeapon()->GetWeaponID();
				pTeamPlayer->SwitchToNextBestWeapon(pTeamPlayer->GetActiveTFWeapon());
				pTeamPlayer->TFWeaponRemove(iGun);
			}
		}
	}
	else
	{
		m_nWeaponProgress = 2;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLFWeaponManager::InputSetWeaponProgress( inputdata_t &inputdata )
{
	m_nWeaponProgress = inputdata.value.Int();

	SetThink( &CLFWeaponManager::AllowedWeaponThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// L4D info_gamemode
//-----------------------------------------------------------------------------
class CInfoGameMode : public CPointEntity
{
public:
	DECLARE_CLASS( CInfoGameMode, CPointEntity );
	DECLARE_DATADESC();

	void	Spawn( void );

	void	InputRoundSpawn( inputdata_t &inputdata );
	void	InputRoundActivate( inputdata_t &inputdata );

	COutputEvent			m_OnCoOp;
	COutputEvent			m_OnBluCoOp;
	COutputEvent			m_OnVersus;
	COutputEvent			m_OnHorde;
	COutputEvent			m_OnFree;
	COutputEvent			m_OnInfection;

	COutputEvent			m_OnCoOpPostIO;
	COutputEvent			m_OnBluCoOpPostIO;
	COutputEvent			m_OnVersusPostIO;
	COutputEvent			m_OnHordePostIO;
	COutputEvent			m_OnFreePostIO;
	COutputEvent			m_OnInfectionPostIO;
};

BEGIN_DATADESC( CInfoGameMode )
	DEFINE_INPUTFUNC( FIELD_VOID, "RoundSpawn", InputRoundSpawn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RoundActivate", InputRoundActivate ),

	DEFINE_OUTPUT( m_OnCoOp, "OnCoOp" ),
	DEFINE_OUTPUT( m_OnBluCoOp, "OnBluCoOp" ),
	DEFINE_OUTPUT( m_OnVersus, "OnVersus" ),
	DEFINE_OUTPUT( m_OnHorde, "OnHorde" ),
	DEFINE_OUTPUT( m_OnFree, "OnFree" ),
	DEFINE_OUTPUT( m_OnInfection, "OnInfection" ),

	DEFINE_OUTPUT( m_OnCoOpPostIO, "OnCoOpPostIO" ),
	DEFINE_OUTPUT( m_OnBluCoOpPostIO, "OnBluCoOpPostIO" ),
	DEFINE_OUTPUT( m_OnVersusPostIO, "OnVersusPostIO" ),
	DEFINE_OUTPUT( m_OnHordePostIO, "OnHordePostIO" ),
	DEFINE_OUTPUT( m_OnFreePostIO, "OnFreePostIO" ),
	DEFINE_OUTPUT( m_OnInfectionPostIO, "OnInfectionPostIO" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( info_gamemode, CInfoGameMode );

void CInfoGameMode::Spawn( void )
{
	BaseClass::Spawn();
}

void CInfoGameMode::InputRoundSpawn( inputdata_t &inputdata )
{
	if ( TFGameRules()->IsCoOp() )
		m_OnCoOp.FireOutput( this, this );

	if ( TFGameRules()->IsBluCoOp() )
		m_OnBluCoOp.FireOutput( this, this );

	if ( TFGameRules()->IsVersus() )
		m_OnVersus.FireOutput( this, this );

	if ( TFGameRules()->IsHordeMode() )
		m_OnHorde.FireOutput( this, this );

	if ( TFGameRules()->IsFreeMode() )
		m_OnFree.FireOutput( this, this );
	
	if ( TFGameRules()->IsInfectionMode() )
		m_OnInfection.FireOutput( this, this );	
}

void CInfoGameMode::InputRoundActivate( inputdata_t &inputdata )
{
	if ( TFGameRules()->IsCoOp() )
		m_OnCoOpPostIO.FireOutput( this, this );

	if ( TFGameRules()->IsBluCoOp() )
		m_OnBluCoOpPostIO.FireOutput( this, this );

	if ( TFGameRules()->IsVersus() )
		m_OnVersusPostIO.FireOutput( this, this );

	if ( TFGameRules()->IsHordeMode() )
		m_OnHordePostIO.FireOutput( this, this );

	if ( TFGameRules()->IsFreeMode() )
		m_OnFreePostIO.FireOutput( this, this );
	
	if ( TFGameRules()->IsInfectionMode() )
		m_OnInfectionPostIO.FireOutput( this, this );	
}

class CArenaLogic : public CPointEntity
{
public:
	DECLARE_CLASS( CArenaLogic, CPointEntity );
	DECLARE_DATADESC();

	CArenaLogic();

	void	Spawn( void );
	void	ArenaLogicThink( void );

	COutputEvent	m_OnArenaRoundStart;

private:
	float			m_flCapEnableDelay;
	bool			m_bCapUnlocked;

	COutputEvent	m_OnCapEnabled;
};

BEGIN_DATADESC( CArenaLogic )

	DEFINE_KEYFIELD( m_flCapEnableDelay, FIELD_FLOAT, "CapEnableDelay" ),

	// Outputs
	DEFINE_OUTPUT(	m_OnArenaRoundStart, "OnArenaRoundStart" ),
	DEFINE_OUTPUT(	m_OnCapEnabled, "OnCapEnabled" ),

	DEFINE_THINKFUNC( ArenaLogicThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_logic_arena, CArenaLogic );

CArenaLogic::CArenaLogic()
{
	m_flCapEnableDelay = 0.0f;
	m_bCapUnlocked = false;
}

void CArenaLogic::Spawn( void )
{
	BaseClass::Spawn();
#if 0
	SetThink( &CArenaLogic::ArenaLogicThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

void CArenaLogic::ArenaLogicThink( void )
{
	// Live TF2 checks m_fCapEnableTime from TFGameRules here.
#if 0
	SetNextThink( gpGlobals->curtime + 0.1 );

	if ( TFGameRules()->State_Get() == GR_STATE_STALEMATE )
	{
		if ( ObjectiveResource()->GetCPLocked( m_iCapIndex ) )
		{
			m_bCapUnlocked = false;
		}
		else if ( !m_bCapUnlocked )
		{
			m_bCapUnlocked = true;
			m_OnCapEnabled.FireOutput( this, this );
		}
	}
#endif
}


class CKothLogic : public CPointEntity
{
public:
	DECLARE_CLASS( CKothLogic, CPointEntity );
	DECLARE_DATADESC();

	CKothLogic();

	virtual void	InputAddBlueTimer( inputdata_t &inputdata );
	virtual void	InputAddRedTimer( inputdata_t &inputdata );
	virtual void	InputSetBlueTimer( inputdata_t &inputdata );
	virtual void	InputSetRedTimer( inputdata_t &inputdata );
	virtual void	InputRoundSpawn( inputdata_t &inputdata );
	virtual void	InputRoundActivate( inputdata_t &inputdata );

private:
	int m_iTimerLength;
	int m_iUnlockPoint;

};

BEGIN_DATADESC( CKothLogic )

	DEFINE_KEYFIELD( m_iTimerLength, FIELD_INTEGER, "timer_length" ),
	DEFINE_KEYFIELD( m_iUnlockPoint, FIELD_INTEGER, "unlock_point" ),

	// Inputs.
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddBlueTimer", InputAddBlueTimer ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddRedTimer", InputAddRedTimer ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetBlueTimer", InputSetBlueTimer ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetRedTimer", InputSetRedTimer ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RoundSpawn", InputRoundSpawn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RoundActivate", InputRoundActivate ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_logic_koth, CKothLogic );

CKothLogic::CKothLogic()
{
	m_iTimerLength = 180;
	m_iUnlockPoint = 30;
}

void CKothLogic::InputRoundSpawn( inputdata_t &inputdata )
{
	variant_t sVariant;

	if ( TFGameRules() )
	{
		sVariant.SetInt( m_iTimerLength );

		TFGameRules()->SetBlueKothRoundTimer( (CTeamRoundTimer*)CBaseEntity::Create( "team_round_timer", vec3_origin, vec3_angle ) );

		if ( TFGameRules()->GetBlueKothRoundTimer() )
		{
			TFGameRules()->GetBlueKothRoundTimer()->SetName( MAKE_STRING( "zz_blue_koth_timer" ) );
			TFGameRules()->GetBlueKothRoundTimer()->SetShowInHud( false );
			TFGameRules()->GetBlueKothRoundTimer()->AcceptInput( "SetTime", NULL, NULL, sVariant, 0 );
			TFGameRules()->GetBlueKothRoundTimer()->AcceptInput( "Pause", NULL, NULL, sVariant, 0 );
			TFGameRules()->GetBlueKothRoundTimer()->ChangeTeam( TF_TEAM_BLUE );
		}

		TFGameRules()->SetRedKothRoundTimer( (CTeamRoundTimer*)CBaseEntity::Create( "team_round_timer", vec3_origin, vec3_angle ) );

		if ( TFGameRules()->GetRedKothRoundTimer() )
		{
			TFGameRules()->GetRedKothRoundTimer()->SetName( MAKE_STRING( "zz_red_koth_timer" ) );
			TFGameRules()->GetRedKothRoundTimer()->SetShowInHud( false );
			TFGameRules()->GetRedKothRoundTimer()->AcceptInput( "SetTime", NULL, NULL, sVariant, 0 );
			TFGameRules()->GetRedKothRoundTimer()->AcceptInput( "Pause", NULL, NULL, sVariant, 0 );
			TFGameRules()->GetRedKothRoundTimer()->ChangeTeam( TF_TEAM_RED );
		}
	}
}

void CKothLogic::InputRoundActivate( inputdata_t &inputdata )
{
	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
	if ( !pMaster )
		return;

	for ( int i = 0; i < pMaster->GetNumPoints(); i++ )
	{
		CTeamControlPoint *pPoint = pMaster->GetControlPoint( i );

		variant_t sVariant;
		sVariant.SetInt( m_iUnlockPoint );
		pPoint->AcceptInput( "SetLocked", NULL, NULL, sVariant, 0 );
		g_EventQueue.AddEvent( pPoint, "SetUnlockTime", sVariant, 0.1, NULL, NULL );
	}
}

void CKothLogic::InputAddBlueTimer( inputdata_t &inputdata )
{
	if ( TFGameRules() && TFGameRules()->GetBlueKothRoundTimer() )
	{
		TFGameRules()->GetBlueKothRoundTimer()->AddTimerSeconds(inputdata.value.Int());
	}
}

void CKothLogic::InputAddRedTimer( inputdata_t &inputdata )
{
	if ( TFGameRules() && TFGameRules()->GetRedKothRoundTimer() )
	{
		TFGameRules()->GetRedKothRoundTimer()->AddTimerSeconds( inputdata.value.Int() );
	}
}

void CKothLogic::InputSetBlueTimer( inputdata_t &inputdata )
{
	if ( TFGameRules() && TFGameRules()->GetBlueKothRoundTimer() )
	{
		TFGameRules()->GetBlueKothRoundTimer()->SetTimeRemaining( inputdata.value.Int() );
	}
}

void CKothLogic::InputSetRedTimer( inputdata_t &inputdata )
{
	if ( TFGameRules() && TFGameRules()->GetRedKothRoundTimer() )
	{
		TFGameRules()->GetRedKothRoundTimer()->SetTimeRemaining( inputdata.value.Int() );
	}
}

class CMultipleEscort : public CPointEntity
{
public:
	DECLARE_CLASS( CMultipleEscort, CPointEntity );
};

LINK_ENTITY_TO_CLASS( tf_logic_multiple_escort, CMultipleEscort );

class CMedievalLogic : public CPointEntity
{
public:
	DECLARE_CLASS( CMedievalLogic, CPointEntity );

	void Spawn(void);
};

void CMedievalLogic::Spawn( void )
{
	BaseClass::Spawn();
}

LINK_ENTITY_TO_CLASS( tf_logic_medieval, CMedievalLogic );

class CTFHolidayEntity : public CPointEntity, public CGameEventListener
{
public:
	DECLARE_CLASS( CTFHolidayEntity, CPointEntity );
	DECLARE_DATADESC();

	CTFHolidayEntity();
	virtual ~CTFHolidayEntity() { }

	void	Spawn( void );

	virtual int UpdateTransmitState( void ) { return SetTransmitState( FL_EDICT_ALWAYS ); }
	virtual void FireGameEvent( IGameEvent *event );

	void InputHalloweenSetUsingSpells( inputdata_t& inputdata );
	void InputHalloweenTeleportToHell( inputdata_t& inputdata );
	//void HalloweenTeleportToHellDanceThink( void );
	void Teleport( void );

	int m_nHolidayType;
private:
	int m_nTauntInHell;
	int m_nAllowHaunting;
};

BEGIN_DATADESC( CTFHolidayEntity )

	DEFINE_KEYFIELD( m_nHolidayType, FIELD_INTEGER, "holiday_type" ),
	DEFINE_KEYFIELD( m_nTauntInHell, FIELD_INTEGER, "tauntInHell" ),
	DEFINE_KEYFIELD( m_nAllowHaunting, FIELD_INTEGER, "allowHaunting" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "HalloweenSetUsingSpells", InputHalloweenSetUsingSpells ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Halloween2013TeleportToHell", InputHalloweenTeleportToHell ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( tf_logic_holiday, CTFHolidayEntity );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHolidayEntity::CTFHolidayEntity()
{
	ListenForGameEvent( "player_turned_to_ghost" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_disconnect" );

	m_nHolidayType = kHoliday_None;
	m_nTauntInHell = 0;
	m_nAllowHaunting = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHolidayEntity::Spawn()
{
	BaseClass::Spawn();

	//SetThink( NULL );
	//SetNextThink( TICK_NEVER_THINK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHolidayEntity::FireGameEvent( IGameEvent *event )
{
	const char *eventName = event->GetName();
	if ( !Q_strcmp( eventName, "round_start" ) || !Q_strcmp( eventName, "teamplay_round_start" ) )
	{

	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHolidayEntity::InputHalloweenSetUsingSpells( inputdata_t& inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose: sad redmond blutarch noise
//-----------------------------------------------------------------------------
void CTFHolidayEntity::InputHalloweenTeleportToHell( inputdata_t& inputdata )
{
	int nTeamCount = TFTeamMgr()->GetTeamCount();
	for ( int iTeam = FIRST_GAME_TEAM; iTeam < nTeamCount; ++iTeam )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( pTeam )
		{
			for ( int i = 1; i < pTeam->GetNumPlayers(); i++ )
			{
				CTFPlayer *pPlayer = ToTFPlayer( pTeam->GetPlayer( i ) );
				if ( pPlayer )
				{
					color32 white = {255,255,255,255};
					UTIL_ScreenFade( pPlayer, white, 4, 4, FFADE_OUT );
					//UTIL_ScreenShake( pPlayer->GetAbsOrigin(), 100.0f, 150.0, 4.0, 2.0f, SHAKE_START );
				}
			}
		}
	}

	if ( TFGameRules() )
		TFGameRules()->BroadcastSound( 255, "Halloween.hellride" );
	
	Teleport();

	//SetThink( &CTFHolidayEntity::Teleport );
	//SetNextThink( gpGlobals->curtime + 4.0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHolidayEntity::Teleport( void )
{
	int nTeamCount = TFTeamMgr()->GetTeamCount();
	for ( int iTeam = FIRST_GAME_TEAM; iTeam < nTeamCount; ++iTeam )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( pTeam )
		{
			for ( int i = 1; i < pTeam->GetNumPlayers(); i++ )
			{
				CTFPlayer *pPlayer = ToTFPlayer( pTeam->GetPlayer( i ) );
				if ( pPlayer )
				{
					pPlayer->ForceRespawn();

					if ( pPlayer->GetTeamNumber() == TFGameRules()->GetWinningTeam() )
					{
						CBaseEntity *pTargetWinner = gEntList.FindEntityByName( NULL, "spawn_loot_winner" );
						if ( pTargetWinner )
							pPlayer->Teleport( &pTargetWinner->GetAbsOrigin(), &pTargetWinner->GetAbsAngles(), NULL );
					}
					else
					{
						CBaseEntity *pTargetLoser = gEntList.FindEntityByName( NULL, "spawn_loot_loser" );
						if ( pTargetLoser )
							pPlayer->Teleport( &pTargetLoser->GetAbsOrigin(), &pTargetLoser->GetAbsAngles(), NULL );
					}
				}
			}
		}
	}

	//SetThink( NULL );
	//SetNextThink( TICK_NEVER_THINK );
}

class CHybridMap_CTF_CP : public CPointEntity
{
public:
	DECLARE_CLASS( CHybridMap_CTF_CP, CPointEntity );
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( tf_logic_hybrid_ctf_cp, CHybridMap_CTF_CP );

void CHybridMap_CTF_CP::Spawn( void )
{
	BaseClass::Spawn();
}

class CLogicMannPower : public CPointEntity
{
public:
	DECLARE_CLASS( CLogicMannPower, CPointEntity );
	CLogicMannPower() { }
	virtual ~CLogicMannPower() { }
};

LINK_ENTITY_TO_CLASS( tf_logic_mannpower, CLogicMannPower );


#endif

// HL2 Ammo
ConVar	sk_plr_dmg_ar2			( "sk_plr_dmg_ar2","8", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_ar2			( "sk_npc_dmg_ar2","3", FCVAR_REPLICATED);
ConVar	sk_max_ar2				( "sk_max_ar2","60", FCVAR_REPLICATED);
ConVar	sk_max_ar2_altfire		( "sk_max_ar2_altfire","3", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_alyxgun		( "sk_plr_dmg_alyxgun","5", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_alyxgun		( "sk_npc_dmg_alyxgun","3", FCVAR_REPLICATED);
ConVar	sk_max_alyxgun			( "sk_max_alyxgun","150", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_pistol		( "sk_plr_dmg_pistol","5", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_pistol		( "sk_npc_dmg_pistol","3", FCVAR_REPLICATED);
ConVar	sk_max_pistol			( "sk_max_pistol","150", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_smg1			( "sk_plr_dmg_smg1","4", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_smg1			( "sk_npc_dmg_smg1","3", FCVAR_REPLICATED);
ConVar	sk_max_smg1				( "sk_max_smg1","225", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_buckshot		( "sk_plr_dmg_buckshot","8", FCVAR_REPLICATED);	
ConVar	sk_npc_dmg_buckshot		( "sk_npc_dmg_buckshot","3", FCVAR_REPLICATED);
ConVar	sk_max_buckshot			( "sk_max_buckshot","30", FCVAR_REPLICATED);
ConVar	sk_plr_num_shotgun_pellets( "sk_plr_num_shotgun_pellets","7", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_rpg_round	( "sk_plr_dmg_rpg_round","100", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_rpg_round	( "sk_npc_dmg_rpg_round","50", FCVAR_REPLICATED);
ConVar	sk_max_rpg_round		( "sk_max_rpg_round","3", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_sniper_round	( "sk_plr_dmg_sniper_round","20", FCVAR_REPLICATED);	
ConVar	sk_npc_dmg_sniper_round	( "sk_npc_dmg_sniper_round","100", FCVAR_REPLICATED);
ConVar	sk_max_sniper_round		( "sk_max_sniper_round","30", FCVAR_REPLICATED);

//ConVar	sk_max_slam				( "sk_max_slam","0", FCVAR_REPLICATED);
//ConVar	sk_max_tripwire			( "sk_max_tripwire","0", FCVAR_REPLICATED);

//ConVar	sk_plr_dmg_molotov		( "sk_plr_dmg_molotov","0", FCVAR_REPLICATED);
//ConVar	sk_npc_dmg_molotov		( "sk_npc_dmg_molotov","0", FCVAR_REPLICATED);
//ConVar	sk_max_molotov			( "sk_max_molotov","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_grenade		( "sk_plr_dmg_grenade","100", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_grenade		( "sk_npc_dmg_grenade","50", FCVAR_REPLICATED);
ConVar	sk_max_grenade			( "sk_max_grenade","5", FCVAR_REPLICATED);

ConVar	sk_max_hopwire			( "sk_max_hopwire", "3", FCVAR_REPLICATED);
ConVar	sk_max_striderbuster	( "sk_max_striderbuster", "3", FCVAR_REPLICATED);

//ConVar sk_plr_dmg_brickbat	( "sk_plr_dmg_brickbat","0", FCVAR_REPLICATED);
//ConVar sk_npc_dmg_brickbat	( "sk_npc_dmg_brickbat","0", FCVAR_REPLICATED);
//ConVar sk_max_brickbat		( "sk_max_brickbat","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_smg1_grenade	( "sk_plr_dmg_smg1_grenade","100", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_smg1_grenade	( "sk_npc_dmg_smg1_grenade","50", FCVAR_REPLICATED);
ConVar	sk_max_smg1_grenade		( "sk_max_smg1_grenade","3", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_357			( "sk_plr_dmg_357", "40", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_357			( "sk_npc_dmg_357", "30", FCVAR_REPLICATED );
ConVar	sk_max_357				( "sk_max_357", "12", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_crossbow		( "sk_plr_dmg_crossbow", "100", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_crossbow		( "sk_npc_dmg_crossbow", "10", FCVAR_REPLICATED );
ConVar	sk_max_crossbow			( "sk_max_crossbow", "10", FCVAR_REPLICATED );

ConVar	sk_dmg_sniper_penetrate_plr( "sk_dmg_sniper_penetrate_plr","10", FCVAR_REPLICATED);
ConVar	sk_dmg_sniper_penetrate_npc( "sk_dmg_sniper_penetrate_npc","100", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_airboat		( "sk_plr_dmg_airboat", "3", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_airboat		( "sk_npc_dmg_airboat", "3", FCVAR_REPLICATED );

ConVar	sk_max_gauss_round		( "sk_max_gauss_round", "30", FCVAR_REPLICATED );

// Gunship & Dropship cannons
ConVar	sk_npc_dmg_gunship			( "sk_npc_dmg_gunship", "40", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_gunship_to_plr	( "sk_npc_dmg_gunship_to_plr", "3", FCVAR_REPLICATED );


// (We clamp ammo ourselves elsewhere).
ConVar ammo_max( "ammo_max", "5000", FCVAR_REPLICATED );

// Half-Life 1
ConVar sk_plr_dmg_hl1_crowbar("sk_plr_dmg_hl1_crowbar", "0", FCVAR_REPLICATED);

ConVar sk_npc_dmg_9mm_bullet("sk_npc_dmg_9mm_bullet", "0", FCVAR_REPLICATED);
ConVar sk_plr_dmg_9mm_bullet("sk_plr_dmg_9mm_bullet", "0", FCVAR_REPLICATED);
ConVar sk_max_9mm_bullet("sk_max_9mm_bullet", "0", FCVAR_REPLICATED);

ConVar sk_npc_dmg_9mmAR_bullet("sk_npc_dmg_9mmAR_bullet", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_hl1_357_bullet("sk_plr_dmg_hl1_357_bullet", "0", FCVAR_REPLICATED);
ConVar sk_max_hl1_357_bullet("sk_max_hl1_357_bullet", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_hl1_buckshot("sk_plr_dmg_hl1_buckshot", "0", FCVAR_REPLICATED);
ConVar sk_max_hl1_buckshot("sk_max_hl1_buckshot", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_mp5_grenade("sk_plr_dmg_mp5_grenade", "0", FCVAR_REPLICATED);
ConVar sk_max_mp5_grenade("sk_max_mp5_grenade", "0", FCVAR_REPLICATED);
ConVar sk_mp5_grenade_radius("sk_mp5_grenade_radius", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_hl1_rpg("sk_plr_dmg_hl1_rpg", "0", FCVAR_REPLICATED);
ConVar sk_max_hl1_rpg_rocket("sk_max_hl1_rpg_rocket", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_xbow_hl1_bolt_plr("sk_plr_dmg_xbow_hl1_bolt_plr", "0", FCVAR_REPLICATED);
ConVar sk_plr_dmg_xbow_hl1_bolt_npc("sk_plr_dmg_xbow_hl1_bolt_npc", "0", FCVAR_REPLICATED);
ConVar sk_max_xbow_hl1_bolt("sk_max_xbow_hl1_bolt", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_egon_narrow("sk_plr_dmg_egon_narrow", "0", FCVAR_REPLICATED);
ConVar sk_plr_dmg_egon_wide("sk_plr_dmg_egon_wide", "0", FCVAR_REPLICATED);
ConVar sk_max_uranium("sk_max_uranium", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_gauss("sk_plr_dmg_gauss", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_hl1_grenade("sk_plr_dmg_hl1_grenade", "0", FCVAR_REPLICATED);
ConVar sk_max_hl1_grenade("sk_max_hl1_grenade", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_hornet("sk_plr_dmg_hornet", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_hornet("sk_npc_dmg_hornet", "0", FCVAR_REPLICATED);
ConVar sk_max_hornet("sk_max_hornet", "0", FCVAR_REPLICATED);

ConVar sk_max_snark("sk_max_snark", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_tripmine("sk_plr_dmg_tripmine", "0", FCVAR_REPLICATED);
ConVar sk_max_tripmine("sk_max_tripmine", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_satchel("sk_plr_dmg_satchel", "0", FCVAR_REPLICATED);
ConVar sk_max_satchel("sk_max_satchel", "0", FCVAR_REPLICATED);

ConVar sk_npc_dmg_12mm_bullet("sk_npc_dmg_12mm_bullet", "0", FCVAR_REPLICATED);

ConVar sk_mp_dmg_multiplier("sk_mp_dmg_multiplier", "2.0");

#if 0
#ifndef CLIENT_DLL
ConVar sk_plr_dmg_grenade( "sk_plr_dmg_grenade","0");		// Very lame that the base code needs this defined
#endif
#endif

bool CTFGameRules::IsInHL1Map()
{
#ifdef GAME_DLL
	const char *map = STRING(gpGlobals->mapname);
#else
	const char *map = engine->GetLevelName();
	char mapfile[32];
	V_FileBase(map, mapfile, sizeof(mapfile));
	V_StripExtension(mapfile, mapfile, sizeof(mapfile));
	map = mapfile;
#endif

	if (map[0] == '\0')
		return false;

	//t0_*
	if (V_strnicmp(map, "t0", 2) == 0)
		return true;

	//c0_*
	if (V_strnicmp(map, "c0", 2) == 0)
		return true;

	//c1_*
	if (V_strnicmp(map, "c1", 2) == 0)
		return true;

	//c2_*
	if (V_strnicmp(map, "c2", 2) == 0)
		return true;

	//c3_*
	if (V_strnicmp(map, "c3", 2) == 0)
		return true;

	//c4_*
	if (V_strnicmp(map, "c4", 2) == 0)
		return true;

	//c5_*
	if (V_strnicmp(map, "c5", 2) == 0)
		return true;
	
	//lfe_hl1_*
	if (V_strnicmp(map, "lfe_hl1", 7) == 0)
		return true;

	//t0a0b1
	int size = V_strlen(map);
	if (size > 6)
		return false;

	//t0a0
	//t0a0a
	//t0a0b1
	bool basecheck = (V_isalpha(map[0]) && V_isdigit(map[1]) &&
		V_isalpha(map[2]) && V_isdigit(map[3]));

	if (size == 4)
		return basecheck;
	else if (size == 5)
		return (basecheck && V_isalpha(map[4]));
	else if (size == 6)
		return (basecheck && V_isalpha(map[4]) && V_isdigit(map[5]));

	return false;
}

bool CTFGameRules::IsInHL2Map()
{
#ifdef GAME_DLL
	const char *map = STRING(gpGlobals->mapname);
#else
	const char *map = engine->GetLevelName();
	char mapfile[32];
	V_FileBase(map, mapfile, sizeof(mapfile));
	V_StripExtension(mapfile, mapfile, sizeof(mapfile));
	map = mapfile;
#endif

	if (map[0] == '\0')
		return false;

	if (V_strnicmp(map, "d1_", 3) == 0)
		return true;

	if (V_strnicmp(map, "d2_", 3) == 0)
		return true;

	if (V_strnicmp(map, "d3_", 3) == 0)
		return true;

	return false;
}

bool CTFGameRules::IsInHL2EP1Map()
{
#ifdef GAME_DLL
	const char *map = STRING(gpGlobals->mapname);
#else
	const char *map = engine->GetLevelName();
	char mapfile[32];
	V_FileBase(map, mapfile, sizeof(mapfile));
	V_StripExtension(mapfile, mapfile, sizeof(mapfile));
	map = mapfile;
#endif

	if (map[0] == '\0')
		return false;

	if (V_strnicmp(map, "ep1_", 4) == 0)
		return true;

	return false;
}

bool CTFGameRules::IsInHL2EP2Map()
{
#ifdef GAME_DLL
	const char *map = STRING(gpGlobals->mapname);
#else
	const char *map = engine->GetLevelName();
	char mapfile[32];
	V_FileBase(map, mapfile, sizeof(mapfile));
	V_StripExtension(mapfile, mapfile, sizeof(mapfile));
	map = mapfile;
#endif

	if (map[0] == '\0')
		return false;

	if (V_strnicmp(map, "ep2_", 4) == 0)
		return true;

	return false;
}


bool CTFGameRules::IsInPortalMap()
{
#ifdef GAME_DLL
	const char *map = STRING(gpGlobals->mapname);
#else
	const char *map = engine->GetLevelName();
	char mapfile[32];
	V_FileBase(map, mapfile, sizeof(mapfile));
	V_StripExtension(mapfile, mapfile, sizeof(mapfile));
	map = mapfile;
#endif

	if (map[0] == '\0')
		return false;

	if (V_strnicmp(map, "testchmb_a_", 11) == 0)
		return true;

	if (V_strnicmp(map, "escape_", 7) == 0)
		return true;

	if (V_strnicmp(map, "lfe_portal_", 11) == 0)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: point_servercommand's official maps
//-----------------------------------------------------------------------------
bool CTFGameRules::IsValveMap()
{
	if ( IsInHL1Map() )
		return true;

	if ( IsInHL2Map() )
		return true;

	if ( IsInHL2EP1Map() )
		return true;

	if ( IsInHL2EP2Map() )
		return true;

	if ( IsInPortalMap() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFGameRules::Damage_IsTimeBased( int iDmgType )
{
	// Damage types that are time-based.
	return ( ( iDmgType & ( DMG_PARALYZE /*| DMG_NERVEGAS*/ | DMG_DROWNRECOVER ) ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFGameRules::Damage_ShowOnHUD( int iDmgType )
{
	// Damage types that have client HUD art.
	return ( ( iDmgType & ( DMG_DROWN | DMG_BURN /*| DMG_NERVEGAS*/ | DMG_SHOCK ) ) != 0 );
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFGameRules::Damage_ShouldNotBleed( int iDmgType )
{
	// Should always bleed currently.
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::Damage_GetTimeBased( void )
{
	int iDamage = ( DMG_PARALYZE /*| DMG_NERVEGAS*/ | DMG_DROWNRECOVER );
	return iDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::Damage_GetShowOnHud( void )
{
	int iDamage = ( DMG_DROWN | DMG_BURN /*| DMG_NERVEGAS*/ | DMG_SHOCK );
	return iDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFGameRules::Damage_GetShouldNotBleed( void )
{
	return 0;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Whether or not the NPC should drop a health vial
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFGameRules::NPC_ShouldDropHealth( CBasePlayer *pRecipient )
{
	// Can only do this every so often
	if (m_flLastHealthDropTime > gpGlobals->curtime)
		return false;

	//Try to throw dynamic health
	float healthPerc = ((float)pRecipient->m_iHealth / (float)pRecipient->m_iMaxHealth);

	if (random->RandomFloat(0.0f, 1.0f) > healthPerc*1.5f)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Update the drop counter for health
//-----------------------------------------------------------------------------
void CTFGameRules::NPC_DroppedHealth( void )
{
	m_flLastHealthDropTime = gpGlobals->curtime + sk_plr_health_drop_time.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the NPC should drop a health vial
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFGameRules::NPC_ShouldDropGrenade( CBasePlayer *pRecipient )
{
	// Can only do this every so often
	if ( m_flLastGrenadeDropTime > gpGlobals->curtime )
		return false;

	CTFPlayer *pTFPlayer = ToTFPlayer( pRecipient );
	if ( pTFPlayer )
	{
		// If we're not maxed out on grenades and we've randomly okay'd it
		if ( ( pTFPlayer->GetAmmoCount( TF_AMMO_PRIMARY ) < pTFPlayer->GetMaxAmmo( TF_AMMO_PRIMARY )  ) && ( random->RandomInt( 0, 2 ) == 0 ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Update the drop counter for grenades
//-----------------------------------------------------------------------------
void CTFGameRules::NPC_DroppedGrenade( void )
{
	m_flLastGrenadeDropTime = gpGlobals->curtime + sk_plr_grenade_drop_time.GetFloat();
}
	
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGameRules::ChangeLevel( const char *pszMap, bool bWait, bool bSave, bool bForceWin )
{
	/*if ( !m_hLevelTransitionTimer )
	{
		variant_t sVariant;
		m_hLevelTransitionTimer = (CTeamRoundTimer*)CBaseEntity::Create( "team_round_timer", vec3_origin, vec3_angle );
		m_hLevelTransitionTimer->SetName( MAKE_STRING("zz_lfe_level_transition_timer") );
		m_hLevelTransitionTimer->KeyValue( "show_in_hud", "1" );
		sVariant.SetInt( 30 );
		m_hLevelTransitionTimer->AcceptInput( "SetTime", NULL, NULL, sVariant, 0 );
		m_hLevelTransitionTimer->AcceptInput( "Resume", NULL, NULL, sVariant, 0 );
		m_hLevelTransitionTimer->AcceptInput( "Enable", NULL, NULL, sVariant, 0 );
	}*/

	for ( int i = 0; i < gpGlobals->maxClients; i++ )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Level Transition...\n" );

		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			/*if ( someonerushtheneveryonenegativereaction )
				pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_STALEMATE );
			else*/
				pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_CAPTURED_POINT );
		}

		if ( bSave )
		{
			if ( pPlayer )
				pPlayer->SaveForTransition();
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "tf_changelevel" );
		if ( event )
		{
			gameeventmanager->FireEvent( event );
		}

		engine->ChangeLevel( pszMap, NULL );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFGameRules::CTFGameRules()
{
	m_bMegaPhysgun = false;
	m_bDarknessMode = false;
	m_iDirectorAnger = 0;
	m_iMaxDirectorAnger = 100;
#ifdef GAME_DLL
	m_iDifficultyLevel = sv_difficulty.GetInt();
	// Create teams.
	TFTeamMgr()->Init();

	ResetMapTime();

	m_flIntermissionEndTime = 0.0f;
	m_flNextPeriodicThink = 0.0f;

	m_flLastHealthDropTime = 0.0f;
	m_flLastGrenadeDropTime = 0.0f;

	ListenForGameEvent( "teamplay_point_captured" );
	ListenForGameEvent( "teamplay_capture_blocked" );
	ListenForGameEvent( "teamplay_round_win" );
	ListenForGameEvent( "teamplay_flag_event" );

	Q_memset( m_vecPlayerPositions,0, sizeof(m_vecPlayerPositions) );

	m_iPrevRoundState = -1;
	m_iCurrentRoundState = -1;
	m_iCurrentMiniRoundMask = 0;

	m_bFirstBlood = false;
	m_iArenaTeamCount = 0;
	m_flCTFBonusTime = -1;

	// Lets execute a map specific cfg file
	// ** execute this after server.cfg!
	char szCommand[32];
	Q_snprintf( szCommand, sizeof( szCommand ), "exec %s.cfg\n", STRING( gpGlobals->mapname ) );
	engine->ServerCommand( szCommand );
#else // GAME_DLL

	ListenForGameEvent( "game_newmap" );
	
#endif

	m_flCapturePointEnableTime = 0;

	// Initialize the game type
	m_nGameType.Set( TF_GAMETYPE_UNDEFINED );
	m_nServerGameType.Set( TF_GAMETYPE_UNDEFINED );

	// Initialize the team manager here, etc...

	// If you hit these asserts its because you added or removed a weapon type 
	// and didn't also add or remove the weapon name or damage type from the
	// arrays defined in tf_shareddefs.cpp
	Assert( g_aWeaponDamageTypes[TF_WEAPON_COUNT] == TF_DMG_SENTINEL_VALUE );
	Assert( FStrEq( g_aWeaponNames[TF_WEAPON_COUNT], "TF_WEAPON_COUNT" ) );
//	Assert( FStrEq( g_TFCondNames[TF_COND_LAST], "TF_COND_LAST" ) );

	m_flGravityScale = 1.0f;

	m_iPreviousRoundWinners = TEAM_UNASSIGNED;
	m_nMapHolidayType = kHoliday_None;

	m_pszTeamGoalStringRed.GetForModify()[0] = '\0';
	m_pszTeamGoalStringBlue.GetForModify()[0] = '\0';

	m_bIsInTraining = false;
	m_bIsInItemTestingMode = false;
	m_bPlayingKoth = false;
	m_bPlayingMedieval = false;
	m_bPlayingHybrid_CTF_CP = false;
	m_bPlayingSpecialDeliveryMode = false;
	m_bPlayingRobotDestructionMode = false;
	m_bPlayingMannVsMachine = false;
	m_bCompetitiveMode = false;
	m_bPowerupMode = false;
	m_bTruceActive = false;
	m_bMapHasMatchSummaryStage = false;

	m_pszCustomUpgradesFile.GetForModify()[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::FlagsMayBeCapped( void )
{
	if ( State_Get() != GR_STATE_TEAM_WIN )
		return true;

	return false;
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: Determines whether we should allow mp_timelimit to trigger a map change
//-----------------------------------------------------------------------------
bool CTFGameRules::CanChangelevelBecauseOfTimeLimit( void )
{
	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;

	// we only want to deny a map change triggered by mp_timelimit if we're not forcing a map reset,
	// we're playing mini-rounds, and the master says we need to play all of them before changing (for maps like Dustbowl)
	if ( !m_bForceMapReset && pMaster && pMaster->PlayingMiniRounds() && pMaster->ShouldPlayAllControlPointRounds() )
	{
		if ( pMaster->NumPlayableControlPointRounds() )
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::CanGoToStalemate( void )
{
	// In CTF, don't go to stalemate if one of the flags isn't at home
	if ( m_nGameType == TF_GAMETYPE_CTF )
	{
		CCaptureFlag *pFlag = dynamic_cast<CCaptureFlag*> ( gEntList.FindEntityByClassname( NULL, "item_teamflag" ) );
		while( pFlag )
		{
			if ( pFlag->IsDropped() || pFlag->IsStolen() )
				return false;

			pFlag = dynamic_cast<CCaptureFlag*> ( gEntList.FindEntityByClassname( pFlag, "item_teamflag" ) );
		}

		// check that one team hasn't won by capping
		if ( CheckCapsPerRound() )
			return false;
	}

	return BaseClass::CanGoToStalemate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::PlayWinSong( int team )
{
	if ( team == TEAM_UNASSIGNED )
	{
		PlayStalemateSong();
	}
	else
	{
		// rocket launch
		if ( TFGameRules() && TFGameRules()->IsPlayingSpecialDeliveryMode() )
			return;

		BroadcastSound( TEAM_UNASSIGNED, UTIL_VarArgs("Game.TeamWin%d", team ) );

		for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
		{
			if ( i == team )
			{
				if ( TFGameRules() && TFGameRules()->IsAnyCoOpGameRunning() )
					return;

				BroadcastSound( i, WinSongName( i ) );
			}
			else
			{
				const char *pchLoseSong = LoseSongName( i );
				if ( pchLoseSong )
				{
					BroadcastSound( i, pchLoseSong );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CTFGameRules::GetStalemateSong( int nTeam )
{
	const char *pszSong = "Game.Stalemate";
	switch ( nTeam )
	{
	case TF_TEAM_BLUE:
	case TF_TEAM_RED:
		pszSong = "Game.Stalemate";
		break;

	case TEAM_UNASSIGNED:
		pszSong = "Game.Stalemate";
		break;
	}

	return pszSong;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CTFGameRules::WinSongName( int nTeam )
{
	const char *pszSong = "Game.YourTeamWon";
	switch ( nTeam )
	{
	case TF_TEAM_BLUE:
	case TF_TEAM_RED:
		pszSong = "Game.YourTeamWon";
		break;

	case TEAM_UNASSIGNED:
		pszSong = "Game.YourTeamWon";
		break;
	}

	return pszSong;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CTFGameRules::LoseSongName( int nTeam )
{
	const char *pszSong = "Game.YourTeamLost";
	switch ( nTeam )
	{
	case TF_TEAM_BLUE:
	case TF_TEAM_RED:
		pszSong = "Game.YourTeamLost";
		break;

	case TEAM_UNASSIGNED:
		pszSong = "Game.YourTeamLost";
		break;
	}

	return pszSong;
}

// Classnames of entities that are preserved across round restarts
static const char *s_PreserveEnts[] =
{
	"tf_gamerules",
	"tf_team_manager",
	"tf_player_manager",
	"tf_team",
	"tf_objective_resource",
	"tf_viewmodel",
	"tf_logic_training",
	"tf_logic_training_mode",
	"tf_powerup_bottle",
	"tf_mann_vs_machine_stats",
	"tf_wearable",
	"tf_wearable_demoshield",
	"tf_wearable_robot_arm",
	"tf_wearable_vm",
	"tf_logic_bonusround",
	"vote_controller",
	"monster_resource",
	"tf_logic_medieval",
	"tf_logic_cp_timer",
	"tf_logic_tower_defense",
	"tf_logic_mann_vs_machine",
	"func_upgradestation"
	"entity_rocket",
	"entity_carrier",
	"entity_sign",
	"entity_saucer",
	"lfe_logic_coop",
	"lfe_logic_blucoop",	
	"lfe_logic_versus",
	"lfe_logic_infection",
	"lfe_logic_longjump",
	"lfe_logic_date",
	"", // END Marker
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::Activate()
{
	m_nMapHolidayType = kHoliday_None;

	m_nGameType.Set( TF_GAMETYPE_UNDEFINED );
	m_nServerGameType.Set( TF_GAMETYPE_UNDEFINED );

	tf_gamemode_arena	.SetValue(0);
	tf_gamemode_cp		.SetValue(0);
	tf_gamemode_ctf		.SetValue(0);
	tf_gamemode_sd		.SetValue(0);
	tf_gamemode_payload	.SetValue(0);
	tf_gamemode_mvm		.SetValue(0);
	tf_gamemode_rd		.SetValue(0);
	tf_gamemode_pd		.SetValue(0);
	tf_gamemode_tc		.SetValue(0);
	tf_beta_content		.SetValue(0);
	tf_gamemode_passtime.SetValue(0);
	tf_gamemode_misc	.SetValue(0);
	lfe_gamemode_coop	.SetValue(0);
	lfe_gamemode_versus	.SetValue(0);
	lfe_gamemode_blucoop.SetValue(0);
	lfe_gamemode_horde	.SetValue(0);
	lfe_gamemode_free	.SetValue(0);

	SetMultipleTrains( false );

	if ( IsInHL2EP1Map() || IsInHL2EP2Map() )
		hl2_episodic.SetValue( 1 );

	OnSkillLevelChanged( sv_difficulty.GetInt() );
	
	CMedievalLogic *pMedieval = dynamic_cast< CMedievalLogic* >( gEntList.FindEntityByClassname( NULL, "tf_logic_medieval" ) );
	if ( pMedieval )
	{
		m_bPlayingMedieval = true;
		tf_medieval.SetValue( 1 );
	}

	CTFHolidayEntity *pHoliday = dynamic_cast< CTFHolidayEntity* >( gEntList.FindEntityByClassname( NULL, "tf_logic_holiday" ) );
	if ( pHoliday )
	{
		m_nMapHolidayType = pHoliday->m_nHolidayType+1;
	}
	else
	{
		if ( tf_birthday.GetBool() || lfe_forced_holiday.GetInt() == 1 )
		{
			m_nMapHolidayType = kHoliday_TF2Birthday;
		}
		else if ( lfe_forced_holiday.GetInt() == 2 || lfe_forced_holiday.GetInt() == 9 || lfe_forced_holiday.GetInt() == 10 )
		{
			m_nMapHolidayType = kHoliday_Halloween;
		}
		else if ( lfe_forced_holiday.GetInt() == 3 )
		{
			m_nMapHolidayType = kHoliday_Christmas;
		}
		else if ( lfe_forced_holiday.GetInt() == 4 )
		{
			m_nMapHolidayType = kHoliday_CommunityUpdate;
		}
		else if ( lfe_forced_holiday.GetInt() == 5 )
		{
			m_nMapHolidayType = kHoliday_EOTL;
		}
		else if ( lfe_forced_holiday.GetInt() == 6 )
		{
			m_nMapHolidayType = kHoliday_ValentinesDay;
		}
		else if ( lfe_forced_holiday.GetInt() == 7 )
		{
			m_nMapHolidayType = kHoliday_MeetThePyro;
		}
		else if ( lfe_forced_holiday.GetInt() == 8 )
		{
			m_nMapHolidayType = kHoliday_FullMoon;
		}
		else if ( lfe_forced_holiday.GetInt() == 11 )
		{
			m_nMapHolidayType = kHoliday_AprilFools;
		}
		else if ( lfe_forced_holiday.GetInt() == 12 )
		{
			m_nMapHolidayType = kHoliday_LFBirthday;
		}
		else if ( lfe_forced_holiday.GetInt() == 13 )
		{
			m_nMapHolidayType = kHoliday_NewYears;
		}
	}

	if ( gEntList.FindEntityByName( NULL, "competitive_stage_logic_case" ) ) 
		m_bMapHasMatchSummaryStage = true;

	if ( ( mp_gamemode.GetInt() == 1 ) || gEntList.FindEntityByClassname( NULL, "lfe_logic_coop" ) && !gEntList.FindEntityByClassname( NULL, "lfe_logic_blucoop" ) )
	{
		m_nServerGameType.Set( TF_GAMETYPE_COOP );
		lfe_gamemode_coop.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 1 ) 
			mp_gamemode.SetValue( 1 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );

		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server coop config file\n" );
		engine->ServerCommand( "exec config_coop.cfg \n" );
		engine->ServerExecute();
	}
	else if ( ( mp_gamemode.GetInt() == 2 ) || gEntList.FindEntityByClassname( NULL, "lfe_logic_versus" ) || !Q_strncmp( STRING( gpGlobals->mapname ), "vs_", 3 ) )
	{
		m_nServerGameType.Set( TF_GAMETYPE_VS );
		lfe_gamemode_versus.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 2 ) 
			mp_gamemode.SetValue( 2 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server versus config file\n" );
		engine->ServerCommand( "exec config_vs.cfg \n" );
		engine->ServerExecute();
	}
	else if ( ( mp_gamemode.GetInt() == 3 ) || gEntList.FindEntityByClassname( NULL, "lfe_logic_blucoop" ) )
	{
		m_nServerGameType.Set( TF_GAMETYPE_BLUCOOP );
		lfe_gamemode_blucoop.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 3 ) 
			mp_gamemode.SetValue( 3 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server blue coop file\n" );
		engine->ServerCommand( "exec config_blucoop.cfg \n ");
		engine->ServerExecute();
	}
	else if ( ( mp_gamemode.GetInt() == 4 ) || gEntList.FindEntityByClassname( NULL, "lfe_logic_horde" ) || !Q_strncmp( STRING( gpGlobals->mapname ), "hor_", 4 ) )
	{
		m_nServerGameType.Set( TF_GAMETYPE_HORDE );
		lfe_gamemode_horde.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 4 ) 
			mp_gamemode.SetValue( 4 );

		alyx_darkness_force.SetValue( 1 );
		hl2_episodic.SetValue( 1 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server horde config file\n" );
		engine->ServerCommand( "exec config_horde.cfg \n" );
		engine->ServerExecute();
	}
	else if ( mp_gamemode.GetInt() == 5 )
	{
		m_nServerGameType.Set( TF_GAMETYPE_FREE );
		lfe_gamemode_free.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 5 ) 
			mp_gamemode.SetValue( 5 );

		hl2_episodic.SetValue( 1 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server Free config file\n" );
		engine->ServerCommand( "exec config_free.cfg \n" );
		engine->ServerExecute();
	}
	else if ( ( mp_gamemode.GetInt() == 6 ) || gEntList.FindEntityByClassname( NULL, "lfe_logic_infection" ) || !Q_strncmp( STRING( gpGlobals->mapname ), "inf_", 4 ) )
	{
		m_nServerGameType.Set( TF_GAMETYPE_INFECTION );
		lfe_gamemode_free.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 6 ) 
			mp_gamemode.SetValue( 6 );

		hl2_episodic.SetValue( 1 );
	}

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "tc_", 3 ) == 0 || !Q_strncmp( STRING( gpGlobals->mapname ), "sd_", 3 ) == 0 || !Q_strncmp( STRING( gpGlobals->mapname ), "pd_", 3 ) == 0 || tf_medieval.GetBool() ) 
		tf_gamemode_misc.SetValue( 1 );

	if ( gEntList.FindEntityByClassname( NULL, "tf_logic_training_mode" ) )
		m_bIsInTraining = true;

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "itemtest", 8 ) == 0 )
		m_bIsInItemTestingMode = true;

	if ( gEntList.FindEntityByClassname( NULL, "tf_logic_mannpower" ) || tf_powerup_mode.GetBool() )
	{
		m_bPowerupMode = true;
		tf_powerup_mode.SetValue( 1 );
	}

	if ( gEntList.FindEntityByClassname( NULL, "lfe_logic_coop" ) && !gEntList.FindEntityByClassname( NULL, "lfe_logic_blucoop" ) )
	{
		m_nGameType.Set( TF_GAMETYPE_COOP );
		lfe_gamemode_coop.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 1 ) 
			mp_gamemode.SetValue( 1 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );

		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server coop config file\n" );
		engine->ServerCommand( "exec config_coop.cfg \n" );
		engine->ServerExecute();
		return;
	}

	if ( gEntList.FindEntityByClassname( NULL, "lfe_logic_versus" ) || !Q_strncmp( STRING( gpGlobals->mapname ), "vs_", 3 ) )
	{
		m_nGameType.Set( TF_GAMETYPE_VS );
		lfe_gamemode_versus.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 2 ) 
			mp_gamemode.SetValue( 2 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server versus config file\n" );
		engine->ServerCommand( "exec config_vs.cfg \n" );
		engine->ServerExecute();
		return;
	}

	if ( gEntList.FindEntityByClassname( NULL, "lfe_logic_blucoop" ) )
	{
		m_nGameType.Set( TF_GAMETYPE_BLUCOOP );
		lfe_gamemode_blucoop.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 3 ) 
			mp_gamemode.SetValue( 3 );

		if ( IsInHL2Map() )
			hl2_episodic.SetValue( 0 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server blue coop file\n" );
		engine->ServerCommand( "exec config_blucoop.cfg \n ");
		engine->ServerExecute();
		return;
	}

	if ( gEntList.FindEntityByClassname( NULL, "lfe_logic_horde" ) || !Q_strncmp( STRING( gpGlobals->mapname ), "hor_", 4 ) )
	{
		m_nGameType.Set( TF_GAMETYPE_HORDE );
		lfe_gamemode_horde.SetValue( 1 );

		if ( mp_gamemode.GetInt() != 4 ) 
			mp_gamemode.SetValue( 4 );

		alyx_darkness_force.SetValue( 1 );
		hl2_episodic.SetValue( 1 );
		ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Executing server horde config file\n" );
		engine->ServerCommand( "exec config_horde.cfg \n" );
		engine->ServerExecute();
		return;
	}

	CArenaLogic *pArena = dynamic_cast<CArenaLogic*>( gEntList.FindEntityByClassname( NULL, "tf_logic_arena" ) );
	if ( pArena )
	{
		m_nGameType.Set( TF_GAMETYPE_ARENA );
		tf_gamemode_arena.SetValue( 1 );
		Msg( "[TFGameRules] Executing server arena config file\n", 1 );
		engine->ServerCommand( "exec config_arena.cfg \n" );
		engine->ServerExecute();
		return;
	}

	CKothLogic *pKoth = dynamic_cast<CKothLogic*> ( gEntList.FindEntityByClassname( NULL, "tf_logic_koth" ) );
	if ( pKoth )
	{
		m_nGameType.Set( TF_GAMETYPE_CP );
		m_bPlayingKoth = true;
		return;
	}

	CHybridMap_CTF_CP *pHybridEnt = dynamic_cast<CHybridMap_CTF_CP*> ( gEntList.FindEntityByClassname( NULL, "tf_logic_hybrid_ctf_cp" ) );
	if ( pHybridEnt )
	{
		m_nGameType.Set( TF_GAMETYPE_CP );
		m_bPlayingHybrid_CTF_CP = true;
		return;
	}

	CCaptureFlag *pFlag = dynamic_cast<CCaptureFlag*> ( gEntList.FindEntityByClassname( NULL, "item_teamflag" ) );
	if ( pFlag )
	{
		m_nGameType.Set( TF_GAMETYPE_CTF );
		tf_gamemode_ctf.SetValue( 1 );
		return;
	}

	CTeamTrainWatcher *pTrain = dynamic_cast<CTeamTrainWatcher*> ( gEntList.FindEntityByClassname( NULL, "team_train_watcher" ) );
	if ( pTrain )
	{
		m_nGameType.Set( TF_GAMETYPE_ESCORT );

		if ( gEntList.FindEntityByClassname( NULL, "tf_logic_multiple_escort" ) )
			SetMultipleTrains( true );

		tf_gamemode_payload.SetValue( 1 );
		return;
	}

	if ( g_hControlPointMasters.Count() )
	{
		m_nGameType.Set( TF_GAMETYPE_CP );
		tf_gamemode_cp.SetValue( 1 );
		return;
	}

	if ( gEntList.FindEntityByClassname( NULL, "passtime_logic" ) )
	{
		m_nGameType.Set( TF_GAMETYPE_PASSTIME );
		tf_gamemode_passtime.SetValue( 1 );
		return;
	}

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "tc_", 3 ) == 0 )
	{
		tf_gamemode_tc.SetValue( 1 );
		return;
	}

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "sd_", 3 ) == 0 )
	{
		tf_gamemode_sd.SetValue( 1 );
		m_bPlayingSpecialDeliveryMode = true;
		return;
	}

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "rd_", 3 ) == 0 )
	{
		tf_gamemode_rd.SetValue( 1 );
		m_nGameType.Set( TF_GAMETYPE_RD );
		tf_beta_content.SetValue( 1 );
		return;
	}

	if ( !Q_strncmp( STRING( gpGlobals->mapname ), "pd_", 3 ) == 0 )
	{
		tf_gamemode_pd.SetValue( 1 );
		m_nGameType.Set( TF_GAMETYPE_PD );
		return;
	}
}

int CTFGameRules::GetClassLimit( int iDesiredClassIndex, int iTeam )
{
	int result = -1;

	if ( IsInTournamentMode() )
	{
		if ( iDesiredClassIndex <= TF_LAST_NORMAL_CLASS )
		{
			switch ( iDesiredClassIndex )
			{
				default:
					result = -1;
				case TF_CLASS_ENGINEER:
					result = tf_tournament_classlimit_engineer.GetInt();
					break;
				case TF_CLASS_SPY:
					result = tf_tournament_classlimit_spy.GetInt();
					break;
				case TF_CLASS_PYRO:
					result = tf_tournament_classlimit_pyro.GetInt();
					break;
				case TF_CLASS_HEAVYWEAPONS:
					result = tf_tournament_classlimit_heavy.GetInt();
					break;
				case TF_CLASS_MEDIC:
					result = tf_tournament_classlimit_medic.GetInt();
					break;
				case TF_CLASS_DEMOMAN:
					result = tf_tournament_classlimit_demoman.GetInt();
					break;
				case TF_CLASS_SOLDIER:
					result = tf_tournament_classlimit_soldier.GetInt();
					break;
				case TF_CLASS_SNIPER:
					result = tf_tournament_classlimit_sniper.GetInt();
					break;
				case TF_CLASS_SCOUT:
					result = tf_tournament_classlimit_scout.GetInt();
					break;
			}
		}
		else
		{
			result = -1;
		}
	}
	else if ( IsInHighlanderMode() )
	{
		result = 1;
	}
	else if ( tf_classlimit.GetBool() )
	{
		result = tf_classlimit.GetInt();
	}
	else if ( !IsInTournamentMode() )
	{
		if ( iDesiredClassIndex <= TF_LAST_NORMAL_CLASS )
		{
			switch ( iDesiredClassIndex )
			{
				default:
					result = -1;
				case TF_CLASS_ENGINEER:
					result = lfe_classlimit_engineer.GetInt();
					break;
				case TF_CLASS_SPY:
					result = lfe_classlimit_spy.GetInt();
					break;
				case TF_CLASS_PYRO:
					result = lfe_classlimit_pyro.GetInt();
					break;
				case TF_CLASS_HEAVYWEAPONS:
					result = lfe_classlimit_heavy.GetInt();
					break;
				case TF_CLASS_MEDIC:
					result = lfe_classlimit_medic.GetInt();
					break;
				case TF_CLASS_DEMOMAN:
					result = lfe_classlimit_demoman.GetInt();
					break;
				case TF_CLASS_SOLDIER:
					result = lfe_classlimit_soldier.GetInt();
					break;
				case TF_CLASS_SNIPER:
					result = lfe_classlimit_sniper.GetInt();
					break;
				case TF_CLASS_SCOUT:
					result = lfe_classlimit_scout.GetInt();
					break;
			}
		}
		else
		{
			result = -1;
		}
	}
	else
	{
		result = -1;
	}

	return result;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::CanPlayerChooseClass( CBasePlayer *pPlayer, int iDesiredClassIndex )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	CTFTeam *pTFTeam = pTFPlayer->GetTFTeam();
	int iClassLimit = 0;
	int iClassCount = 0;

	iClassLimit = GetClassLimit( iDesiredClassIndex, pTFTeam->GetTeamNumber() );
	
	if ( iClassLimit != -1 && pTFTeam && pTFPlayer->GetTeamNumber() >= TF_TEAM_RED )
	{
		for ( int i = 0; i < pTFTeam->GetNumPlayers(); i++ )
		{
			if ( pTFTeam->GetPlayer( i ) && pTFTeam->GetPlayer( i ) != pPlayer )
				iClassCount += iDesiredClassIndex == ToTFPlayer( pTFTeam->GetPlayer( i ) )->GetPlayerClass()->GetClassIndex();
		}

		return iClassLimit > iClassCount;
	}
	else
	{
		return true;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	bool bRetVal = true;

	if ( ( State_Get() == GR_STATE_TEAM_WIN ) && pVictim )
	{
		if ( pVictim->GetTeamNumber() == GetWinningTeam() )
		{
			CBaseTrigger *pTrigger = dynamic_cast< CBaseTrigger *>( info.GetInflictor() );

			// we don't want players on the winning team to be
			// hurt by team-specific trigger_hurt entities during the bonus time
			if ( pTrigger && pTrigger->UsesFilter() )
			{
				bRetVal = false;
			}
		}
	}

	return bRetVal;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetTeamGoalString( int iTeam, const char *pszGoal )
{
	if ( iTeam == TF_TEAM_RED )
	{
		if ( !pszGoal || !pszGoal[0] )
		{
			m_pszTeamGoalStringRed.GetForModify()[0] = '\0';
		}
		else
		{
			if ( Q_stricmp( m_pszTeamGoalStringRed.Get(), pszGoal ) )
			{
				Q_strncpy( m_pszTeamGoalStringRed.GetForModify(), pszGoal, MAX_TEAMGOAL_STRING );
			}
		}
	}
	else if ( iTeam == TF_TEAM_BLUE )
	{
		if ( !pszGoal || !pszGoal[0] )
		{
			m_pszTeamGoalStringBlue.GetForModify()[0] = '\0';
		}
		else
		{
			if ( Q_stricmp( m_pszTeamGoalStringBlue.Get(), pszGoal ) )
			{
				Q_strncpy( m_pszTeamGoalStringBlue.GetForModify(), pszGoal, MAX_TEAMGOAL_STRING );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::RoundCleanupShouldIgnore( CBaseEntity *pEnt )
{
	if ( FindInList( s_PreserveEnts, pEnt->GetClassname() ) )
		return true;

	//There has got to be a better way of doing this.
	if ( Q_strstr( pEnt->GetClassname(), "tf_weapon_" ) )
		return true;

	return BaseClass::RoundCleanupShouldIgnore( pEnt );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldCreateEntity( const char *pszClassName )
{
	if ( FindInList( s_PreserveEnts, pszClassName ) )
		return false;

	return BaseClass::ShouldCreateEntity( pszClassName );
}

void CTFGameRules::CleanUpMap( void )
{
	// tf_weapon can't be delete except
	CBaseEntity *pPhyscannon = gEntList.FindEntityByClassname( NULL, "tf_weapon_physcannon" );
	if ( pPhyscannon )
		UTIL_Remove( pPhyscannon );

	CBaseEntity *pPortalgun = gEntList.FindEntityByClassname( NULL, "tf_weapon_portalgun" );
	if ( pPortalgun )
		UTIL_Remove( pPortalgun );

	BaseClass::CleanUpMap();

	if ( HLTVDirector() )
	{
		HLTVDirector()->BuildCameraList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::RecalculateControlPointState( void )
{
	Assert( ObjectiveResource() );

	if ( !g_hControlPointMasters.Count() )
		return;

	if ( g_pObjectiveResource && g_pObjectiveResource->PlayingMiniRounds() )
		return;

	for ( int iTeam = LAST_SHARED_TEAM+1; iTeam < GetNumberOfTeams(); iTeam++ )
	{
		int iFarthestPoint = GetFarthestOwnedControlPoint( iTeam, true );
		if ( iFarthestPoint == -1 )
			continue;

		// Now enable all spawn points for that spawn point
		CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, "info_player_teamspawn" );
		while( pSpot )
		{
			CTFTeamSpawn *pTFSpawn = assert_cast<CTFTeamSpawn*>(pSpot);
			if ( pTFSpawn->GetControlPoint() )
			{
				if ( pTFSpawn->GetTeamNumber() == iTeam )
				{
					if ( pTFSpawn->GetControlPoint()->GetPointIndex() == iFarthestPoint )
					{
						pTFSpawn->SetDisabled( false );
					}
					else
					{
						pTFSpawn->SetDisabled( true );
					}
				}
			}

			pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_teamspawn" );
		}
	}
}

inline void SpawnRunes( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: Called when a new round is being initialized
//-----------------------------------------------------------------------------
void CTFGameRules::SetupOnRoundStart( void )
{
	// Need to re-init AI dynamic links since they get respawned on new round.
	CAI_DynamicLink::gm_bInitialized = false;
	CAI_DynamicLink::InitDynamicLinks();

	// Reset antlion maker manager.
	g_AntlionMakerManager.LevelInitPostEntity();

	for ( int i = 0; i < MAX_TEAMS; i++ )
	{
		ObjectiveResource()->SetBaseCP( -1, i );
	}

	for ( int i = 0; i < TF_TEAM_COUNT; i++ )
	{
		m_iNumCaps[i] = 0;
	}

	m_bossSpawnTimer.Invalidate();
	SetIT( NULL );

	for ( int iTeam = FIRST_GAME_TEAM; iTeam < TFTeamMgr()->GetTeamCount(); ++iTeam )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( pTeam )
		{
			pTeam->RemoveAllWeapons();
		}
	}

	// Let all entities know that a new round is starting
	CBaseEntity *pEnt = gEntList.FirstEnt();
	while( pEnt )
	{
		variant_t emptyVariant;
		pEnt->AcceptInput( "RoundSpawn", NULL, NULL, emptyVariant, 0 );

		pEnt = gEntList.NextEnt( pEnt );
	}

	// All entities have been spawned, now activate them
	pEnt = gEntList.FirstEnt();
	while( pEnt )
	{
		variant_t emptyVariant;
		pEnt->AcceptInput( "RoundActivate", NULL, NULL, emptyVariant, 0 );

		pEnt = gEntList.NextEnt( pEnt );
	}

	if ( g_pObjectiveResource && !g_pObjectiveResource->PlayingMiniRounds() )
	{
		// Find all the control points with associated spawnpoints
		memset( m_bControlSpawnsPerTeam, 0, sizeof(bool) * MAX_TEAMS * MAX_CONTROL_POINTS );
		CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, "info_player_teamspawn" );
		while( pSpot )
		{
			CTFTeamSpawn *pTFSpawn = assert_cast<CTFTeamSpawn*>(pSpot);
			if ( pTFSpawn->GetControlPoint() )
			{
				m_bControlSpawnsPerTeam[ pTFSpawn->GetTeamNumber() ][ pTFSpawn->GetControlPoint()->GetPointIndex() ] = true;
				pTFSpawn->SetDisabled( true );
			}

			pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_teamspawn" );
		}

		RecalculateControlPointState();

		SetRoundOverlayDetails();
	}

#ifdef GAME_DLL
	m_szMostRecentCappers[0] = 0;

	TFMapAddSystem()->SetMapAddMode( TFGameRules()->IsMapAddAllowed() );

	if ( LFPopulationManager() )
		LFPopulationManager()->SetupOnRoundStart();
#endif

	if ( IsHolidayActive( kHoliday_TF2Birthday ) || IsHolidayActive( kHoliday_LFBirthday ) )
	{
		GetTFInventory()->AddItem( "Noise Maker - TF Birthday" );
	}
	else
	{
		GetTFInventory()->RemoveItem( "Noise Maker - TF Birthday" );
	}

	if ( IsHolidayActive( kHoliday_Christmas ) )
	{
		GetTFInventory()->AddItem( "Noise Maker - Winter 2011" );
	}
	else
	{
		GetTFInventory()->RemoveItem( "Noise Maker - Winter 2011" );
	}

	if ( IsUsingSpells() )
	{
		GetTFInventory()->AddItem( "TF_WEAPON_SPELLBOOK" );
	}
	else
	{
		GetTFInventory()->RemoveItem( "TF_WEAPON_SPELLBOOK" );
	}

	if ( IsUsingGrapplingHook() )
	{
		GetTFInventory()->AddItem( "TF_WEAPON_GRAPPLINGHOOK" );
	}
	else
	{
		GetTFInventory()->RemoveItem( "TF_WEAPON_GRAPPLINGHOOK" );
	}

	GetTFInventory()->SetupOnRoundStart();
}

//-----------------------------------------------------------------------------
// Purpose: Called when a new round is off and running
//-----------------------------------------------------------------------------
void CTFGameRules::SetupOnRoundRunning( void )
{
	// Let out control point masters know that the round has started
	for ( int i = 0; i < g_hControlPointMasters.Count(); i++ )
	{
		variant_t emptyVariant;
		if ( g_hControlPointMasters[i] )
		{
			g_hControlPointMasters[i]->AcceptInput( "RoundStart", NULL, NULL, emptyVariant, 0 );
		}
	}

	// Reset player speeds after preround lock
	CTFPlayer *pPlayer;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		pPlayer->TeamFortress_SetSpeed();
		pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_ROUND_START );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called before a new round is started (so the previous round can end)
//-----------------------------------------------------------------------------
void CTFGameRules::PreviousRoundEnd( void )
{
	// before we enter a new round, fire the "end output" for the previous round
	if ( g_hControlPointMasters.Count() && g_hControlPointMasters[0] )
	{
		g_hControlPointMasters[0]->FireRoundEndOutput();
	}

	m_iPreviousRoundWinners = GetWinningTeam();
}

//-----------------------------------------------------------------------------
// Purpose: Called when a round has entered stalemate mode (timer has run out)
//-----------------------------------------------------------------------------
void CTFGameRules::SetupOnStalemateStart( void )
{
	// Respawn all the players
	RespawnPlayers( true );

	if ( TFGameRules()->IsInArenaMode() )
	{
		CArenaLogic *pArena = dynamic_cast<CArenaLogic *>( gEntList.FindEntityByClassname( NULL, "tf_logic_arena" ) );
		if ( pArena )
		{
			pArena->m_OnArenaRoundStart.FireOutput( pArena, pArena );

			IGameEvent *event = gameeventmanager->CreateEvent( "arena_round_start" );
			if ( event )
			{
				gameeventmanager->FireEvent( event );
			}

			for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
			{
				BroadcastSound( i, "Announcer.AM_RoundStartRandom" );
			}

		}
		return;
	}

	// Remove everyone's objects
	for ( int i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			pPlayer->RemoveAllOwnedEntitiesFromWorld();
		}
	}

	// Disable all the active health packs in the world
	m_hDisabledHealthKits.Purge();
	CHealthKit *pHealthPack = gEntList.NextEntByClass( (CHealthKit *)NULL );
	while ( pHealthPack )
	{
		if ( !pHealthPack->IsDisabled() )
		{
			pHealthPack->SetDisabled( true );
			m_hDisabledHealthKits.AddToTail( pHealthPack );
		}
		pHealthPack = gEntList.NextEntByClass( pHealthPack );
	}

	CTFPlayer *pPlayer;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		pPlayer->SpeakConceptIfAllowed( MP_CONCEPT_SUDDENDEATH_START );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetupOnStalemateEnd( void )
{
	// Reenable all the health packs we disabled
	for ( int i = 0; i < m_hDisabledHealthKits.Count(); i++ )
	{
		if ( m_hDisabledHealthKits[i] )
		{
			m_hDisabledHealthKits[i]->SetDisabled( false );
		}
	}

	m_hDisabledHealthKits.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::InitTeams( void )
{
	BaseClass::InitTeams();

	// clear the player class data
	ResetFilePlayerClassInfoDatabase();
}

// Skips players except for the specified one.
class CTraceFilterHitPlayer : public CTraceFilterSimple
{
public:
	DECLARE_CLASS( CTraceFilterIgnorePlayers, CTraceFilterSimple );

	CTraceFilterHitPlayer( const IHandleEntity *passentity, IHandleEntity *pHitEntity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
		m_pHitEntity = pHitEntity;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );

		if ( !pEntity )
			return false;

		if ( pEntity->IsPlayer() && pEntity != m_pHitEntity )
			return false;

		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}

private:
	const IHandleEntity *m_pHitEntity;
};

CTFRadiusDamageInfo::CTFRadiusDamageInfo()
{
	m_flRadius = 0.0f;
	m_iClassIgnore = CLASS_NONE;
	m_pEntityIgnore = NULL;
	m_flSelfDamageRadius = 0.0f;
	m_bStockSelfDamage = true;
}

ConVar tf_fixedup_damage_radius( "tf_fixedup_damage_radius", "1", FCVAR_DEVELOPMENTONLY );

bool CTFRadiusDamageInfo::ApplyToEntity( CBaseEntity *pEntity )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT&( ~CONTENTS_HITBOX );
	trace_t		tr;
	float		falloff;
	Vector		vecSpot;

	if ( info->GetDamageType() & DMG_RADIUS_MAX )
		falloff = 0.0;
	else if ( info->GetDamageType() & DMG_HALF_FALLOFF )
		falloff = 0.5;
	else if ( m_flRadius )
		falloff = info->GetDamage() / m_flRadius;
	else
		falloff = 1.0;

	float flFalloffMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( info->GetWeapon(), flFalloffMult, mult_dmg_falloff );
	if ( flFalloffMult != 1.0f )
		falloff += flFalloffMult;

	CBaseEntity *pInflictor = info->GetInflictor();

	//	float flHalfRadiusSqr = Square( flRadius / 2.0f );

	// This value is used to scale damage when the explosion is blocked by some other object.
	float flBlockedDamagePercent = 0.0f;

	// Check that the explosion can 'see' this entity, trace through players.
	vecSpot = pEntity->BodyTarget( m_vecSrc, false );
	CTraceFilterHitPlayer filter( info->GetInflictor(), pEntity, COLLISION_GROUP_PROJECTILE );
	Ray_t ray; ray.Init( m_vecSrc, vecSpot );
	UTIL_Portal_TraceRay( ray, MASK_RADIUS_DAMAGE, &filter, &tr );

	if ( tr.startsolid && tr.m_pEnt )
	{
		if ( tr.m_pEnt->IsCombatItem() )
		{
			if ( pEntity->InSameTeam( tr.m_pEnt ) && pEntity != tr.m_pEnt )
				return false;
		}

		filter.SetPassEntity( tr.m_pEnt );
		UTIL_TraceLine( m_vecSrc, vecSpot, MASK_RADIUS_DAMAGE, &filter, &tr );
	}

	if ( tr.fraction != 1.0 && tr.m_pEnt != pEntity )
		return false;

	// Adjust the damage - apply falloff.
	float flAdjustedDamage = 0.0f;

	float flDistanceToEntity;

	// Rockets store the ent they hit as the enemy and have already
	// dealt full damage to them by this time
	if ( pInflictor && ( pEntity == pInflictor->GetEnemy() ) )
	{
		// Full damage, we hit this entity directly
		flDistanceToEntity = 0;
	}
	else if ( pEntity->IsPlayer() )
	{
		// Use whichever is closer, absorigin or worldspacecenter
		float flToWorldSpaceCenter = ( m_vecSrc - pEntity->WorldSpaceCenter() ).Length();
		float flToOrigin = ( m_vecSrc - pEntity->GetAbsOrigin() ).Length();

		flDistanceToEntity = min( flToWorldSpaceCenter, flToOrigin );
	}
	else if ( pEntity->IsNPC() )
	{
		// Use whichever is closer, absorigin or worldspacecenter
		float flToWorldSpaceCenter = ( m_vecSrc - pEntity->WorldSpaceCenter() ).Length();
		float flToOrigin = ( m_vecSrc - pEntity->GetAbsOrigin() ).Length();

		flDistanceToEntity = Min( flToWorldSpaceCenter, flToOrigin );
	}
	else
	{
		flDistanceToEntity = ( m_vecSrc - tr.endpos ).Length();
	}

	if ( tf_fixedup_damage_radius.GetBool() )
	{
		flAdjustedDamage = RemapValClamped( flDistanceToEntity, 0, m_flRadius, info->GetDamage(), info->GetDamage() * falloff );
	}
	else
	{
		flAdjustedDamage = flDistanceToEntity * falloff;
		flAdjustedDamage = info->GetDamage() - flAdjustedDamage;
	}

	// Take a little less damage from yourself
	if ( tr.m_pEnt == info->GetAttacker() )
	{
		flAdjustedDamage = flAdjustedDamage * 0.75;
	}

	if ( flAdjustedDamage <= 0 )
		return false;

	// the explosion can 'see' this entity, so hurt them!
	if ( tr.startsolid )
	{
		// if we're stuck inside them, fixup the position and distance
		tr.endpos = m_vecSrc;
		tr.fraction = 0.0;
	}

	CTakeDamageInfo adjustedInfo = *info;
	//Msg("%s: Blocked damage: %f percent (in:%f  out:%f)\n", pEntity->GetClassname(), flBlockedDamagePercent * 100, flAdjustedDamage, flAdjustedDamage - (flAdjustedDamage * flBlockedDamagePercent) );
	adjustedInfo.SetDamage( flAdjustedDamage - ( flAdjustedDamage * flBlockedDamagePercent ) );

	// Now make a consideration for skill level!
	if ( info->GetAttacker() && info->GetAttacker()->IsPlayer() && pEntity->IsNPC() )
	{
		// An explosion set off by the player is harming an NPC. Adjust damage accordingly.
		adjustedInfo.AdjustPlayerDamageInflictedForSkillLevel();
	}

	Vector dir = vecSpot - m_vecSrc;
	VectorNormalize( dir );

	// If we don't have a damage force, manufacture one
	if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
	{
		CalculateExplosiveDamageForce( &adjustedInfo, dir, m_vecSrc );
	}
	else
	{
		// Assume the force passed in is the maximum force. Decay it based on falloff.
		float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
		adjustedInfo.SetDamageForce( dir * flForce );
		adjustedInfo.SetDamagePosition( m_vecSrc );
	}

	if ( tr.fraction != 1.0 && pEntity == tr.m_pEnt )
	{
		ClearMultiDamage();
		pEntity->DispatchTraceAttack( adjustedInfo, dir, &tr );
		ApplyMultiDamage();
	}
	else
	{
		pEntity->TakeDamage( adjustedInfo );
	}

	// Now hit all triggers along the way that respond to damage... 
	pEntity->TraceAttackToTriggers( adjustedInfo, m_vecSrc, tr.endpos, dir );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Logic for jar-based throwable object collisions
//-----------------------------------------------------------------------------
bool CTFGameRules::RadiusJarEffect( CTFRadiusDamageInfo &radiusInfo, int iCond )
{
	bool bExtinguished = false;
	CBaseEntity *pAttacker = radiusInfo.info->GetAttacker();

	CBaseEntity *pEntity = NULL;
	for ( CEntitySphereQuery sphere( radiusInfo.m_vecSrc, radiusInfo.m_flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity == radiusInfo.m_pEntityIgnore )
			continue;

		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// UNDONE: this should check a damage mask, not an ignore
		if ( radiusInfo.m_iClassIgnore != CLASS_NONE && pEntity->Classify() == radiusInfo.m_iClassIgnore )
		{
			continue;
		}

		// Checking distance from source because Valve were apparently too lazy to fix the engine function.
		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( radiusInfo.m_vecSrc, &vecHitPoint );
		Vector vecDir = vecHitPoint - radiusInfo.m_vecSrc;

		if ( vecDir.LengthSqr() > ( radiusInfo.m_flRadius * radiusInfo.m_flRadius ) )
			continue;

		CTFPlayer *pTFAttacker = ToTFPlayer( pAttacker );
		CTFPlayer *pTFPlayer = ToTFPlayer( pEntity );
		if ( pTFPlayer )
		{
			if ( !pTFPlayer->InSameTeam( pAttacker ) )
			{
				pTFPlayer->m_Shared.AddCond( iCond, 10.0f );
				switch ( iCond )
				{
				case TF_COND_URINE:
					pTFPlayer->m_Shared.m_hUrineAttacker.Set( pAttacker );
					break;
				case TF_COND_MAD_MILK:
					pTFPlayer->m_Shared.m_hMilkAttacker.Set( pAttacker );
					break;
				default:
					break;
				}
			}
			else
			{
				if ( !pTFPlayer->InSameTeam( pAttacker ) && pTFPlayer->m_Shared.InCond( TF_COND_BURNING ) )
				{
					pTFPlayer->m_Shared.RemoveCond( TF_COND_BURNING );
					pTFPlayer->EmitSound( "TFPlayer.FlameOut" );

					if ( pEntity != pAttacker )
					{
						bExtinguished = true;

						if ( pTFAttacker )
						{
							// Bonus points.
							IGameEvent *event_bonus = gameeventmanager->CreateEvent( "player_bonuspoints" );
							if ( event_bonus )
							{
								event_bonus->SetInt( "player_entindex", pEntity->entindex() );
								event_bonus->SetInt( "source_entindex", pAttacker->entindex() );
								event_bonus->SetInt( "points", 1 );
								gameeventmanager->FireEvent( event_bonus );
							}
							CTF_GameStats.Event_PlayerAwardBonusPoints( pTFAttacker, pEntity, 1 );
						}
					}
				}
			}
		}

		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );
		if ( pNPC )
		{
			if ( !pNPC->InSameTeam( pAttacker ) )
			{
				if ( pNPC->AllowJar() )
				{
					pNPC->AddCond( iCond, 10.0f );
					switch ( iCond )
					{
					case TF_COND_URINE:
					#ifdef GAME_DLL
						pNPC->m_hUrineAttacker.Set( pAttacker );
					#endif
						break;
					case TF_COND_MAD_MILK:
					#ifdef GAME_DLL
						pNPC->m_hMilkAttacker.Set( pAttacker );
					#endif
						break;
					default:
						break;
					}
				}
			}
			else
			{
				if ( !pNPC->InSameTeam( pAttacker ) && pNPC->InCond( TF_COND_BURNING ) )
				{
					pNPC->RemoveCond( TF_COND_BURNING );
					pNPC->EmitSound( "TFPlayer.FlameOut" );

					if ( pEntity != pAttacker )
					{
						bExtinguished = true;

						if ( pTFAttacker )
						{
							// Bonus points.
							IGameEvent *event_bonus = gameeventmanager->CreateEvent( "player_bonuspoints" );
							if ( event_bonus )
							{
								event_bonus->SetInt( "player_entindex", pEntity->entindex() );
								event_bonus->SetInt( "source_entindex", pAttacker->entindex() );
								event_bonus->SetInt( "points", 1 );
								gameeventmanager->FireEvent( event_bonus );
							}
							CTF_GameStats.Event_PlayerAwardBonusPoints( pTFAttacker, pEntity, 1 );
						}
					}
				}
			}
		}
	}

	return bExtinguished;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::RadiusDamage( CTFRadiusDamageInfo &radiusInfo )
{
	CTakeDamageInfo *info = (CTakeDamageInfo *)radiusInfo.info;
	CBaseEntity *pAttacker = info->GetAttacker();
	int iPlayersDamaged = 0;

	CBaseEntity *pEntity = NULL;
	for ( CEntitySphereQuery sphere( radiusInfo.m_vecSrc, radiusInfo.m_flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity == radiusInfo.m_pEntityIgnore )
			continue;

		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// UNDONE: this should check a damage mask, not an ignore
		if ( radiusInfo.m_iClassIgnore != CLASS_NONE && pEntity->Classify() == radiusInfo.m_iClassIgnore )
		{
			continue;
		}

		// Skip the attacker as we'll handle him separately.
		if ( pEntity == pAttacker && radiusInfo.m_flSelfDamageRadius != 0.0f )
			continue;

		// Checking distance from source because Valve were apparently too lazy to fix the engine function.
		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( radiusInfo.m_vecSrc, &vecHitPoint );
		Vector vecDir = vecHitPoint - radiusInfo.m_vecSrc;

		if ( vecDir.LengthSqr() > ( radiusInfo.m_flRadius * radiusInfo.m_flRadius ) )
			continue;

		if ( radiusInfo.ApplyToEntity( pEntity ) )
		{
			if ( pEntity->IsPlayer() && !pEntity->InSameTeam( pAttacker ) )
			{
				iPlayersDamaged++;
			}
			else if ( pEntity->IsNPC() && !pEntity->InSameTeam( pAttacker ) )
			{
				iPlayersDamaged++;
			}
		}
	}

	info->SetDamagedOtherPlayers( iPlayersDamaged );

	// For attacker, radius and damage need to be consistent so custom weapons don't screw up rocket jumping.
	if ( radiusInfo.m_flSelfDamageRadius != 0.0f )
	{
		if ( pAttacker )
		{
			if ( radiusInfo.m_bStockSelfDamage )
			{
				// Get stock damage.
				CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( info->GetWeapon() );
				if ( pWeapon )
				{
					info->SetDamage( (float)pWeapon->GetTFWpnData().GetWeaponData( TF_WEAPON_PRIMARY_MODE ).m_nDamage );
					info->CopyDamageToBaseDamage();
				}
			}

			// Use stock radius.
			radiusInfo.m_flRadius = radiusInfo.m_flSelfDamageRadius;

			Vector vecHitPoint;
			pAttacker->CollisionProp()->CalcNearestPoint( radiusInfo.m_vecSrc, &vecHitPoint );
			Vector vecDir = vecHitPoint - radiusInfo.m_vecSrc;

			if ( vecDir.LengthSqr() <= ( radiusInfo.m_flRadius * radiusInfo.m_flRadius ) )
			{
				radiusInfo.ApplyToEntity( pAttacker );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//			&vecSrcIn - 
//			flRadius - 
//			iClassIgnore - 
//			*pEntityIgnore - 
//-----------------------------------------------------------------------------
void CTFGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore )
{
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info = &info;
	radiusInfo.m_vecSrc = vecSrcIn;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_iClassIgnore = iClassIgnore;
	radiusInfo.m_pEntityIgnore = pEntityIgnore;

	RadiusDamage( radiusInfo );
}

// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// Dead players can only be heard by other dead team mates but only if a match is in progress
		if ( TFGameRules()->State_Get() != GR_STATE_TEAM_WIN && TFGameRules()->State_Get() != GR_STATE_GAME_OVER ) 
		{
			if ( pTalker->IsAlive() == false )
			{
				if ( pListener->IsAlive() == false || tf_gravetalk.GetBool() )
					return ( pListener->InSameTeam( pTalker ) );

				return false;
			}
		}

		return ( pListener->InSameTeam( pTalker ) );
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

// Load the objects.txt file.
class CObjectsFileLoad : public CAutoGameSystem
{
public:
	virtual bool Init()
	{
		LoadObjectInfos( filesystem );
		return true;
	}
} g_ObjectsFileLoad;

// --------------------------------------------------------------------------------------------------- //
// Globals.
// --------------------------------------------------------------------------------------------------- //
/*
// NOTE: the indices here must match TEAM_UNASSIGNED, TEAM_SPECTATOR, TF_TEAM_RED, TF_TEAM_BLUE, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Red",
	"Blue"
};
*/
// --------------------------------------------------------------------------------------------------- //
// Global helper functions.
// --------------------------------------------------------------------------------------------------- //
	
// World.cpp calls this but we don't use it in TF.
void InitBodyQue()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFGameRules::~CTFGameRules()
{
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	TFTeamMgr()->Shutdown();
	ShutdownCustomResponseRulesDicts();
}

//-----------------------------------------------------------------------------
// Purpose: TF2 Specific Client Commands
//-----------------------------------------------------------------------------
bool CTFGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	CTFPlayer *pPlayer = ToTFPlayer( pEdict );

	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "objcmd" ) )
	{
		if ( args.ArgC() < 3 )
			return true;

		int entindex = atoi( args[1] );
		edict_t* pEdict = INDEXENT(entindex);
		if ( pEdict )
		{
			CBaseEntity* pBaseEntity = GetContainingEntity(pEdict);
			CBaseObject* pObject = dynamic_cast<CBaseObject*>(pBaseEntity);

			if ( pObject )
			{
				// We have to be relatively close to the object too...

				// BUG! Some commands need to get sent without the player being near the object, 
				// eg delayed dismantle commands. Come up with a better way to ensure players aren't
				// entering these commands in the console.

				//float flDistSq = pObject->GetAbsOrigin().DistToSqr( pPlayer->GetAbsOrigin() );
				//if (flDistSq <= (MAX_OBJECT_SCREEN_INPUT_DISTANCE * MAX_OBJECT_SCREEN_INPUT_DISTANCE))
				{
					// Strip off the 1st two arguments and make a new argument string
					CCommand objectArgs( args.ArgC() - 2, &args.ArgV()[2]);
					pObject->ClientCommand( pPlayer, objectArgs );
				}
			}
		}

		return true;
	}

	// Handle some player commands here as they relate more directly to gamerules state
	if ( FStrEq( pcmd, "nextmap" ) )
	{
		if ( pPlayer->m_flNextTimeCheck < gpGlobals->curtime )
		{
			char szNextMap[32];

			if ( nextlevel.GetString() && *nextlevel.GetString() && engine->IsMapValid( nextlevel.GetString() ) )
			{
				Q_strncpy( szNextMap, nextlevel.GetString(), sizeof( szNextMap ) );
			}
			else
			{
				GetNextLevelName( szNextMap, sizeof( szNextMap ) );
			}

			ClientPrint( pPlayer, HUD_PRINTTALK, "#TF_nextmap", szNextMap);

			pPlayer->m_flNextTimeCheck = gpGlobals->curtime + 1;
		}

		return true;
	}
	else if ( FStrEq( pcmd, "timeleft" ) )
	{	
		if ( pPlayer->m_flNextTimeCheck < gpGlobals->curtime )
		{
			if ( mp_timelimit.GetInt() > 0 )
			{
				int iTimeLeft = GetTimeLeft();

				char szMinutes[5];
				char szSeconds[3];

				if ( iTimeLeft <= 0 )
				{
					Q_snprintf( szMinutes, sizeof(szMinutes), "0" );
					Q_snprintf( szSeconds, sizeof(szSeconds), "00" );
				}
				else
				{
					Q_snprintf( szMinutes, sizeof(szMinutes), "%d", iTimeLeft / 60 );
					Q_snprintf( szSeconds, sizeof(szSeconds), "%02d", iTimeLeft % 60 );
				}				

				ClientPrint( pPlayer, HUD_PRINTTALK, "#TF_timeleft", szMinutes, szSeconds );
			}
			else
			{
				ClientPrint( pPlayer, HUD_PRINTTALK, "#TF_timeleft_nolimit" );
			}

			pPlayer->m_flNextTimeCheck = gpGlobals->curtime + 1;
		}
		return true;
	}
	else if ( FStrEq( pcmd, "freezecam_taunt" ) )
	{	
		// let's check this came from the client .dll and not the console
		//	int iCmdPlayerID = pPlayer->GetUserID();
		//	unsigned short mask = UTIL_GetAchievementEventMask();

		//int iAchieverIndex = atoi( args[1] ) ^ mask;
		//int code = ( iCmdPlayerID ^ iAchieverIndex ) ^ mask;
		//if ( code == atoi( args[2] ) )
		//{
		//	CTFPlayer *pAchiever = ToTFPlayer( UTIL_PlayerByIndex( iAchieverIndex ) );
		//	if ( pAchiever && ( pAchiever->GetUserID() != iCmdPlayerID ) )
		//	{
		//		int iClass = pAchiever->GetPlayerClass()->GetClassIndex();
		//		pAchiever->AwardAchievement( g_TauntCamAchievements[ iClass ] );
		//	}
		//}

		return true;
	}
	else if( pPlayer->ClientCommand( args ) )
	{
		   return true;
	}

	return BaseClass::ClientCommand( pEdict, args );
}

//-----------------------------------------------------------------------------
// Purpose: Add the ability to ignore the world trace
//-----------------------------------------------------------------------------
void CTFGameRules::Think()
{
	if ( !g_fGameOver )
	{
		if ( gpGlobals->curtime > m_flNextPeriodicThink )
		{
			if ( State_Get() != GR_STATE_TEAM_WIN )
			{
				if ( CheckCapsPerRound() )
					return;
			}
		}

		// Check to see if there is still a vote running
		if ( g_voteController->IsVoteActive() )
			return;
	}

	if (m_iDirectorAnger > m_iMaxDirectorAnger)
	{
		SetDirectorAnger( 100 );
	}
	if (m_iDirectorAnger < 0)
	{
		SetDirectorAnger( 0 );
	}
	if (m_iDirectorAnger == 100)
	{
		CBaseEntity *pBossSpawnpoint = gEntList.FindEntityByClassname( NULL, "info_director_boss" );
		variant_t sVariant2;
		if ( pBossSpawnpoint )
			pBossSpawnpoint->AcceptInput( "SpawnBoss", NULL, NULL, sVariant2, NULL );

		SetDirectorAnger( 20 );
	}

	if ( physcannon_mega_enabled.GetBool() == true )
		m_bMegaPhysgun = true;
	else
		m_bMegaPhysgun = ( GlobalEntity_GetState( "super_phys_gun" ) == GLOBAL_ON ); // FIXME: Is there a better place for this?

	if ( alyx_darkness_force.GetBool() == true )
		m_bDarknessMode = true;
	else
		m_bDarknessMode = ( GlobalEntity_GetState( "ep_alyx_darknessmode" ) == GLOBAL_ON );

#ifdef GAME_DLL
	if (lfe_player_lives.GetInt() != -1 && lfe_player_lives.GetInt() < 1)
		lfe_player_lives.SetValue(-1);

	if ( LFPopulationManager() )
		LFPopulationManager()->GameRulesThink();
#endif

	SpawnHalloweenBoss();

	BaseClass::Think();
}

//------------------------------------------------------------------------------
// Purpose : Initialize all default class relationships
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CTFGameRules::InitDefaultAIRelationships( void )
{
	int i, j;

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// --------------------------------------------------------------
	// First initialize table so we can report missing relationships
	// --------------------------------------------------------------
	for (i=0;i<NUM_AI_CLASSES;i++)
	{
		for (j=0;j<NUM_AI_CLASSES;j++)
		{
			// By default all relationships are neutral of priority zero
			CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_NU, 0 );
		}
	}

		// ------------------------------------------------------------
		//	> CLASS_ANTLION
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ANTLION,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_BARNACLE
		//
		//  In this case, the relationship D_HT indicates which characters
		//  the barnacle will try to eat.
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BARNACLE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_EARTH_FAUNA,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_BULLSEYE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_BULLSQUID
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HEADCRAB,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HOUNDEYE,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		// ------------------------------------------------------------
		//	> CLASS_CITIZEN_PASSIVE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_HUNTER,	D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HEADCRAB,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HOUNDEYE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_VORTIGAUNT,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ZOMBIE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_CITIZEN_REBEL
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE_GUNSHIP
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE_HUNTER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_CONSCRIPT
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		
		// ------------------------------------------------------------
		//	> CLASS_FLARE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_HEADCRAB
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HACKED_ROLLERMINE,D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_HOUNDEYE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_MANHACK
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HEADCRAB,			D_HT,-1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HOUNDEYE,			D_HT,-1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_METROPOLICE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_MILITARY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_MISSILE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_NONE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSEYE,			D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_REBEL,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_ALLY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HEADCRAB,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ZOMBIE,			D_FR, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PROTOSNIPER,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_ALLY_VITAL
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_HUNTER,	D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PROTOSNIPER,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HACKED_ROLLERMINE,D_LI, 0);

		// ------------------------------------------------------------
		//	> CLASS_SCANNER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MANHACK,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_METROPOLICE,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MILITARY,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_SCANNER,			D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_STALKER,			D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PROTOSNIPER,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_STALKER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_VORTIGAUNT
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_REBEL,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HACKED_ROLLERMINE,D_LI, 0);

		// ------------------------------------------------------------
		//	> CLASS_ZOMBIE
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MILITARY,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_PROTOSNIPER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MISSILE,			D_NU, 5);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_EARTH_FAUNA
		//
		// Hates pretty much everything equally except other earth fauna.
		// This will make the critter choose the nearest thing as its enemy.
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_NONE,				D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_FLARE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HEADCRAB,			D_HT, 0);
		//CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MISSILE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_SCANNER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_STALKER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_VORTIGAUNT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HACKED_ROLLERMINE,D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_HACKED_ROLLERMINE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ZOMBIE,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_EARTH_FAUNA,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HACKED_ROLLERMINE,D_LI, 0);

		//HALF-LIFE 1 CLASSES
		// ------------------------------------------------------------
		//	> CLASS_NONE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_PLAYER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_HUMAN_PASSIVE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_PLAYER_ALLY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIEN_PREY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIEN_MONSTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIEN_PREDATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_HUMAN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_INSECT, D_NU, 0);


		// ------------------------------------------------------------
		//	> CLASS_HUMAN_PASSIVE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_PLAYER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_HUMAN_PASSIVE, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_PLAYER_ALLY, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_ALIEN_PREY, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_ALIEN_MILITARY, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_ALIEN_PREDATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_PASSIVE, CLASS_INSECT, D_NU, 0);



		// ------------------------------------------------------------
		//	> CLASS_PLAYER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_PLAYER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_HUMAN_PASSIVE, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_PLAYER_ALLY, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIEN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIEN_PREDATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_MACHINE_HL1, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIEN_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_PLAYER_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_ALLY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_PLAYER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_HUMAN_PASSIVE, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_PLAYER_ALLY, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIEN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIEN_PREDATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_MACHINE_HL1, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIEN_PREY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_ALIEN_PREY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_ALIEN_MONSTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_ALIEN_PREDATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREY, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIEN_MILITARY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_ALIEN_PREY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_ALIEN_MONSTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_ALIEN_PREDATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_MACHINE_HL1, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MILITARY, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIEN_MONSTER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_ALIEN_PREY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_ALIEN_MONSTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_ALIEN_PREDATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_MACHINE_HL1, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_MONSTER, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIEN_PREDATOR
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_ALIEN_MONSTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_ALIEN_PREDATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_PREDATOR, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_HUMAN_MILITARY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_ALIEN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_ALIEN_PREDATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_HUMAN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HUMAN_MILITARY, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_MACHINE_HL1
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_HUMAN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_ALIEN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_ALIEN_PREDATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_ALIEN_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_PLAYER_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MACHINE_HL1, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIEN_BIOWEAPON
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_ALIEN_MILITARY, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_ALIEN_PREDATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_PLAYER_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIEN_BIOWEAPON, CLASS_INSECT, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_BIOWEAPON
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_MACHINE_HL1, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_HUMAN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_HUMAN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_ALIEN_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_ALIEN_MONSTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_ALIEN_PREY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_ALIEN_PREDATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_ALIEN_BIOWEAPON, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_BIOWEAPON, CLASS_INSECT, D_NU, 0);


		// ------------------------------------------------------------
		//	> CLASS_INSECT
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_NONE, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_MACHINE_HL1, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_PLAYER, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_HUMAN_PASSIVE, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_HUMAN_MILITARY, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_ALIEN_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_ALIEN_MONSTER, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_ALIEN_PREY, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_ALIEN_PREDATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_PLAYER_ALLY, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_ALIEN_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_PLAYER_BIOWEAPON, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_INSECT, CLASS_INSECT, D_NU, 0);
}

//Runs think for all player's conditions
//Need to do this here instead of the player so players that crash still run their important thinks
void CTFGameRules::RunPlayerConditionThink ( void )
{
	for ( int i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer )
		{
			pPlayer->m_Shared.ConditionGameRulesThink();
		}
	}
	// Run through all NPCs as well.
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC )
			continue;

		pNPC->ConditionGameRulesThink();
	}
}

void CTFGameRules::FrameUpdatePostEntityThink()
{
	BaseClass::FrameUpdatePostEntityThink();

	RunPlayerConditionThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SpawnHalloweenBoss( void )
{
	if ( !IsHolidayActive( kHoliday_Halloween ) )
		return;

	float fSpawnInterval = tf_halloween_boss_spawn_interval.GetFloat();
	float fSpawnVariation = tf_halloween_boss_spawn_interval_variation.GetFloat();

	if ( !m_hBosses.IsEmpty() )
	{
		if ( m_hBosses[0] )
		{
			isBossForceSpawning = false;
			m_bossSpawnTimer.Start( RandomFloat( fSpawnInterval - fSpawnVariation, fSpawnInterval + fSpawnVariation ) );
			return;
		}
	}

	if ( !m_bossSpawnTimer.HasStarted() && !isBossForceSpawning )
	{
		m_bossSpawnTimer.Start( RandomFloat( 0, fSpawnInterval + fSpawnVariation ) * 0.5 );
		return;
	}

	if ( m_bossSpawnTimer.IsElapsed() || isBossForceSpawning )
	{
		if ( !isBossForceSpawning )
		{
			if ( InSetup() || IsInWaitingForPlayers() )
				return;

			CUtlVector<CTFPlayer *> players;
			CollectPlayers( &players, TF_TEAM_RED, true );
			CollectPlayers( &players, TF_TEAM_BLUE, true, true );

			int nNumHumans = 0;
			for ( int i=0; i<players.Count(); ++i )
			{
				if ( !players[i]->IsBot() )
					nNumHumans++;
			}

			if ( tf_halloween_bot_min_player_count.GetInt() > nNumHumans )
				return;
		}

		Vector vecSpawnLoc = vec3_origin;
		if ( !g_hControlPointMasters.IsEmpty() && g_hControlPointMasters[0] )
		{
			CTeamControlPointMaster *pMaster = g_hControlPointMasters[0];
			for ( int i=0; i<pMaster->GetNumPoints(); ++i )
			{
				CTeamControlPoint *pPoint = pMaster->GetControlPoint( i );
				if ( pMaster->IsInRound( pPoint ) &&
					 ObjectiveResource()->GetOwningTeam( pPoint->GetPointIndex() ) != TF_TEAM_BLUE &&
					 TFGameRules()->TeamMayCapturePoint( TF_TEAM_BLUE, pPoint->GetPointIndex() ) )
				{
					CBaseEntity *pSpawnPoint = gEntList.FindEntityByClassname( NULL, "spawn_boss" );
					if ( pSpawnPoint )
					{
						vecSpawnLoc = pSpawnPoint->GetAbsOrigin();
					}
					else
					{
						vecSpawnLoc = pPoint->GetAbsOrigin();
					}
				}
			}
		}
		else
		{
			CBaseEntity *pSpawnPoint = gEntList.FindEntityByClassname( NULL, "spawn_boss" );
			if ( pSpawnPoint )
			{
				vecSpawnLoc = pSpawnPoint->GetAbsOrigin();
			}
			else
			{
				for ( int i = 1; i < GetGlobalTFTeam( TF_TEAM_RED )->GetNumPlayers(); i++ )
				{
					int iPlayer = random->RandomInt( 0, i );
					CTFPlayer *pPlayer = ToTFPlayer( GetGlobalTFTeam( TF_TEAM_RED )->GetPlayer( iPlayer ) );
					if ( pPlayer || pPlayer->IsAlive() )
					{
						int node = g_pBigAINet->NearestNodeToPoint( pPlayer->GetAbsOrigin(), false );
						CAI_Node *pNode = g_pBigAINet->GetNode( node );
						if ( pNode )
							vecSpawnLoc = pNode->GetPosition( HULL_MEDIUM_TALL );
					}
				}

				CUtlVector<CNavArea *> candidates;
				for ( int i=0; i<TheNavAreas.Count(); ++i )
				{
					CNavArea *area = TheNavAreas[i];
					if ( area->GetSizeX() >= 100.0f && area->GetSizeY() >= 100.0f )
						candidates.AddToTail( area );
				}

				if ( !candidates.IsEmpty() )
				{
					CNavArea *area = candidates.Random();
					vecSpawnLoc = area->GetCenter();
				}
			}
		}

		CAI_BaseNPC *pBoss = dynamic_cast<CAI_BaseNPC *>( CreateEntityByName( "headless_hatman" ) );
		if ( pBoss )
		{
			pBoss->SetAbsOrigin( vecSpawnLoc + Vector( 0, 0, 10 ) );
			DispatchSpawn( pBoss );
		}
		isBossForceSpawning = false;
		m_bossSpawnTimer.Start( RandomFloat( fSpawnInterval - fSpawnVariation, fSpawnInterval + fSpawnVariation ) );
	}
}

/*void CTFGameRules::SpawnZombieMob( void )
{
}*/

bool CTFGameRules::CheckCapsPerRound()
{
	if ( tf_flag_caps_per_round.GetInt() > 0 )
	{
		int iMaxCaps = -1;
		CTFTeam *pMaxTeam = NULL;

		// check to see if any team has won a "round"
		int nTeamCount = TFTeamMgr()->GetTeamCount();
		for ( int iTeam = FIRST_GAME_TEAM; iTeam < nTeamCount; ++iTeam )
		{
			CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
			if ( !pTeam )
				continue;

			// we might have more than one team over the caps limit (if the server op lowered the limit)
			// so loop through to see who has the most among teams over the limit
			if ( pTeam->GetFlagCaptures() >= tf_flag_caps_per_round.GetInt() )
			{
				if ( pTeam->GetFlagCaptures() > iMaxCaps )
				{
					iMaxCaps = pTeam->GetFlagCaptures();
					pMaxTeam = pTeam;
				}
			}
		}

		if ( iMaxCaps != -1 && pMaxTeam != NULL )
		{
			SetWinningTeam( pMaxTeam->GetTeamNumber(), WINREASON_FLAG_CAPTURE_LIMIT );
			return true;
		}
	}

	return false;
}

bool CTFGameRules::CheckWinLimit()
{
	if ( mp_winlimit.GetInt() != 0 )
	{
		bool bWinner = false;

		if ( TFTeamMgr()->GetTeam( TF_TEAM_BLUE )->GetScore() >= mp_winlimit.GetInt() )
		{
			UTIL_LogPrintf( "Team \"BLUE\" triggered \"Intermission_Win_Limit\"\n" );
			bWinner = true;
		}
		else if ( TFTeamMgr()->GetTeam( TF_TEAM_RED )->GetScore() >= mp_winlimit.GetInt() )
		{
			UTIL_LogPrintf( "Team \"RED\" triggered \"Intermission_Win_Limit\"\n" );
			bWinner = true;
		}

		if ( bWinner )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "tf_game_over" );
			if ( event )
			{
				event->SetString( "reason", "Reached Win Limit" );
				gameeventmanager->FireEvent( event );
			}

			GoToIntermission();
			return true;
		}
	}

	return false;
}

bool CTFGameRules::CheckFragLimit( void )
{
	if ( fraglimit.GetInt() <= 0 )
		return false;

	for ( int i = 1; i <= CountActivePlayers(); i++ )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if (pTFPlayer)
		{
			PlayerStats_t *pStats = CTF_GameStats.FindPlayerStats( pTFPlayer );
			int iScore = CalcPlayerScore( &pStats->statsCurrentRound );

			if (iScore >= fraglimit.GetInt())
			{
				GoToIntermission();
				return true;
			}
		}
	}

	return false;
}

bool CTFGameRules::IsInPreMatch( void ) const
{
	// TFTODO    return (cb_prematch_time > gpGlobals->time)
	return false;
}

float CTFGameRules::GetPreMatchEndTime( void ) const
{
	//TFTODO: implement this.
	return gpGlobals->curtime;
}

void CTFGameRules::GoToIntermission( void )
{
	BaseClass::GoToIntermission();
}

bool CTFGameRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, const CTakeDamageInfo &info )
{
	// guard against NULL pointers if players disconnect
	if ( !pPlayer || !pAttacker )
		return false;

	// if pAttacker is an object, we can only do damage if pPlayer is our builder
	if ( pAttacker->IsBaseObject() )
	{
		CBaseObject *pObj = ( CBaseObject *)pAttacker;

		if ( pObj->GetBuilder() == pPlayer || !pPlayer->InSameTeam( pObj ) )
			return true;
		else
			return false;
	}

	// if pAttacker is a npc, we can only do damage if pPlayer is not teammate
	if ( pAttacker->IsNPC() )
	{
		CAI_BaseNPC *pNPC = ( CAI_BaseNPC *)pAttacker;

		if ( !pPlayer->InSameTeam( pNPC ) )
			return true;
		else
			return false;
	}

	return BaseClass::FPlayerCanTakeDamage( pPlayer, pAttacker, info );
}

int CTFGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	if ( IsFriendlyFire() )
		return GR_NOTTEAMMATE;

	if ( pTarget->IsNPC() )
	{
		CAI_BaseNPC *pNPC = ( CAI_BaseNPC *)pTarget;

		if ( !pPlayer->InSameTeam( pNPC ) )
			return GR_NOTTEAMMATE;
		else
			return GR_TEAMMATE;
	}

	return BaseClass::PlayerRelationship( pPlayer, pTarget );
}

void CTFGameRules::PlayTrainCaptureAlert( CTeamControlPoint *pPoint, bool bFinalPointInMap )
{
	const char *pszSound = bFinalPointInMap ? TEAM_TRAIN_FINAL_ALERT : TEAM_TRAIN_ALERT;
	BroadcastSound( 255, pszSound );
}

bool CTFGameRules::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
{
	return BaseClass::ClientConnected( pEntity, pszName, pszAddress, reject, maxrejectlen );
}

Vector DropToGround( 
	CBaseEntity *pMainEnt, 
	const Vector &vPos, 
	const Vector &vMins, 
	const Vector &vMaxs )
{
	trace_t trace;
	UTIL_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );
	return trace.endpos;
}


void TestSpawnPointType( const char *pEntClassName )
{
	// Find the next spawn spot.
	CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, pEntClassName );

	while( pSpot )
	{
		// trace a box here
		Vector vTestMins = pSpot->GetAbsOrigin() + VEC_HULL_MIN;
		Vector vTestMaxs = pSpot->GetAbsOrigin() + VEC_HULL_MAX;

		if ( UTIL_IsSpaceEmpty( pSpot, vTestMins, vTestMaxs ) )
		{
			// the successful spawn point's location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 0, 255, 0, 100, 60 );

			// drop down to ground
			Vector GroundPos = DropToGround( NULL, pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX );

			// the location the player will spawn at
			NDebugOverlay::Box( GroundPos, VEC_HULL_MIN, VEC_HULL_MAX, 0, 0, 255, 100, 60 );

			// draw the spawn angles
			QAngle spotAngles = pSpot->GetLocalAngles();
			Vector vecForward;
			AngleVectors( spotAngles, &vecForward );
			NDebugOverlay::HorzArrow( pSpot->GetAbsOrigin(), pSpot->GetAbsOrigin() + vecForward * 32, 10, 255, 0, 0, 255, true, 60 );
		}
		else
		{
			// failed spawn point location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 255, 0, 0, 100, 60 );
		}

		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}
}

// -------------------------------------------------------------------------------- //

void TestSpawns()
{
	TestSpawnPointType( "info_player_teamspawn" );
}
ConCommand cc_TestSpawns( "map_showspawnpoints", TestSpawns, "Dev - test the spawn points, draws for 60 seconds", FCVAR_CHEAT );

// -------------------------------------------------------------------------------- //

void cc_ShowRespawnTimes()
{
	CTFGameRules *pRules = TFGameRules();
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() );

	if ( pRules && pPlayer )
	{
		float flRedMin = ( pRules->m_TeamRespawnWaveTimes[TF_TEAM_RED] >= 0 ? pRules->m_TeamRespawnWaveTimes[TF_TEAM_RED] : mp_respawnwavetime.GetFloat() );
		float flRedScalar = pRules->GetRespawnTimeScalar( TF_TEAM_RED );
		float flNextRedRespawn = pRules->GetNextRespawnWave( TF_TEAM_RED, NULL ) - gpGlobals->curtime;

		float flBlueMin = ( pRules->m_TeamRespawnWaveTimes[TF_TEAM_BLUE] >= 0 ? pRules->m_TeamRespawnWaveTimes[TF_TEAM_BLUE] : mp_respawnwavetime.GetFloat() );
		float flBlueScalar = pRules->GetRespawnTimeScalar( TF_TEAM_BLUE );
		float flNextBlueRespawn = pRules->GetNextRespawnWave( TF_TEAM_BLUE, NULL ) - gpGlobals->curtime;

		char tempRed[128];
		Q_snprintf( tempRed, sizeof( tempRed ),   "Red:  Min Spawn %2.2f, Scalar %2.2f, Next Spawn In: %.2f\n", flRedMin, flRedScalar, flNextRedRespawn );

		char tempBlue[128];
		Q_snprintf( tempBlue, sizeof( tempBlue ), "Blue: Min Spawn %2.2f, Scalar %2.2f, Next Spawn In: %.2f\n", flBlueMin, flBlueScalar, flNextBlueRespawn );

		ClientPrint( pPlayer, HUD_PRINTTALK, tempRed );
		ClientPrint( pPlayer, HUD_PRINTTALK, tempBlue );
	}
}

ConCommand mp_showrespawntimes( "mp_showrespawntimes", cc_ShowRespawnTimes, "Show the min respawn times for the teams" );

// -------------------------------------------------------------------------------- //

CBaseEntity *CTFGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	// get valid spawn point
	CBaseEntity *pSpawnSpot = pPlayer->EntSelectSpawnPoint();

	// drop down to ground
	//Vector GroundPos = DropToGround( pPlayer, pSpawnSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX );

	// Move the player to the place it said.
	pPlayer->SetLocalOrigin( pSpawnSpot->GetAbsOrigin() + Vector(0,0,1) );
	pPlayer->SetAbsVelocity( vec3_origin );
	pPlayer->SetLocalAngles( pSpawnSpot->GetLocalAngles() );
	pPlayer->m_Local.m_vecPunchAngle = vec3_angle;
	pPlayer->m_Local.m_vecPunchAngleVel = vec3_angle;
	pPlayer->SnapEyeAngles( pSpawnSpot->GetLocalAngles() );

	return pSpawnSpot;
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if the player is on the correct team and whether or
//          not the spawn point is available.
//-----------------------------------------------------------------------------
bool CTFGameRules::IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer, bool bIgnorePlayers )
{
	// Check the team.
	if ( !pSpot->InSameTeam( pPlayer ) )
		return false;

	if ( !pSpot->IsTriggered( pPlayer ) )
		return false;

	CTFTeamSpawn *pCTFSpawn = dynamic_cast<CTFTeamSpawn*>( pSpot );
	if ( pCTFSpawn )
	{
		if ( pCTFSpawn->IsDisabled() )
			return false;
	}

	Vector mins = GetViewVectors()->m_vHullMin;
	Vector maxs = GetViewVectors()->m_vHullMax;

	if ( !bIgnorePlayers )
	{
		Vector vTestMins = pSpot->GetAbsOrigin() + mins;
		Vector vTestMaxs = pSpot->GetAbsOrigin() + maxs;
		return UTIL_IsSpaceEmpty( pPlayer, vTestMins, vTestMaxs );
	}

	trace_t trace;
	UTIL_TraceHull( pSpot->GetAbsOrigin(), pSpot->GetAbsOrigin(), mins, maxs, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );
	return ( trace.fraction == 1 && trace.allsolid != 1 && (trace.startsolid != 1) );
}

Vector CTFGameRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

QAngle CTFGameRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
}

int CTFGameRules::ItemShouldRespawn( CItem *pItem )
{
	// TEMP - don't respawn non-TF items.
	if ( dynamic_cast<CTFPowerup *>( pItem ) == NULL )
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return BaseClass::ItemShouldRespawn( pItem );
}

float CTFGameRules::FlItemRespawnTime( CItem *pItem )
{
	return ITEM_RESPAWN_TIME;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFGameRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	CTFPlayer *pTFPlayer = ToTFPlayer(pPlayer);

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == true )
	{
		if ( pTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "TF_Chat_Spec";
		}
		else
		{
			if ( pTFPlayer->IsAlive() == false && State_Get() != GR_STATE_TEAM_WIN )
			{
				pszFormat = "TF_Chat_Team_Dead";
			}
			else
			{
				const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
				if ( chatLocation && *chatLocation )
				{
					pszFormat = "TF_Chat_Team_Loc";
				}
				else
				{
					pszFormat = "TF_Chat_Team";
				}
			}
		}
	}
	else if ( pTFPlayer->IsDeveloper() )
	{
		if ( pTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "TF_Chat_DevSpec";
		}
		else
		{
			if (pTFPlayer->IsAlive() == false && State_Get() != GR_STATE_TEAM_WIN)
			{
				pszFormat = "TF_Chat_DevDead";
			}
			else
			{
				pszFormat = "TF_Chat_Dev";
			}
		}
	}
	else if ( pTFPlayer->IsNicknine() )
	{
		if ( pTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "TF_Chat_NicknineSpec";
		}
		else
		{
			if (pTFPlayer->IsAlive() == false && State_Get() != GR_STATE_TEAM_WIN)
			{
				pszFormat = "TF_Chat_NicknineDead";
			}
			else
			{
				pszFormat = "TF_Chat_Nicknine";
			}
		}
	}
	else
	{
		if ( IsHolidayActive( kHoliday_AprilFools ) )
		{
			if ( pTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			{
				pszFormat = "TF_Chat_GaySpec";	
			}
			else
			{
				if ( pTFPlayer->IsAlive() == false && State_Get() != GR_STATE_TEAM_WIN )
				{
					pszFormat = "TF_Chat_GayDead";
				}
				else
				{
					pszFormat = "TF_Chat_Gay";	
				}
			}
		}
		else
		{
			if ( pTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			{
				pszFormat = "TF_Chat_AllSpec";	
			}
			else
			{
				if ( pTFPlayer->IsAlive() == false && State_Get() != GR_STATE_TEAM_WIN )
				{
					pszFormat = "TF_Chat_AllDead";
				}
				else
				{
					pszFormat = "TF_Chat_All";	
				}
			}
		}
	}

	return pszFormat;
}

VoiceCommandMenuItem_t *CTFGameRules::VoiceCommand( CBaseMultiplayerPlayer *pPlayer, int iMenu, int iItem )
{
	VoiceCommandMenuItem_t *pItem = BaseClass::VoiceCommand( pPlayer, iMenu, iItem );

	if ( pItem )
	{
		int iActivity = ActivityList_IndexForName( pItem->m_szGestureActivity );

		if ( iActivity != ACT_INVALID )
		{
			CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );

			if ( pTFPlayer )
			{
				pTFPlayer->DoAnimationEvent( PLAYERANIMEVENT_VOICE_COMMAND_GESTURE, iActivity );
			}
		}
	}

	return pItem;
}

//-----------------------------------------------------------------------------
// Purpose: Actually change a player's name.  
//-----------------------------------------------------------------------------
void CTFGameRules::ChangePlayerName( CTFPlayer *pPlayer, const char *pszNewName )
{
	const char *pszOldName = pPlayer->GetPlayerName();

	CReliableBroadcastRecipientFilter filter;
	UTIL_SayText2Filter( filter, pPlayer, false, "#TF_Name_Change", pszOldName, pszNewName );

	IGameEvent * event = gameeventmanager->CreateEvent( "player_changename" );
	if ( event )
	{
		event->SetInt( "userid", pPlayer->GetUserID() );
		event->SetString( "oldname", pszOldName );
		event->SetString( "newname", pszNewName );
		gameeventmanager->FireEvent( event );
	}

	pPlayer->SetPlayerName( pszNewName );

	pPlayer->m_flNextNameChangeTime = gpGlobals->curtime + 10.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
	const char *pszName = engine->GetClientConVarValue( pPlayer->entindex(), "name" );

	const char *pszOldName = pPlayer->GetPlayerName();

	CTFPlayer *pTFPlayer = (CTFPlayer*)pPlayer;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	// Note, not using FStrEq so that this is case sensitive
	if ( pszOldName[0] != 0 && Q_strncmp( pszOldName, pszName, MAX_PLAYER_NAME_LENGTH-1 ) )		
	{
		if ( pTFPlayer->m_flNextNameChangeTime < gpGlobals->curtime )
		{
			ChangePlayerName( pTFPlayer, pszName );
		}
		else
		{
			// no change allowed, force engine to use old name again
			engine->ClientCommand( pPlayer->edict(), "name \"%s\"", pszOldName );

			// tell client that he hit the name change time limit
			ClientPrint( pTFPlayer, HUD_PRINTTALK, "#Name_change_limit_exceeded" );
		}
	}

	if ( pTFPlayer->IsFakeClient() )
		return;

	// keep track of their hud_classautokill value
	int nClassAutoKill = Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "hud_classautokill" ) );
	pTFPlayer->SetHudClassAutoKill( nClassAutoKill > 0 ? true : false );

	// keep track of their tf_medigun_autoheal value
	pTFPlayer->SetMedigunAutoHeal( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "tf_medigun_autoheal" ) ) > 0 );

	// keep track of their cl_autorezoom value
	pTFPlayer->SetAutoRezoom( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "cl_autorezoom" ) ) > 0 );

	// keep track of their cl_autoreload value
	pTFPlayer->SetAutoReload( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "cl_autoreload" ) ) > 0 );

	pTFPlayer->SetFlipViewModel( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "cl_flipviewmodels" ) ) > 0 );

	// Keep track of their spawn particle.
	//pTFPlayer->m_Shared.SetRespawnParticleID( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "tf2c_setmercparticle" ) ) );

	const char *pszFov = engine->GetClientConVarValue( pPlayer->entindex(), "fov_desired" );
	int iFov = atoi( pszFov );
	iFov = clamp( iFov, 75, MAX_FOV );
	pTFPlayer->SetDefaultFOV( iFov );

	pTFPlayer->m_Shared.m_bIsPlayerADev = pTFPlayer->PlayerHasPowerplay() && ( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "lfe_dev_mark" ) ) > 0 );
	pTFPlayer->m_Shared.m_bIsPlayerNicknine = pTFPlayer->IsNicknine() && ( Q_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "lfe_dev_mark" ) ) > 0 );
}

static const char *g_aTaggedConVars[] =
{
	"tf_birthday",
	"tf2birthday",

	"mp_fadetoblack",
	"fadetoblack",

	"mp_friendlyfire",
	"friendlyfire",

	"tf_weapon_criticals",
	"nocrits",

	"tf_damage_disablespread",
	"dmgspread",

	"tf_use_fixed_weaponspreads",
	"nospread",

	"lfe_force_legacy",
	"legacy",

	"lfe_allow_thirdperson",
	"thirdperson",

	"lfe_random_weapons",
	"randomizer",

	"lfe_autojump",
	"autojump",

	"lfe_duckjump",
	"duckjump",

	"lfe_allow_airblast",
	"noairblast",

	"mp_highlander",
	"highlander",

	"mp_disable_respawn_times",
	"norespawntime",

	"mp_respawnwavetime",
	"respawntimes",

	"mp_stalemate_enable",
	"suddendeath",

	"tf_gamemode_arena",
	"arena",

	"tf_gamemode_cp",
	"cp",

	"tf_gamemode_ctf",
	"ctf",

	"tf_gamemode_sd",
	"sd",

	"tf_gamemode_rd",
	"rd",

	"tf_gamemode_pd",
	"pd",

	"tf_gamemode_payload",
	"payload",

	"tf_gamemode_mvm",
	"mvm",

	"tf_gamemode_passtime",
	"passtime",

	"tf_gamemode_tc",
	"tc",

	"tf_beta_content",
	"beta",

	"tf_gamemode_misc",
	"misc",

	"lfe_coop",
	"coop",

	"lfe_gamemode_versus",
	"versus",

	"lfe_gamemode_blucoop",
	"blucoop",

	"lfe_gamemode_horde",
	"horde",

	"lfe_gamemode_free",
	"free",

	"sv_cheats",
	"cheats",

	"sv_infinite_ammo",
	"infinite",
};

//-----------------------------------------------------------------------------
// Purpose: Tags
//-----------------------------------------------------------------------------
void CTFGameRules::GetTaggedConVarList( KeyValues *pCvarTagList )
{
	COMPILE_TIME_ASSERT( ARRAYSIZE( g_aTaggedConVars ) % 2 == 0 );

	BaseClass::GetTaggedConVarList( pCvarTagList );

	for ( int i = 0; i < ARRAYSIZE( g_aTaggedConVars ); i += 2 )
	{
		KeyValues *pKeyValue = new KeyValues( g_aTaggedConVars[i] );
		pKeyValue->SetString( "convar", g_aTaggedConVars[i] );
		pKeyValue->SetString( "tag", g_aTaggedConVars[i+1] );

		pCvarTagList->AddSubKey( pKeyValue );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified player can carry any more of the ammo type
//-----------------------------------------------------------------------------
bool CTFGameRules::CanHaveAmmo( CBaseCombatCharacter *pPlayer, int iAmmoIndex )
{
	if ( iAmmoIndex > -1 )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );

		if ( pTFPlayer )
		{
			// Get the max carrying capacity for this ammo
			int iMaxCarry = pTFPlayer->GetMaxAmmo( iAmmoIndex );

			// Does the player have room for more of this type of ammo?
			if ( pTFPlayer->GetAmmoCount( iAmmoIndex ) < iMaxCarry )
			{
				return true;
			}
		}
		else
		{
			return BaseClass::CanHaveAmmo( pPlayer, iAmmoIndex );
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBaseMultiplayerPlayer *pScorer = ToBaseMultiplayerPlayer( GetDeathScorer( pKiller, pInflictor, pVictim ) );
	CTFPlayer *pAssister = NULL;
	CBaseObject *pObject = NULL;

	// if inflictor or killer is a base object, tell them that they got a kill
	// ( depends if a sentry rocket got the kill, sentry may be inflictor or killer )
	if ( pInflictor )
	{
		if ( pInflictor->IsBaseObject() )
		{
			pObject = dynamic_cast<CBaseObject *>( pInflictor );
		}
		else 
		{
			CBaseEntity *pInflictorOwner = pInflictor->GetOwnerEntity();
			if ( pInflictorOwner && pInflictorOwner->IsBaseObject() )
			{
				pObject = dynamic_cast<CBaseObject *>( pInflictorOwner );
			}
		}
		
	}
	else if( pKiller && pKiller->IsBaseObject() )
	{
		pObject = dynamic_cast<CBaseObject *>( pKiller );
	}

	if ( pObject )
	{
		pObject->IncrementKills();
		pInflictor = pObject;

		if ( pObject->ObjectType() == OBJ_SENTRYGUN )
		{
			CTFPlayer *pOwner = pObject->GetOwner();
			if ( pOwner )
			{
				int iKills = pObject->GetKills();

				// keep track of max kills per a single sentry gun in the player object
				if ( pOwner->GetMaxSentryKills() < iKills )
				{
					pOwner->SetMaxSentryKills( iKills );
					CTF_GameStats.Event_MaxSentryKills( pOwner, iKills );
				}

				// if we just got 10 kills with one sentry, tell the owner's client, which will award achievement if it doesn't have it already
				if ( iKills == 10 )
				{
					pOwner->AwardAchievement( ACHIEVEMENT_TF_GET_TURRETKILLS );
				}
			}
		}
	}

	// if not killed by  suicide or killed by world, see if the scorer had an assister, and if so give the assister credit
	if ( ( pVictim != pScorer ) && pKiller )
	{
		pAssister = ToTFPlayer( GetAssister( pVictim, pScorer, pInflictor ) );
	}	

	//find the area the player is in and see if his death causes a block
	CTriggerAreaCapture *pArea = dynamic_cast<CTriggerAreaCapture *>( gEntList.FindEntityByClassname( NULL, "trigger_capture_area" ) );
	while( pArea )
	{
		if ( pArea->CheckIfDeathCausesBlock( ToTFPlayer( pVictim ), pScorer ) )
			break;

		pArea = dynamic_cast<CTriggerAreaCapture *>( gEntList.FindEntityByClassname( pArea, "trigger_capture_area" ) );
	}

	// determine if this kill affected a nemesis relationship
	int iDeathFlags = 0;
	CTFPlayer *pTFPlayerVictim = ToTFPlayer( pVictim );
	CTFPlayer *pTFPlayerScorer = ToTFPlayer( pScorer );
	if ( pScorer )
	{	
		CalcDominationAndRevenge( pTFPlayerScorer, pTFPlayerVictim, false, &iDeathFlags );
		if ( pAssister )
		{
			CalcDominationAndRevenge( pAssister, pTFPlayerVictim, true, &iDeathFlags );
		}
	}
	pTFPlayerVictim->SetDeathFlags( iDeathFlags );	

	if ( pAssister )
	{
		CTF_GameStats.Event_AssistKill( ToTFPlayer( pAssister ), pVictim );
		if ( pObject )
			pObject->IncrementAssists();
	}

	UpdateDirectorAnger();

	BaseClass::PlayerKilled( pVictim, info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::NPCKilled( CAI_BaseNPC *pVictim, const CTakeDamageInfo &info )
{
	CBaseEntity *pKiller = info.GetAttacker();
	CBaseEntity *pInflictor = info.GetInflictor();

	CBaseObject *pObject = NULL;

	if ( pInflictor )
	{
		pObject = dynamic_cast<CBaseObject *>( pInflictor );
	}
	else if ( pKiller && pKiller->IsBaseObject() )
	{
		pObject = dynamic_cast<CBaseObject *>( pKiller );
	}

	if ( pObject )
	{
		pObject->IncrementKills();
		pInflictor = pObject;

		if ( pObject->ObjectType() == OBJ_SENTRYGUN )
		{
			CTFPlayer *pOwner = pObject->GetOwner();
			if ( pOwner )
			{
				int iKills = pObject->GetKills();

				// keep track of max kills per a single sentry gun in the player object
				if ( pOwner->GetMaxSentryKills() < iKills )
				{
					pOwner->SetMaxSentryKills( iKills );
					CTF_GameStats.Event_MaxSentryKills( pOwner, iKills );
				}

				// if we just got 10 kills with one sentry, tell the owner's client, which will award achievement if it doesn't have it already
				if ( iKills == 10 )
				{
					pOwner->AwardAchievement( ACHIEVEMENT_TF_GET_TURRETKILLS );
				}
			}
		}
	}

	//find the area the npc is in and see if his death causes a block
	/*CTriggerAreaCapture *pArea = dynamic_cast<CTriggerAreaCapture *>( gEntList.FindEntityByClassname( NULL, "trigger_capture_area" ) );
	while( pArea )
	{
		if ( pArea->CheckIfDeathCausesBlock( pVictim, pKiller ) )
			break;

		pArea = dynamic_cast<CTriggerAreaCapture *>( gEntList.FindEntityByClassname( pArea, "trigger_capture_area" ) );
	}*/

	UpdateDirectorAnger();
}

//-----------------------------------------------------------------------------
// Purpose: Determines if attacker and victim have gotten domination or revenge
//-----------------------------------------------------------------------------
void CTFGameRules::CalcDominationAndRevenge( CTFPlayer *pAttacker, CTFPlayer *pVictim, bool bIsAssist, int *piDeathFlags )
{
	PlayerStats_t *pStatsVictim = CTF_GameStats.FindPlayerStats( pVictim );

	// calculate # of unanswered kills between killer & victim - add 1 to include current kill
	int iKillsUnanswered = pStatsVictim->statsKills.iNumKilledByUnanswered[pAttacker->entindex()] + 1;		
	if ( TF_KILLS_DOMINATION == iKillsUnanswered )
	{			
		// this is the Nth unanswered kill between killer and victim, killer is now dominating victim
		*piDeathFlags |= ( bIsAssist ? TF_DEATH_ASSISTER_DOMINATION : TF_DEATH_DOMINATION );
		// set victim to be dominated by killer
		pAttacker->m_Shared.SetPlayerDominated( pVictim, true );
		// record stats
		CTF_GameStats.Event_PlayerDominatedOther( pAttacker );
	}
	else if ( pVictim->m_Shared.IsPlayerDominated( pAttacker->entindex() ) )
	{
		// the killer killed someone who was dominating him, gains revenge
		*piDeathFlags |= ( bIsAssist ? TF_DEATH_ASSISTER_REVENGE : TF_DEATH_REVENGE );
		// set victim to no longer be dominating the killer
		pVictim->m_Shared.SetPlayerDominated( pAttacker, false );
		// record stats
		CTF_GameStats.Event_PlayerRevenge( pAttacker );
	}

}

//-----------------------------------------------------------------------------
// Purpose: create some proxy entities that we use for transmitting data */
//-----------------------------------------------------------------------------
void CTFGameRules::CreateStandardEntities()
{
	// Create the player resource
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create( "tf_player_manager", vec3_origin, vec3_angle );

	// Create the objective resource
	g_pObjectiveResource = (CTFObjectiveResource *)CBaseEntity::Create( "tf_objective_resource", vec3_origin, vec3_angle );

	Assert( g_pObjectiveResource );

	// Create the entity that will send our data to the client.
	CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, "tf_gamerules" );
	if ( !pEnt )
	{
		pEnt = CBaseEntity::Create( "tf_gamerules", vec3_origin, vec3_angle );
		pEnt->SetName( AllocPooledString( "tf_gamerules" ) );
	}

	CBaseEntity::Create("vote_controller", vec3_origin, vec3_angle);

	new CKickIssue();
	new CRestartGameIssue();
	new CChangeLevelIssue();
	new CNextLevelIssue();
	new CChangeDifficultyIssue();
	new CChangeMapAddIssue();

	if ( IsHordeMode() )
	{
		CBaseEntity *pHorde = gEntList.FindEntityByClassname( NULL, "lfe_logic_horde" );
		if ( !pHorde )
		{
			pHorde = CBaseEntity::Create( "lfe_logic_horde", vec3_origin, vec3_angle );
			pHorde->SetName( AllocPooledString( "lfe_logic_horde" ) );
		}
	}

	// To do: once all antirush is redone with func_antirush in all maps (perhaps never?),
	// remove the findentitybyname antirush removal things. Please.

	// also: does this even work? Stoneman said it does as long as it's called antirushX/antibrushX/antiteleX,
	// X being an number, but I tested it out and it didn't work --Lecs

	// func_antirush removal is now handled in func_antirush.cpp

	if ( IsFreeMode() )
	{
		CBaseEntity *pNoBuild = gEntList.FindEntityByClassname( NULL, "func_nobuild" );
		if ( pNoBuild )
			UTIL_Remove( pNoBuild );

		CBaseEntity *pWpn = gEntList.FindEntityByClassname( NULL, "lfe_weapon_progress" );
		if ( pWpn )
			UTIL_Remove( pWpn );

		CBaseEntity *pRushes = gEntList.FindEntityByName( NULL, "antirush*" );
		if ( pRushes )
			UTIL_Remove( pRushes );

		CBaseEntity *pBRushes = gEntList.FindEntityByName( NULL, "antibrush*" );
		if ( pBRushes )
			UTIL_Remove( pBRushes );

		CBaseEntity *pTRushes = gEntList.FindEntityByName( NULL, "antitele*" );
		if ( pTRushes )
			UTIL_Remove( pTRushes );

//		CBaseEntity *pFRushes = gEntList.FindEntityByClassname(NULL, "func_antirush");
//		if (pFRushes)
//			UTIL_Remove(pFRushes);

		if ( IsInHL2Map() )
		{
			CBaseEntity *pChangelevel = gEntList.FindEntityByClassname( NULL, "trigger_changelevel" );
			variant_t sVariant;
			if ( pChangelevel )
				pChangelevel->AcceptInput( "Enable", NULL, NULL, sVariant, NULL );
		}
	}

	if ( !lfe_allow_antirush.GetBool() )
	{
		CBaseEntity *pRushes = gEntList.FindEntityByName( NULL, "antirush*" );
		if ( pRushes )
			UTIL_Remove( pRushes );

		CBaseEntity *pBRushes = gEntList.FindEntityByName( NULL, "antibrush*" );
		if ( pBRushes )
			UTIL_Remove( pBRushes );

		CBaseEntity *pTRushes = gEntList.FindEntityByName( NULL, "antitele*" );
		if ( pTRushes )
			UTIL_Remove( pTRushes );

//		CBaseEntity *pFRushes = gEntList.FindEntityByClassname(NULL, "func_antirush");
//		if (pFRushes)
//			UTIL_Remove(pFRushes);

	}
}

//-----------------------------------------------------------------------------
// Purpose: determine the class name of the weapon that got a kill
//-----------------------------------------------------------------------------
const char *CTFGameRules::GetKillingWeaponName( const CTakeDamageInfo &info, CTFPlayer *pVictim, int &iOutputID )
{
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CTFPlayer *pScorer = ToTFPlayer( TFGameRules()->GetDeathScorer( pKiller, pInflictor, pVictim ) );
	CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( info.GetWeapon() );
	int iWeaponID = TF_WEAPON_NONE;

	const char *killer_weapon_name = "world";

	// Handle special kill types first.
	const char *pszCustomKill = NULL;

	switch ( info.GetDamageCustom() )
	{
	case TF_DMG_CUSTOM_SUICIDE:
		pszCustomKill = "world";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_HADOUKEN:
		pszCustomKill = "taunt_pyro";
		break;
	case TF_DMG_CUSTOM_BURNING_FLARE:
		pszCustomKill = "flaregun";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON:
		pszCustomKill = "taunt_heavy";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_FENCING:
		pszCustomKill = "taunt_spy";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB:
		pszCustomKill = "taunt_sniper";
		break;
	case TF_DMG_CUSTOM_TELEFRAG:
		pszCustomKill = "telefrag";
		break;
	case TF_DMG_CUSTOM_BURNING_ARROW:
		pszCustomKill = "huntsman_burning";
		break;
	case TF_DMG_CUSTOM_FLYINGBURN:
		pszCustomKill = "huntsman_flyingburn";
		break;
	case TF_DMG_CUSTOM_PUMPKIN_BOMB:
		pszCustomKill = "pumpkindeath";
		break;
	case TF_DMG_CUSTOM_PICKAXE:
		pszCustomKill = "pickaxe";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_GRENADE:
		pszCustomKill = "taunt_soldier";
		break;
	case TF_DMG_CUSTOM_BASEBALL:
		pszCustomKill = "ball";
		break;
	case TF_DMG_CUSTOM_CHARGE_IMPACT:
		pszCustomKill = "demoshield";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING:
		pszCustomKill = "taunt_demoman";
		break;
	case TF_DMG_CUSTOM_DEFENSIVE_STICKY:
		pszCustomKill = "sticky_resistance";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_UBERSLICE:
		pszCustomKill = "taunt_medic";
		break;
	case TF_DMG_CUSTOM_PLAYER_SENTRY:
		pszCustomKill = "player_sentry";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH:
		pszCustomKill = "taunt_guitar_kill";
		break;
	case TF_DMG_CUSTOM_BLEEDING:
		pszCustomKill = "bleed_kill";
		break;
	case TF_DMG_CUSTOM_CARRIED_BUILDING:
		pszCustomKill = "building_carried_destroyed";
		break;
	case TF_DMG_CUSTOM_COMBO_PUNCH:
		pszCustomKill = "robot_arm_combo_kill";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL:
		pszCustomKill = "robot_arm_blender_kill";
		break;
	case TF_DMG_CUSTOM_STICKBOMB_EXPLOSION:
		pszCustomKill = "ullapool_caber_explosion";
		break;
	case TF_DMG_CUSTOM_BOOTS_STOMP:
		pszCustomKill = "mantreads";
		break;
	case TF_DMG_CUSTOM_DECAPITATION_BOSS:
		pszCustomKill = "battleaxe";
		break;
	case TF_DMG_CUSTOM_EYEBALL_ROCKET:
		pszCustomKill = "eyeball_rocket";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON:
		pszCustomKill = "armageddon";
		break;
	case TF_DMG_CUSTOM_CANNONBALL_PUSH:
		pszCustomKill = "loose_cannon_impact";
		break;
	case TF_DMG_CUSTOM_MERASMUS_DECAPITATION:
		pszCustomKill = "merasmus_decap";
		break;
	case TF_DMG_CUSTOM_MERASMUS_ZAP:
		pszCustomKill = "merasmus_zap";
		break;
	case TF_DMG_CUSTOM_MERASMUS_GRENADE:
		pszCustomKill = "merasmus_grenade";
		break;
	case TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB:
		pszCustomKill = "merasmus_player_bomb";
		break;
	case TF_DMG_CUSTOM_SPELL_TELEPORT:
		pszCustomKill = "spellbook_teleport";
		break;
	case TF_DMG_CUSTOM_SPELL_SKELETON:
		pszCustomKill = "spellbook_skeleton";
		break;
	case TF_DMG_CUSTOM_SPELL_MIRV:
		pszCustomKill = "spellbook_mirv";
		break;
	case TF_DMG_CUSTOM_SPELL_METEOR:
		pszCustomKill = "spellbook_meteor";
		break;
	case TF_DMG_CUSTOM_SPELL_LIGHTNING:
		pszCustomKill = "spellbook_lightning";
		break;
	case TF_DMG_CUSTOM_SPELL_FIREBALL:
		pszCustomKill = "spellbook_fireball";
		break;
	case TF_DMG_CUSTOM_SPELL_MONOCULUS:
		pszCustomKill = "spellbook_boss";
		break;
	case TF_DMG_CUSTOM_SPELL_BLASTJUMP:
		pszCustomKill = "spellbook_blastjump";
		break;
	case TF_DMG_CUSTOM_SPELL_BATS:
		pszCustomKill = "spellbook_bats";
		break;
	case TF_DMG_CUSTOM_SPELL_TINY:
		pszCustomKill = "spellbook_athletic";
		break;
	case TF_DMG_CUSTOM_KART:
		pszCustomKill = "bumper_kart";
		break;
	case TF_DMG_CUSTOM_GIANT_HAMMER:
		pszCustomKill = "necro_smasher";
		break;
	case TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING:
		pszCustomKill = "dragons_fury_bonus";
		break;
	case TF_DMG_CUSTOM_CROC:
		pszCustomKill = "crocodile";
		break;
	case TF_DMG_CUSTOM_TAUNTATK_GASBLAST:
		pszCustomKill = "gas_blast";
		break;
	case LFE_DMG_CUSTOM_LEECHES:
		pszCustomKill = "leeches";
		break;
	case LFE_DMG_CUSTOM_JEEP:
		pszCustomKill = "jeep";
		break;
	case LFE_DMG_CUSTOM_AIRBOAT:
		pszCustomKill = "airboat";
		break;
	case LFE_DMG_CUSTOM_JALOPY:
		pszCustomKill = "jeep_episodic";
		break;
	case LFE_DMG_CUSTOM_HUNTER_FLECHETTE:
		pszCustomKill = "flechette";
		break;
	case LFE_DMG_CUSTOM_HUNTER_FLECHETTE_EXPLODE:
		pszCustomKill = "flechette_explode";
		break;
	case LFE_DMG_CUSTOM_ANTLION_WORKER_EXPLODE:
		pszCustomKill = "antworker_explosion";
		break;
	case LFE_DMG_CUSTOM_PHYSCANNON_MEGA:
		pszCustomKill = "physcannon_mega";
		break;
	case LFE_DMG_CUSTOM_PHYSCANNON_MEGA_TERTIARY:
		pszCustomKill = "physcannon_mega_tertiary";
		break;
	}

	if ( pszCustomKill != NULL )
		return pszCustomKill;

	if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING )
	{
		// Player stores last weapon that burned him so if he burns to death we know what killed him.
		if ( pWeapon )
		{
			killer_weapon_name = pWeapon->GetClassname();
			iWeaponID = pWeapon->GetWeaponID();

			if ( pInflictor && pInflictor != pScorer )
			{
				CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pInflictor );

				if ( pRocket && pRocket->m_iDeflected )
				{
					// Fire weapon deflects go here.
					switch ( pRocket->GetWeaponID() )
					{
					case TF_WEAPON_FLAREGUN:
						killer_weapon_name = "deflect_flare";
						break;
					}
				}
			}
		}
		else
		{
			// Default to flamethrower if no burn weapon is specified.
			killer_weapon_name = "tf_weapon_flamethrower";
			iWeaponID = TF_WEAPON_FLAMETHROWER;
		}
	}
	else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE )
	{
		// Player stores last weapon that burned him so if he burns to death we know what killed him.
		if ( pWeapon )
		{
			killer_weapon_name = pWeapon->GetClassname();
			iWeaponID = pWeapon->GetWeaponID();

			if ( pInflictor && pInflictor != pScorer )
			{
				CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pInflictor );

				if ( pRocket && pRocket->m_iDeflected )
				{
					// Fire weapon deflects go here.
					switch ( pRocket->GetWeaponID() )
					{
					case TF_WEAPON_ROCKETLAUNCHER_FIREBALL:
						killer_weapon_name = "dragons_fury";
						break;
					}
				}
			}
		}
		else
		{
			// Default to flamethrower if no burn weapon is specified.
			killer_weapon_name = "dragons_fury";
			iWeaponID = TF_WEAPON_ROCKETLAUNCHER_FIREBALL;
		}
	}
	else if ( pScorer && pInflictor && ( pInflictor == pScorer ) )
	{
		// If the inflictor is the killer, then it must be their current weapon doing the damage
		CTFWeaponBase *pActiveWpn = pScorer->GetActiveTFWeapon();
		if ( pActiveWpn )
		{
			killer_weapon_name = pActiveWpn->GetClassname();
			iWeaponID = pActiveWpn->GetWeaponID();
		}
	}
	else if ( pKiller && pKiller->IsNPC() && ( pInflictor == pKiller ) )
	{
		// Similar case for NPCs.
		CAI_BaseNPC *pNPC = assert_cast<CAI_BaseNPC *>( pKiller );

		if ( pNPC->GetActiveWeapon() )
		{
			killer_weapon_name = pNPC->GetActiveWeapon()->GetClassname(); 
		}
		else
		{
			killer_weapon_name = pNPC->GetClassname();
		}
	}
	else if ( pInflictor )
	{
		killer_weapon_name = pInflictor->GetClassname();

		if ( CTFWeaponBase *pTFWeapon = dynamic_cast<CTFWeaponBase *>( pInflictor ) )
		{
			iWeaponID = pTFWeapon->GetWeaponID();
		}
		// See if this was a deflect kill.
		else if ( CTFBaseRocket *pRocket = dynamic_cast<CTFBaseRocket *>( pInflictor ) )
		{
			iWeaponID = pRocket->GetWeaponID();

			if ( pRocket->m_iDeflected )
			{
				switch ( pRocket->GetWeaponID() )
				{
				case TF_WEAPON_ROCKETLAUNCHER:
				case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
					killer_weapon_name = "deflect_rocket";
					break;
				case TF_WEAPON_COMPOUND_BOW:
					if( info.GetDamageType() & DMG_IGNITE )
						killer_weapon_name = "deflect_huntsman_flyingburn";
					else
						killer_weapon_name = "deflect_arrow";
					break;
				}
			}
		}
		else if ( CTFWeaponBaseGrenadeProj *pGrenade = dynamic_cast<CTFWeaponBaseGrenadeProj *>( pInflictor ) )
		{
			iWeaponID = pGrenade->GetWeaponID();

			// Most grenades have their own kill icons except for pipes and stickies, those use weapon icons.
			if ( iWeaponID == TF_WEAPON_GRENADE_DEMOMAN || iWeaponID == TF_WEAPON_GRENADE_PIPEBOMB )
			{
				CTFWeaponBase *pLauncher = dynamic_cast<CTFWeaponBase *>( pGrenade->m_hLauncher.Get() );
				if ( pLauncher )
				{
					iWeaponID = pLauncher->GetWeaponID();
				}
			}

			if ( pGrenade->m_iDeflected )
			{
				switch ( pGrenade->GetWeaponID() )
				{
				case TF_WEAPON_GRENADE_PIPEBOMB:
					killer_weapon_name = "deflect_sticky";
					break;
				case TF_WEAPON_GRENADE_DEMOMAN:
					killer_weapon_name = "deflect_promode";
					break;
				}
			}
		}
	}

	// Fix for HL2 pistol to prevent conflict with TF2 pistol. Must do this before stripping prefix.
	if ( V_strcmp( killer_weapon_name, "weapon_pistol" ) == 0 )
	{
		killer_weapon_name = "weapon_pistol_hl";
	}

	// strip certain prefixes from inflictor's classname
	const char *prefix[] = { "tf_weapon_grenade_", "tf_weapon_", "weapon_", "npc_", "func_", "prop_", "monster_", "grenade_" };
	for ( int i = 0; i < ARRAYSIZE( prefix ); i++ )
	{
		// if prefix matches, advance the string pointer past the prefix
		int len = V_strlen( prefix[i] );
		if ( V_strncmp( killer_weapon_name, prefix[i], len ) == 0 )
		{
			killer_weapon_name += len;
			break;
		}
	}

	// In case of a sentry kill change the icon according to sentry level.
	if (V_strcmp(killer_weapon_name, "obj_sentrygun") == 0)
	{
		CObjectSentrygun *pObject = assert_cast<CObjectSentrygun *>(pInflictor);

		if (pObject)
		{
			switch (pObject->GetUpgradeLevel())
			{
			case 2:
				killer_weapon_name = "obj_sentrygun2";
				break;
			case 3:
				killer_weapon_name = "obj_sentrygun3";
				break;
			}

			if ( pObject->IsMiniBuilding() )
				killer_weapon_name = "obj_minisentry";

			if ( pObject->IsWrangled() )
				killer_weapon_name = "wrangler_kill";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "tf_projectile_sentryrocket" ) == 0 )
	{
		// look out for sentry rocket as weapon and map it to sentry gun, so we get the L3 sentry death icon
		killer_weapon_name = "obj_sentrygun3";
	}
	// make sure arrow kills are mapping to the huntsman
	else if ( 0 == V_strcmp( killer_weapon_name, "tf_projectile_arrow" ) )
	{
		if( info.GetDamageType() & DMG_IGNITE )
			killer_weapon_name = "huntsman_flyingburn";
		else
			killer_weapon_name = "huntsman";
	}
	else if ( V_strcmp( killer_weapon_name, "tf_projectile_healing_bolt" ) == 0 )
	{
		killer_weapon_name = "crusaders_crossbow";
	}
	// Some special cases for NPCs.
	else if ( V_strcmp( killer_weapon_name, "strider" ) == 0 )
	{
		if ( info.GetDamageType() & DMG_BULLET )
		{
			killer_weapon_name = "strider_minigun";
		}
		else if ( info.GetDamageType() & DMG_CRUSH )
		{
			killer_weapon_name = "strider_skewer";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "concussiveblast" ) == 0 )
	{
		if ( pKiller && FClassnameIs( pKiller, "npc_strider" ) )
		{
			killer_weapon_name = "strider_beam";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "vortigaunt" ) == 0 )
	{
		if ( info.GetDamageType() & DMG_SHOCK )
		{
			killer_weapon_name = "vortigaunt_beam";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "vehicle_jeep" ) == 0 )
	{
		if ( info.GetDamageType() & DMG_SHOCK )
		{
			killer_weapon_name = "gauss";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "vehicle_airboat" ) == 0 )
	{
		if ( info.GetAmmoType() & DMG_BULLET )
		{
			killer_weapon_name = "helicopter";
		}
	}
	else if ( V_strcmp( killer_weapon_name, "hunter" ) == 0 )
	{
		if ( info.GetDamageType() & DMG_CLUB )
		{
			killer_weapon_name = "hunter_charge";
		}
		else if ( info.GetDamageType() & DMG_CLUB )
		{
			killer_weapon_name = "hunter_skewer";
		}
	}

	else if ( iWeaponID )
	{
		iOutputID = iWeaponID;
	}

	return killer_weapon_name;
}

//-----------------------------------------------------------------------------
// Purpose: returns the player who assisted in the kill, or NULL if no assister
//-----------------------------------------------------------------------------
CBasePlayer *CTFGameRules::GetAssister( CBasePlayer *pVictim, CBasePlayer *pScorer, CBaseEntity *pInflictor )
{
	CTFPlayer *pTFScorer = ToTFPlayer( pScorer );
	CTFPlayer *pTFVictim = ToTFPlayer( pVictim );
	if ( pTFScorer && pTFVictim )
	{
		// if victim killed himself, don't award an assist to anyone else, even if there was a recent damager
		if ( pTFScorer == pTFVictim )
			return NULL;

		// If a player is healing the scorer, give that player credit for the assist
		CTFPlayer *pHealer = ToTFPlayer( static_cast<CBaseEntity *>( pTFScorer->m_Shared.GetFirstHealer() ) );
		// Must be a medic to receive a healing assist, otherwise engineers get credit for assists from dispensers doing healing.
		// Also don't give an assist for healing if the inflictor was a sentry gun, otherwise medics healing engineers get assists for the engineer's sentry kills.
		if ( pHealer && pHealer->IsPlayerClass( TF_CLASS_MEDIC ) && ( dynamic_cast<CObjectSentrygun *>( pInflictor ) ) == NULL )
		{
			return pHealer;
		}

		// Players who apply jarate should get next priority
		CTFPlayer *pThrower = ToTFPlayer( static_cast<CBaseEntity *>( pTFVictim->m_Shared.m_hUrineAttacker.Get() ) );
		if( pThrower && pThrower != pTFScorer )
		{
			return pThrower;
		}
		// Mad milk after jarate
		CTFPlayer *pThrowerMilk = ToTFPlayer( static_cast<CBaseEntity *>( pTFVictim->m_Shared.m_hMilkAttacker.Get() ) );
		if ( pThrowerMilk )
		{
			if ( pThrowerMilk != pTFScorer )
				return pThrowerMilk;
		}

		// See who has damaged the victim 2nd most recently (most recent is the killer), and if that is within a certain time window.
		// If so, give that player an assist.  (Only 1 assist granted, to single other most recent damager.)
		CTFPlayer *pRecentDamager = ToTFPlayer( GetRecentDamager( pTFVictim, 1, TF_TIME_ASSIST_KILL ) );
		if ( pRecentDamager && ( pRecentDamager != pScorer ) )
			return pRecentDamager;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: returns the player/NPC who assisted in the kill, or NULL if no assister
//-----------------------------------------------------------------------------
CBaseEntity *CTFGameRules::GetAssister( CBaseEntity *pVictim, CBaseEntity *pKiller, CBaseEntity *pInflictor )
{
	// See if the killer is a player.
	CTFPlayer *pTFScorer = ToTFPlayer( GetDeathScorer( pKiller, pInflictor, pVictim ) );
	//CTFPlayer *pTFVictim = ToTFPlayer( pVictim );
	if ( pTFScorer )
	{
		// if victim killed himself, don't award an assist to anyone else, even if there was a recent damager
		if ( pVictim == pTFScorer )
			return NULL;

		// If a player is healing the scorer, give that player credit for the assist
		CTFPlayer *pHealer = ToTFPlayer( static_cast<CBaseEntity *>( pTFScorer->m_Shared.GetFirstHealer() ) );
		// Must be a medic to receive a healing assist, otherwise engineers get credit for assists from dispensers doing healing.
		// Also don't give an assist for healing if the inflictor was a sentry gun, otherwise medics healing engineers get assists for the engineer's sentry kills.
		if ( pHealer && pHealer->IsPlayerClass( TF_CLASS_MEDIC ) && ( dynamic_cast<CObjectSentrygun *>( pInflictor ) ) == NULL )
		{
			return pHealer;
		}

		// See who has damaged the victim 2nd most recently (most recent is the killer), and if that is within a certain time window.
		// If so, give that player an assist.  (Only 1 assist granted, to single other most recent damager.)
		CBaseEntity *pRecentDamager = GetRecentDamager( pVictim, 1, TF_TIME_ASSIST_KILL );
		if ( pRecentDamager && ( pRecentDamager != pTFScorer ) )
			return pRecentDamager;
	}
	else if ( pKiller && pKiller->IsNPC() )	// See if the killer is NPC.
	{
		CAI_BaseNPC *pNPCKiller = assert_cast<CAI_BaseNPC *>( pKiller );

		// if victim killed himself, don't award an assist to anyone else, even if there was a recent damager
		if ( pVictim == pKiller )
			return NULL;

		CTFPlayer *pHealer = ToTFPlayer( static_cast<CBaseEntity *>( pNPCKiller->GetFirstHealer() ) );

		if( pHealer && pHealer->IsPlayerClass( TF_CLASS_MEDIC ) )
			return pHealer;

		// See who has damaged the victim 2nd most recently (most recent is the killer), and if that is within a certain time window.
		// If so, give that player an assist.  (Only 1 assist granted, to single other most recent damager.)
		CBaseEntity *pRecentDamager = GetRecentDamager( pVictim, 1, TF_TIME_ASSIST_KILL );
		if ( pRecentDamager && ( pRecentDamager != pKiller ) )
			return pRecentDamager;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Returns specifed recent damager, if there is one who has done damage
//			within the specified time period.  iDamager=0 returns the most recent
//			damager, iDamager=1 returns the next most recent damager.
//-----------------------------------------------------------------------------
CBaseEntity *CTFGameRules::GetRecentDamager( CBaseEntity *pVictim, int iDamager, float flMaxElapsed )
{
	Assert( iDamager < MAX_DAMAGER_HISTORY );

	DamagerHistory_t *pDamagerHistory = NULL;

	if ( pVictim->IsPlayer() )
	{
		CTFPlayer *pTFVictim = ToTFPlayer( pVictim );
		pDamagerHistory = &pTFVictim->GetDamagerHistory( iDamager );
	}
	else if ( pVictim->IsNPC() )
	{
		CAI_BaseNPC *pNPC = assert_cast<CAI_BaseNPC *>( pVictim );
		pDamagerHistory = &pNPC->GetDamagerHistory( iDamager );
	}

	if ( pDamagerHistory )
	{
		if ( ( pDamagerHistory->hDamager.Get() != NULL ) && ( gpGlobals->curtime - pDamagerHistory->flTimeDamage <= flMaxElapsed ) )
		{
			return pDamagerHistory->hDamager.Get();
		}

		return NULL;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Returns who should be awarded the kill
//-----------------------------------------------------------------------------
CBasePlayer *CTFGameRules::GetDeathScorer( CBaseEntity *pKiller, CBaseEntity *pInflictor, CBaseEntity *pVictim )
{
	if ( ( pKiller == pVictim ) && ( pKiller == pInflictor ) )
	{
		// If this was an explicit suicide, see if there was a damager within a certain time window.  If so, award this as a kill to the damager.
		CTFPlayer *pTFVictim = ToTFPlayer( pVictim );
		if ( pTFVictim )
		{
			CTFPlayer *pRecentDamager = ToTFPlayer( GetRecentDamager( pTFVictim, 0, TF_TIME_SUICIDE_KILL_CREDIT ) );
			if ( pRecentDamager )
				return pRecentDamager;
		}
	}

	return BaseClass::GetDeathScorer( pKiller, pInflictor, pVictim );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//			*pKiller - 
//			*pInflictor - 
//-----------------------------------------------------------------------------
void CTFGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	int killer_ID = 0;
	int killer_index = 0;

	// Find the killer & the scorer
	CTFPlayer *pTFPlayerVictim = ToTFPlayer( pVictim );
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CTFPlayer *pScorer = ToTFPlayer( GetDeathScorer( pKiller, pInflictor, pVictim ) );
	CBaseEntity *pAssister =  GetAssister( pVictim, pScorer, pInflictor );
	CTFPlayer *pTFAssister = ToTFPlayer( pAssister );

	int iWeaponID = TF_WEAPON_NONE;

	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = GetKillingWeaponName( info, pTFPlayerVictim, iWeaponID );
	const char *killer_weapon_log_name = NULL;

	if ( iWeaponID && pScorer )
	{
		CTFWeaponBase *pWeapon = pScorer->Weapon_OwnsThisID( iWeaponID );
		if ( pWeapon )
		{
			CEconItemDefinition *pItemDef = pWeapon->GetItem()->GetStaticData();
			if ( pItemDef )
			{
				if ( pItemDef->item_iconname[0] )
					killer_weapon_name = pItemDef->item_iconname;

				if ( pItemDef->item_logname[0] )
					killer_weapon_log_name = pItemDef->item_logname;
			}
		}
	}

	if ( pScorer )	// Is the killer a client?
	{
		killer_ID = pScorer->GetUserID();
		killer_index = pScorer->entindex();
	}
	else if ( pKiller && pKiller->IsNPC() )
	{
		killer_index = pKiller->entindex();
	}

	int iDeathFlags = pTFPlayerVictim->GetDeathFlags();

	if ( IsInArenaMode() && !m_bFirstBlood && pScorer && pScorer != pTFPlayerVictim )
	{
		m_bFirstBlood = true;
		float flElapsedTime = gpGlobals->curtime - m_flRoundStartTime;

		if ( flElapsedTime <= 20.0 )
		{
			for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
			{
				BroadcastSound( i, "Announcer.AM_FirstBloodFast" );
			}
		}
		else if ( flElapsedTime < 50.0 )
		{
			for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
			{
				BroadcastSound( i, "Announcer.AM_FirstBloodRandom" );
			}
		}
		else
		{
			for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
			{
				BroadcastSound( i, "Announcer.AM_FirstBloodFinally" );
			}
		}

		iDeathFlags |= TF_DEATH_FIRST_BLOOD;
		pScorer->m_Shared.AddCond( TF_COND_CRITBOOSTED_FIRST_BLOOD, 5.0f );
	}
	// Feign death, purgatory death, australium death etc are all processed here.

	pTFPlayerVictim->SetDeathFlags( iDeathFlags );

	IGameEvent *event = NULL;
	event = gameeventmanager->CreateEvent("player_death");

	if ( event && event != NULL )
	{
		event->SetInt( "userid", pVictim->GetUserID() );
		event->SetInt( "victim_index", pVictim->entindex() );
		event->SetInt( "attacker", killer_ID );
		event->SetInt( "attacker_index", killer_index );
		event->SetString( "attacker_name", pKiller ? pKiller->GetClassname() : NULL );
		event->SetInt( "attacker_team", pKiller ? pKiller->GetTeamNumber() : 0 );
		event->SetInt( "assister", pTFAssister ? pTFAssister->GetUserID() : -1 );
		event->SetInt( "assister_index", pAssister ? pAssister->entindex() : -1 );
		event->SetString( "assister_name", pAssister ? pAssister->GetClassname() : NULL );
		event->SetInt( "assister_team", pAssister ? pAssister->GetTeamNumber() : 0 );
		event->SetString( "weapon", killer_weapon_name );
		event->SetInt( "weaponid", iWeaponID );
		event->SetString( "weapon_logclassname", killer_weapon_log_name );
		event->SetInt( "playerpenetratecount", info.GetPlayerPenetrationCount() );
		event->SetInt( "damagebits", info.GetDamageType() );
		event->SetInt( "customkill", info.GetDamageCustom() );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted
		event->SetInt( "death_flags", pTFPlayerVictim->GetDeathFlags() );
#if 0
		if ( pTFPlayerVictim->GetDeathFlags() & TF_DEATH_DOMINATION )
		{
			event->SetInt( "dominated", 1 );
		}
		if ( pTFPlayerVictim->GetDeathFlags() & TF_DEATH_ASSISTER_DOMINATION )
		{
			event->SetInt( "assister_dominated", 1 );
		}
		if ( pTFPlayerVictim->GetDeathFlags() & TF_DEATH_REVENGE )
		{
			event->SetInt( "revenge", 1 );
		}
		if ( pTFPlayerVictim->GetDeathFlags() & TF_DEATH_ASSISTER_REVENGE )
		{
			event->SetInt( "assister_revenge", 1 );
		}
#endif

		gameeventmanager->FireEvent( event );
	}
}

void CTFGameRules::ClientDisconnected( edict_t *pClient )
{
	// clean up anything they left behind
	CTFPlayer *pPlayer = ToTFPlayer( GetContainingEntity( pClient ) );
	if ( pPlayer )
	{
		pPlayer->TeamFortress_ClientDisconnected();
	}

	// are any of the spies disguising as this player?
	for ( int i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		CTFPlayer *pTemp = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pTemp && pTemp != pPlayer )
		{
			if ( pTemp->m_Shared.GetDisguiseTarget() == pPlayer )
			{
				// choose someone else...
				pTemp->m_Shared.FindDisguiseTarget();
			}
		}
	}

	BaseClass::ClientDisconnected( pClient );
}

//-----------------------------------------------------------------------------
// Purpose: How can Players Take fall damage?
//-----------------------------------------------------------------------------
float CTFGameRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	if ( pPlayer->m_Local.m_flFallVelocity > TF_PLAYER_MAX_SAFE_FALL_SPEED )
	{
		// Old TFC damage formula
		float flFallDamage = 5 * ( pPlayer->m_Local.m_flFallVelocity / 300 );

		// Fall damage needs to scale according to the player's max health, or
		// it's always going to be much more dangerous to weaker classes than larger.
		float flRatio = (float)pPlayer->GetMaxHealth() / 100.0;
		flFallDamage *= flRatio;

		flFallDamage *= random->RandomFloat( 0.8, 1.2 );

		int iCancelFallDmg = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pPlayer, iCancelFallDmg, cancel_falling_damage );
		if ( iCancelFallDmg )
			return 0;

		CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
		if ( pTFPlayer )
		{
			if ( pTFPlayer->m_Shared.InCond( TF_COND_RUNE_AGILITY ) )
				return 0;

			if ( pTFPlayer->m_Shared.InCond( TF_COND_GRAPPLINGHOOK_SAFEFALL ) )
				return 0;
		}

		return flFallDamage;
	}

	// Fall caused no damage
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: How can NPCs Take fall damage?
//-----------------------------------------------------------------------------
float CTFGameRules::FlNPCFallDamage( CAI_BaseNPC *pNPC )
{
	// only with airblast
	if ( pNPC->m_flFallVelocity > TF_PLAYER_MAX_SAFE_FALL_SPEED && pNPC->InCond( TF_COND_KNOCKED_INTO_AIR ) )
	{
		// Old TFC damage formula
		float flFallDamage = 5 * ( pNPC->m_flFallVelocity / 300 );

		// Fall damage needs to scale according to the player's max health, or
		// it's always going to be much more dangerous to weaker classes than larger.
		float flRatio = (float)pNPC->GetMaxHealth() / 100.0;
		flFallDamage *= flRatio;

		flFallDamage *= random->RandomFloat( 0.8, 1.2 );

		return flFallDamage;
	}

	// Fall caused no damage
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Send info to winpanel
//-----------------------------------------------------------------------------
void CTFGameRules::SendWinPanelInfo( void )
{
	IGameEvent *winEvent = gameeventmanager->CreateEvent( "teamplay_win_panel" );

	if ( winEvent )
	{
		int iBlueScore = GetGlobalTeam( TF_TEAM_BLUE )->GetScore();
		int iRedScore = GetGlobalTeam( TF_TEAM_RED )->GetScore();
		int iBlueScorePrev = iBlueScore;
		int iRedScorePrev = iRedScore;

		bool bRoundComplete = m_bForceMapReset || ( IsGameUnderTimeLimit() && ( GetTimeLeft() <= 0 ) );

		CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
		bool bScoringPerCapture = ( pMaster ) ? ( pMaster->ShouldScorePerCapture() ) : false;

		if ( bRoundComplete && !bScoringPerCapture )
		{
			// if this is a complete round, calc team scores prior to this win
			switch ( m_iWinningTeam )
			{
			case TF_TEAM_BLUE:
				iBlueScorePrev = ( iBlueScore - TEAMPLAY_ROUND_WIN_SCORE >= 0 ) ? ( iBlueScore - TEAMPLAY_ROUND_WIN_SCORE ) : 0;
				break;
			case TF_TEAM_RED:
				iRedScorePrev = ( iRedScore - TEAMPLAY_ROUND_WIN_SCORE >= 0 ) ? ( iRedScore - TEAMPLAY_ROUND_WIN_SCORE ) : 0;
				break;

			case TEAM_UNASSIGNED:
				break;	// stalemate; nothing to do
			}
		}

		winEvent->SetInt( "panel_style", WINPANEL_BASIC );
		winEvent->SetInt( "winning_team", m_iWinningTeam );
		winEvent->SetInt( "winreason", m_iWinReason );
		winEvent->SetString( "cappers",  ( m_iWinReason == WINREASON_ALL_POINTS_CAPTURED || m_iWinReason == WINREASON_FLAG_CAPTURE_LIMIT ) ?
			m_szMostRecentCappers : "" );
		winEvent->SetInt( "flagcaplimit", tf_flag_caps_per_round.GetInt() );
		winEvent->SetInt( "blue_score", iBlueScore );
		winEvent->SetInt( "red_score", iRedScore );
		winEvent->SetInt( "blue_score_prev", iBlueScorePrev );
		winEvent->SetInt( "red_score_prev", iRedScorePrev );
		winEvent->SetInt( "round_complete", bRoundComplete );

		CTFPlayerResource *pResource = GetTFPlayerResource();
		if ( !pResource )
			return;
 
		// determine the 3 players on winning team who scored the most points this round

		// build a vector of players & round scores
		CUtlVector<PlayerRoundScore_t> vecPlayerScore;
		int iPlayerIndex;
		for( iPlayerIndex = 1 ; iPlayerIndex <= MAX_PLAYERS; iPlayerIndex++ )
		{
			CTFPlayer *pTFPlayer = ToTFPlayer( UTIL_PlayerByIndex( iPlayerIndex ) );
			if ( !pTFPlayer || !pTFPlayer->IsConnected() )
				continue;
			// filter out spectators and, if not stalemate, all players not on winning team
			int iPlayerTeam = pTFPlayer->GetTeamNumber();
			if ( ( iPlayerTeam < FIRST_GAME_TEAM ) || ( m_iWinningTeam != TEAM_UNASSIGNED && ( m_iWinningTeam != iPlayerTeam ) ) )
				continue;

			int iRoundScore = 0, iTotalScore = 0;
			int iKills = 0, iDeaths = 0;
			PlayerStats_t *pStats = CTF_GameStats.FindPlayerStats( pTFPlayer );
			if ( pStats )
			{
				iRoundScore = CalcPlayerScore( &pStats->statsCurrentRound );
				iTotalScore = CalcPlayerScore( &pStats->statsAccumulated );
				iKills = pStats->statsCurrentRound.m_iStat[TFSTAT_KILLS];
				iDeaths = pStats->statsCurrentRound.m_iStat[TFSTAT_DEATHS];
			}
			PlayerRoundScore_t &playerRoundScore = vecPlayerScore[vecPlayerScore.AddToTail()];
			playerRoundScore.iPlayerIndex = iPlayerIndex;
			playerRoundScore.iRoundScore = iRoundScore;
			playerRoundScore.iTotalScore = iTotalScore;
			playerRoundScore.iKills = iKills;
			playerRoundScore.iDeaths = iDeaths;
		}
		// sort the players by round score
		vecPlayerScore.Sort( PlayerRoundScoreSortFunc );

		// set the top (up to) 3 players by round score in the event data
		int numPlayers = min( 3, vecPlayerScore.Count() );
		for ( int i = 0; i < numPlayers; i++ )
		{
			// only include players who have non-zero points this round; if we get to a player with 0 round points, stop
			if ( 0 == vecPlayerScore[i].iRoundScore )
				break;

			// set the player index and their round score in the event
			char szPlayerIndexVal[64] = "", szPlayerScoreVal[64] = "";
			char szPlayerKillsVal[64] = "", szPlayerDeathsVal[64] = "";
			Q_snprintf( szPlayerIndexVal, ARRAYSIZE( szPlayerIndexVal ), "player_%d", i + 1 );
			Q_snprintf( szPlayerScoreVal, ARRAYSIZE( szPlayerScoreVal ), "player_%d_points", i + 1 );
			Q_snprintf(szPlayerKillsVal, ARRAYSIZE(szPlayerKillsVal), "player_%d_kills", i + 1);
			Q_snprintf(szPlayerDeathsVal, ARRAYSIZE(szPlayerDeathsVal), "player_%d_deaths", i + 1);
			winEvent->SetInt( szPlayerIndexVal, vecPlayerScore[i].iPlayerIndex );
			winEvent->SetInt( szPlayerScoreVal, vecPlayerScore[i].iRoundScore );				
			winEvent->SetInt(szPlayerKillsVal, vecPlayerScore[i].iKills);
			winEvent->SetInt(szPlayerDeathsVal, vecPlayerScore[i].iDeaths);
		}

		if ( !bRoundComplete && ( TEAM_UNASSIGNED != m_iWinningTeam ) )
		{
			// if this was not a full round ending, include how many mini-rounds remain for winning team to win
			if ( g_hControlPointMasters.Count() && g_hControlPointMasters[0] )
			{
				winEvent->SetInt( "rounds_remaining", g_hControlPointMasters[0]->CalcNumRoundsRemaining( m_iWinningTeam ) );
			}
		}

		// Send the event
		gameeventmanager->FireEvent( winEvent );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sorts players by round score
//-----------------------------------------------------------------------------
int CTFGameRules::PlayerRoundScoreSortFunc( const PlayerRoundScore_t *pRoundScore1, const PlayerRoundScore_t *pRoundScore2 )
{
	// sort first by round score	
	if ( pRoundScore1->iRoundScore != pRoundScore2->iRoundScore )
		return pRoundScore2->iRoundScore - pRoundScore1->iRoundScore;

	// if round scores are the same, sort next by total score
	if ( pRoundScore1->iTotalScore != pRoundScore2->iTotalScore )
		return pRoundScore2->iTotalScore - pRoundScore1->iTotalScore;

	// if scores are the same, sort next by player index so we get deterministic sorting
	return ( pRoundScore2->iPlayerIndex - pRoundScore1->iPlayerIndex );
}

//-----------------------------------------------------------------------------
// Purpose: Called when the teamplay_round_win event is about to be sent, gives
//			this method a chance to add more data to it
//-----------------------------------------------------------------------------
void CTFGameRules::FillOutTeamplayRoundWinEvent( IGameEvent *event )
{
	// determine the losing team
	int iLosingTeam;

	switch( event->GetInt( "team" ) )
	{
	case TF_TEAM_RED:
		iLosingTeam = TF_TEAM_BLUE;
		break;
	case TF_TEAM_BLUE:
		iLosingTeam = TF_TEAM_RED;
		break;
	case TEAM_UNASSIGNED:
	default:
		iLosingTeam = TEAM_UNASSIGNED;
		break;
	}

	// set the number of caps that team got any time during the round
	event->SetInt( "losing_team_num_caps", m_iNumCaps[iLosingTeam] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetupSpawnPointsForRound( void )
{
	if ( !g_hControlPointMasters.Count() || !g_hControlPointMasters[0] || !g_hControlPointMasters[0]->PlayingMiniRounds() )
		return;

	CTeamControlPointRound *pCurrentRound = g_hControlPointMasters[0]->GetCurrentRound();
	if ( !pCurrentRound )
	{
		return;
	}

	// loop through the spawn points in the map and find which ones are associated with this round or the control points in this round
	CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, "info_player_teamspawn" );
	while( pSpot )
	{
		CTFTeamSpawn *pTFSpawn = assert_cast<CTFTeamSpawn*>(pSpot);

		if ( pTFSpawn )
		{
			CHandle<CTeamControlPoint> hControlPoint = pTFSpawn->GetControlPoint();
			CHandle<CTeamControlPointRound> hRoundBlue = pTFSpawn->GetRoundBlueSpawn();
			CHandle<CTeamControlPointRound> hRoundRed = pTFSpawn->GetRoundRedSpawn();

			if ( hControlPoint && pCurrentRound->IsControlPointInRound( hControlPoint ) )
			{
				// this spawn is associated with one of our control points
				pTFSpawn->SetDisabled( false );
				pTFSpawn->ChangeTeam( hControlPoint->GetOwner() );
			}
			else if ( hRoundBlue && ( hRoundBlue == pCurrentRound ) )
			{
				pTFSpawn->SetDisabled( false );
				pTFSpawn->ChangeTeam( TF_TEAM_BLUE );
			}
			else if ( hRoundRed && ( hRoundRed == pCurrentRound ) )
			{
				pTFSpawn->SetDisabled( false );
				pTFSpawn->ChangeTeam( TF_TEAM_RED );
			}
			else
			{
				// this spawn isn't associated with this round or the control points in this round
				pTFSpawn->SetDisabled( true );
			}
		}

		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_teamspawn" );
	}
}


int CTFGameRules::SetCurrentRoundStateBitString( void )
{
	m_iPrevRoundState = m_iCurrentRoundState;

	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;

	if ( !pMaster )
	{
		return 0;
	}

	int iState = 0;

	for ( int i=0; i<pMaster->GetNumPoints(); i++ )
	{
		CTeamControlPoint *pPoint = pMaster->GetControlPoint( i );

		if ( pPoint->GetOwner() == TF_TEAM_BLUE )
		{
			// Set index to 1 for the point being owned by blue
			iState |= ( 1<<i );
		}
	}

	m_iCurrentRoundState = iState;

	return iState;
}


void CTFGameRules::SetMiniRoundBitMask( int iMask )
{
	m_iCurrentMiniRoundMask = iMask;
}

//-----------------------------------------------------------------------------
// Purpose: NULL pPlayer means show the panel to everyone
//-----------------------------------------------------------------------------
void CTFGameRules::ShowRoundInfoPanel( CTFPlayer *pPlayer /* = NULL */ )
{
	KeyValues *data = new KeyValues( "data" );

	if ( m_iCurrentRoundState < 0 )
	{
		// Haven't set up the round state yet
		return;
	}

	// if prev and cur are equal, we are starting from a fresh round
	if ( m_iPrevRoundState >= 0 && pPlayer == NULL )	// we have data about a previous state
	{
		data->SetInt( "prev", m_iPrevRoundState );
	}
	else
	{
		// don't send a delta if this is just to one player, they are joining mid-round
		data->SetInt( "prev", m_iCurrentRoundState );	
	}

	data->SetInt( "cur", m_iCurrentRoundState );

	// get bitmask representing the current miniround
	data->SetInt( "round", m_iCurrentMiniRoundMask );

	if ( pPlayer )
	{
		pPlayer->ShowViewPortPanel( PANEL_ROUNDINFO, true, data );
	}
	else
	{
		for ( int i = 1;  i <= MAX_PLAYERS; i++ )
		{
			CTFPlayer *pTFPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

			if ( pTFPlayer && pTFPlayer->IsReadyToPlay() )
			{
				pTFPlayer->ShowViewPortPanel( PANEL_ROUNDINFO, true, data );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::TimerMayExpire( void )
{
	// Prevent timers expiring while control points are contested
	int iNumControlPoints = ObjectiveResource()->GetNumControlPoints();
	for ( int iPoint = 0; iPoint < iNumControlPoints; iPoint ++ )
	{
		if ( ObjectiveResource()->GetCappingTeam(iPoint) )
			return false;
	}

	return BaseClass::TimerMayExpire();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::HandleCTFCaptureBonus( int iTeam )
{	
	float flBoostTime = tf_ctf_bonus_time.GetFloat();

	if ( m_flCTFBonusTime > -1 )
		flBoostTime = m_flCTFBonusTime;

	if ( flBoostTime > 0.0 )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( pTeam )
		{
			for ( int i = 1; i < pTeam->GetNumPlayers(); i++ )
			{
				CTFPlayer *pPlayer = ToTFPlayer( pTeam->GetPlayer( i ) );
				if ( pPlayer || pPlayer->IsAlive() )
					pPlayer->m_Shared.AddCond( TF_COND_CRITBOOSTED_CTF_CAPTURE, flBoostTime );
			}

			for ( int i = 1; i < pTeam->GetNumNPCs(); i++ )
			{
				CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pTeam->GetNPC( i ) );
				if ( pNPC || pNPC->IsAlive() )	
					pNPC->AddCond( TF_COND_CRITBOOSTED_CTF_CAPTURE, flBoostTime );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::Arena_ResetLosersScore( bool bUnknown )
{
	// TODO: Figure out what bUnknown does
	if ( bUnknown )
	{
		for ( int i = 0; i < GetNumberOfTeams(); i++ )
		{
			if ( i != GetWinningTeam() )
				GetWinningTeam();
			
			CTeam *pTeam = GetGlobalTeam( i );

			if ( pTeam )
				pTeam->ResetScores();
		}
	}
	else
	{
		for ( int i = 0; i < GetNumberOfTeams(); i++ )
		{
			if ( i != GetWinningTeam() && GetWinningTeam() > 1)
			{
				CTeam *pTeam = GetGlobalTeam( i );

				if (pTeam)
					pTeam->ResetScores();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::Arena_NotifyTeamSizeChange( void )
{
	CTeam *pTeam = GetGlobalTFTeam( TF_TEAM_BLUE );
	int iTeamCount = pTeam->GetNumPlayers();
	if ( iTeamCount != m_iArenaTeamCount )
	{
		if ( m_iArenaTeamCount )
		{
			if ( iTeamCount >= m_iArenaTeamCount )
			{
				UTIL_ClientPrintAll( HUD_PRINTTALK, "#TF_Arena_TeamSizeIncreased", UTIL_VarArgs( "%d", iTeamCount ) );
			}
			else
			{
				UTIL_ClientPrintAll( HUD_PRINTTALK, "#TF_Arena_TeamSizeDecreased", UTIL_VarArgs( "%d", iTeamCount ) );
			}
		}
		m_iArenaTeamCount = iTeamCount;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::Arena_PlayersNeededForMatch( void )
{
	// stub
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::Arena_RunTeamLogic( void )
{
	if ( !TFGameRules()->IsInArenaMode() )
		return;

	if ( tf_arena_max_streak.GetInt() > 0 )
	{
		bool bStreakReached = false;
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; iTeam++ )
		{
			if (TFTeamMgr()->GetTeam(iTeam))
			{
				bStreakReached = TFTeamMgr()->GetTeam(iTeam)->GetScore() >= tf_arena_max_streak.GetInt();
				if ( bStreakReached )
				{
					IGameEvent *event = gameeventmanager->CreateEvent("arena_match_maxstreak");
					if (event)
					{
						event->SetInt( "team", iTeam );
						event->SetInt( "streak", tf_arena_max_streak.GetInt() );
						gameeventmanager->FireEvent(event);
					}
				}
			}
		}

		if ( bStreakReached )
		{
			for ( int i = FIRST_GAME_TEAM; i < GetNumberOfTeams(); i++ )
			{
				BroadcastSound( i, "Announcer.AM_TeamScrambleRandom" );
			}
		}
	}

	Arena_NotifyTeamSizeChange();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::RoundRespawn( void )
{
	// remove any buildings, grenades, rockets, etc. the player put into the world
	for ( int i = 1;  i <= MAX_PLAYERS; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer )
		{
			pPlayer->RemoveAllOwnedEntitiesFromWorld();
			pPlayer->m_Shared.SetKillstreak(0);
		}
	}

	if ( !IsInTournamentMode() )
		Arena_RunTeamLogic();

	// reset the flag captures
	int nTeamCount = TFTeamMgr()->GetTeamCount();
	for ( int iTeam = FIRST_GAME_TEAM; iTeam < nTeamCount; ++iTeam )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( !pTeam )
			continue;

		pTeam->SetFlagCaptures( 0 );
	}

	CTF_GameStats.ResetRoundStats();

	BaseClass::RoundRespawn();

	// ** AFTER WE'VE BEEN THROUGH THE ROUND RESPAWN, SHOW THE ROUNDINFO PANEL
	if ( !IsInWaitingForPlayers() )
	{
		ShowRoundInfoPanel();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::InternalHandleTeamWin( int iWinningTeam )
{
	// remove any spies' disguises and make them visible (for the losing team only)
	// and set the speed for both teams (winners get a boost and losers have reduced speed)
	for ( int i = 1;  i <= MAX_PLAYERS; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer )
		{
			if ( pPlayer->GetTeamNumber() > LAST_SHARED_TEAM )
			{
				if ( pPlayer->GetTeamNumber() != iWinningTeam )
				{
					pPlayer->RemoveInvisibility();
//					pPlayer->RemoveDisguise();
					pPlayer->ClearExpression();

					if ( pPlayer->HasTheFlag() )
					{
						pPlayer->DropFlag();
					}

					// Hide their weapon.
					CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
					if ( pWeapon )
					{
						pWeapon->SetWeaponVisible( false );
					}
				}
				else if ( pPlayer->IsAlive() )
				{
					pPlayer->m_Shared.AddCond( TF_COND_CRITBOOSTED_BONUS_TIME );
				}

				pPlayer->TeamFortress_SetSpeed();
			}
		}
	}

	// disable any sentry guns the losing team has built
	CBaseEntity *pEnt = NULL;
	while ( ( pEnt = gEntList.FindEntityByClassname( pEnt, "obj_sentrygun" ) ) != NULL )
	{
		CObjectSentrygun *pSentry = dynamic_cast<CObjectSentrygun *>( pEnt );
		if ( pSentry )
		{
			if ( pSentry->GetTeamNumber() != iWinningTeam )
			{
				pSentry->SetDisabled( true );
			}
		}
	}

	if ( m_bForceMapReset )
	{
		m_iPrevRoundState = -1;
		m_iCurrentRoundState = -1;
		m_iCurrentMiniRoundMask = 0;
		m_bFirstBlood = false;
	}
}

// sort function for the list of players that we're going to use to scramble the teams
int ScramblePlayersSort( CTFPlayer* const *p1, CTFPlayer* const *p2 )
{
	CTFPlayerResource *pResource = GetTFPlayerResource();

	if ( pResource )
	{
		// check the priority
		if ( pResource->GetTotalScore( (*p2)->entindex() ) > pResource->GetTotalScore( (*p1)->entindex() ) )
		{
			return 1;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::HandleScrambleTeams( void )
{
	int i = 0;
	CTFPlayer *pTFPlayer = NULL;
	CUtlVector<CTFPlayer *> pListPlayers;

	// add all the players (that are on blue or red) to our temp list
	for ( i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		pTFPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pTFPlayer && ( pTFPlayer->GetTeamNumber() >= TF_TEAM_RED ) )
		{
			pListPlayers.AddToHead( pTFPlayer );
		}
	}

	// sort the list
	pListPlayers.Sort( ScramblePlayersSort );

	// loop through and put everyone on Spectator to clear the teams (or the autoteam step won't work correctly)
	for ( i = 0 ; i < pListPlayers.Count() ; i++ )
	{
		pTFPlayer = pListPlayers[i];

		if ( pTFPlayer )
		{
			pTFPlayer->ForceChangeTeam( TEAM_SPECTATOR );
		}
	}

	// loop through and auto team everyone
	for ( i = 0 ; i < pListPlayers.Count() ; i++ )
	{
		pTFPlayer = pListPlayers[i];

		if ( pTFPlayer )
		{
			pTFPlayer->ForceChangeTeam( TF_TEAM_AUTOASSIGN );
		}
	}
	ResetTeamsRoundWinTracking();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::HandleSwitchTeams( void )
{
	int i = 0;

	// respawn the players
	for ( i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			pPlayer->RemoveAllOwnedEntitiesFromWorld();

			// Ignore players who aren't on an active team
			if ( pPlayer->GetTeamNumber() != TF_TEAM_RED && pPlayer->GetTeamNumber() != TF_TEAM_BLUE )
			{
				continue;
			}

			if ( pPlayer->GetTeamNumber() == TF_TEAM_RED )
			{
				pPlayer->ForceChangeTeam( TF_TEAM_BLUE );
			}
			else if ( pPlayer->GetTeamNumber() == TF_TEAM_BLUE )
			{
				pPlayer->ForceChangeTeam( TF_TEAM_RED );
			}
		}
	}

	// switch the team scores
	CTFTeam *pRedTeam = GetGlobalTFTeam( TF_TEAM_RED );
	CTFTeam *pBlueTeam = GetGlobalTFTeam( TF_TEAM_BLUE );
	if ( pRedTeam && pBlueTeam )
	{
		int nRed = pRedTeam->GetScore();
		int nBlue = pBlueTeam->GetScore();

		pRedTeam->SetScore( nBlue );
		pBlueTeam->SetScore( nRed );
	}
}

bool CTFGameRules::CanChangeClassInStalemate( void ) 
{ 
	return (gpGlobals->curtime < (m_flStalemateStartTime + tf_stalematechangeclasstime.GetFloat())); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetRoundOverlayDetails( void )
{
	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;

	if ( pMaster && pMaster->PlayingMiniRounds() )
	{
		CTeamControlPointRound *pRound = pMaster->GetCurrentRound();

		if ( pRound )
		{
			CHandle<CTeamControlPoint> pRedPoint = pRound->GetPointOwnedBy( TF_TEAM_RED );
			CHandle<CTeamControlPoint> pBluePoint = pRound->GetPointOwnedBy( TF_TEAM_BLUE );

			// do we have opposing points in this round?
			if ( pRedPoint && pBluePoint )
			{
				int iMiniRoundMask = ( 1<<pBluePoint->GetPointIndex() ) | ( 1<<pRedPoint->GetPointIndex() );
				SetMiniRoundBitMask( iMiniRoundMask );
			}
			else
			{
				SetMiniRoundBitMask( 0 );
			}

			SetCurrentRoundStateBitString();
		}
	}

	BaseClass::SetRoundOverlayDetails();
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether a team should score for each captured point
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldScorePerRound( void )
{ 
	bool bRetVal = true;

	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
	if ( pMaster && pMaster->ShouldScorePerCapture() )
	{
		bRetVal = false;
	}

	return bRetVal;
}

#endif  // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::GetFarthestOwnedControlPoint( int iTeam, bool bWithSpawnpoints )
{
	int iOwnedEnd = ObjectiveResource()->GetBaseControlPointForTeam( iTeam );
	if ( iOwnedEnd == -1 )
		return -1;

	int iNumControlPoints = ObjectiveResource()->GetNumControlPoints();
	int iWalk = 1;
	int iEnemyEnd = iNumControlPoints-1;
	if ( iOwnedEnd != 0 )
	{
		iWalk = -1;
		iEnemyEnd = 0;
	}

	// Walk towards the other side, and find the farthest owned point that has spawn points
	int iFarthestPoint = iOwnedEnd;
	for ( int iPoint = iOwnedEnd; iPoint != iEnemyEnd; iPoint += iWalk )
	{
		// If we've hit a point we don't own, we're done
		if ( ObjectiveResource()->GetOwningTeam( iPoint ) != iTeam )
			break;

		if ( bWithSpawnpoints && !m_bControlSpawnsPerTeam[iTeam][iPoint] )
			continue;

		iFarthestPoint = iPoint;
	}

	return iFarthestPoint;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::TeamMayCapturePoint( int iTeam, int iPointIndex ) 
{ 
	// Is point capturing allowed at all?
	if ( !PointsMayBeCaptured() )
		return false;

	// If the point is explicitly locked it can't be capped.
	if ( ObjectiveResource()->GetCPLocked( iPointIndex ) )
		return false;

	if ( !tf_caplinear.GetBool() )
		return true;

	// Any previous points necessary?
	int iPointNeeded = ObjectiveResource()->GetPreviousPointForPoint( iPointIndex, iTeam, 0 );

	// Points set to require themselves are always cappable 
	if ( iPointNeeded == iPointIndex )
		return true;

	// No required points specified? Require all previous points.
	if ( iPointNeeded == -1 )
	{
		if ( !ObjectiveResource()->PlayingMiniRounds() )
		{
			// No custom previous point, team must own all previous points
			int iFarthestPoint = GetFarthestOwnedControlPoint( iTeam, false );
			return (abs(iFarthestPoint - iPointIndex) <= 1);
		}
		else
		{
			// No custom previous point, team must own all previous points in the current mini-round
			//tagES TFTODO: need to figure out a good algorithm for this
			return true;
		}
	}

	// Loop through each previous point and see if the team owns it
	for ( int iPrevPoint = 0; iPrevPoint < MAX_PREVIOUS_POINTS; iPrevPoint++ )
	{
		int iPointNeeded = ObjectiveResource()->GetPreviousPointForPoint( iPointIndex, iTeam, iPrevPoint );
		if ( iPointNeeded != -1 )
		{
			if ( ObjectiveResource()->GetOwningTeam( iPointNeeded ) != iTeam )
				return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::PlayerMayCapturePoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason /* = NULL */, int iMaxReasonLength /* = 0 */ )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( !pTFPlayer )
		return false;
	// Disguised and invisible spies cannot capture points
	if ( pTFPlayer->m_Shared.IsStealthed() )
	{
		if ( pszReason )
		{
			Q_snprintf( pszReason, iMaxReasonLength, "#Cant_cap_stealthed" );
		}
		return false;
	}

	if ( pTFPlayer->m_Shared.IsInvulnerable() || pTFPlayer->m_Shared.InCond( TF_COND_PHASE ) || pTFPlayer->m_Shared.InCond( TF_COND_MEGAHEAL ) )
	{
		if ( pszReason )
		{
			Q_snprintf( pszReason, iMaxReasonLength, "#Cant_cap_invuln" );
		}
		return false;
	}

	if ( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && pTFPlayer->m_Shared.GetDisguiseTeam() != pTFPlayer->GetTeamNumber() )
	{
		if ( pszReason )
		{
			Q_snprintf( pszReason, iMaxReasonLength, "#Cant_cap_disguised" );
		}
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::PlayerMayBlockPoint( CBasePlayer *pPlayer, int iPointIndex, char *pszReason, int iMaxReasonLength )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( !pTFPlayer )
		return false;

	// Invuln players can block points
	if ( pTFPlayer->m_Shared.IsInvulnerable() || pTFPlayer->m_Shared.InCond( TF_COND_PHASE ) || pTFPlayer->m_Shared.InCond( TF_COND_MEGAHEAL ) )
	{
		if ( pszReason )
		{
			Q_snprintf( pszReason, iMaxReasonLength, "#Cant_cap_invuln" );
		}
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Calculates score for player
//-----------------------------------------------------------------------------
int CTFGameRules::CalcPlayerScore( RoundStats_t *pRoundStats )
{
	int iScore = (pRoundStats->m_iStat[TFSTAT_KILLS] * TF_SCORE_KILL) +
		(pRoundStats->m_iStat[TFSTAT_CAPTURES] * TF_SCORE_CAPTURE) +
		(pRoundStats->m_iStat[TFSTAT_DEFENSES] * TF_SCORE_DEFEND) +
		(pRoundStats->m_iStat[TFSTAT_BUILDINGSDESTROYED] * TF_SCORE_DESTROY_BUILDING) +
		(pRoundStats->m_iStat[TFSTAT_HEADSHOTS] * TF_SCORE_HEADSHOT) +
		(pRoundStats->m_iStat[TFSTAT_BACKSTABS] * TF_SCORE_BACKSTAB) +
		(pRoundStats->m_iStat[TFSTAT_HEALING] / TF_SCORE_HEAL_HEALTHUNITS_PER_POINT) +
		(pRoundStats->m_iStat[TFSTAT_KILLASSISTS] / TF_SCORE_KILL_ASSISTS_PER_POINT) +
		(pRoundStats->m_iStat[TFSTAT_TELEPORTS] / TF_SCORE_TELEPORTS_PER_POINT) +
		(pRoundStats->m_iStat[TFSTAT_INVULNS] / TF_SCORE_INVULN) +
		(pRoundStats->m_iStat[TFSTAT_REVENGE] / TF_SCORE_REVENGE) +
		(pRoundStats->m_iStat[TFSTAT_BONUS_POINTS] / TF_SCORE_BONUS_PER_POINT);
	return max(iScore, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFGameRules::IsHolidayActive( int nHoliday ) const
{
	if ( m_nMapHolidayType == nHoliday )
		return true;

	return UTIL_IsHolidayActive( nHoliday );
}

//-----------------------------------------------------------------------------
// Purpose: TF's Birthday
//-----------------------------------------------------------------------------
bool CTFGameRules::IsBirthday( void )
{
	return IsHolidayActive( kHoliday_TF2Birthday );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: TF's Birthday or in Pyrovision
//-----------------------------------------------------------------------------
bool CTFGameRules::IsBirthdayOrPyroVision( void )
{
	return ( IsBirthday() || IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: LF's Birthday
//-----------------------------------------------------------------------------
bool CTFGameRules::IsLFBirthday( void )
{
	return IsHolidayActive( kHoliday_LFBirthday );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsTFCAllowed( void )
{
	if ( lfe_tfc_content_override.GetBool() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsGrenadesAllowed( void )
{
	return IsTFCAllowed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsMvMModelsAllowed( void )
{
	return ( lfe_player_bots.GetBool() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsMapAddAllowed( void )
{
	return lfe_mapadd_enable.GetBool();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsUsingSpells( void ) const
{
	return ( tf_spells_enabled.GetBool() || IsHolidayActive( kHoliday_Halloween ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsUsingGrapplingHook( void ) const
{
	return ( tf_grapplinghook_enable.GetBool() || m_bPowerupMode );
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGameRules::SetIT( CBaseEntity *pEnt )
{
	CBaseCombatCharacter *pVictim = ToBaseCombatCharacter( pEnt );
	if ( pVictim )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pEnt );
		if ( pPlayer )
		{
			if ( !m_itHandle.Get() || pPlayer != m_itHandle.Get() )
			{
				ClientPrint( pPlayer, HUD_PRINTTALK, "#TF_HALLOWEEN_BOSS_WARN_VICTIM", pPlayer->GetPlayerName() );
				ClientPrint( pPlayer, HUD_PRINTCENTER, "#TF_HALLOWEEN_BOSS_WARN_VICTIM", pPlayer->GetPlayerName() );

				CSingleUserRecipientFilter filter( pPlayer );
				CBaseEntity::EmitSound( filter, pEnt->entindex(), "Player.YouAreIT" );
			}
		}

		pVictim->EmitSound( "Halloween.PlayerScream" );
	}

	CBasePlayer *pIT = ToBasePlayer( m_itHandle.Get() );
	if ( pIT && pEnt != pIT )
	{
		if ( pIT->IsAlive() )
		{
			CSingleUserRecipientFilter filter( pIT );
			CBaseEntity::EmitSound( filter, pIT->entindex(), "Player.TaggedOtherIT" );
			ClientPrint( pIT, HUD_PRINTTALK, "#TF_HALLOWEEN_BOSS_LOST_AGGRO" );
			ClientPrint( pIT, HUD_PRINTCENTER, "#TF_HALLOWEEN_BOSS_LOST_AGGRO" );
		}
	}

	m_itHandle = pEnt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::AllowThirdPersonCamera( void )
{
#ifdef CLIENT_DLL
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer )
	{
		if ( pPlayer->IsObserver() )
			return false;

		if ( pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
			return false;
	}
#endif

	return lfe_allow_thirdperson.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		V_swap( collisionGroup0, collisionGroup1 );
	}
	
	//Don't stand on COLLISION_GROUP_WEAPONs
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	// Don't stand on projectiles
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_PROJECTILE )
	{
		return false;
	}

	// Rockets need to collide with players when they hit, but
	// be ignored by player movement checks
	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC )  && 
		( collisionGroup1 == TFCOLLISION_GROUP_ROCKETS ) )
		return true;

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) && 
		( collisionGroup1 == TFCOLLISION_GROUP_ROCKETS ) )
		return false;

	// Rockets doesn't collide with weapons
	if ( ( collisionGroup0 == COLLISION_GROUP_WEAPON ) && 
		( collisionGroup1 == TFCOLLISION_GROUP_ROCKETS ) )
		return false;

	// or grenades
	if ( ( collisionGroup0 == TF_COLLISIONGROUP_GRENADES ) && 
		( collisionGroup1 == TFCOLLISION_GROUP_ROCKETS ) )
		return false;

	// Grenades don't collide with players. They handle collision while flying around manually.
	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC ) && 
		( collisionGroup1 == TF_COLLISIONGROUP_GRENADES ) )
		return false;

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) && 
		( collisionGroup1 == TF_COLLISIONGROUP_GRENADES ) )
		return false;

	// Respawn rooms only collide with players
	if ( collisionGroup1 == TFCOLLISION_GROUP_RESPAWNROOMS )
		return ( collisionGroup0 == COLLISION_GROUP_PLAYER ) || ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) || ( collisionGroup0 == COLLISION_GROUP_NPC ) ;

	// Arrows only collide with players
	if ( collisionGroup1 == TFCOLLISION_GROUP_ARROWS )
		return ( collisionGroup0 == COLLISION_GROUP_PLAYER ) || ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) || ( collisionGroup0 == COLLISION_GROUP_NPC );

	// don't want caltrops and other grenades colliding with each other
	// caltops getting stuck on other caltrops, etc.)
	if ( ( collisionGroup0 == TF_COLLISIONGROUP_GRENADES ) && 
		 ( collisionGroup1 == TF_COLLISIONGROUP_GRENADES ) )
	{
		return false;
	}

	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == TFCOLLISION_GROUP_COMBATOBJECT )
	{
		return false;
	}

	// Owners don't collide with their own building
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER &&
		collisionGroup1 == TFCOLLISION_GROUP_COMBATOBJECT )
	{
		return false;
	}

	if ( collisionGroup0 == COLLISION_GROUP_NPC &&
		collisionGroup1 == TFCOLLISION_GROUP_COMBATOBJECT )
	{
		return false;
	}

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) &&
		( collisionGroup1 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) )
		return false;

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC ) &&
		( collisionGroup1 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) )
		return true;

	if ( ( collisionGroup0 == TF_COLLISIONGROUP_GRENADES ) && 
		 ( collisionGroup1 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) )
		 return false;

	if ( ( collisionGroup1 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) && 
		 ( collisionGroup0 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) || ( collisionGroup0 == TFCOLLISION_GROUP_ROCKETS ) )
		 return false;

	if ( ( collisionGroup1 == TFCOLLISION_GROUP_PUMPKIN_BOMB ) && 
		 ( collisionGroup0 == COLLISION_GROUP_WEAPON ) || ( collisionGroup0 == COLLISION_GROUP_PROJECTILE ) )
		 return false;

	// This is only for the super physcannon
	if ( m_bMegaPhysgun )
	{
		if ( collisionGroup0 == COLLISION_GROUP_INTERACTIVE_DEBRIS && collisionGroup1 == COLLISION_GROUP_PLAYER )
			return false;
	}

	// Prevent the player movement from colliding with spit globs (caused the player to jump on top of globs while in water)
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;

	//If collisionGroup0 is not a player then NPC_ACTOR behaves just like an NPC.
	if ( collisionGroup1 == COLLISION_GROUP_NPC_ACTOR && collisionGroup0 != COLLISION_GROUP_PLAYER )
	{
		collisionGroup1 = COLLISION_GROUP_NPC;
	}

	// Combine ball don't collide with other combine ball
	if ( collisionGroup0 == HL2COLLISION_GROUP_COMBINE_BALL )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return false;
	}

	if ( collisionGroup0 == HL2COLLISION_GROUP_COMBINE_BALL && collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL_NPC )
		return false;

	if ( ( collisionGroup0 == COLLISION_GROUP_WEAPON ) ||
		( collisionGroup0 == COLLISION_GROUP_PLAYER ) ||
		( collisionGroup0 == COLLISION_GROUP_PROJECTILE ) )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return false;
	}

	if ( collisionGroup0 == COLLISION_GROUP_DEBRIS )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return true;
	}

	// Do we really need this?
	if ( collisionGroup0 == HL2COLLISION_GROUP_HOUNDEYE && collisionGroup1 == HL2COLLISION_GROUP_HOUNDEYE )
		return false;

	if ( collisionGroup0 == HL2COLLISION_GROUP_HOMING_MISSILE && collisionGroup1 == HL2COLLISION_GROUP_HOMING_MISSILE )
		return false;

	// Npcs and Players don't collide with birds
	if ( collisionGroup1 == HL2COLLISION_GROUP_CROW )
	{
		if ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup0 == COLLISION_GROUP_NPC || collisionGroup0 == HL2COLLISION_GROUP_CROW )
			return false;
	}

	// Npcs and Players don't collide with headcrab (not really)
	/*if ( collisionGroup1 == HL2COLLISION_GROUP_HEADCRAB )
	{
		if ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC || collisionGroup0 == HL2COLLISION_GROUP_HEADCRAB )
			return false;
	}*/

	// Strider don't collide with npc or other strider
	if ( collisionGroup1 == HL2COLLISION_GROUP_STRIDER )
	{
		if ( collisionGroup0 == COLLISION_GROUP_NPC || collisionGroup0 == HL2COLLISION_GROUP_STRIDER )
			return false;
	}

	// Gunship don't collide with npc or other Gunship
	if ( collisionGroup1 == HL2COLLISION_GROUP_GUNSHIP )
	{
		if ( collisionGroup0 == COLLISION_GROUP_NPC || collisionGroup0 == HL2COLLISION_GROUP_GUNSHIP )
			return false;
	}

	// weapons and NPCs don't collide
	if ( collisionGroup0 == COLLISION_GROUP_WEAPON && (collisionGroup1 >= HL2COLLISION_GROUP_FIRST_NPC && collisionGroup1 <= HL2COLLISION_GROUP_LAST_NPC ) )
		return false;

	//players don't collide against NPC Actors.
	//I could've done this up where I check if collisionGroup0 is NOT a player but I decided to just
	//do what the other checks are doing in this function for consistency sake.
	if ( collisionGroup1 == COLLISION_GROUP_NPC_ACTOR && collisionGroup0 == COLLISION_GROUP_PLAYER )
		return false;

	// In cases where NPCs are playing a script which causes them to interpenetrate while riding on another entity,
	// such as a train or elevator, you need to disable collisions between the actors so the mover can move them.
	if ( collisionGroup0 == COLLISION_GROUP_NPC_SCRIPTED && collisionGroup1 == COLLISION_GROUP_NPC_SCRIPTED )
		return false;

	// Spit doesn't touch other spit
	if ( collisionGroup0 == HL2COLLISION_GROUP_SPIT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

//-----------------------------------------------------------------------------
// Purpose: Return the value of this player towards capturing a point
//-----------------------------------------------------------------------------
int	CTFGameRules::GetCaptureValueForPlayer( CBasePlayer *pPlayer )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( !pTFPlayer )
		return BaseClass::GetCaptureValueForPlayer( pPlayer );

	int iCapRate = 1;
	if ( pTFPlayer->IsPlayerClass( TF_CLASS_SCOUT ) )
	{
		if ( mp_capstyle.GetInt() == 1 )
		{
			// Scouts count for 2 people in timebased capping.
			iCapRate = 2;
		}
		else
		{
			// Scouts can cap all points on their own.
			iCapRate = 10;
		}
	}

	CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFPlayer, iCapRate, add_player_capturevalue );

	return iCapRate;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::GetTimeLeft( void )
{
	float flTimeLimit = mp_timelimit.GetInt() * 60;

	Assert( flTimeLimit > 0 && "Should not call this function when !IsGameUnderTimeLimit" );

	float flMapChangeTime = m_flMapResetTime + flTimeLimit;

	return ( (int)(flMapChangeTime - gpGlobals->curtime) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::FireGameEvent( IGameEvent *event )
{
	const char *eventName = event->GetName();

	if ( !Q_strcmp( eventName, "teamplay_point_captured" ) )
	{
#ifdef GAME_DLL
		RecalculateControlPointState();

		// keep track of how many times each team caps
		int iTeam = event->GetInt( "team" );
		Assert( iTeam >= FIRST_GAME_TEAM && iTeam < TF_TEAM_COUNT );
		m_iNumCaps[iTeam]++;

		// award a capture to all capping players
		const char *cappers = event->GetString( "cappers" );

		Q_strncpy( m_szMostRecentCappers, cappers, ARRAYSIZE( m_szMostRecentCappers ) );	
		for ( int i =0; i < Q_strlen( cappers ); i++ )
		{
			int iPlayerIndex = (int) cappers[i];
			CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( iPlayerIndex ) );
			if ( pPlayer )
			{
				CTF_GameStats.Event_PlayerCapturedPoint( pPlayer );				
			}
		}
#endif
	}
	else if ( !Q_strcmp( eventName, "teamplay_capture_blocked" ) )
	{
#ifdef GAME_DLL
		int iPlayerIndex = event->GetInt( "blocker" );
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( iPlayerIndex ) );
		if ( pPlayer )
			CTF_GameStats.Event_PlayerDefendedPoint( pPlayer );
#endif
	}	
	else if ( !Q_strcmp( eventName, "teamplay_round_win" ) )
	{
#ifdef GAME_DLL
		int iWinningTeam = event->GetInt( "team" );
		bool bFullRound = event->GetBool( "full_round" );
		float flRoundTime = event->GetFloat( "round_time" );
		bool bWasSuddenDeath = event->GetBool( "was_sudden_death" );
		CTF_GameStats.Event_RoundEnd( iWinningTeam, bFullRound, flRoundTime, bWasSuddenDeath );
#endif
	}
	else if ( !Q_strcmp( eventName, "teamplay_flag_event" ) )
	{
#ifdef GAME_DLL
		// if this is a capture event, remember the player who made the capture		
		int iEventType = event->GetInt( "eventtype" );
		if ( TF_FLAGEVENT_CAPTURE == iEventType )
		{
			int iPlayerIndex = event->GetInt( "player" );
			m_szMostRecentCappers[0] = iPlayerIndex;
			m_szMostRecentCappers[1] = 0;
		}
#endif
	}
#ifdef CLIENT_DLL
	else if ( !Q_strcmp( eventName, "game_newmap" ) )
	{
		m_nMapHolidayType = kHoliday_None;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Init ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			1	

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef* GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;
		
		// Start at 1 here and skip the dummy ammo type to make CAmmoDef use the same indices
		// as our #defines.
		for ( int i=1; i < TF_AMMO_COUNT; i++ )
		{
			def.AddAmmoType( g_aAmmoNames[i], DMG_BULLET, TRACER_LINE, 0, 0, "ammo_max", 2400, 10, 14 );
			Assert( def.Index( g_aAmmoNames[i] ) == i );
		}

		// HL2 Ammo types
		def.AddAmmoType("AR2",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_ar2",			"sk_npc_dmg_ar2",			"sk_max_ar2",			BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("AlyxGun",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_alyxgun",		"sk_npc_dmg_alyxgun",		"sk_max_alyxgun",		BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_pistol",		"sk_npc_dmg_pistol",		"sk_max_pistol",		BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_smg1",			"sk_npc_dmg_smg1",			"sk_max_smg1",			BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_357",			"sk_npc_dmg_357",			"sk_max_357",			BULLET_IMPULSE(800, 5000), 0 );
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_crossbow",		"sk_npc_dmg_crossbow",		"sk_max_crossbow",		BULLET_IMPULSE(800, 8000), 0 );

		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			"sk_plr_dmg_buckshot",		"sk_npc_dmg_buckshot",		"sk_max_buckshot",		BULLET_IMPULSE(400, 1200), 0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_rpg_round",		"sk_npc_dmg_rpg_round",		"sk_max_rpg_round",		0, 0 );
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_smg1_grenade",	"sk_npc_dmg_smg1_grenade",	"sk_max_smg1_grenade",	0, 0 );
		def.AddAmmoType("SniperRound",		DMG_BULLET | DMG_SNIPER,	TRACER_NONE,			"sk_plr_dmg_sniper_round",	"sk_npc_dmg_sniper_round",	"sk_max_sniper_round",	BULLET_IMPULSE(650, 6000), 0 );
		def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE,			"sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE(150, 6000), 0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_grenade",		0, 0);
		def.AddAmmoType("Thumper",			DMG_SONIC,					TRACER_NONE,			10, 10, 2, 0, 0 );
		def.AddAmmoType("Gravity",			DMG_CLUB,					TRACER_NONE,			0,	0, 8, 0, 0 );
//		def.AddAmmoType("Extinguisher",		DMG_BURN,					TRACER_NONE,			0,	0, 100, 0, 0 );
		def.AddAmmoType("Battery",			DMG_CLUB,					TRACER_NONE,			NULL, NULL, NULL, 0, 0 );
		def.AddAmmoType("GaussEnergy",		DMG_SHOCK,					TRACER_NONE,			"sk_jeep_gauss_damage",		"sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE(650, 8000), 0 ); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType("CombineCannon",	DMG_BULLET,					TRACER_LINE,			"sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5 * 750 * 12, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType("AirboatGun",		DMG_AIRBOAT,				TRACER_LINE,			"sk_plr_dmg_airboat",		"sk_npc_dmg_airboat",		NULL,					BULLET_IMPULSE(10, 600), 0 );

		// Half-Life 1
		def.AddAmmoType("9mmRound", DMG_BULLET | DMG_NEVERGIB, TRACER_LINE, "sk_plr_dmg_9mm_bullet", "sk_npc_dmg_9mm_bullet", "sk_max_9mm_bullet", BULLET_IMPULSE(500, 1325), 0);
		def.AddAmmoType("357Round", DMG_BULLET | DMG_NEVERGIB, TRACER_NONE, "sk_plr_dmg_hl1_357_bullet", NULL, "sk_max_hl1_357_bullet", BULLET_IMPULSE(650, 6000), 0);
		def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_LINE, "sk_plr_dmg_hl1_buckshot", NULL, "sk_max_hl1_buckshot", BULLET_IMPULSE(200, 1200), 0);
		def.AddAmmoType("XBowBolt", DMG_BULLET | DMG_NEVERGIB, TRACER_LINE, "sk_plr_dmg_xbow_hl1_bolt_plr", NULL, "sk_max_xbow_hl1_bolt", BULLET_IMPULSE(200, 1200), 0);
		def.AddAmmoType("MP5_Grenade", DMG_BURN | DMG_BLAST, TRACER_NONE, "sk_plr_dmg_mp5_grenade", NULL, "sk_max_mp5_grenade", 0, 0);
		def.AddAmmoType("RPG_Rocket", DMG_BURN | DMG_BLAST, TRACER_NONE, "sk_plr_dmg_hl1_rpg", NULL, "sk_max_hl1_rpg_rocket", 0, 0);
		def.AddAmmoType("Uranium", DMG_ENERGYBEAM, TRACER_NONE, NULL, NULL, "sk_max_uranium", 0, 0);
		def.AddAmmoType("Grenade", DMG_BURN | DMG_BLAST, TRACER_NONE, "sk_plr_dmg_hl1_grenade", NULL, "sk_max_hl1_grenade", 0, 0);
		def.AddAmmoType("Hornet", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_hornet", "sk_npc_dmg_hornet", "sk_max_hornet", BULLET_IMPULSE(100, 1200), 0);
		def.AddAmmoType("Snark", DMG_SLASH, TRACER_NONE, "sk_snark_dmg_bite", NULL, "sk_max_snark", 0, 0);
		def.AddAmmoType("TripMine", DMG_BURN | DMG_BLAST, TRACER_NONE, "sk_plr_dmg_tripmine", NULL, "sk_max_tripmine", 0, 0);
		def.AddAmmoType("Satchel", DMG_BURN | DMG_BLAST, TRACER_NONE, "sk_plr_dmg_satchel", NULL, "sk_max_satchel", 0, 0);

		def.AddAmmoType("12mmRound", DMG_BULLET | DMG_NEVERGIB, TRACER_LINE, NULL, "sk_npc_dmg_12mm_bullet", NULL, BULLET_IMPULSE(300, 1200), 0);

		def.AddAmmoType("Gravity", DMG_CRUSH, TRACER_NONE, 0, 0, 8, 0, 0);

		//=====================================================================
		// STRIDER MINIGUN DAMAGE - Pull up a chair and I'll tell you a tale.
		//
		// When we shipped Half-Life 2 in 2004, we were unaware of a bug in
		// CAmmoDef::NPCDamage() which was returning the MaxCarry field of
		// an ammotype as the amount of damage that should be done to a NPC
		// by that type of ammo. Thankfully, the bug only affected Ammo Types 
		// that DO NOT use ConVars to specify their parameters. As you can see,
		// all of the important ammotypes use ConVars, so the effect of the bug
		// was limited. The Strider Minigun was affected, though.
		//
		// According to my perforce Archeology, we intended to ship the Strider
		// Minigun ammo type to do 15 points of damage per shot, and we did. 
		// To achieve this we, unaware of the bug, set the Strider Minigun ammo 
		// type to have a maxcarry of 15, since our observation was that the 
		// number that was there before (8) was indeed the amount of damage being
		// done to NPC's at the time. So we changed the field that was incorrectly
		// being used as the NPC Damage field.
		//
		// The bug was fixed during Episode 1's development. The result of the 
		// bug fix was that the Strider was reduced to doing 5 points of damage
		// to NPC's, since 5 is the value that was being assigned as NPC damage
		// even though the code was returning 15 up to that point.
		//
		// Now as we go to ship Orange Box, we discover that the Striders in 
		// Half-Life 2 are hugely ineffective against citizens, causing big
		// problems in maps 12 and 13. 
		//
		// In order to restore balance to HL2 without upsetting the delicate 
		// balance of ep2_outland_12, I have chosen to build Episodic binaries
		// with 5 as the Strider->NPC damage, since that's the value that has
		// been in place for all of Episode 2's development. Half-Life 2 will
		// build with 15 as the Strider->NPC damage, which is how HL2 shipped
		// originally, only this time the 15 is located in the correct field
		// now that the AmmoDef code is behaving correctly.
		//
		//=====================================================================
		def.AddAmmoType("StriderMinigun",	DMG_BULLET,					TRACER_LINE,			5, 5, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("StriderMinigunDirect",	DMG_BULLET,				TRACER_LINE,			2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("HelicopterGun",	DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter",	"sk_max_smg1",	BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			0, 0, "sk_max_ar2_altfire", 0, 0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_grenade",		0, 0);
		def.AddAmmoType("Hopwire",			DMG_BLAST,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_hopwire",		0, 0);
		def.AddAmmoType("CombineHeavyCannon",	DMG_BULLET,				TRACER_LINE,			40,	40, NULL, 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType("ammo_proto1",			DMG_BULLET,				TRACER_LINE,			0, 0, 10, 0, 0 );
	}

	return &def;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFGameRules::GetTeamGoalString( int iTeam )
{
	if ( iTeam == TF_TEAM_RED )
		return m_pszTeamGoalStringRed.Get();
	if ( iTeam == TF_TEAM_BLUE )
		return m_pszTeamGoalStringBlue.Get();

	return NULL;
}

#ifdef GAME_DLL

	Vector MaybeDropToGround( 
							CBaseEntity *pMainEnt, 
							bool bDropToGround, 
							const Vector &vPos, 
							const Vector &vMins, 
							const Vector &vMaxs )
	{
		if ( bDropToGround )
		{
			trace_t trace;
			UTIL_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );
			return trace.endpos;
		}
		else
		{
			return vPos;
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: This function can be used to find a valid placement location for an entity.
	//			Given an origin to start looking from and a minimum radius to place the entity at,
	//			it will sweep out a circle around vOrigin and try to find a valid spot (on the ground)
	//			where mins and maxs will fit.
	// Input  : *pMainEnt - Entity to place
	//			&vOrigin - Point to search around
	//			fRadius - Radius to search within
	//			nTries - Number of tries to attempt
	//			&mins - mins of the Entity
	//			&maxs - maxs of the Entity
	//			&outPos - Return point
	// Output : Returns true and fills in outPos if it found a spot.
	//-----------------------------------------------------------------------------
	bool EntityPlacementTest( CBaseEntity *pMainEnt, const Vector &vOrigin, Vector &outPos, bool bDropToGround )
	{
		// This function moves the box out in each dimension in each step trying to find empty space like this:
		//
		//											  X  
		//							   X			  X  
		// Step 1:   X     Step 2:    XXX   Step 3: XXXXX
		//							   X 			  X  
		//											  X  
		//

		Vector mins, maxs;
		pMainEnt->CollisionProp()->WorldSpaceAABB( &mins, &maxs );
		mins -= pMainEnt->GetAbsOrigin();
		maxs -= pMainEnt->GetAbsOrigin();

		// Put some padding on their bbox.
		float flPadSize = 5;
		Vector vTestMins = mins - Vector( flPadSize, flPadSize, flPadSize );
		Vector vTestMaxs = maxs + Vector( flPadSize, flPadSize, flPadSize );

		// First test the starting origin.
		if ( UTIL_IsSpaceEmpty( pMainEnt, vOrigin + vTestMins, vOrigin + vTestMaxs ) )
		{
			outPos = MaybeDropToGround( pMainEnt, bDropToGround, vOrigin, vTestMins, vTestMaxs );
			return true;
		}

		Vector vDims = vTestMaxs - vTestMins;


		// Keep branching out until we get too far.
		int iCurIteration = 0;
		int nMaxIterations = 15;

		int offset = 0;
		do
		{
			for ( int iDim=0; iDim < 3; iDim++ )
			{
				float flCurOffset = offset * vDims[iDim];

				for ( int iSign=0; iSign < 2; iSign++ )
				{
					Vector vBase = vOrigin;
					vBase[iDim] += (iSign*2-1) * flCurOffset;

					if ( UTIL_IsSpaceEmpty( pMainEnt, vBase + vTestMins, vBase + vTestMaxs ) )
					{
						// Ensure that there is a clear line of sight from the spawnpoint entity to the actual spawn point.
						// (Useful for keeping things from spawning behind walls near a spawn point)
						trace_t tr;
						UTIL_TraceLine( vOrigin, vBase, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &tr );

						if ( tr.fraction != 1.0 )
						{
							continue;
						}

						outPos = MaybeDropToGround( pMainEnt, bDropToGround, vBase, vTestMins, vTestMaxs );
						return true;
					}
				}
			}

			++offset;
		} while ( iCurIteration++ < nMaxIterations );

		//	Warning( "EntityPlacementTest for ent %d:%s failed!\n", pMainEnt->entindex(), pMainEnt->GetClassname() );
		return false;
	}

#else // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( State_Get() == GR_STATE_STARTGAME )
	{
		m_nMapHolidayType = kHoliday_None;
	}
}

void CTFGameRules::HandleOvertimeBegin()
{
	C_TFPlayer *pTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pTFPlayer )
	{
		pTFPlayer->EmitSound( "Game.Overtime" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldShowTeamGoal( void )
{
	if ( State_Get() == GR_STATE_PREROUND || State_Get() == GR_STATE_RND_RUNNING || InSetup() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::GetTeamGlowColor( int nTeam, float &r, float &g, float &b )
{
	switch ( nTeam )
	{
		case TF_TEAM_BLUE:
			r = 0.49f; g = 0.66f; b = 0.7699971f;
			break;

		case TF_TEAM_RED:
			r = 0.74f; g = 0.23f; b = 0.23f;
			break;

		case TF_TEAM_GREEN:
			r = 0.03f; g = 0.68f; b = 0;
			break;

		case TF_TEAM_YELLOW:
			r = 1.0f; g = 0.62f; b = 0;
			break;

		default:
			r = 0.76f; g = 0.76f; b = 0.76f;
			break;
	}
}
#endif

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::ShutdownCustomResponseRulesDicts()
{
	DestroyCustomResponseSystems();

	if ( m_ResponseRules.Count() != 0 )
	{
		int nRuleCount = m_ResponseRules.Count();
		for ( int iRule = 0; iRule < nRuleCount; ++iRule )
		{
			m_ResponseRules[iRule].m_ResponseSystems.Purge();
		}
		m_ResponseRules.Purge();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::InitCustomResponseRulesDicts()
{
	MEM_ALLOC_CREDIT();

	// Clear if necessary.
	ShutdownCustomResponseRulesDicts();

	// Initialize the response rules for TF.
	m_ResponseRules.AddMultipleToTail( TF_CLASS_COUNT_ALL );

	char szName[512];
	for ( int iClass = TF_FIRST_NORMAL_CLASS; iClass < TF_CLASS_COUNT_ALL; ++iClass )
	{
		m_ResponseRules[iClass].m_ResponseSystems.AddMultipleToTail( MP_TF_CONCEPT_COUNT );

		for ( int iConcept = 0; iConcept < MP_TF_CONCEPT_COUNT; ++iConcept )
		{
			AI_CriteriaSet criteriaSet;
			criteriaSet.AppendCriteria( "playerclass", g_aPlayerClassNames_NonLocalized[iClass] );
			criteriaSet.AppendCriteria( "Concept", g_pszMPConcepts[iConcept] );

			// 1 point for player class and 1 point for concept.
			float flCriteriaScore = 2.0f;

			// Name.
			V_snprintf( szName, sizeof( szName ), "%s_%s\n", g_aPlayerClassNames_NonLocalized[iClass], g_pszMPConcepts[iConcept] );
			m_ResponseRules[iClass].m_ResponseSystems[iConcept] = BuildCustomResponseSystemGivenCriteria( "scripts/talker/response_rules.txt", szName, criteriaSet, flCriteriaScore );
		}		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SendHudNotification( IRecipientFilter &filter, HudNotification_t iType )
{
	UserMessageBegin( filter, "HudNotify" );
		WRITE_BYTE( iType );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SendHudNotification( IRecipientFilter &filter, const char *pszText, const char *pszIcon, int iTeam /*= TEAM_UNASSIGNED*/ )
{
	UserMessageBegin( filter, "HudNotifyCustom" );
		WRITE_STRING( pszText );
		WRITE_STRING( pszIcon );
		WRITE_BYTE( iTeam );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Is the player past the required delays for spawning
//-----------------------------------------------------------------------------
bool CTFGameRules::HasPassedMinRespawnTime( CBasePlayer *pPlayer )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );

	if ( pTFPlayer && pTFPlayer->GetPlayerClass()->GetClassIndex() == TF_CLASS_UNDEFINED )
		return true;

	float flMinSpawnTime = GetMinTimeWhenPlayerMaySpawn( pPlayer ); 

	return ( gpGlobals->curtime > flMinSpawnTime );
}

//-----------------------------------------------------------------------------
// Purpose: Sets the game description in the server browser
//-----------------------------------------------------------------------------
const char *CTFGameRules::GetGameDescription(void)
{
	switch (m_nGameType)
	{
		case TF_GAMETYPE_CTF:
			return "TF2 (CTF)";
			break;
		case TF_GAMETYPE_CP:
			if ( IsInKothMode() )
				return "TF2 (Koth)";

			return "TF2 (CP)";
			break;
		case TF_GAMETYPE_ESCORT:
			return "TF2 (Payload)";
			break;
		case TF_GAMETYPE_ARENA:
			return "TF2 (Arena)";
			break;
		case TF_GAMETYPE_COOP:
			return "LFE (Co-Op)";
			break;
		case TF_GAMETYPE_VS:
			return "LFE (Versus)";
			break;
		case TF_GAMETYPE_HORDE:
			return "LFE (Horde)";
			break;
		case TF_GAMETYPE_FREE:
			return "LFE (Free)";
			break;
		case TF_GAMETYPE_MVM:
			return "MvM?";
			break;
		default:
			return "LFE";
			break;
	}
}

#endif

float CTFGameRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = mp_timelimit.GetInt() - gpGlobals->curtime;

	return timeleft;
}


float CTFGameRules::GetRespawnWaveMaxLength( int iTeam, bool bScaleWithNumPlayers /* = true */ )
{
	return BaseClass::GetRespawnWaveMaxLength( iTeam, bScaleWithNumPlayers );
}

bool CTFGameRules::ShouldBalanceTeams( void )
{
	// No team balancing in coop since everybody should be on RED.
	if ( IsAnyCoOp() || IsHordeMode() || !IsVersus() )
		return false;

	return BaseClass::ShouldBalanceTeams();
}

bool CTFGameRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
#ifdef CLIENT_DLL
	pPlayer = C_BasePlayer::GetLocalPlayer();
#endif

	if ( pPlayer && !pPlayer->IsAlive() && pPlayer->GetTeamNumber() <= LAST_SHARED_TEAM )
	{
		return true;
	}

	return false;
}

#ifdef CLIENT_DLL
const char *CTFGameRules::GetVideoFileForMap( bool bWithExtension /*= true*/ )
{
	char mapname[MAX_MAP_NAME];

	Q_FileBase( engine->GetLevelName(), mapname, sizeof( mapname ) );
	Q_strlower( mapname );

	static char strFullpath[MAX_PATH];
	Q_strncpy( strFullpath, "media/", MAX_PATH );	// Assume we must play out of the media directory
	Q_strncat( strFullpath, mapname, MAX_PATH );

	if ( bWithExtension )
	{
		Q_strncat( strFullpath, ".bik", MAX_PATH );		// Assume we're a .bik extension type
	}

	return strFullpath;
}
#endif

#ifdef CLIENT_DLL
void AddSubKeyNamed( KeyValues *pKeys, const char *pszName )
{
	KeyValues *pKeyvalToAdd = new KeyValues( pszName );

	if ( pKeyvalToAdd )
		pKeys->AddSubKey( pKeyvalToAdd );
}
#else
//-----------------------------------------------------------------------------
// This takes the long way around to see if a prop should emit a DLIGHT when it
// ignites, to avoid having Alyx-related code in props.cpp.
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldBurningPropsEmitLight()
{
	if ( IsAlyxInDarknessMode() )
		return true;

	if ( ( FStrEq(STRING(gpGlobals->mapname), "ep1_c17_00" ) ) || ( FStrEq(STRING(gpGlobals->mapname), "ep1_c17_00a" ) ) )
		return true;


	return false;
}

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
void CTFGameRules::OnSkillLevelChanged( int iNewLevel )
{
	ConColorMsg( Color( 77, 116, 85, 255 ), "[TFGameRules] Difficulty level changed\n" );
	
	g_iSkillLevel = iNewLevel;
	
	if ( iNewLevel == SKILL_EASY )
	{
		ConColorMsg( Color( 178, 178, 178, 255 ), "[TFGameRules] Executing Original Difficulty config file\n" );
		engine->ServerCommand( "exec skill.cfg\n" );
		engine->ServerExecute();
		return;
	}

	if ( iNewLevel == SKILL_MEDIUM )
	{
		ConColorMsg( Color( 255, 215, 0, 255 ), "[TFGameRules] Executing Medium Difficulty config file\n" );
		engine->ServerCommand( "exec skill.cfg\n" );
		engine->ServerExecute();
		return;
	}

	if ( iNewLevel == SKILL_HARD )
	{
		ConColorMsg( Color( 207, 160, 50, 255 ), "[TFGameRules] Executing Hard Difficulty config file\n" );
		engine->ServerCommand( "exec skill.cfg\n" );
		engine->ServerExecute();
		return;
	}

	if ( iNewLevel == SKILL_UNUSUAL )
	{
		ConColorMsg( Color( 134, 80, 172, 255 ), "[TFGameRules] Executing Unusual Difficulty config file\n" );
		engine->ServerCommand( "exec skill.cfg\n" );
		engine->ServerExecute();
		return;
	}

	if ( iNewLevel == SKILL_HAUNTED )
	{
		ConColorMsg( Color( 134, 80, 172, 255 ), "[TFGameRules] Executing Haunted Difficulty config file\n" );
		engine->ServerCommand( "exec skill.cfg\n" );
		engine->ServerExecute();
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetSkillLevel( int iLevel )
{
	int oldLevel = m_iDifficultyLevel;

	if ( !( iLevel == SKILL_EASY || iLevel == SKILL_MEDIUM || iLevel == SKILL_HARD || iLevel == SKILL_UNUSUAL || iLevel == SKILL_HAUNTED ) )
	{
		sv_difficulty.SetValue( oldLevel );
		return;
	}

	m_iDifficultyLevel = iLevel;

	if ( m_iDifficultyLevel != oldLevel )
	{
		OnSkillLevelChanged( m_iDifficultyLevel );
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::SetDynamicNPC( bool bEnable )
{
	sv_dynamicnpcs.SetValue( bEnable );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::UpdateDirectorAnger( void )
{
#ifdef GAME_DLL
	if ( sv_dynamicnpcs_debug.GetBool() )
		ConColorMsg( Color( 77, 116, 85, 255 ), "[Dynamic NPC]: Anger Level %d\n", DirectorAnger() );
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: mtp.cfg parser
//-----------------------------------------------------------------------------
void CTFGameRules::SetUpVisionFilterKeyValues( void )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsBonusChallengeTimeBased( void )
{
	CBasePlayer* pPlayer = UTIL_PlayerByIndex( 1 );
	if ( !pPlayer )
		return true;

	int iBonusChallenge = pPlayer->GetBonusChallenge();
	if ( iBonusChallenge == PORTAL_CHALLENGE_TIME || iBonusChallenge == PORTAL_CHALLENGE_NONE )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: blocking mapper from using specific particle
//-----------------------------------------------------------------------------
bool CTFGameRules::AllowMapParticleEffect( const char *pszParticleEffect )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: delet rain
//-----------------------------------------------------------------------------
bool CTFGameRules::AllowWeatherParticles( void )
{
	return !tf_particles_disable_weather.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: are we allowed to filter maps?
//-----------------------------------------------------------------------------
bool CTFGameRules::AllowMapVisionFilterShaders( void )
{
	// in live tf2 valve use mtp.cfg to check maps.
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: translate!
//-----------------------------------------------------------------------------
const char *CTFGameRules::TranslateEffectForVisionFilter( const char *pchEffectType, const char *pchEffectName )
{
	// particles
	if ( !Q_strcmp( pchEffectType, "particles" ) )
	{
		if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
		{
			if ( !Q_strcmp( pchEffectName, "projectile_fireball" ) )
				return "projectile_fireball_pyrovision";

			if ( !Q_strcmp( pchEffectName, "flaregun_trail_red" ) )
				return "pyrovision_flaregun_trail_red";

			if ( !Q_strcmp( pchEffectName, "flaregun_trail_blue" ) )
				return "pyrovision_flaregun_trail_blue";

			if ( !Q_strcmp( pchEffectName, "flaregun_trail_crit_red" ) )
				return "pyrovision_flaregun_trail_crit_red";

			if ( !Q_strcmp( pchEffectName, "flaregun_trail_crit_blue" ) )
				return "pyrovision_flaregun_trail_crit_blue";

			if ( !Q_strcmp( pchEffectName, "v_flaming_arrow" ) )
				return "pyrovision_v_flaming_arrow";

			if ( !Q_strcmp( pchEffectName, "rockettrail" ) || !Q_strcmp( pchEffectName, "rockettrail_!" ) || !Q_strcmp( pchEffectName, "sentry_rocket" ) )
				return "pyrovision_rockettrail";

			if ( !Q_strcmp( pchEffectName, "flaming_arrow" ) )
				return "pyrovision_flaming_arrow";

			if ( !Q_strcmp( pchEffectName, "ExplosionCore_buildings" ) || !Q_strcmp( pchEffectName, "ExplosionCore_MidAir" ) || !Q_strcmp( pchEffectName, "ExplosionCore_sapperdestroyed" ) || !Q_strcmp( pchEffectName, "ExplosionCore_Wall" ) || !Q_strcmp( pchEffectName, "rd_robot_explosion" ) )
				return "pyrovision_explosion";

			if ( !Q_strcmp( pchEffectName, "flaregun_destroyed" ) || !Q_strcmp( pchEffectName, "ExplosionCore_MidAir_Flare" ) )
				return "pyrovision_flaregun_destroyed";

			if ( !Q_strcmp( pchEffectName, "scorchshot_trail_blue" ) )
				return "pyrovision_scorchshot_trail_blue";

			if ( !Q_strcmp( pchEffectName, "scorchshot_trail_red" ) )
				return "pyrovision_scorchshot_trail_red";

			if ( !Q_strcmp( pchEffectName, "scorchshot_trail_crit_blue" ) )
				return "pyrovision_scorchshot_trail_crit_blue";

			if ( !Q_strcmp( pchEffectName, "scorchshot_trail_crit_red" ) )
				return "pyrovision_scorchshot_trail_crit_red";

			if ( !Q_strcmp( pchEffectName, "pyrotaunt_rainbow_norainbow" ) )
				return "pyrotaunt_rainbow";

			if ( !Q_strcmp( pchEffectName, "pyrotaunt_rainbow_bubbles_flame" ) )
				return "pyrotaunt_rainbow_bubbles";

			if ( !Q_strcmp( pchEffectName, "taunt_pyro_balloon" ) )
				return "taunt_pyro_balloon_vision";

			if ( !Q_strcmp( pchEffectName, "taunt_pyro_balloon_explosion" ) )
				return "taunt_pyro_balloon_explosion_vision";

			if ( !Q_strcmp( pchEffectName, "taunt_pyro_gasblast_fireblast" ) )
				return "taunt_pyro_gasblast_rainbow";

			if ( !Q_strcmp( pchEffectName, "burningplayer_red" ) )
				return "burningplayer_rainbow_red";

			if ( !Q_strcmp( pchEffectName, "burningplayer_blue" ) )
				return "burningplayer_rainbow_blue";

			if ( !Q_strcmp( pchEffectName, "burningplayer_corpse" ) )
				return "burningplayer_corpse_rainbow";

			if ( !Q_strcmp( pchEffectName, "flamethrower" ) || !Q_strcmp( pchEffectName, "flamethrower_blue" ) )
				return "flamethrower_rainbow";

			if ( !Q_strcmp( pchEffectName, "new_flame" ) )
				return "flamethrower_rainbow_new_flame";

			if ( !Q_strcmp( pchEffectName, "burningplayer_flyingbits" ) )
				return "pyrovision_flaming_arrow";

			if ( !Q_strcmp( pchEffectName, "blood_impact_red_01" ) || !Q_strcmp( pchEffectName, "water_blood_impact_red_01" ) || !Q_strcmp( pchEffectName, "blood_impact_green_01" ) || !Q_strcmp( pchEffectName, "blood_impact_yellow_01" ) || !Q_strcmp( pchEffectName, "blood_impact_antlion_01" ) || !Q_strcmp( pchEffectName, "blood_impact_zombie_01" ) || !Q_strcmp( pchEffectName, "blood_impact_antlion_worker_01" ) || !Q_strcmp( pchEffectName, "blood_spray_red_01" ) || !Q_strcmp( pchEffectName, "blood_spray_red_01_far" ) )
				return "pyrovision_blood";
		}

		if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_HALLOWEEN ) )
		{
			if ( !Q_strcmp( pchEffectName, "blood_impact_red_01" ) || !Q_strcmp( pchEffectName, "water_blood_impact_red_01" ) || !Q_strcmp( pchEffectName, "blood_impact_green_01" ) || !Q_strcmp( pchEffectName, "blood_impact_yellow_01" ) || !Q_strcmp( pchEffectName, "blood_impact_antlion_01" ) || !Q_strcmp( pchEffectName, "blood_impact_zombie_01" ) || !Q_strcmp( pchEffectName, "blood_impact_antlion_worker_01" ) || !Q_strcmp( pchEffectName, "blood_spray_red_01" ) || !Q_strcmp( pchEffectName, "blood_spray_red_01_far" ) )
				return "env_sawblood";
		}
	}

	// weapon model
	/*if ( !Q_strcmp( pchEffectType, "weapons" ) )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		if ( pPlayer )
		{
			C_TFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
			if ( pWeapon )
			{
				if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
				{
					if ( !Q_strcmp( pchEffectName, pWeapon->GetItem()->GetWorldDisplayModel() ) )
						return pWeapon->GetItem()->GetVisionFilteredDisplayModel();
				}
			}
		}
	}*/

	// sounds
	if ( !Q_strcmp( pchEffectType, "sounds" ) )
	{
		if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
		{
			if ( !Q_strcmp( pchEffectName, "Taunt.Party_Trick" ) )
				return "Taunt.Party_Trick_Pyro_Vision";

			if ( !Q_strcmp( pchEffectName, "Taunt.GasBlast" ) )
				return "Taunt.GasBlastPyrovision";

			if ( !Q_strcmp( pchEffectName, "Weapon_FlameThrower.Fire" ) )
				return "Weapon_Rainblower.Fire";

			if ( !Q_strcmp( pchEffectName, "Weapon_FlameThrower.PilotLoop" ) )
				return "Weapon_Rainblower.PilotLoop";

			if ( !Q_strcmp( pchEffectName, "Weapon_FlameThrower.WindDown" ) )
				return "Weapon_Rainblower.WindDown";

			if ( !Q_strcmp( pchEffectName, "Weapon_FlameThrower.FireLoopCrit" ) )
				return "Weapon_Rainblower.FireLoopCrit";

			if ( !Q_strcmp( pchEffectName, "Taunt.PyroHellicorn" ) )
				return "Taunt.PyroBalloonicorn";

			// (chuckles) i'm in danger
			if ( !Q_strcmp( pchEffectName, "Demoman.Death" ) || !Q_strcmp( pchEffectName, "Demoman.PainSevere01" ) || !Q_strcmp( pchEffectName, "Demoman.PainSevere02" ) || !Q_strcmp( pchEffectName, "Demoman.PainSevere03" ) || !Q_strcmp( pchEffectName, "Demoman.PainSevere04" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp01" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp02" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp03" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp04" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp05" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp06" ) || !Q_strcmp( pchEffectName, "Demoman.PainSharp07" ) )
				return "Demoman.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Engineer.Death" ) || !Q_strcmp( pchEffectName, "Engineer.PainSevere01" ) || !Q_strcmp( pchEffectName, "Engineer.PainSevere02" ) || !Q_strcmp( pchEffectName, "Engineer.PainSevere03" ) || !Q_strcmp( pchEffectName, "Engineer.PainSevere04" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp01" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp02" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp03" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp04" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp05" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp06" ) || !Q_strcmp( pchEffectName, "Engineer.PainSharp07" ) )
				return "Engineer.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Heavy.Death" ) || !Q_strcmp( pchEffectName, "Heavy.PainSevere01" ) || !Q_strcmp( pchEffectName, "Heavy.PainSevere02" ) || !Q_strcmp( pchEffectName, "Heavy.PainSevere03" ) || !Q_strcmp( pchEffectName, "Heavy.PainSevere04" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp01" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp02" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp03" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp04" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp05" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp06" ) || !Q_strcmp( pchEffectName, "Heavy.PainSharp07" ) )
				return "Heavy.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Medic.Death" ) || !Q_strcmp( pchEffectName, "Medic.PainSevere01" ) || !Q_strcmp( pchEffectName, "Medic.PainSevere02" ) || !Q_strcmp( pchEffectName, "Medic.PainSevere03" ) || !Q_strcmp( pchEffectName, "Medic.PainSevere04" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp01" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp02" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp03" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp04" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp05" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp06" ) || !Q_strcmp( pchEffectName, "Medic.PainSharp07" ) )
				return "Medic.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Pyro.Death" ) || !Q_strcmp( pchEffectName, "Pyro.PainSevere01" ) || !Q_strcmp( pchEffectName, "Pyro.PainSevere02" ) || !Q_strcmp( pchEffectName, "Pyro.PainSevere03" ) || !Q_strcmp( pchEffectName, "Pyro.PainSevere04" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp01" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp02" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp03" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp04" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp05" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp06" ) || !Q_strcmp( pchEffectName, "Pyro.PainSharp07" ) )
				return "Pyro.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Scout.Death" ) || !Q_strcmp( pchEffectName, "Scout.PainSevere01" ) || !Q_strcmp( pchEffectName, "Scout.PainSevere02" ) || !Q_strcmp( pchEffectName, "Scout.PainSevere03" ) || !Q_strcmp( pchEffectName, "Scout.PainSevere04" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp01" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp02" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp03" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp04" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp05" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp06" ) || !Q_strcmp( pchEffectName, "Scout.PainSharp07" ) )
				return "Scout.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Sniper.Death" ) || !Q_strcmp( pchEffectName, "Sniper.PainSevere01" ) || !Q_strcmp( pchEffectName, "Sniper.PainSevere02" ) || !Q_strcmp( pchEffectName, "Sniper.PainSevere03" ) || !Q_strcmp( pchEffectName, "Sniper.PainSevere04" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp01" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp02" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp03" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp04" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp05" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp06" ) || !Q_strcmp( pchEffectName, "Sniper.PainSharp07" ) )
				return "Sniper.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Soldier.Death" ) || !Q_strcmp( pchEffectName, "Soldier.PainSevere01" ) || !Q_strcmp( pchEffectName, "Soldier.PainSevere02" ) || !Q_strcmp( pchEffectName, "Soldier.PainSevere03" ) || !Q_strcmp( pchEffectName, "Soldier.PainSevere04" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp01" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp02" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp03" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp04" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp05" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp06" ) || !Q_strcmp( pchEffectName, "Soldier.PainSharp07" ) )
				return "Soldier.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "Spy.Death" ) || !Q_strcmp( pchEffectName, "Spy.PainSevere01" ) || !Q_strcmp( pchEffectName, "Spy.PainSevere02" ) || !Q_strcmp( pchEffectName, "Spy.PainSevere03" ) || !Q_strcmp( pchEffectName, "Spy.PainSevere04" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp01" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp02" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp03" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp04" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp05" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp06" ) || !Q_strcmp( pchEffectName, "Spy.PainSharp07" ) )
				return "Spy.LaughShort01";

			if ( !Q_strcmp( pchEffectName, "NPC_MetroPolice.Die" ) || !Q_strcmp( pchEffectName, "NPC_Sniper.Die" ) )
				return "NPC_MetroPolice.Cupcop.Chuckle";

			if ( !Q_strcmp( pchEffectName, "NPC_MetroPolice.Pain" ) || !Q_strcmp( pchEffectName, "NPC_MetroPolice.OnFireScream" ) )
				return "NPC_MetroPolice.Cupcop.Chuckle";

			if ( !Q_strcmp( pchEffectName, "npc_citizen.pain01" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain02" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain03" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain04" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain05" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain06" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain07" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain08" ) || !Q_strcmp( pchEffectName, "npc_citizen.pain09" ) || !Q_strcmp( pchEffectName, "NPC_Citizen.Die" ) )
				return "odessa.nlo_cheer03";

			if ( !Q_strcmp( pchEffectName, "npc_alyx.die" ) || !Q_strcmp( pchEffectName, "npc_alyx.hurt04" ) || !Q_strcmp( pchEffectName, "npc_alyx.hurt05" ) || !Q_strcmp( pchEffectName, "npc_alyx.hurt06" ) || !Q_strcmp( pchEffectName, "npc_alyx.hurt08" ) )
				return "npc_alyx.gasp03";

			/*if ( !Q_strcmp( pchEffectName, "BaseExplosionEffect.Sound" ) )
				return "Game.HappyBirthdayNoiseMaker";*/
		}
	}

	return pchEffectName;
}

//-----------------------------------------------------------------------------
// Purpose: medieval chat
//-----------------------------------------------------------------------------
void CTFGameRules::ModifySentChat( char *pBuf, int iBufSize )
{
	if ( !IsInMedievalMode() )
		return;
}

//-----------------------------------------------------------------------------
// Purpose: comp/mvm disconnect prompt
//-----------------------------------------------------------------------------
bool CTFGameRules::ShouldWarnOfAbandonOnQuit( void )
{
	if ( IsCompetitiveMode() )
		return true;

	return false;
}

#endif

bool CTFGameRules::UsePlayerReadyStatusMode( void )
{
	if ( IsInTournamentMode() )
		return true;

	if ( IsMannVsMachineMode() )
		return true;

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::CanPlayerSearchSpawn( void )
{
	return lfe_coop_spawn_search.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::CanPlayerStopSearchSpawn( void )
{
	return lfe_coop_spawn_search_instant.GetBool();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::PlayerReadyStatus_ArePlayersOnTeamReady( int iTeam )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*bool CTFGameRules::PlayerReadyStatus_HaveMinPlayersToEnable( void )
{
	return false;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::PlayerReadyStatus_ResetState( void )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::PlayerReadyStatus_ShouldStartCountdown( void )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGameRules::PlayerReadyStatus_UpdatePlayerState( CTFPlayer *pPlayer, bool bReady )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFGameRules::IsVehicleSpawningAllowed( void )
{
	const char *map = STRING(gpGlobals->mapname);
	CBaseEntity *pCheckJeep = gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep");
	CBaseEntity *pCheckAirboat = gEntList.FindEntityByClassname(NULL, "prop_vehicle_airboat");
	CTFLogicCoOp *pLogicCoop = static_cast<CTFLogicCoOp*>(gEntList.FindEntityByClassname(NULL, "lfe_logic_coop"));
	if (IsInHL2EP2Map() && pCheckJeep && (V_stricmp(map, "ep2_outland_06") != 0) || IsInHL2Map() && pCheckAirboat && (V_stricmp(map, "d1_canals_05") != 0) || TFGameRules()->IsInHL2Map() && pCheckJeep && (V_stricmp(map, "d2_coast_10") != 0) || pLogicCoop && pLogicCoop->m_bVehicleSpawningAllowed)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGameRules::GetSpawningVehicleType( void )
{
	CBaseEntity *pCheckJeep = gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep");
	CBaseEntity *pCheckAirboat = gEntList.FindEntityByClassname(NULL, "prop_vehicle_airboat");
	CTFLogicCoOp *pLogicCoop = static_cast<CTFLogicCoOp*>(gEntList.FindEntityByClassname(NULL, "lfe_logic_coop"));
	if (IsInHL2EP2Map())
	{
		return 3;
	}
	else if (IsInHL2Map() && pCheckAirboat)
	{
		return 1;
	}
	else if (IsInHL2Map() && pCheckJeep)
	{
		return 2;
	}
	if (pLogicCoop)
	{
		return pLogicCoop->m_iVehicleType;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGameRules::SetupTimerExpired( void )
{
	if ( LFPopulationManager() )
		LFPopulationManager()->SetupTimerExpired();
}

#endif
#ifdef GAME_DLL
class CTFMapeditTeleport : public CBaseEntity
{
public:
	DECLARE_CLASS(CTFMapeditTeleport, CBaseEntity);
	DECLARE_DATADESC();

	CTFMapeditTeleport();
	~CTFMapeditTeleport();

	bool	m_bReverse;
	bool	m_bEnabled;
	int		m_iDistance;
	int		m_iTeamToTeleport;
	string_t	m_strTeleportEnt;
	void	Spawn(void);
	void	InputToggleMode(inputdata_t &inputdata);
	void	InputSetDistance(inputdata_t &inputdata);
	void	InputSetTeleportEnt(inputdata_t &inputdata);
	void	InputSetTeamToTeleport(inputdata_t &inputdata);
	void	InputToggle(inputdata_t &inputdata);
	void	Think(void);
};

BEGIN_DATADESC(CTFMapeditTeleport)
	DEFINE_KEYFIELD(m_iDistance, FIELD_INTEGER, "distance"),
	DEFINE_KEYFIELD(m_bReverse, FIELD_BOOLEAN, "reverse"),
	DEFINE_KEYFIELD(m_iTeamToTeleport, FIELD_INTEGER, "team_to_teleport"),
	DEFINE_KEYFIELD(m_strTeleportEnt, FIELD_STRING, "teleport_ent"),
	DEFINE_INPUTFUNC(FIELD_VOID, "ToggleMode", InputToggleMode),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetDistance", InputSetDistance),
	DEFINE_INPUTFUNC(FIELD_STRING, "SetTeleportEnt", InputSetTeleportEnt),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetTeamToTeleport", InputSetTeamToTeleport),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
END_DATADESC()

LINK_ENTITY_TO_CLASS(lfe_mapedit_teleport, CTFMapeditTeleport);

CTFMapeditTeleport::CTFMapeditTeleport()
{
	m_strTeleportEnt = GetEntityName();
	m_iDistance = 512;
	m_iTeamToTeleport = TF_TEAM_RED;
}

void CTFMapeditTeleport::Spawn(void)
{
	Think();
	BaseClass::Spawn();
}

CTFMapeditTeleport::~CTFMapeditTeleport()
{
}

void CTFMapeditTeleport::InputToggle(inputdata_t &inputdata)
{
	if (!m_bEnabled)
	{
		m_bEnabled = true;
	}
	else
	{
		m_bEnabled = false;
	}
}

void CTFMapeditTeleport::InputSetDistance(inputdata_t &inputdata)
{
	m_iDistance = inputdata.value.Int();
}

void CTFMapeditTeleport::InputSetTeleportEnt(inputdata_t &inputdata)
{
	m_strTeleportEnt = gEntList.FindEntityByName(NULL, inputdata.value.String())->GetEntityName();
}

void CTFMapeditTeleport::InputSetTeamToTeleport(inputdata_t &inputdata)
{
	m_iTeamToTeleport = inputdata.value.Int();
}

void CTFMapeditTeleport::InputToggleMode(inputdata_t &inputdata)
{
	if (!m_bReverse)
	{
		m_bReverse = true;
	}
	else
	{
		m_bReverse = false;
	}
}

void CTFMapeditTeleport::Think(void)
{
	CTFPlayer *pPlayer = ToTFPlayer(gEntList.FindEntityByClassname(NULL, "player"));
	CBaseEntity *pTeleportEnt = gEntList.FindEntityByName(NULL, m_strTeleportEnt);
	if (pPlayer && m_bEnabled)
	{
		float m_flDist = GetAbsOrigin().DistTo(pPlayer->GetAbsOrigin());
		if ((m_flDist > m_iDistance && !m_bReverse || m_flDist < m_iDistance && m_bReverse) && pPlayer->GetTeamNumber() == m_iTeamToTeleport)
		{
			if (pTeleportEnt)
			{
				Vector vecOrigin = pTeleportEnt->GetAbsOrigin();
				QAngle vecAngles = pTeleportEnt->GetAbsAngles();
				pPlayer->Teleport(&vecOrigin, &vecAngles, NULL);
			}
			else
			{
				Vector vecOrigin = GetAbsOrigin();
				QAngle vecAngles = GetAbsAngles();
				pPlayer->Teleport(&vecOrigin, &vecAngles, NULL);
			}
		}
	}
	SetContextThink(&CTFMapeditTeleport::Think, gpGlobals->curtime + 0.1, "Think");
}
#endif