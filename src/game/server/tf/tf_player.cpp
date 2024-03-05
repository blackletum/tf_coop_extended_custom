//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "tf_player.h"
#include "tf_gamerules.h"
#include "tf_gamestats.h"
#include "KeyValues.h"
#include "viewport_panel_names.h"
#include "client.h"
#include "team.h"
#include "tf_weaponbase.h"
#include "tf_client.h"
#include "tf_team.h"
#include "tf_viewmodel.h"
#include "tf_item.h"
#include "in_buttons.h"
#include "entity_capture_flag.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "game.h"
#include "tf_weapon_builder.h"
#include "tf_obj.h"
#include "tf_obj_dispenser.h"
#include "tf_ammo_pack.h"
#include "datacache/imdlcache.h"
#include "particle_parse.h"
#include "props_shared.h"
#include "filesystem.h"
#include "toolframework_server.h"
#include "IEffects.h"
#include "func_respawnroom.h"
#include "networkstringtable_gamedll.h"
#include "team_control_point_master.h"
#include "tf_weapon_pda.h"
#include "sceneentity.h"
#include "fmtstr.h"
#include "tf_weapon_sniperrifle.h"
#include "tf_weapon_minigun.h"
#include "trigger_area_capture.h"
#include "triggers.h"
#include "tf_weapon_medigun.h"
#include "tf_weapon_buff_item.h"
#include "achievements_tf.h"
#include "npc_citizen17.h"
#include "te_tfblood.h"
#include "activitylist.h"
#include "steam/steam_api.h"
#include "cdll_int.h"
#include "tf_weaponbase.h"
#include "econ_wearable.h"
#include "tf_wearable.h"
#include "tf_dropped_weapon.h"
#include "tf_wearable_demoshield.h"
#include "tf_item_powerup_bottle.h"
#include "econ_item_schema.h"
#include "baseprojectile.h"
#include "tf_weapon_flamethrower.h"
#include "tf_weapon_shovel.h"
#include "entity_rune.h"
#include "tf_weapon_lunchbox.h"
#include "tf_weapon_sword.h"
#include "player_pickup.h"
#include "weapon_physcannon.h"
#include "eventqueue.h"
#include "ai_basenpc.h"
#include "ai_squad.h"
#include "iservervehicle.h"
#include "globalstate.h"
#include "grenade_bugbait.h"
#include "antlion_maker.h"
#include "npc_barnacle.h"
#include "trains.h"
#include "nav_mesh.h"
#include "tf_fx.h"
#include "npc_alyx_episodic.h"
#include "tf_weapon_invis.h"
#include "tf_weapon_portalgun.h"
#include "tf_weapon_grapplinghook.h"
#include "tf_fx_shared.h"
#include "tf_taunt_prop.h"
#include "func_achievement.h"

#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "prop_portal_shared.h"
#include "vphysics/player_controller.h"
#include "bone_setup.h"
#include "portal_gamestats.h"
#include "physicsshadowclone.h"
#include "physics_prop_ragdoll.h"
#include "soundenvelope.h"
#include "ai_speech.h"
#include "info_camera_link.h"
#include "point_camera.h"
#include "script_intro.h"
#include "prop_combine_ball.h"

#include "tf_weapon_cheatgun.h"

#include "tf_weapon_grenade_caltrop.h"
#include "tf_weapon_grenade_concussion.h"
#include "tf_weapon_grenade_emp.h"
#include "tf_weapon_grenade_gas.h"
#include "tf_weapon_grenade_heal.h"
#include "tf_weapon_grenade_mirv.h"
#include "tf_weapon_grenade_nail.h"
#include "tf_weapon_grenade_napalm.h"
#include "tf_weapon_grenade_normal.h"
#include "tf_weapon_grenade_pipebomb.h"
#include "tf_weapon_grenade_smoke_bomb.h"
#include "tf_weapon_grenade_flare.h"
#include "tf_weapon_bottle.h"
#include "grenade_frag.h"
#include "halloween/zombie.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Max mass the player can lift with +use
#define PORTAL_PLAYER_MAX_LIFT_MASS 85
#define PORTAL_PLAYER_MAX_LIFT_SIZE 128

#define DAMAGE_FORCE_SCALE_SELF				9

#define TF_AMMOPACK_MODEL "models/items/ammopack_medium.mdl"

extern ConVar sv_turbophysics;

#ifdef POSIX
#undef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m) // something happened to this define so we need to redefine it here
#endif

//----------------------------------------------------
// Player Physics Shadow
//----------------------------------------------------
#define VPHYS_MAX_DISTANCE		2.0
#define VPHYS_MAX_VEL			10
#define VPHYS_MAX_DISTSQR		(VPHYS_MAX_DISTANCE*VPHYS_MAX_DISTANCE)
#define VPHYS_MAX_VELSQR		(VPHYS_MAX_VEL*VPHYS_MAX_VEL)

extern float IntervalDistance( float x, float x0, float x1 );

extern bool IsInCommentaryMode( void );

extern ConVar	sk_player_head;
extern ConVar	sk_player_chest;
extern ConVar	sk_player_stomach;
extern ConVar	sk_player_arm;
extern ConVar	sk_player_leg;

extern ConVar	mp_disable_respawn_times;
extern ConVar	tf_spy_invis_time;
extern ConVar	tf_spy_invis_unstealth_time;
extern ConVar	tf_stalematechangeclasstime;

extern ConVar	tf_damage_disablespread;

extern ConVar	lfe_force_legacy;
extern ConVar	lfe_allow_revive_marker;

extern ConVar	tf_scout_energydrink_consume_rate;

extern ConVar	tf_feign_death_duration;
extern ConVar	tf_feign_death_damage_scale;
extern ConVar	tf_feign_death_activate_damage_scale;

extern ConVar	tf_airblast_cray_debug;
extern ConVar	tf_airblast_cray_ground_minz;
extern ConVar	tf_airblast_cray_ground_reflect;

EHANDLE g_pLastSpawnPoints[TF_TEAM_COUNT];

ConVar tf_playerstatetransitions( "tf_playerstatetransitions", "-2", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "tf_playerstatetransitions <ent index or -1 for all>. Show player state transitions." );
ConVar tf_playergib( "tf_playergib", "1", FCVAR_PROTECTED, "Allow player gibbing. 0: never, 1: normal, 2: always", true, 0, true, 2 );
ConVar tf_spawn_glows_duration( "tf_spawn_glows_duration", "10", FCVAR_NOTIFY | FCVAR_REPLICATED, "How long should teammates glow after respawning." );

ConVar tf_weapon_ragdoll_velocity_min( "tf_weapon_ragdoll_velocity_min", "100", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_weapon_ragdoll_velocity_max( "tf_weapon_ragdoll_velocity_max", "150", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_weapon_ragdoll_maxspeed( "tf_weapon_ragdoll_maxspeed", "300", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

ConVar tf_damageforcescale_other( "tf_damageforcescale_other", "6.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_damageforcescale_pyro_jump( "tf_damageforcescale_pyro_jump", "8.5", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_damageforcescale_self_soldier_rj( "tf_damageforcescale_self_soldier_rj", "10.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_damageforcescale_self_soldier_badrj( "tf_damageforcescale_self_soldier_badrj", "5.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar tf_damagescale_self_soldier( "tf_damagescale_self_soldier", "0.60", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

ConVar tf_damage_lineardist( "tf_damage_lineardist", "0", FCVAR_DEVELOPMENTONLY );
ConVar tf_damage_range( "tf_damage_range", "0.5", FCVAR_DEVELOPMENTONLY );

ConVar tf_double_donk_window( "tf_double_donk_window", "0.5", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

ConVar tf_max_voice_speak_delay( "tf_max_voice_speak_delay", "1.5", FCVAR_NOTIFY, "Max time after a voice command until player can do another one" );

ConVar tf_maxhealth_drain_hp_min( "tf_maxhealth_drain_hp_min", "100", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

ConVar lfe_allow_spectate_npc( "lfe_allow_spectate_npc", "0", FCVAR_REPLICATED, "Allow spectating NPC. Enabling this is not recommended." );
ConVar lfe_player_lives( "lfe_player_lives", "-1", FCVAR_REPLICATED, "Amount of lives RED players start with in co-op. Set to -1 for unlimited lives." );

ConVar tf_allow_player_use( "tf_allow_player_use", "1", FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY, "Allow players to execute + use while playing." );

ConVar lfe_allow_transition( "lfe_allow_transition", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow level transition system." );
ConVar lfe_allow_tf_dropped_weapon( "lfe_allow_tf_dropped_weapon", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow weapons to drop on death." );
ConVar lfe_allow_medigun_shield( "lfe_allow_medigun_shield", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow medic's projectile shield." );
ConVar lfe_allow_minigun_deflect( "lfe_allow_minigun_deflect", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow heavy's to destroy projectiles." );

ConVar tf_allow_sliding_taunt( "tf_allow_sliding_taunt", "1", FCVAR_REPLICATED, "Allow player to slide for a bit after taunting." );

ConVar tf_halloween_giant_health_scale( "tf_halloween_giant_health_scale", "10", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

extern ConVar spec_freeze_time;
extern ConVar spec_freeze_traveltime;
extern ConVar sv_maxunlag;

extern ConVar sv_alltalk;
extern ConVar tf_gravetalk;

ConVar lfe_allow_team_weapons( "lfe_allow_team_weapons", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Makes players spawn with gravity gun." );

ConVar lfe_force_random_weapons( "lfe_force_random_weapons", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Makes players spawn with random loadout." );
ConVar lfe_allow_special_classes( "lfe_allow_special_classes", "0", FCVAR_NOTIFY, "Enables gamemode specific classes in normal gameplay." );

// Cvars from HL2 player
ConVar hl2_walkspeed( "hl2_walkspeed", "180" );
ConVar hl2_normspeed( "hl2_normspeed", "210" );
ConVar hl2_sprintspeed( "hl2_sprintspeed", "350" );

ConVar hl2_darkness_flashlight_factor ( "hl2_darkness_flashlight_factor", "1" );

ConVar player_squad_transient_commands( "player_squad_transient_commands", "1", FCVAR_REPLICATED );
ConVar player_squad_double_tap_time( "player_squad_double_tap_time", "0.25" );

extern ConVar lfe_use_hl2_player_hull;

//==============================================================================================
// CAPPED PLAYER PHYSICS DAMAGE TABLE
//==============================================================================================
static impactentry_t cappedPlayerLinearTable[] =
{
	{ 150*150, 5 },
	{ 250*250, 10 },
	{ 450*450, 20 },
	{ 550*550, 30 },
	//{ 700*700, 100 },
	//{ 1000*1000, 500 },
};

static impactentry_t cappedPlayerAngularTable[] =
{
	{ 100*100, 10 },
	{ 150*150, 20 },
	{ 200*200, 30 },
	//{ 300*300, 500 },
};

static impactdamagetable_t gCappedPlayerImpactDamageTable =
{
	cappedPlayerLinearTable,
	cappedPlayerAngularTable,

	ARRAYSIZE(cappedPlayerLinearTable),
	ARRAYSIZE(cappedPlayerAngularTable),

	24*24.0f,	// minimum linear speed
	360*360.0f,	// minimum angular speed
	2.0f,		// can't take damage from anything under 2kg

	5.0f,		// anything less than 5kg is "small"
	5.0f,		// never take more than 5 pts of damage from anything under 5kg
	36*36.0f,	// <5kg objects must go faster than 36 in/s to do damage

	0.0f,		// large mass in kg (no large mass effects)
	1.0f,		// large mass scale
	2.0f,		// large mass falling scale
	320.0f,		// min velocity for player speed to cause damage

};

// -------------------------------------------------------------------------------- //
//Transitions
// -------------------------------------------------------------------------------- //
struct TFPlayerTransitionStruct
{
	int			playerClass;
	float		ubercharge;
	float		itemmeter;
	int			kills;

};

CUtlMap<uint64, TFPlayerTransitionStruct> g_TFPlayerTransitions( DefLessFunc( uint64 ) );


// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //
class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
	{
		m_iPlayerIndex = TF_PLAYER_INDEX_NONE;
	}

	CNetworkVar( int, m_iPlayerIndex );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropInt( SENDINFO( m_iPlayerIndex ), 7, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	// BUGBUG:  ywb  we assume this is either 0 or an animation sequence #, but it could also be an activity, which should fit within this limit, but we're not guaranteed.
	SendPropInt( SENDINFO( m_nData ), ANIMATION_SEQUENCE_BITS ),
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	Vector vecEyePos = pPlayer->EyePosition();
	CPVSFilter filter( vecEyePos );
	if ( !IsCustomPlayerAnimEvent( event ) && ( event != PLAYERANIMEVENT_SNAP_YAW ) && ( event != PLAYERANIMEVENT_VOICE_COMMAND_GESTURE ) )
	{
		filter.RemoveRecipient( pPlayer );
	}

	Assert( pPlayer->entindex() >= 1 && pPlayer->entindex() <= MAX_PLAYERS );
	g_TEPlayerAnimEvent.m_iPlayerIndex = pPlayer->entindex();
	g_TEPlayerAnimEvent.m_iEvent = event;
	Assert( nData < (1<<ANIMATION_SEQUENCE_BITS) );
	Assert( (1<<ANIMATION_SEQUENCE_BITS) >= ActivityList_HighestIndex() );
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

//=================================================================================
//
// Ragdoll Entity
//
//=================================================================================
class CTFRagdoll : public CBaseAnimatingOverlay
{
public:

	DECLARE_CLASS( CTFRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	CTFRagdoll()
	{
		m_iPlayerIndex.Set( TF_PLAYER_INDEX_NONE );
		m_bGib = false;
		m_bBurning = false;
		m_bCloaked = false;
		m_bFeignDeath = false;
		m_bElectrocuted = false;
		m_bWasDisguised = false;
		m_bOnGround = true;
		m_bBecomeAsh = false;
		m_bGoldRagdoll = false;
		m_bIceRagdoll = false;
		m_bCritOnHardHit = false;
		m_iDamageCustom = TF_DMG_CUSTOM_NONE;
		m_vecRagdollOrigin.Init();
		m_vecRagdollVelocity.Init();
		UseClientSideAnimation();
	}

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	CNetworkVar( int, m_iPlayerIndex );
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
	CNetworkVar( bool, m_bGib );
	CNetworkVar( bool, m_bBurning );
	CNetworkVar( bool, m_bElectrocuted );
	CNetworkVar( bool, m_bFeignDeath );
	CNetworkVar( bool, m_bWasDisguised );
	CNetworkVar( bool, m_bBecomeAsh );
	CNetworkVar( bool, m_bOnGround );
	CNetworkVar( bool, m_bCloaked );
	CNetworkVar( int, m_iDamageCustom );
	CNetworkVar( int, m_iTeam );
	CNetworkVar( int, m_iClass );
	CNetworkArray( EHANDLE, m_hRagWearables, LOADOUT_POSITION_COUNT );
	CNetworkVar( bool, m_bGoldRagdoll );
	CNetworkVar( bool, m_bIceRagdoll );
	CNetworkVar( bool, m_bCritOnHardHit );
};

LINK_ENTITY_TO_CLASS( tf_ragdoll, CTFRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTFRagdoll, DT_TFRagdoll )
	SendPropVector( SENDINFO( m_vecRagdollOrigin ), -1, SPROP_COORD ),
	SendPropInt( SENDINFO( m_iPlayerIndex ), 7, SPROP_UNSIGNED ),
	SendPropVector( SENDINFO( m_vecForce ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ), 13, SPROP_ROUNDDOWN, -2048.0f, 2048.0f ),
	SendPropInt( SENDINFO( m_nForceBone ) ),
	SendPropBool( SENDINFO( m_bGib ) ),
	SendPropBool( SENDINFO( m_bBurning ) ),
	SendPropBool( SENDINFO( m_bElectrocuted ) ),
	SendPropBool( SENDINFO( m_bFeignDeath ) ),
	SendPropBool( SENDINFO( m_bWasDisguised ) ),
	SendPropBool( SENDINFO( m_bBecomeAsh ) ),
	SendPropBool( SENDINFO( m_bOnGround ) ),
	SendPropBool( SENDINFO( m_bCloaked ) ),
	SendPropInt( SENDINFO( m_iDamageCustom ) ),
	SendPropInt( SENDINFO( m_iTeam ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iClass ), 4, SPROP_UNSIGNED ),
	SendPropArray3( SENDINFO_ARRAY3( m_hRagWearables ), SendPropEHandle( SENDINFO_ARRAY( m_hRagWearables ) ) ),
	SendPropBool( SENDINFO( m_bGoldRagdoll ) ),
	SendPropBool( SENDINFO( m_bIceRagdoll ) ),
	SendPropBool( SENDINFO( m_bCritOnHardHit ) ),
END_SEND_TABLE()

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //

//-----------------------------------------------------------------------------
// Purpose: Filters updates to a variable so that only non-local players see
// the changes.  This is so we can send a low-res origin to non-local players
// while sending a hi-res one to the local player.
// Input  : *pVarData - 
//			*pOut - 
//			objectID - 
//-----------------------------------------------------------------------------

void* SendProxy_SendNonLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient( objectID - 1 );
	return ( void * )pVarData;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendNonLocalDataTable );

//-----------------------------------------------------------------------------
// Purpose: SendProxy that converts the UtlVector list of objects to entindexes, where it's reassembled on the client
//-----------------------------------------------------------------------------
void SendProxy_PlayerObjectList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CTFPlayer *pPlayer = (CTFPlayer*)pStruct;

	// If this fails, then SendProxyArrayLength_PlayerObjects didn't work.
	Assert( iElement < pPlayer->GetObjectCount() );

	CBaseObject *pObject = pPlayer->GetObject(iElement);

	EHANDLE hObject;
	hObject = pObject;

	SendProxy_EHandleToInt( pProp, pStruct, &hObject, pOut, iElement, objectID );
}

int SendProxyArrayLength_PlayerObjects( const void *pStruct, int objectID )
{
	CTFPlayer *pPlayer = (CTFPlayer*)pStruct;
	int iObjects = pPlayer->GetObjectCount();
	Assert( iObjects <= MAX_OBJECTS_PER_PLAYER );
	return iObjects;
}

BEGIN_DATADESC( CTFPlayer )
	DEFINE_INPUTFUNC( FIELD_STRING,	"SpeakResponseConcept",	InputSpeakResponseConcept ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"IgnitePlayer",	InputIgnitePlayer ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ExtinguishPlayer",	InputExtinguishPlayer ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"BleedPlayer",	InputBleedPlayer ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetForcedTauntCam", InputSetForcedTauntCam ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"SetCustomModel",	InputSetCustomModel ),

	DEFINE_OUTPUT( m_OnDeath, "OnDeath" ),

	DEFINE_FIELD( m_bHasLongJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flAdmireGlovesAnimTime, FIELD_TIME ),
	DEFINE_FIELD( m_flNextFlashlightCheckTime, FIELD_TIME ),
	DEFINE_FIELD( m_flFlashlightPowerDrainScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_bFlashlightDisabled, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bUseCappedPhysicsDamageTable, FIELD_BOOLEAN ),

	DEFINE_SOUNDPATCH( m_sndLeeches ),
	DEFINE_SOUNDPATCH( m_sndWaterSplashes ),
	DEFINE_SOUNDPATCH( m_pWooshSound ),

	DEFINE_SOUNDPATCH( m_sndTauntLoop ),

	DEFINE_FIELD( m_hLocatorTargetEntity, FIELD_EHANDLE ),

	DEFINE_FIELD( m_iSquadMemberCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSquadMedicCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_fSquadInFollowMode, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecLocatorOrigin, FIELD_POSITION_VECTOR ),

	DEFINE_FIELD( m_bHeldObjectOnOppositeSideOfPortal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_pHeldObjectPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bIntersectingPortalPlane, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bStuckOnPortalCollisionObject, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_StatsThisLevel.iNumPortalsPlaced, FIELD_INTEGER ),
	DEFINE_FIELD( m_StatsThisLevel.iNumStepsTaken, FIELD_INTEGER ),
	DEFINE_FIELD( m_StatsThisLevel.fNumSecondsTaken, FIELD_FLOAT ),
	DEFINE_FIELD( m_iNumCamerasDetatched, FIELD_INTEGER ),
	DEFINE_FIELD( m_bPitchReorientation, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fNeuroToxinDamageTime, FIELD_TIME ),
	DEFINE_FIELD( m_hPortalEnvironment, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bSilentDropAndPickup, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_qPrePortalledViewAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_bFixEyeAnglesFromPortalling, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_matLastPortalled, FIELD_VMATRIX_WORLDSPACE ),
	DEFINE_FIELD( m_hSurroundingLiquidPortal, FIELD_EHANDLE ),

END_DATADESC()
extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

// specific to the local player
BEGIN_SEND_TABLE_NOBASE( CTFPlayer, DT_TFLocalPlayerExclusive )
	// send a hi-res origin to the local player for use in prediction
	SendPropVector (SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropArray2( 
		SendProxyArrayLength_PlayerObjects,
		SendPropInt("player_object_array_element", 0, SIZEOF_IGNORE, NUM_NETWORKED_EHANDLE_BITS, SPROP_UNSIGNED, SendProxy_PlayerObjectList), 
		MAX_OBJECTS_PER_PLAYER, 
		0, 
		"player_object_array"
		),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -360.0f, 360.0f ),
//	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_iSquadMemberCount ) ),
	SendPropInt( SENDINFO( m_iSquadMedicCount ) ),
	SendPropBool( SENDINFO( m_fSquadInFollowMode ) ),
	SendPropEHandle( SENDINFO( m_hLadder ) ),
	SendPropVector( SENDINFO( m_vecLocatorOrigin ) ),
	SendPropString( SENDINFO( m_iszCustomModel ) ),

END_SEND_TABLE()

// all players except the local player
BEGIN_SEND_TABLE_NOBASE( CTFPlayer, DT_TFNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -360.0f, 360.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

END_SEND_TABLE()


//============

LINK_ENTITY_TO_CLASS( player, CTFPlayer );
PRECACHE_REGISTER(player);

IMPLEMENT_SERVERCLASS_ST( CTFPlayer, DT_TFPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_BaseEntity", "m_nModelIndex" ),
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),

	// cs_playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	SendPropExclude( "DT_BaseFlex", "m_flexWeight" ),
	SendPropExclude( "DT_BaseFlex", "m_blinktoggle" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

	SendPropBool( SENDINFO( m_bSaveMeParity ) ),

	// This will create a race condition will the local player, but the data will be the same so.....
	SendPropInt( SENDINFO( m_nWaterLevel ), 2, SPROP_UNSIGNED ),

	SendPropEHandle( SENDINFO( m_hItem ) ),
	SendPropEHandle( SENDINFO( m_hGrapplingHookTarget ) ),

	// Ragdoll.
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropDataTable( SENDINFO_DT( m_PlayerClass ), &REFERENCE_SEND_TABLE( DT_TFPlayerClassShared ) ),
	SendPropDataTable( SENDINFO_DT( m_Shared ), &REFERENCE_SEND_TABLE( DT_TFPlayerShared ) ),
	SendPropDataTable( SENDINFO_DT( m_AttributeManager ), &REFERENCE_SEND_TABLE( DT_AttributeManager ) ),

	// Data that only gets sent to the local player
	SendPropDataTable( "tflocaldata", 0, &REFERENCE_SEND_TABLE(DT_TFLocalPlayerExclusive), SendProxy_SendLocalDataTable ),

	// Data that gets sent to all other players
	SendPropDataTable( "tfnonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_TFNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

	SendPropInt( SENDINFO( m_iSpawnCounter ) ),
	SendPropInt( SENDINFO( m_nForceTauntCam ), 2, SPROP_UNSIGNED ),
	SendPropTime( SENDINFO( m_flLastDamageTime ) ),
	SendPropTime( SENDINFO( m_flNextNoiseMakerTime ) ),
	SendPropBool( SENDINFO( m_bTyping ) ),
	SendPropBool( SENDINFO( m_bSearchingSpawn ) ),

	SendPropBool( SENDINFO( m_bAllowMoveDuringTaunt ) ),
	SendPropBool( SENDINFO( m_bIsReadyToHighFive ) ),
	SendPropEHandle( SENDINFO( m_hHighFivePartner ) ),
	SendPropFloat( SENDINFO( m_flCurrentTauntMoveSpeed ) ),
	SendPropFloat( SENDINFO( m_flTauntYaw ) ),
	SendPropInt( SENDINFO( m_iTauntItemDefIndex ) ),
	SendPropInt( SENDINFO( m_nActiveTauntSlot ) ),

	SendPropFloat( SENDINFO( m_flHeadScale ) ),
	SendPropFloat( SENDINFO( m_flTorsoScale ) ),
	SendPropFloat( SENDINFO( m_flHandScale ) ),

	SendPropBool( SENDINFO( m_bHeldObjectOnOppositeSideOfPortal ) ),
	SendPropEHandle( SENDINFO( m_pHeldObjectPortal ) ),
	SendPropBool( SENDINFO( m_bPitchReorientation ) ),
	SendPropEHandle( SENDINFO( m_hPortalEnvironment ) ),
	SendPropEHandle( SENDINFO( m_hSurroundingLiquidPortal ) ),

END_SEND_TABLE()



// -------------------------------------------------------------------------------- //

void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

// Hint callbacks
bool HintCallbackNeedsResources_Sentrygun( CBasePlayer *pPlayer )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( !pTFPlayer )
		return false;
	return ( pPlayer->GetAmmoCount( TF_AMMO_METAL ) > CalculateObjectCost( OBJ_SENTRYGUN, pTFPlayer->HasGunslinger() ) );
}
bool HintCallbackNeedsResources_Dispenser( CBasePlayer *pPlayer )
{
	return ( pPlayer->GetAmmoCount( TF_AMMO_METAL ) > CalculateObjectCost( OBJ_DISPENSER ) );
}
bool HintCallbackNeedsResources_Teleporter( CBasePlayer *pPlayer )
{
	return ( pPlayer->GetAmmoCount( TF_AMMO_METAL ) > CalculateObjectCost( OBJ_TELEPORTER ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFPlayer::CTFPlayer()
{
	m_pAttributes = this;

	m_PlayerAnimState = CreateTFPlayerAnimState( this );
	item_list = 0;

	SetArmorValue( 10 );

	m_hItem = NULL;
	m_hTauntScene = NULL;
	m_hGrapplingHookTarget = NULL;

	UseClientSideAnimation();
	m_angEyeAngles.Init();
	m_pStateInfo = NULL;
	m_lifeState = LIFE_DEAD; // Start "dead".
	m_iMaxSentryKills = 0;
	m_flNextNameChangeTime = 0;

	m_flNextHealthRegen = 0;
	m_flNextTimeCheck = gpGlobals->curtime;
	m_flSpawnTime = 0;

	m_flNextCarryTalkTime = 0.0f;

	SetViewOffset( TF_PLAYER_VIEW_OFFSET );

	m_Shared.Init( this );

	m_iLastSkin = -1;

	m_bHudClassAutoKill = false;
	m_bMedigunAutoHeal = false;

	m_vecLastDeathPosition = Vector( FLT_MAX, FLT_MAX, FLT_MAX );

	m_flHeadScale = 1.0f;
	m_flTorsoScale = 1.0f;
	m_flHandScale = 1.0f;

	SetDesiredPlayerClassIndex( TF_CLASS_UNDEFINED );

	SetContextThink( &CTFPlayer::TFPlayerThink, gpGlobals->curtime, "TFPlayerThink" );

	ResetScores();

	m_flLastAction = gpGlobals->curtime;

	m_bInitTaunt = false;

	m_bSpeakingConceptAsDisguisedSpy = false;

	m_pPlayerAISquad = NULL;

	m_bInTransition = false;
	m_bTransitioned = false;

	m_flTauntAttackTime = 0.0f;
	m_iTauntAttack = TAUNTATK_NONE;
	m_iTauntItemDefIndex = 0;

	m_nBlastJumpFlags = 0;
	m_bBlastLaunched = false;
	m_bJumpEffect = false;
	m_bForceByNature = false;

	memset( m_WeaponPreset, 0, TF_CLASS_COUNT_ALL * LOADOUT_POSITION_COUNT * sizeof( int ) );

	//m_iSpawnInterpCounter = 0;

	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;
	m_bIntersectingPortalPlane = false;
	m_bPitchReorientation = false;

	m_bSilentDropAndPickup = false;

	m_purgatoryDuration.Invalidate();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::TFPlayerThink()
{
	if ( m_pStateInfo && m_pStateInfo->pfnThink )
	{
		(this->*m_pStateInfo->pfnThink)();
	}

	// Time to finish the current random expression? Or time to pick a new one?
	if ( IsAlive() && m_flNextRandomExpressionTime >= 0 && gpGlobals->curtime > m_flNextRandomExpressionTime )
	{
		// Random expressions need to be cleared, because they don't loop. So if we
		// pick the same one again, we want to restart it.
		ClearExpression();
		m_iszExpressionScene = NULL_STRING;
		UpdateExpression();
	}

	// Check to see if we are in the air and taunting.  Stop if so.
	if ( ( GetGroundEntity() == NULL ) && m_Shared.InCond( TF_COND_TAUNTING ) )
	{
		if ( m_hTauntScene.Get() )
		{
			if ( m_iTauntItemDefIndex != 0 )
			{
				CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_iTauntItemDefIndex );
				if ( pItemDef && pItemDef->stop_taunt_if_moved )
				{
					StopScriptedScene( this, m_hTauntScene );
					m_Shared.m_flTauntRemoveTime = 0.0f;
					m_hTauntScene = NULL;
				}
			}
			else
			{
				StopScriptedScene( this, m_hTauntScene );
				m_Shared.m_flTauntRemoveTime = 0.0f;
				m_hTauntScene = NULL;
			}
		}
	}

	// If players is hauling a building have him talk about it from time to time.
	if ( m_flNextCarryTalkTime != 0.0f && m_flNextCarryTalkTime < gpGlobals->curtime )
	{
		CBaseObject *pObject = m_Shared.GetCarriedObject();
		if ( pObject )
		{
			const char *pszModifier = pObject->GetResponseRulesModifier();
			SpeakConceptIfAllowed( MP_CONCEPT_CARRYING_BUILDING, pszModifier );
			m_flNextCarryTalkTime = gpGlobals->curtime + RandomFloat( 6.0f, 12.0f );
		}
		else
		{
			// No longer hauling, shut up.
			m_flNextCarryTalkTime = 0.0f;
		}
	}

	// Add rocket trail if we haven't already.
	if ( !m_bJumpEffect && ( m_nBlastJumpFlags & ( TF_JUMP_ROCKET | TF_JUMP_STICKY ) ) && IsAlive() )
	{
		DispatchParticleEffect( "rocketjump_smoke", PATTACH_POINT_FOLLOW, this, "foot_L" );
		DispatchParticleEffect( "rocketjump_smoke", PATTACH_POINT_FOLLOW, this, "foot_R" );
		m_bJumpEffect = true;
	}

	if( !IsPlayerClass( TF_CLASS_MEDIC ) && gpGlobals->curtime > m_flNextHealthRegen )
	{
		int iHealthDrain = 0;
		CALL_ATTRIB_HOOK_INT( iHealthDrain, add_health_regen );

		if( iHealthDrain )
		{
			RegenThink();
			m_flNextHealthRegen = gpGlobals->curtime + TF_MEDIC_REGEN_TIME;
		}

		if( m_Shared.InCond( TF_COND_RUNE_REGEN ) )
		{
			RuneRegenThink();
			m_flNextHealthRegen = gpGlobals->curtime + TF_RUNE_REGEN_TIME;
		}
	}

	m_flHeadScale  = Approach( GetDesiredHeadScale(),  m_flHeadScale,  GetHeadScaleSpeed() );
	m_flTorsoScale = Approach( GetDesiredTorsoScale(), m_flTorsoScale, GetTorsoScaleSpeed() );
	m_flHandScale  = Approach( GetDesiredHandScale(),  m_flHandScale,  GetHandScaleSpeed() );


//	if ( IsAlive() && lfe_use_hl2_player_hull.GetInt() != 0 && GetModelScale() != 1 && !TFGameRules()->IsTFCAllowed() )
//		SetModelScale( 1 );
//	else if ( IsAlive() && (lfe_use_hl2_player_hull.GetInt() != 1 || TFGameRules()->IsTFCAllowed()) && GetModelScale() != 1 )
//		SetModelScale( 1 );

	
	SetContextThink( &CTFPlayer::TFPlayerThink, gpGlobals->curtime, "TFPlayerThink" );
}

void CTFPlayer::CheckTeam( void )
{
	if ( GetTeamNumber() == TF_TEAM_BLUE && TFGameRules()->IsCoOp() )
	{
		ChangeTeam( TF_TEAM_RED );
	}
	else if ( GetTeamNumber() == TF_TEAM_RED && TFGameRules()->IsBluCoOp() )
	{
		ChangeTeam( TF_TEAM_BLUE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::RegenThink( void )
{
	// Health drain attribute
	int iHealthDrain = 0;
	CALL_ATTRIB_HOOK_INT( iHealthDrain, add_health_regen );

	// Ammo attribute
	int iAmmoRegen = 0;
	CALL_ATTRIB_HOOK_INT( iAmmoRegen, addperc_ammo_regen );

	int iMetalRegen = 0;
	CALL_ATTRIB_HOOK_INT( iMetalRegen, add_metal_regen );

	if( IsAlive() )
	{
		if ( IsPlayerClass( TF_CLASS_MEDIC ) )
		{
			// Heal faster if we haven't been in combat for a while
			float flTimeSinceDamage = gpGlobals->curtime - GetLastDamageTime();
			float flScale = RemapValClamped( flTimeSinceDamage, 5, 10, 3.0, 6.0 );

			float flHealingMastery = 0;
			CALL_ATTRIB_HOOK_FLOAT( flHealingMastery, healing_mastery );
			if ( flHealingMastery )
			{
				float flScaleMaster = RemapValClamped( flTimeSinceDamage, 0.0f, 4.0f, 1.00f, 2.00f );
				flScale *= flScaleMaster;
			}

			int iHealAmount = ceil( TF_MEDIC_REGEN_AMOUNT * flScale );
			TakeHealth( iHealAmount + iHealthDrain, DMG_GENERIC );

			SetContextThink( &CTFPlayer::RegenThink, gpGlobals->curtime + TF_MEDIC_REGEN_TIME, "RegenThink" );
		}
		else
		{
			int iHealthRestored = TakeHealth( iHealthDrain, DMG_GENERIC );
			if ( iHealthRestored )
			{
				IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
				if ( event )
				{
					event->SetInt( "amount", iHealthRestored );
					event->SetInt( "entindex", entindex() );

					gameeventmanager->FireEvent( event );
				}
			}

			if ( iAmmoRegen )
			{
				for ( int iAmmo = 0; iAmmo < TF_AMMO_COUNT; ++iAmmo )
				{
					if ( ( iAmmo == TF_AMMO_METAL ) || ( iAmmo == TF_AMMO_GRENADES1 ) || ( iAmmo == TF_AMMO_GRENADES2 ) || ( iAmmo == TF_AMMO_GRENADES3 ) )
						continue;

					GiveAmmo( iAmmoRegen, iAmmo, false, TF_AMMO_SOURCE_RESUPPLY );
				}
			}

			if ( iMetalRegen )
			{
				GiveAmmo( iMetalRegen, TF_AMMO_METAL, false, TF_AMMO_SOURCE_RESUPPLY );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::RuneRegenThink( void )
{
	if( IsAlive() )
	{
		TakeHealth( 25, DMG_GENERIC );
		SetContextThink( &CTFPlayer::RuneRegenThink, gpGlobals->curtime + TF_RUNE_REGEN_TIME, "RuneRegenThink" );
	}
}

CTFPlayer::~CTFPlayer()
{
	DestroyRagdoll();

	if ( m_PlayerAnimState )
		m_PlayerAnimState->Release();

	RemoveGlowEffect();
}


CTFPlayer *CTFPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CTFPlayer::s_PlayerEdict = ed;
	return (CTFPlayer*)CreateEntityByName( className );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateTimers( void )
{
	m_Shared.ConditionThink();
	m_Shared.InvisibilityThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PreThink()
{
	// Update timers.
	UpdateTimers();

	// Pass through to the base class think.
	BaseClass::PreThink();

	if( m_hLocatorTargetEntity != NULL )
	{
		// Keep track of the entity here, the client will pick up the rest of the work
		m_vecLocatorOrigin = m_hLocatorTargetEntity->WorldSpaceCenter();
	}
	else
	{
		m_vecLocatorOrigin = vec3_invalid; // This tells the client we have no locator target.
	}

	if ( IsInAVehicle() )
	{
		// Skip most of the things if we're in a vehicle.
		UpdateTimers();
		CheckTimeBasedDamage();
		CheckSuitUpdate();
		WaterMove();

		m_vecTotalBulletForce = vec3_origin;

		CheckForIdle();
		return;
	}

	if ( m_nButtons & IN_GRENADE1 )
	{
		TFPlayerClassData_t *pData = m_PlayerClass.GetData();
		if ( !TFGameRules()->IsTFCAllowed() )
		{
			CTFWeaponBase *pGrenade = Weapon_OwnsThisID(pData->m_aGrenades[0]);
			if (pGrenade)
			{
				pGrenade->Deploy();
			}
		}
		else
		{
			if ( ( GetAmmoCount( LFE_AMMO_GRENADES1 ) > 0 ) && m_flNextGrenadesFire < gpGlobals->curtime )
			{
				ThrowGrenade( pData->m_aGrenades[0] );

				RemoveAmmo( 1, LFE_AMMO_GRENADES1 );
				m_flNextGrenadesFire = gpGlobals->curtime + 1;
			}
		}
	}

	if ( m_nButtons & IN_GRENADE2 )
	{
		TFPlayerClassData_t *pData = m_PlayerClass.GetData();
		if ( !TFGameRules()->IsTFCAllowed() )
		{
			CTFWeaponBase *pGrenade = Weapon_OwnsThisID(pData->m_aGrenades[1]);
			if (pGrenade)
			{
				pGrenade->Deploy();
			}
		}
		else
		{
			if ( ( GetAmmoCount( LFE_AMMO_GRENADES2 ) > 0 ) && m_flNextGrenadesFire < gpGlobals->curtime )
			{
				ThrowGrenade( pData->m_aGrenades[1] );

				RemoveAmmo( 1, LFE_AMMO_GRENADES2 );
				m_flNextGrenadesFire = gpGlobals->curtime + 1;
			}
		}
	}

	if ( m_afButtonPressed & IN_TAUNT )
	{
		if ( !IsTaunting() )
		{
			if ( IsAllowedToTaunt() )
			{
				IGameEvent *event = gameeventmanager->CreateEvent( "tauntmenupleaseshow" );
				if ( event )
				{
					event->SetInt( "userid", GetUserID() );
					gameeventmanager->FireEvent( event );
				}
				m_afButtonPressed &= ~IN_TAUNT;
			}
		}
		else
		{
			HandleTauntCommand( 0 );
		}
	}

	if ( m_afButtonPressed & IN_USE_ACTION )
		UseActionSlotItemPressed();

	if ( m_afButtonReleased & IN_USE_ACTION )
		UseActionSlotItemReleased();

	if ( m_afButtonPressed & IN_INSPECT )
		InspectButtonPressed();

	if ( m_afButtonReleased & IN_INSPECT )
		InspectButtonReleased();

	// Reset bullet force accumulator, only lasts one frame, for ragdoll forces from multiple shots.
	m_vecTotalBulletForce = vec3_origin;

	CommanderUpdate();

	CheckForIdle();

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
		AddFlag( FL_ONTRAIN );
	else 
		RemoveFlag( FL_ONTRAIN );

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
	{
		CBaseEntity *pTrain = GetGroundEntity();

		if ( pTrain )
		{
			if ( !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) )
				pTrain = NULL;
		}
		
		if ( !pTrain )
		{
			if ( GetActiveWeapon() && (GetActiveWeapon()->ObjectCaps() & FCAP_DIRECTIONAL_USE) )
			{
				m_iTrain = TRAIN_ACTIVE | TRAIN_NEW;

				if ( m_nButtons & IN_FORWARD )
				{
					m_iTrain |= TRAIN_FAST;
				}
				else if ( m_nButtons & IN_BACK )
				{
					m_iTrain |= TRAIN_BACK;
				}
				else
				{
					m_iTrain |= TRAIN_NEUTRAL;
				}
				return;
			}
			else
			{
				trace_t trainTrace;
				// Maybe this is on the other side of a level transition
				UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0,0,-38), 
					MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trainTrace );

				if ( trainTrace.fraction != 1.0 && trainTrace.m_pEnt )
					pTrain = trainTrace.m_pEnt;


				if ( !pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(this) )
				{
//					Warning( "In train mode with no train!\n" );
					m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
					m_iTrain = TRAIN_NEW|TRAIN_OFF;
					return;
				}
			}
		}
		else if ( !( GetFlags() & FL_ONGROUND ) || pTrain->HasSpawnFlags( SF_TRACKTRAIN_NOCONTROL ) || (m_nButtons & (IN_MOVELEFT|IN_MOVERIGHT) ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			return;
		}

		SetAbsVelocity( vec3_origin );
		int vel = 0;
		if ( m_afButtonPressed & IN_FORWARD )
		{
			vel = 1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}
		else if ( m_afButtonPressed & IN_BACK )
		{
			vel = -1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(((CFuncTrackTrain*)pTrain)->GetDesiredSpeed(), ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
			m_iTrain |= TRAIN_ACTIVE|TRAIN_NEW;
		}
	} 
	else if (m_iTrain & TRAIN_ACTIVE)
	{
		m_iTrain = TRAIN_NEW; // turn off train
	}

	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		bool bOnBarnacle = false;
		CNPC_Barnacle *pBarnacle = NULL;
		do
		{
			// FIXME: Not a good or fast solution, but maybe it will catch the bug!
			pBarnacle = (CNPC_Barnacle*)gEntList.FindEntityByClassname( pBarnacle, "npc_barnacle" );
			if ( pBarnacle )
			{
				if ( pBarnacle->GetEnemy() == this )
				{
					bOnBarnacle = true;
				}
			}
		} while ( pBarnacle );
		
		if ( !bOnBarnacle )
		{
			Warning( "Attached to barnacle?\n" );
			Assert( 0 );
			m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
		}
		else
		{
			SetAbsVelocity( vec3_origin );
		}
	}

	if( !IsAlive() )
	{
		// CSS would like their players to continue to update their LastArea even when dead since it is displayed in the observer screen now.
		// But we won't do the population tracking while dead.
		CNavArea *area = TheNavMesh->GetNavArea( GetAbsOrigin(), 1000 );
		if (area && area != m_lastNavArea)
		{
			m_lastNavArea = area;
			if ( area->GetPlace() != UNDEFINED_PLACE )
			{
				const char *placeName = TheNavMesh->PlaceToName( area->GetPlace() );
				if ( placeName && *placeName )
				{
					Q_strncpy( m_szLastPlaceName.GetForModify(), placeName, MAX_PLACE_NAME_LENGTH );
				}
			}
		}
	}

	CheckFlashlight();

	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );
	SetLocalAngles( vOldAngles );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveMeleeCrit( void )
{
	m_Shared.SetNextMeleeCrit( kCritType_None );
}

ConVar mp_idledealmethod( "mp_idledealmethod", "1", FCVAR_GAMEDLL, "Deals with Idle Players. 1 = Sends them into Spectator mode then kicks them if they're still idle, 2 = Kicks them out of the game;" );
ConVar mp_idlemaxtime( "mp_idlemaxtime", "3", FCVAR_GAMEDLL, "Maximum time a player is allowed to be idle (in minutes)" );

void CTFPlayer::CheckForIdle( void )
{
	if ( m_afButtonLast != m_nButtons )
		m_flLastAction = gpGlobals->curtime;

	if ( mp_idledealmethod.GetInt() )
	{
		if ( IsHLTV() )
			return;

		if ( IsFakeClient() )
			return;

		//Don't mess with the host on a listen server (probably one of us debugging something)
		if ( engine->IsDedicatedServer() == false && entindex() == 1 )
			return;

		if ( m_bIsIdle == false )
		{
			if ( StateGet() == TF_STATE_OBSERVER || StateGet() != TF_STATE_ACTIVE )
				return;
		}
		
		float flIdleTime = mp_idlemaxtime.GetFloat() * 60;

		if ( TFGameRules()->InStalemate() )
		{
			flIdleTime = mp_stalemate_timelimit.GetInt() * 0.5f;
		}
		
		if ( (gpGlobals->curtime - m_flLastAction) > flIdleTime  )
		{
			bool bKickPlayer = false;

			ConVarRef mp_allowspectators( "mp_allowspectators" );
			if ( mp_allowspectators.IsValid() && ( mp_allowspectators.GetBool() == false ) )
			{
				// just kick the player if this server doesn't allow spectators
				bKickPlayer = true;
			}
			else if ( mp_idledealmethod.GetInt() == 1 )
			{
				//First send them into spectator mode then kick him.
				if ( m_bIsIdle == false )
				{
					ForceChangeTeam( TEAM_SPECTATOR );
					m_flLastAction = gpGlobals->curtime;
					m_bIsIdle = true;
					return;
				}
				else
				{
					bKickPlayer = true;
				}
			}
			else if ( mp_idledealmethod.GetInt() == 2 )
			{
				bKickPlayer = true;
			}

			if ( bKickPlayer == true )
			{
				UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "#game_idle_kick", GetPlayerName() );
				engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", GetUserID() ) );
				m_flLastAction = gpGlobals->curtime;
			}
		}
	}
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CTFPlayer::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTFPlayer::FlashlightTurnOn( void )
{
	if ( flashlight.GetInt() < 0 )
		return;

	if ( !IsAlive() )
		return;

	if( m_bFlashlightDisabled )
		return;

	if ( IsTaunting() )
		return;
	
	if ( m_Shared.IsInCutScene() )
		return;

	m_Shared.AddCond( LFE_COND_FLASHLIGHT );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTFPlayer::FlashlightTurnOff( void )
{
	m_Shared.RemoveCond( LFE_COND_FLASHLIGHT );
	RemoveEffects( EF_DIMLIGHT );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define FLASHLIGHT_RANGE	Square(600)
bool CTFPlayer::IsIlluminatedByFlashlight( CBaseEntity *pEntity, float *flReturnDot )
{
	if( !FlashlightIsOn() )
		return false;

	if( pEntity->Classify() == CLASS_BARNACLE && pEntity->GetEnemy() == this )
	{
		// As long as my flashlight is on, the barnacle that's pulling me in is considered illuminated.
		// This is because players often shine their flashlights at Alyx when they are in a barnacle's 
		// grasp, and wonder why Alyx isn't helping. Alyx isn't helping because the light isn't pointed
		// at the barnacle. This will allow Alyx to see the barnacle no matter which way the light is pointed.
		return true;
	}

	// Within 50 feet?
	float flDistSqr = GetAbsOrigin().DistToSqr(pEntity->GetAbsOrigin());
	if( flDistSqr > FLASHLIGHT_RANGE )
		return false;

	// Within 45 degrees?
	Vector vecSpot = pEntity->WorldSpaceCenter();
	Vector los;

	// If the eyeposition is too close, move it back. Solves problems
	// caused by the player being too close the target.
	if ( flDistSqr < (128 * 128) )
	{
		Vector vecForward;
		EyeVectors( &vecForward );
		Vector vecMovedEyePos = EyePosition() - (vecForward * 128);
		los = ( vecSpot - vecMovedEyePos );
	}
	else
	{
		los = ( vecSpot - EyePosition() );
	}

	VectorNormalize( los );
	Vector facingDir = EyeDirection3D( );
	float flDot = DotProduct( los, facingDir );

	if ( flReturnDot )
	{
		 *flReturnDot = flDot;
	}

	if ( flDot < 0.92387f )
		return false;

	if( !FVisible(pEntity) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Let NPCs know when the flashlight is trained on them
//-----------------------------------------------------------------------------
void CTFPlayer::CheckFlashlight( void )
{
	if ( !FlashlightIsOn() )
		return;

	if ( m_flNextFlashlightCheckTime > gpGlobals->curtime )
		return;

	m_flNextFlashlightCheckTime = gpGlobals->curtime + FLASHLIGHT_NPC_CHECK_INTERVAL;

	// Loop through NPCs looking for illuminated ones
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		CAI_BaseNPC *pNPC = g_AI_Manager.AccessAIs()[i];

		float flDot;

		if ( IsIlluminatedByFlashlight( pNPC, &flDot ) )
		{
			pNPC->PlayerHasIlluminatedNPC( this, flDot );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PostThink()
{
	BaseClass::PostThink();

	ProcessSceneEvents();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	if ( m_PlayerAnimState )
		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	if ( m_flTauntAttackTime > 0.0f && m_flTauntAttackTime < gpGlobals->curtime )
	{
		m_flTauntAttackTime = 0.0f;
		DoTauntAttack();
	}

	// Check if player is typing.
	m_bTyping = ( m_nButtons & IN_TYPING ) != 0;

	UpdatePortalPlaneSounds();
	UpdateWooshSounds();

	UpdateSecondsTaken();

	// Try to fix the player if they're stuck
	if ( m_bStuckOnPortalCollisionObject )
	{
		Vector vForward = ((CProp_Portal*)m_hPortalEnvironment.Get())->m_vPrevForward;
		Vector vNewPos = GetAbsOrigin() + vForward * gpGlobals->frametime * -1000.0f;
		Teleport( &vNewPos, NULL, &vForward );
		m_bStuckOnPortalCollisionObject = false;
	}

	/*if ( m_Shared.InCond( TF_COND_BLASTJUMPING ) )
	{		
		if ( m_Local.m_flFallVelocity > 350 )
		{
			EmitSound( "BlastJump.Whistle" );
		}
	}*/
}

void CTFPlayer::UpdatePortalPlaneSounds( void )
{
	CProp_Portal *pPortal = m_hPortalEnvironment;
	if ( pPortal && pPortal->m_bActivated )
	{
		Vector vVelocity;
		GetVelocity( &vVelocity, NULL );

		if ( !vVelocity.IsZero() )
		{
			Vector vMin, vMax;
			CollisionProp()->WorldSpaceAABB( &vMin, &vMax );

			Vector vEarCenter = ( vMax + vMin ) / 2.0f;
			Vector vDiagonal = vMax - vMin;

			if ( !m_bIntersectingPortalPlane )
			{
				vDiagonal *= 0.25f;

				if ( UTIL_IsBoxIntersectingPortal( vEarCenter, vDiagonal, pPortal ) )
				{
					m_bIntersectingPortalPlane = true;

					CPASAttenuationFilter filter( this );
					CSoundParameters params;
					if ( GetParametersForSound( "PortalPlayer.EnterPortal", params, NULL ) )
					{
						EmitSound_t ep( params );
						ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
						ep.m_flVolume = min( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

						EmitSound( filter, entindex(), ep );
					}
				}
			}
			else
			{
				vDiagonal *= 0.30f;

				if ( !UTIL_IsBoxIntersectingPortal( vEarCenter, vDiagonal, pPortal ) )
				{
					m_bIntersectingPortalPlane = false;

					CPASAttenuationFilter filter( this );
					CSoundParameters params;
					if ( GetParametersForSound( "PortalPlayer.ExitPortal", params, NULL ) )
					{
						EmitSound_t ep( params );
						ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
						ep.m_flVolume = min( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

						EmitSound( filter, entindex(), ep );
					}
				}
			}
		}
	}
	else if ( m_bIntersectingPortalPlane )
	{
		m_bIntersectingPortalPlane = false;

		CPASAttenuationFilter filter( this );
		CSoundParameters params;
		if ( GetParametersForSound( "PortalPlayer.ExitPortal", params, NULL ) )
		{
			EmitSound_t ep( params );
			Vector vVelocity;
			GetVelocity( &vVelocity, NULL );
			ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
			ep.m_flVolume = min( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

			EmitSound( filter, entindex(), ep );
		}
	}
}

void CTFPlayer::UpdateWooshSounds( void )
{
	if ( m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		float fWooshVolume = GetAbsVelocity().Length() - MIN_FLING_SPEED;

		if ( fWooshVolume < 0.0f )
		{
			controller.SoundChangeVolume( m_pWooshSound, 0.0f, 0.1f );
			return;
		}

		fWooshVolume /= 2000.0f;
		if ( fWooshVolume > 1.0f )
			fWooshVolume = 1.0f;

		controller.SoundChangeVolume( m_pWooshSound, fWooshVolume, 0.1f );
		//		controller.SoundChangePitch( m_pWooshSound, fWooshVolume + 0.5f, 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Precache()
{
	// Precache the player models and gibs.
	PrecachePlayerModels();

	// Precache the player sounds.
	PrecacheScriptSound( "Player.Spawn" );
	PrecacheScriptSound( "TFPlayer.Pain" );
	PrecacheScriptSound( "TFPlayer.CritHit" );
	PrecacheScriptSound( "TFPlayer.CritPain" );
	PrecacheScriptSound( "TFPlayer.CritDeath" );
	PrecacheScriptSound( "TFPlayer.FreezeCam" );
	PrecacheScriptSound( "TFPlayer.Drown" );
	PrecacheScriptSound( "TFPlayer.AttackerPain" );
	PrecacheScriptSound( "TFPlayer.SaveMe" );
	PrecacheScriptSound( "Camera.SnapShot" );
	PrecacheScriptSound( "TFPlayer.ReCharged" );
	PrecacheScriptSound( "TFPlayer.Dissolve" );
	PrecacheScriptSound( "TFPlayer.DoubleDonk" );
	PrecacheScriptSound( "TFPlayer.CritHitMini" );
	PrecacheScriptSound( "Player.Spy_UnCloakFeignDeath" );
	PrecacheScriptSound( "TFPlayer.StunImpact" );
	PrecacheScriptSound( "TFPlayer.StunImpactRange" );
	PrecacheScriptSound( "Weapon_Mantreads.Impact" );
	PrecacheScriptSound( "General.banana_slip" );
	PrecacheScriptSound( "Icicle.Melt" );
	PrecacheScriptSound( "Parachute_open" );
	PrecacheScriptSound( "Parachute_close" );
	PrecacheScriptSound( "Passtime.AskForBall" );
	PrecacheScriptSound( "Taunt.GuitarRiff" );
	PrecacheScriptSound( "Taunt.MedicHeroic" );

	PrecacheScriptSound( "HL2Player.FlashlightOn" );
	PrecacheScriptSound( "HL2Player.FlashlightOff" );

	PrecacheScriptSound( "Game.YourTeamLost" );
	PrecacheScriptSound( "Game.YourTeamWon" );
	PrecacheScriptSound( "Game.SuddenDeath" );
	PrecacheScriptSound( "Game.Stalemate" );
	PrecacheScriptSound( "TV.Tune" );
	PrecacheScriptSound( "Halloween.PlayerScream" );
	PrecacheScriptSound( "Player.KillSoundDefaultDing" );

	PrecacheScriptSound( "DemoCharge.ChargeCritOn" );
	PrecacheScriptSound( "DemoCharge.ChargeCritOff" );
	PrecacheScriptSound( "DemoCharge.Charging" );

	PrecacheScriptSound( "Achievement.Earned" );
	PrecacheScriptSound( "Hud.AchievementIncremented" );
	PrecacheScriptSound( "Quest.StatusTickNovice" );
	PrecacheScriptSound( "Quest.StatusTickAdvanced" );
	PrecacheScriptSound( "Quest.StatusTickExpert" );
	PrecacheScriptSound( "Quest.StatusTickNoviceComplete" );
	PrecacheScriptSound( "Quest.StatusTickNoviceCompletePDA" );
	PrecacheScriptSound( "Quest.StatusTickAdvancedCompletePDA" );
	PrecacheScriptSound( "Quest.StatusTickExpertCompletePDA" );
	PrecacheScriptSound( "Quest.TurnInAccepted" );
	PrecacheScriptSound( "Quest.TurnInAcceptedLight" );

	PrecacheScriptSound( "DemoCharge.ChargeCritOn" );
	PrecacheScriptSound( "DemoCharge.ChargeCritOff" );
	PrecacheScriptSound( "DemoCharge.Charging" );
	
	PrecacheScriptSound( "MVM_Weapon_Default.HitFlesh" );
	PrecacheScriptSound( "MVM.FallDamageBots" );
	PrecacheScriptSound( "MVM.BotStep" );
	PrecacheScriptSound( "MVM.GiantHeavyStep" );
	PrecacheScriptSound( "MVM.GiantSoldierStep" );
	PrecacheScriptSound( "MVM.GiantDemomanStep" );
	PrecacheScriptSound( "MVM.GiantScoutStep" );
	PrecacheScriptSound( "MVM.GiantScoutStep" );
	PrecacheScriptSound( "MVM.SentryBusterStep" );

	PrecacheScriptSound( "NPC_Combine.GrenadeLaunch" );
	PrecacheScriptSound( "NPC_Combine.WeaponBash" );

	// Precache particle systems
	PrecacheParticleSystem( "crit_text" );
	PrecacheParticleSystem( "minicrit_text" );
	PrecacheParticleSystem( "stomp_text" );
	PrecacheParticleSystem( "doubledonk_text" );
	PrecacheParticleSystem( "cig_smoke" );
	PrecacheParticleSystem( "speech_mediccall" );
	PrecacheParticleSystem( "speech_mediccall_auto" );
	PrecacheParticleSystem( "speech_taunt_red" );
	PrecacheParticleSystem( "speech_taunt_blue" );
	PrecacheParticleSystem( "speech_taunt_all" );
	PrecacheParticleSystem( "blood_spray_red_01" );
	PrecacheParticleSystem( "blood_spray_red_01_far" );
	PrecacheParticleSystem( "water_blood_impact_red_01" );
	PrecacheParticleSystem( "blood_impact_red_01" );
	PrecacheParticleSystem( "water_playerdive" );
	PrecacheParticleSystem( "water_playeremerge" );
	PrecacheParticleSystem( "rocketjump_smoke" );
	PrecacheParticleSystem( "speech_typing" );
	PrecacheParticleSystem( "eye_powerup_green_lvl_1" );
	PrecacheParticleSystem( "eye_powerup_green_lvl_2" );
	PrecacheParticleSystem( "eye_powerup_green_lvl_3" );
	PrecacheParticleSystem( "eye_powerup_green_lvl_4" );
	PrecacheParticleSystem( "peejar_drips" );
	PrecacheParticleSystem( "peejar_drips_milk" );
	PrecacheParticleSystem( "flashlight_thirdperson" );
	PrecacheParticleSystem( "flashlight_firstperson_" );
	PrecacheParticleSystem( "speed_boost_trail" );
	PrecacheParticleSystem( "sapper_sentry1_fx" );
	PrecacheParticleSystem( "powerup_plague_carrier" );
	PrecacheParticleSystem( "mvm_pow_gold_seq_firework_mid" );
	PrecacheParticleSystem( "god_rays" );
	PrecacheParticleSystem( "bl_killtaunt" );

	PrecacheTeamParticles( "healthgained_%s_large", false, g_aTeamNamesShort );
	PrecacheTeamParticles( "player_recent_teleport_%s" );
	PrecacheTeamParticles( "particle_nemesis_%s" );
	PrecacheTeamParticles( "spy_start_disguise_%s" );
	PrecacheTeamParticles( "burningplayer_%s" );
	PrecacheTeamParticles( "critgun_weaponmodel_%s", true, g_aTeamNamesShort );
	PrecacheTeamParticles( "critgun_weaponmodel_%s_glow", true, g_aTeamNamesShort );
	PrecacheTeamParticles( "healthlost_%s", false, g_aTeamNamesShort );
	PrecacheTeamParticles( "healthgained_%s", false, g_aTeamNamesShort );
	PrecacheTeamParticles( "overhealedplayer_%s_pluses" );
	PrecacheTeamParticles( "healhuff_%s", false, g_aTeamNamesShort );
	PrecacheTeamParticles( "soldierbuff_%s_buffed", false );
	PrecacheTeamParticles( "medic_healradius_%s_buffed" );
	PrecacheTeamParticles( "medic_megaheal_%s" );
	PrecacheTeamParticles( "electrocuted_%s" );
	PrecacheTeamParticles( "electrocuted_gibbed_%s" );
	PrecacheTeamParticles( "kart_dust_trail_%s" );
	PrecacheTeamParticles( "highfive_%s", false );

	PrecacheScriptSound( "DisciplineDevice.PowerUp" );
	PrecacheScriptSound( "DisciplineDevice.PowerDown" );
	PrecacheScriptSound( "Building_Speedpad.BoostStop" );
	PrecacheScriptSound( "MVM.PlayerRevived" );

	PrecacheScriptSound( "Weapon_Hands.Push" );
	PrecacheScriptSound( "Weapon_Hands.PushImpact" );

	PrecacheScriptSound( "PortalPlayer.EnterPortal" );
	PrecacheScriptSound( "PortalPlayer.ExitPortal" );

	PrecacheScriptSound( "Player.SpawnTFC" );
	PrecacheScriptSound( "Player.JumpTFC" );

	PrecacheScriptSound( "PortalPlayer.Woosh" );
	PrecacheScriptSound( "PortalPlayer.FallRecover" );

	PrecacheModel ( "sprites/glow01.vmt" );

	PrecacheScriptSound( "TFPlayer.Decapitated" );

	PrecacheScriptSound( "Powerup.PickUpAgility" );
	PrecacheScriptSound( "Powerup.PickUpHaste" );
	PrecacheScriptSound( "Powerup.PickUpKing" );
	PrecacheScriptSound( "Powerup.PickUpKnockout" );
	PrecacheScriptSound( "Powerup.PickUpPlague" );
	PrecacheScriptSound( "Powerup.PickUpPlagueInfected" );
	PrecacheScriptSound( "Powerup.PickUpPlagueInfectedLoop" );
	PrecacheScriptSound( "Powerup.PickUpPrecision" );
	PrecacheScriptSound( "Powerup.PickUpReflect" );
	PrecacheScriptSound( "Powerup.PickUpRegeneration" );
	PrecacheScriptSound( "Powerup.PickUpResistance" );
	PrecacheScriptSound( "Powerup.PickUpStrength" );
	PrecacheScriptSound( "Powerup.PickUpSupernova" );
	PrecacheScriptSound( "Powerup.PickUpSupernovaActivate" );
	PrecacheScriptSound( "Powerup.PickUpTemp.Crit" );
	PrecacheScriptSound( "Powerup.PickUpTemp.Uber" );
	PrecacheScriptSound( "Powerup.PickUpVampire" );
	PrecacheScriptSound( "Powerup.ReducedDamage" );
	PrecacheScriptSound( "Powerup.Reflect.Reflect" );

	PrecacheScriptSound( "TFCPlayer.Death" );
	PrecacheScriptSound( "ui.cratesmash_ultrarare_short" );

	for ( int i = 0; i < TF_RUNE_COUNT; i++ )
	{
		PrecacheTeamParticles( s_pszRuneIcons[i] );
	}

	PrecacheParticleSystem( "tfc_sniper_mist" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the player models and player model gibs.
//-----------------------------------------------------------------------------
void CTFPlayer::PrecachePlayerModels( void )
{
	int i;
	for ( i = 0; i < TF_CLASS_COUNT_ALL; i++ )
	{
		const char *pszModel = GetPlayerClassData( i )->m_szModelName;
		if ( pszModel && pszModel[0] )
		{
			int iModel = PrecacheModel( pszModel );
			PrecacheGibsForModel( iModel );
		}

		// Precache the hardware facial morphed models as well.
		const char *pszHWMModel = GetPlayerClassData( i )->m_szHWMModelName;
		if ( pszHWMModel && pszHWMModel[0] )
		{
			PrecacheModel( pszHWMModel );
		}

		const char *pszHandModel = GetPlayerClassData(i)->m_szModelHandsName;
		if ( pszHandModel && pszHandModel[0] )
		{
			PrecacheModel( pszHandModel );
		}

		// Precache the classics
		const char *pszTFCModel = GetPlayerClassData( i )->m_szTFCModelName;
		if ( pszTFCModel && pszTFCModel[0] )
		{
			PrecacheModel( pszTFCModel );
		}

		// Precache the bots variant
		const char *pszBotModel = GetPlayerClassData( i )->m_szBotModelName;
		if ( pszBotModel && pszBotModel[0] )
		{
			PrecacheModel( pszBotModel );
		}
	}
	
	for ( i = 1; i < ARRAYSIZE( g_pszBDayGibs ); i++ )
	{
		PrecacheModel( g_pszBDayGibs[i] );
	}
	PrecacheModel( "models/effects/bday_hat.mdl" );

	PrecacheModel( "models/props_trainyard/bomb_cart_red.mdl" );
	PrecacheModel( "models/props_trainyard/bomb_cart.mdl" );

	PrecacheModel( "models/props_halloween/ghost_no_hat.mdl" );
	PrecacheModel( "models/props_halloween/ghost_no_hat_red.mdl" );

	PrecacheModel( TF_AMMOPACK_MODEL );
	PrecacheModel( TF_REVIVEMARKER_MODEL );

	// Gunslinger
	PrecacheModel("models/weapons/c_models/c_engineer_gunslinger.mdl");

	// Precache player class sounds
	for ( i = TF_FIRST_NORMAL_CLASS; i < TF_CLASS_COUNT_ALL; ++i )
	{
		TFPlayerClassData_t *pData = GetPlayerClassData( i );

		PrecacheScriptSound( pData->m_szDeathSound );
		PrecacheScriptSound( pData->m_szCritDeathSound );
		PrecacheScriptSound( pData->m_szMeleeDeathSound );
		PrecacheScriptSound( pData->m_szExplosionDeathSound );
	}
}

void CTFPlayer::CreateSounds()
{
	if ( !m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		CPASAttenuationFilter filter( this );

		m_pWooshSound = controller.SoundCreate( filter, entindex(), "PortalPlayer.Woosh" );
		controller.Play( m_pWooshSound, 0, 100 );
	}
}

void CTFPlayer::StopLoopingSounds()
{
	if ( m_sndLeeches )
	{
		 ( CSoundEnvelopeController::GetController() ).SoundDestroy( m_sndLeeches );
		 m_sndLeeches = NULL;
	}

	if ( m_sndWaterSplashes )
	{
		 ( CSoundEnvelopeController::GetController() ).SoundDestroy( m_sndWaterSplashes );
		 m_sndWaterSplashes = NULL;
	}

	if ( m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy( m_pWooshSound );
		m_pWooshSound = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsReadyToPlay( void )
{
	return ( ( GetTeamNumber() > LAST_SHARED_TEAM ) &&
			 ( GetDesiredPlayerClassIndex() > TF_CLASS_UNDEFINED ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsReadyToSpawn( void )
{
	if ( IsClassMenuOpen() )
	{
		return false;
	}

	if ( ShouldUseCoopSpawning() )
	{
		if ( m_bSearchingSpawn || m_Shared.GetLivesCount() == 0 || m_bInTransition )
			return false;
	}

	return ( StateGet() != TF_STATE_DYING );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this player should be allowed to instantly spawn
//			when they next finish picking a class.
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldGainInstantSpawn( void )
{
	if ( ShouldUseCoopSpawning() )
	{
		if ( m_bSearchingSpawn || m_Shared.GetLivesCount() == 0 || m_bInTransition )
			return false;
	}

	return ( GetPlayerClass()->GetClassIndex() == TF_CLASS_UNDEFINED || IsClassMenuOpen() );
}

//-----------------------------------------------------------------------------
// Purpose: Resets player scores
//-----------------------------------------------------------------------------
void CTFPlayer::ResetScores( void )
{
	CTF_GameStats.ResetPlayerStats( this );
	RemoveNemesisRelationships();
	BaseClass::ResetScores();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::InitialSpawn( void )
{
	BaseClass::InitialSpawn();

	m_AttributeManager.InitializeAttributes( this );
	m_AttributeManager.m_hPlayer = this;

	m_AttributeList.SetManager( &m_AttributeManager );

	SetWeaponBuilder( NULL );

	m_iMaxSentryKills = 0;
	CTF_GameStats.Event_MaxSentryKills( this, 0 );

	// Set initial lives count.
	m_Shared.SetLivesCount( lfe_player_lives.GetInt() );

	StateEnter( TF_STATE_WELCOME );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Spawn()
{
	MDLCACHE_CRITICAL_SECTION();

	m_flSpawnTime = gpGlobals->curtime;
	UpdateModel();

	SetMoveType( MOVETYPE_WALK );

	BaseClass::Spawn();

	if ( m_hTempSpawnSpot.Get() )
	{
		// Nuke our temp spawn spot now that we've used it.
		UTIL_Remove( m_hTempSpawnSpot );
		m_hTempSpawnSpot = NULL;
	}

	if ( m_hReviveSpawnSpot.Get() )
	{
		// Nuke our revive marker now that we spawned.
		UTIL_Remove( m_hReviveSpawnSpot );
		m_hReviveSpawnSpot = NULL;
	}

	// Create our off hand viewmodel if necessary
	CreateViewModel( 1 );
	// Do it for one extra wearable too (DISABLED, caused weird issues it seems)
	//CreateViewModel(2);
	// Make sure it has no model set, in case it had one before
	GetViewModel( 1 )->SetWeaponModel( NULL, NULL );
	//GetViewModel(2)->SetWeaponModel(NULL, NULL);

	m_Shared.SetDemoShieldEquipped( false );
	m_Shared.SetParachuteEquipped( false );

	Q_snprintf( m_iszCustomModel.GetForModify(), MAX_PATH, "" );

	// Kind of lame, but CBasePlayer::Spawn resets a lot of the state that we initially want on.
	// So if we're in the welcome state, call its enter function to reset 
	if ( m_Shared.InState( TF_STATE_WELCOME ) )
	{
		StateEnterWELCOME();
	}

	// If they were dead, then they're respawning. Put them in the active state.
	if ( m_Shared.InState( TF_STATE_DYING ) )
	{
		StateTransition( TF_STATE_ACTIVE );
	}

	// If they're spawning into the world as fresh meat, give them items and stuff.
	if ( m_Shared.InState( TF_STATE_ACTIVE ) )
	{
		// remove our disguise each time we spawn
		if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		{
			m_Shared.RemoveDisguise();
		}

		if ( !TFGameRules()->IsTFCAllowed() )
			EmitSound( "Player.Spawn" );
		else
			EmitSound( "Player.SpawnTFC" );

		m_Shared.RemoveAllCond(); // Remove conc'd, burning, rotting, hallucinating, etc.
		InitClass();

		CBaseEntity	*pMapAddEntity = NULL;
		while ( ( pMapAddEntity = gEntList.FindEntityByClassname( pMapAddEntity, "lfe_mapadd_player" ) ) != NULL )
		{
			pMapAddEntity->Touch( this );
		}

		UpdateSkin( GetTeamNumber() );
		TeamFortress_SetSpeed();

		// Prevent firing for a second so players don't blow their faces off
		SetNextAttack( gpGlobals->curtime + 1.0 );

		DoAnimationEvent( PLAYERANIMEVENT_SPAWN );

		// turn on separation so players don't get stuck in each other when spawned
		m_Shared.SetSeparation( true );
		m_Shared.SetSeparationVelocity( vec3_origin );

		RemoveTeleportEffect();

		//If this is true it means I respawned without dying (changing class inside the spawn room) but doesn't necessarily mean that my healers have stopped healing me
		//This means that medics can still be linked to me but my health would not be affected since this condition is not set.
		//So instead of going and forcing every healer on me to stop healing we just set this condition back on. 
		//If the game decides I shouldn't be healed by someone (LOS, Distance, etc) they will break the link themselves like usual.
		if ( m_Shared.GetNumHealers() > 0 )
		{
			m_Shared.AddCond( TF_COND_HEALTH_BUFF );
		}

		if ( !m_bSeenRoundInfo )
		{
			TFGameRules()->ShowRoundInfoPanel( this );
			m_bSeenRoundInfo = true;
		}

		if ( IsInCommentaryMode() && !IsFakeClient() )
		{
			// Player is spawning in commentary mode. Tell the commentary system.
			CBaseEntity *pEnt = NULL;
			variant_t emptyVariant;
			while ( (pEnt = gEntList.FindEntityByClassname( pEnt, "commentary_auto" )) != NULL )
			{
				pEnt->AcceptInput( "MultiplayerSpawned", this, this, emptyVariant, 0 );
			}
		}

		m_bTransitioned = true;
	}

	m_purgatoryDuration.Invalidate();

	m_Shared.SetDesiredWeaponIndex( -1 );

	m_nForceTauntCam = 0;

	CTF_GameStats.Event_PlayerSpawned( this );

	m_iSpawnCounter++;
	m_bAllowInstantSpawn = false;

	m_Shared.ClearDamageEvents();
	ClearDamagerHistory();

	m_flLastDamageTime = 0;
	m_flNextNoiseMakerTime = gpGlobals->curtime;

	m_flNextVoiceCommandTime = gpGlobals->curtime;

	ClearZoomOwner();
	SetFOV( this , 0 );

	SetViewOffset( GetClassEyeHeight() );

	ClearExpression();
	m_flNextSpeakWeaponFire = gpGlobals->curtime;

	m_bIsIdle = false;
	m_flPowerPlayTime = 0.0f;

	m_nBlastJumpFlags = 0;

	m_Shared.m_hUrineAttacker = NULL;
	m_Shared.m_bFeignDeathReady = false;

	GetPlayerProxy();

	m_flNextGrenadesFire = gpGlobals->curtime;

	m_bAllowMoveDuringTaunt = false;
	m_bIsReadyToHighFive = false;
	m_flCurrentTauntMoveSpeed = 0.0f;

	m_Shared.m_bIsPlayerADev = PlayerHasPowerplay(); //same list of people

	m_Shared.RecalculateChargeEffects( true );

	// This makes the surrounding box always the same size as the standing collision box
	// helps with parts of the hitboxes that extend out of the crouching hitbox, eg with the
	// heavyweapons guy
	Vector mins = VEC_HULL_MIN;
	Vector maxs = VEC_HULL_MAX;
	CollisionProp()->SetSurroundingBoundsType( USE_SPECIFIED_BOUNDS, &mins, &maxs );

	// Hack to hide the chat on the background map.
	if ( gpGlobals->eLoadType == MapLoad_Background )
	{
		m_Local.m_iHideHUD |= HIDEHUD_CHAT;
	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_spawn" );
	if ( event )
	{
		event->SetInt( "userid", GetUserID() );
		event->SetInt( "team", GetTeamNumber() );
		event->SetInt( "class", GetPlayerClass()->GetClassIndex() );
		gameeventmanager->FireEvent( event );
	}

	// Reset meters
	m_Shared.ResetRageSystem();
	m_Shared.SetScoutHypeMeter( 0.0f );
	m_Shared.SetShieldChargeMeter( 100.0f );
	m_Shared.SetSpyCloakMeter( 100.0f );
	m_Shared.SetDecapitationCount( 0 );

	m_Shared.SetFlashlightBattery( 100.0f );
	SetFlashlightPowerDrainScale( 1.0f );

	const char *pchSquadName = ( gpGlobals->maxClients > 1 ) ? UTIL_VarArgs( "%s_%i", PLAYER_SQUADNAME, GetClientIndex()) : PLAYER_SQUADNAME;
	m_pPlayerAISquad = g_AI_SquadManager.FindCreateSquad(AllocPooledString(pchSquadName));
	m_pPlayerAISquad->SetPlayerCommander( this );

	if ( m_bTransitioned )
	{
		LoadSavedTransition();
		m_bTransitioned = false;	
	}

	if ( TFGameRules()->IsPowerupMode() )
		m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF, 5 );

	m_Shared.AddCond( TF_COND_TEAM_GLOWS, tf_spawn_glows_duration.GetInt() );

	if ( TFGameRules()->IsVersus() )
	{
		if ( GetTeamNumber() == TF_TEAM_RED )
		{
			if ( m_Shared.GetLivesCount() != 1 )
				m_Shared.SetLivesCount( lfe_player_lives.GetInt() );
		}
		else if ( GetTeamNumber() == TF_TEAM_BLUE )
		{
			m_Shared.AddCond( TF_COND_STEALTHED, 3 );
		}
	}

	if ( TFGameRules()->IsInfectionMode() && GetTeamNumber() == TF_TEAM_BLUE )
		m_Shared.AddCond( LFE_COND_ZOMBIE_SPAWN, 3 );

	if ( TFGameRules()->IsInfectionMode() && ( TFGameRules()->InSetup() || TFGameRules()->State_Get() == GR_STATE_PREROUND ) )
	{
		if ( GetDesiredPlayerClassIndex() != TF_CLASS_CIVILIAN )
			SetDesiredPlayerClassIndex( TF_CLASS_CIVILIAN );

		if ( GetTeamNumber() != TF_TEAM_RED )
			ChangeTeam( TF_TEAM_RED );
	}

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateLocatorPosition( const Vector &vecPosition )
{
	m_vecLocatorOrigin = vecPosition;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFPlayer::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	CTFPlayer *pPlayer = ToTFPlayer( CBaseEntity::Instance( pInfo->m_pClientEnt ) );
	Assert( pPlayer );

	// Always transmit all players to us if we're in spec.
	if ( pPlayer->GetTeamNumber() < FIRST_GAME_TEAM )
	{
		return FL_EDICT_ALWAYS;
	}
	else if ( InSameTeam( pPlayer ) && !pPlayer->IsAlive() )
	{
		// Transmit teammates to us if we're dead.
		return FL_EDICT_ALWAYS;
	}

	return BaseClass::ShouldTransmit( pInfo );
}

//-----------------------------------------------------------------------------
// Purpose: Removes all nemesis relationships between this player and others
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveNemesisRelationships()
{
	for ( int i = 1 ; i <= gpGlobals->maxClients ; i++ )
	{
		CTFPlayer *pTemp = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pTemp && pTemp != this )
		{
			// set this player to be not dominating anyone else
			m_Shared.SetPlayerDominated( pTemp, false );

			// set no one else to be dominating this player
			pTemp->m_Shared.SetPlayerDominated( this, false );
		}
	}	
	// reset the matrix of who has killed whom with respect to this player
	CTF_GameStats.ResetKillHistory( this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::Regenerate( void )
{
	// We may have been boosted over our max health. If we have, 
	// restore it after we reset out class values.
	int iCurrentHealth = GetHealth();
	m_bRegenerating = true;
	InitClass();
	m_bRegenerating = false;
	if ( iCurrentHealth > GetHealth() )
	{
		SetHealth( iCurrentHealth );
	}

	// Remove burning condition
	if ( m_Shared.InCond( TF_COND_BURNING ) )
		m_Shared.RemoveCond( TF_COND_BURNING );

	// Remove bleeding condition
	if ( m_Shared.InCond( TF_COND_BLEEDING ) )
		m_Shared.RemoveCond( TF_COND_BLEEDING );

	// Remove liquid condition
	if ( m_Shared.IsJared() )
	{
		m_Shared.RemoveCond( TF_COND_URINE );
		m_Shared.RemoveCond( TF_COND_MAD_MILK );
		m_Shared.RemoveCond( TF_COND_GAS );
	}

	// Remove bonk condition
	if ( m_Shared.InCond( TF_COND_PHASE ) )
		m_Shared.RemoveCond( TF_COND_PHASE );

	// Fill Spy cloak
	m_Shared.SetSpyCloakMeter( 100.0f );

	// Reset charge meter
	m_Shared.SetShieldChargeMeter( 100.0f );

	/*for ( int iSlot = 0; iSlot < LOADOUT_POSITION_BUFFER; ++iSlot )
	{
		int nItemMeterDenied = 0;
		CALL_ATTRIB_HOOK_INT( nConsumeCloak, item_meter_resupply_denied );
		if ( nItemMeterDenied != 0 )
			m_Shared.SetItemChargeMeter( iSlot, GetDefaultItemChargeMeterValue() );
	}*/
	
	TFPlayerClassData_t *pData = m_PlayerClass.GetData();

	//float flCustomScale = 1;
	//CALL_ATTRIB_HOOK_FLOAT(flCustomScale, player_model_scale);
	//SetModelScale(flCustomScale);

	// Give shared weapon if allowed.
	if ( lfe_allow_team_weapons.GetBool() )
		ManageTeamWeapons( pData );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::InitClass( void )
{
	// Init the anim movement vars
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->SetRunSpeed( GetPlayerClass()->GetMaxSpeed() );
		m_PlayerAnimState->SetWalkSpeed( GetPlayerClass()->GetMaxSpeed() * 0.5 );
	}

	// Give default items for class.
	GiveDefaultItems();

	// Set initial health and armor based on class.
	SetHealth( GetMaxHealth() );

	// Update network variables
	m_Shared.SetMaxHealth( GetMaxHealth() );

	SetArmorValue( GetPlayerClass()->GetMaxArmor() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::CreateViewModel( int iViewModel )
{
	Assert( iViewModel >= 0 && iViewModel < MAX_VIEWMODELS );

	if ( GetViewModel( iViewModel ) )
		return;

	CTFViewModel *pViewModel = ( CTFViewModel * )CreateEntityByName( "tf_viewmodel" );
	if ( pViewModel )
	{
		pViewModel->SetAbsOrigin( GetAbsOrigin() );
		pViewModel->SetOwner( this );
		pViewModel->SetIndex( iViewModel );
		DispatchSpawn( pViewModel );
		pViewModel->FollowEntity( this, false );
		m_hViewModel.Set( iViewModel, pViewModel );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gets the view model for the player's off hand
//-----------------------------------------------------------------------------
CBaseViewModel *CTFPlayer::GetOffHandViewModel()
{
	// off hand model is slot 1
	return GetViewModel( 1 );
}

//-----------------------------------------------------------------------------
// Purpose: Sends the specified animation activity to the off hand view model
//-----------------------------------------------------------------------------
void CTFPlayer::SendOffHandViewModelActivity( Activity activity )
{
	CBaseViewModel *pViewModel = GetOffHandViewModel();
	if ( pViewModel )
	{
		int sequence = pViewModel->SelectWeightedSequence( activity );
		pViewModel->SendViewModelMatchingSequence( sequence );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::ItemsMatch( CEconItemView *pItem1, CEconItemView *pItem2, CTFWeaponBase *pWeapon )
{
	if ( pItem1 && pItem2 )
	{
		if ( pItem1->GetItemDefIndex() != pItem2->GetItemDefIndex() )
			return false;

		// Item might have different entities for each class (i.e. shotgun).
		if ( pWeapon )
		{
			int iClass = m_PlayerClass.GetClassIndex();
			const char *pszClassname = TranslateWeaponEntForClass( pItem1->GetEntityName(), iClass );

			if ( !FClassnameIs( pWeapon, pszClassname ) )
				return false;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Set the player up with the default weapons, ammo, etc.
//-----------------------------------------------------------------------------
void CTFPlayer::GiveDefaultItems()
{
	// Get the player class data.
	TFPlayerClassData_t *pData = m_PlayerClass.GetData();

	RemoveAllAmmo();

	// Reset all bodygroups
	for ( int i = 0; i < GetNumBodyGroups(); i++ )
	{
		SetBodygroup( i, 0 );
	}

	// Give ammo. Must be done before weapons, so weapons know the player has ammo for them.
	for ( int iAmmo = 0; iAmmo < TF_AMMO_COUNT; ++iAmmo )
	{
		GiveAmmo( GetMaxAmmo( iAmmo ), iAmmo, false, TF_AMMO_SOURCE_RESUPPLY );
	}

	// Give weapons.
	if ( lfe_force_legacy.GetBool() || TFGameRules()->IsTFCAllowed() )
		ManageRegularWeaponsLegacy( pData );
	else if ( lfe_force_random_weapons.GetBool() )
		ManageRandomWeapons( pData );
	else
		ManageRegularWeapons( pData );

	// Give grenades.
	//ManageGrenades( pData );

	// Give a builder weapon for each object the playerclass is allowed to build
	ManageBuilderWeapons( pData );

	// Give gravity gun if allowed.
	if ( lfe_allow_team_weapons.GetBool() )
		ManageTeamWeapons( pData );

	// Now that we've got weapons update our ammo counts since weapons may override max ammo.
	for ( int iAmmo = 0; iAmmo < TF_AMMO_COUNT; ++iAmmo )
	{
		SetAmmoCount( GetMaxAmmo( iAmmo ), iAmmo );
	}

	if ( m_bRegenerating == false )
	{
		SetActiveWeapon( NULL );
		Weapon_Switch( Weapon_GetSlot( 0 ) );
		Weapon_SetLast( Weapon_GetSlot( 1 ) );
	}

	// Regenerate Weapons
	for ( int i = 0; i < MAX_ITEMS; i++ )
	{
		CTFWeaponBase *pWeapon = dynamic_cast< CTFWeaponBase * >( Weapon_GetSlot( i ) );
		if ( pWeapon ) 
		{
			// Medieval
			int iAllowedInMedieval = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iAllowedInMedieval, allowed_in_medieval_mode );
			if ( TFGameRules()->IsInMedievalMode() )
			{
				if ( iAllowedInMedieval == 0 && !pWeapon->IsMeleeWeapon() )
				{
					// Not allowed in medieval mode
					if ( pWeapon == GetActiveWeapon() )
						pWeapon->Holster();

					Weapon_Detach( pWeapon );
					UTIL_Remove( pWeapon );
					continue;
				}
			}

			// Can we Disguise?
			int nCannotDisguise = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCannotDisguise, set_cannot_disguise );
			if ( IsPlayerClass( TF_CLASS_SPY ) && nCannotDisguise == 1 )
			{
				// Not allowed
				if ( pWeapon == GetActiveWeapon() )
					pWeapon->Holster();

				Weapon_Detach( Weapon_GetWeaponByType( TF_WPN_TYPE_PDA ) );
				UTIL_Remove( Weapon_GetWeaponByType( TF_WPN_TYPE_PDA ) );
				continue;
			}

			// Regenerate
			pWeapon->WeaponRegenerate();

			// player_bodygroups
			pWeapon->UpdatePlayerBodygroups();

			// Extra wearables
			const char *iszModel = pWeapon->GetItem()->GetExtraWearableModel();
			if ( iszModel[0] )
			{
				CTFWearable *pWearable = ( CTFWearable* )CreateEntityByName( "tf_wearable" );
				pWearable->SetItem( *pWeapon->GetItem() );
				pWearable->SetExtraWearable( true );

				pWearable->SetLocalOrigin( GetLocalOrigin() );
				pWearable->AddSpawnFlags( SF_NORESPAWN );

				DispatchSpawn( pWearable );
				pWearable->Activate();

				if ( pWearable != NULL && !( pWearable->IsMarkedForDeletion() ) )
				{
					pWearable->Touch( this );
					pWearable->GiveTo( this );
				}
			}
		}
	}

	// We may have swapped away our current weapon at resupply locker.
	if ( GetActiveWeapon() == NULL )
		SwitchToNextBestWeapon( NULL );

	// Tell the client to regenerate
	IGameEvent *event_regenerate = gameeventmanager->CreateEvent( "player_regenerate" );
	if ( event_regenerate )
	{
		gameeventmanager->FireEvent( event_regenerate );
	}

	int nMiniBuilding = 0;
	CALL_ATTRIB_HOOK_INT( nMiniBuilding, wrench_builds_minisentry );
	bool bMiniBuilding = nMiniBuilding ? true : false;

	// If we've switched to/from gunslinger destroy all of our buildings
	if ( m_Shared.m_bGunslinger != bMiniBuilding )
	{
		// blow up any carried buildings
		if ( IsPlayerClass( TF_CLASS_ENGINEER ) && m_Shared.GetCarriedObject() )
		{
			// Blow it up at our position.
			CBaseObject *pObject = m_Shared.GetCarriedObject();
			pObject->Teleport( &WorldSpaceCenter(), &GetAbsAngles(), &vec3_origin );
			pObject->DropCarriedObject(this);
			pObject->DetonateObject();
			SwitchToNextBestWeapon( GetActiveWeapon() );
		}
		// Check if we have any planted buildings
		for ( int i = GetObjectCount()-1; i >= 0; i-- )
		{
			CBaseObject *obj = GetObject( i );
			Assert( obj );

			// Destroy our sentry if we have one up
			if ( obj && obj->GetType() == OBJ_SENTRYGUN )
			{
				obj->DetonateObject();
			}		
		}
 		// make sure we update the c_models
		if ( GetActiveWeapon() )
		if ( GetActiveWeapon() && !Weapon_Switch( Weapon_GetSlot( LOADOUT_POSITION_MELEE ) ) )
			GetActiveWeapon()->Deploy();
	}

	m_Shared.m_bGunslinger = bMiniBuilding;

	if ( TFGameRules()->IsFreeMode() || TFGameRules()->IsUsingGrapplingHook() )
	{
		CEconItemView econItem( 1152 );
		CTFWeaponBase *pNewWeapon = (CTFWeaponBase *)GiveNamedItem( econItem.GetEntityName(), 0, &econItem );
		if ( pNewWeapon )
			pNewWeapon->GiveTo( this );
	}

	m_Shared.SetFlashlightBattery( 100.0f );

	CTFWearableDemoShield *pShield = GetEquippedDemoShield( this );
	if ( pShield && !m_Shared.HasDemoShieldEquipped() )
		m_Shared.SetDemoShieldEquipped( true );

	CTFWeaponBase *pParachute = Weapon_OwnsThisID( TF_WEAPON_PARACHUTE );
	if ( pParachute && !m_Shared.HasParachuteEquipped() )
		m_Shared.SetParachuteEquipped( true );

	if ( !lfe_force_legacy.GetBool() )
	{
		if ( lfe_allow_medigun_shield.GetBool() && IsPlayerClass( TF_CLASS_MEDIC ) )
			AddCustomAttribute( "generate rage on heal", 1 );

		if ( lfe_allow_minigun_deflect.GetBool() && IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
			AddCustomAttribute( "attack projectiles", 1 );
	}

	SwitchToNextBestWeapon( GetActiveWeapon() );

	HolsterOffHandWeapon();

	if ( TFGameRules()->IsPowerupMode() && !gEntList.FindEntityByClassname( NULL, "info_powerup_spawn" ) && ( m_Shared.GetCarryingRuneType() == TF_RUNE_NONE ) )
		m_Shared.AddCond( RandomInt( TF_COND_RUNE_STRENGTH, TF_COND_RUNE_AGILITY ) );

	//float flCustomModelScale = 1.0;
	//CALL_ATTRIB_HOOK_FLOAT(flCustomModelScale, player_model_scale);
	//SetModelScale(flCustomModelScale);


}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ManageBuilderWeapons( TFPlayerClassData_t *pData )
{
	if ( pData->m_aBuildable[0] != OBJ_LAST )
	{
		CEconItemView *pItem = GetLoadoutItem( GetPlayerClass()->GetClassIndex(), LOADOUT_POSITION_BUILDING );
		CTFWeaponBase *pBuilder = Weapon_OwnsThisID( TF_WEAPON_BUILDER );

		// Give the player a new builder weapon when they switch between engy and spy 
		if (pBuilder && !GetPlayerClass()->CanBuildObject(pBuilder->GetSubType()))
		{
			if (pBuilder == GetActiveWeapon())
				pBuilder->Holster();

			Weapon_Detach(pBuilder);
			UTIL_Remove(pBuilder);
			pBuilder = NULL;
		}

		if (pBuilder)
		{
			pBuilder->GiveDefaultAmmo();
			pBuilder->ChangeTeam(GetTeamNumber());

			if (m_bRegenerating == false)
			{
				pBuilder->WeaponReset();
			}
		}
		else
		{
			pBuilder = (CTFWeaponBase *)GiveNamedItem("tf_weapon_builder", pData->m_aBuildable[0], pItem);

			if (pBuilder)
			{
				if ( TFGameRules()->IsInMedievalMode() )
				{
					// Medieval
					float iAllowedInMedieval = 0;
					CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pBuilder, iAllowedInMedieval, allowed_in_medieval_mode );
					if ( !iAllowedInMedieval )
					{
						if ( pBuilder == GetActiveWeapon() )
						pBuilder->Holster();

						Weapon_Detach( pBuilder );
						UTIL_Remove( pBuilder );
						pBuilder = NULL;
						return;
					}
				}

				pBuilder->DefaultTouch(this);
			}
		}

		if (pBuilder)
		{
			pBuilder->m_nSkin = GetTeamNumber() - 2;	// color the w_model to the team
		}
	}
	else
	{
		//Not supposed to be holding a builder, nuke it from orbit
		CTFWeaponBase *pWpn = Weapon_OwnsThisID(TF_WEAPON_BUILDER);

		if (pWpn == NULL)
			return;

		if (pWpn == GetActiveWeapon())
			pWpn->Holster();

		Weapon_Detach(pWpn);
		UTIL_Remove(pWpn);
	}
}

void CTFPlayer::ValidateWeapons( bool bRegenerate )
{
	int iClass = m_PlayerClass.GetClassIndex();

	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = assert_cast<CTFWeaponBase *>( GetWeapon( i ) );
		if ( pWeapon == nullptr )
			continue;

		// Skip builder as we'll handle it separately.
		if ( pWeapon->IsWeapon( TF_WEAPON_BUILDER ) )
			continue;

		CEconItemDefinition *pItemDef = pWeapon->GetItem()->GetStaticData();
		if ( pItemDef )
		{
			int iSlot = pItemDef->GetLoadoutSlot( iClass );
			CEconItemView *pLoadoutItem = GetLoadoutItem( iClass, iSlot );

			if ( !ItemsMatch( pWeapon->GetItem(), pLoadoutItem, pWeapon ) )
			{
				if ( pWeapon->GetWeaponID() == TF_WEAPON_BUFF_ITEM )
				{
					// **HACK: Extra wearables aren't dying correctly sometimes so
					// try and remove them here just in case ValidateWearables() fails
					CEconWearable *pWearable = GetWearableForLoadoutSlot( iSlot );
					if ( pWearable )
					{
						RemoveWearable( pWearable );
					}
				}

				m_Shared.ResetRageSystem();

				// If this is not a weapon we're supposed to have in this loadout slot then nuke it.
				// Either changed class or changed loadout.
				// also nuke if it's in hl2 weapon position of a slot
				pWeapon->Holster();
				pWeapon->UnEquip( this );
			}
			else if ( bRegenerate )
			{
				pWeapon->ChangeTeam( GetTeamNumber() );
				pWeapon->GiveDefaultAmmo();

				if ( m_bRegenerating == false )
				{
					pWeapon->WeaponReset();
				}
			}
		}
		else
		{
			// Nuke any weapons without item definitions, they're evil!
			pWeapon->UnEquip( this );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ValidateWearables( void )
{
	int iClass = m_PlayerClass.GetClassIndex();

	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CTFWearable *pWearable = assert_cast<CTFWearable *>( GetWearable( i ) );
		if ( pWearable == nullptr )
			continue;

		// Always remove extra wearables when initializing weapons
		if ( pWearable->IsExtraWearable() )
		{
			RemoveWearable( pWearable );
			continue;
		}

		CEconItemDefinition *pItemDef = pWearable->GetItem()->GetStaticData();
		if ( pItemDef )
		{
			int iSlot = pItemDef->GetLoadoutSlot( iClass );
			CEconItemView *pLoadoutItem = GetLoadoutItem( iClass, iSlot );

			if ( !ItemsMatch( pWearable->GetItem(), pLoadoutItem, NULL ) )
			{
				// Not supposed to carry this wearable, nuke it.
				RemoveWearable( pWearable );
			}
			else if ( m_bRegenerating == false )
			{
				pWearable->UpdateModelToClass();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ManageRegularWeapons( TFPlayerClassData_t *pData )
{
	ValidateWeapons( true );
	ValidateWearables();

	for ( int iSlot = 0; iSlot < LOADOUT_POSITION_BUFFER; ++iSlot )
	{
		// These are special slots, we don't bother to check them.
		if ( (iSlot == LOADOUT_POSITION_BUILDING )  )
			continue;	

		if ( GetEntityForLoadoutSlot( iSlot ) != NULL )
		{
			// Nothing to do here.
			continue;
		}

		// Give us an item from the inventory.
		CEconItemView *pItem = GetLoadoutItem( m_PlayerClass.GetClassIndex(), iSlot );

		if ( pItem )
		{
			const char *pszClassname = pItem->GetEntityName();
			Assert( pszClassname );

			CEconEntity *pEntity = dynamic_cast<CEconEntity *>( GiveNamedItem( pszClassname, 0, pItem ) );

			if ( pEntity )
			{
				pEntity->GiveTo( this );
			}
		}
	}

	PostInventoryApplication();
}

//-----------------------------------------------------------------------------
// Purpose: Give us all weapons shared by our team.
//-----------------------------------------------------------------------------
void CTFPlayer::ManageTeamWeapons( TFPlayerClassData_t *pData )
{
	if ( IsPlayerClass( TF_CLASS_UNDEFINED ) )
		return;

	CTFTeam *pTeam = GetTFTeam();
	if ( !pTeam )
		return;

	int numWeapons = pTeam->GetNumWeapons();
	for ( int i = 0; i < numWeapons; i++ )
	{
		int iWeaponID = pTeam->GetWeapon( i );

		if ( iWeaponID == 9000 && TFGameRules()->MegaPhyscannonActive() )
			iWeaponID = 9001;

		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iWeaponID );
		if ( pItemDef )
		{
			int iClass = GetPlayerClass()->GetClassIndex();
			int iSlot = pItemDef->GetLoadoutSlot( iClass );
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetEntityForLoadoutSlot( iSlot );
			CEconItemView econItem( iWeaponID );
			if (pWeapon && iWeaponID != 9000 && iWeaponID != 9001 && iWeaponID != 9003 && iWeaponID != 9004)
			{
				//Prevent deleting gravity gun
				//TODO: Replace weapon progression & gravity gun giving logic with an item attribute (e.g. loadout_progression)
				//Or maybe a new item slot for team weapons? Hmmm.....
				if ( ItemsMatch( pWeapon->GetItem(), &econItem, pWeapon ) )
				{
					pWeapon->UnEquip( this );
					pWeapon = NULL;
				}
			}
			else
			{
				const char *pszWeaponName = econItem.GetEntityName();
				CTFWeaponBase *pNewWeapon = (CTFWeaponBase *)GiveNamedItem( pszWeaponName, 0, &econItem );

				if ( pNewWeapon )
				{
					SetAmmoCount( pNewWeapon->GetInitialAmmo(), pNewWeapon->GetPrimaryAmmoType() );
					pNewWeapon->GiveTo( this );
					//m_Shared.SetDesiredWeaponIndex( -1 );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PostInventoryApplication( void )
{
	m_Shared.RecalculatePlayerBodygroups();

	IGameEvent *event = gameeventmanager->CreateEvent( "post_inventory_application" );
	if (event)
	{
		event->SetInt( "userid", GetUserID() );

		gameeventmanager->FireEvent( event );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ManageRegularWeaponsLegacy( TFPlayerClassData_t *pData )
{
	for ( int iWeapon = 0; iWeapon < LOADOUT_POSITION_BUFFER; ++iWeapon )
	{
		// These are special slots, we don't bother to check them.
		if ( (iWeapon == LOADOUT_POSITION_BUILDING ))
			continue;	

		int iWeaponID = GetTFInventory()->GetWeapon( m_PlayerClass.GetClassIndex(), iWeapon );

		if ( iWeaponID != TF_WEAPON_NONE )
		{
			const char *pszWeaponName = WeaponIdToClassname( iWeaponID );

			CTFWeaponBase *pWeapon = (CTFWeaponBase *)Weapon_GetSlot( iWeapon );

			//If we already have a weapon in this slot but is not the same type then nuke it (changed classes)
			if ( pWeapon && pWeapon->GetWeaponID() != iWeaponID && pWeapon->GetPosition() >= 0 )
			{
				pWeapon->UnEquip( this );
			}

			pWeapon = Weapon_OwnsThisID( iWeaponID );

			if ( pWeapon )
			{
				pWeapon->ChangeTeam( GetTeamNumber() );
				pWeapon->GiveDefaultAmmo();

				if ( m_bRegenerating == false )
				{
					pWeapon->WeaponReset();
				}
			}
			else
			{
				pWeapon = (CTFWeaponBase *)GiveNamedItem( pszWeaponName );

				if ( pWeapon )
				{
					pWeapon->DefaultTouch( this );
					pWeapon->GiveDefaultAmmo();
				}
			}
		}
		else
		{
			//I shouldn't have any weapons in this slot, so get rid of it
			CTFWeaponBase *pCarriedWeapon = (CTFWeaponBase *)Weapon_GetSlot( iWeapon );

			//Don't nuke builders since they will be nuked if we don't need them later.
			if ( pCarriedWeapon && !pCarriedWeapon->IsWeapon( TF_WEAPON_BUILDER ) && pCarriedWeapon->GetWeaponID() < TF_WEAPON_PHYSCANNON )
			{
				Weapon_Detach( pCarriedWeapon );
				DevMsg("Removed weapon %s", pCarriedWeapon->GetName());
				UTIL_Remove( pCarriedWeapon );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ManageRandomWeapons( TFPlayerClassData_t *pData )
{
	// Nuke wearables
	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CTFWearable *pWearable = assert_cast<CTFWearable *>( GetWearable( i ) );
		if ( pWearable == nullptr )
			continue;

		RemoveWearable( pWearable );
	}

	// Nuke weapons
	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = assert_cast<CTFWeaponBase *>( GetWeapon( i ) );
		if ( pWeapon == nullptr )
			continue;

		// Holster our active weapon
		if ( pWeapon == GetActiveWeapon() )
			pWeapon->Holster();

		Weapon_Detach( pWeapon );
		UTIL_Remove( pWeapon );
	}

	for ( int i = 0; i < TF_PLAYER_WEAPON_COUNT; ++i )
	{


		CTFInventory *pInv = GetTFInventory();
		Assert( pInv );

		// Get a random item for our weapon slot
		int iClass = RandomInt( TF_FIRST_NORMAL_CLASS, TF_CLASS_COUNT );
		int iSlot = i;

		// Spy's equip slots do not correct match the weapon slot so we need to accommodate for that
		if ( iClass == TF_CLASS_SPY )
		{
			switch( i )
			{
			case LOADOUT_POSITION_PRIMARY:
				iSlot = LOADOUT_POSITION_SECONDARY;
				break;
			case LOADOUT_POSITION_SECONDARY:
				iSlot = LOADOUT_POSITION_BUILDING;
				break;
			}
		}

		int iPreset = RandomInt( 0, pInv->GetNumPresets( iClass, iSlot ) - 1 );

		CEconItemView *pItem = NULL;

		// Engineers always get PDAs
		// Spies should get their disguise PDA
		if ( ( m_PlayerClass.GetClassIndex()  == TF_CLASS_ENGINEER || m_PlayerClass.GetClassIndex() == TF_CLASS_SPY ) && ( iSlot == LOADOUT_POSITION_PDA1 || iSlot == LOADOUT_POSITION_PDA2 ) )
		{
			pItem = GetLoadoutItem( m_PlayerClass.GetClassIndex(), iSlot );
		}
		else
		{
			// Give us the item
			pItem = pInv->GetItem( iClass, iSlot, iPreset );
		}

		if ( pItem )
		{
			const char *pszClassname = pItem->GetEntityName();
			CEconEntity *pEntity = dynamic_cast<CEconEntity *>( GiveNamedItem( pszClassname, 0, pItem ) );
			if ( pEntity )
			{
				pEntity->GiveTo( this );
			}
		}
	}

	PostInventoryApplication();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ManageGrenades( TFPlayerClassData_t *pData )
{
	for ( int iGrenade = 0; iGrenade < TF_PLAYER_GRENADE_COUNT; iGrenade++ )
	{
		if ( pData->m_aGrenades[iGrenade] != TF_WEAPON_NONE )
		{
			CTFWeaponBase *pGrenade = (CTFWeaponBase *)GetWeapon( pData->m_aGrenades[iGrenade] );

			//If we already have a weapon in this slot but is not the same type then nuke it (changed classes)
			if ( pGrenade && pGrenade->GetWeaponID() != pData->m_aGrenades[iGrenade] )
			{
				Weapon_Detach( pGrenade );
				UTIL_Remove( pGrenade );
			}

			pGrenade = (CTFWeaponBase *)Weapon_OwnsThisID( pData->m_aGrenades[iGrenade] );

			if ( pGrenade )
			{
				pGrenade->ChangeTeam( GetTeamNumber() );
				pGrenade->GiveDefaultAmmo();

				if ( m_bRegenerating == false )
				{
					pGrenade->WeaponReset();
				}
			}
			else
			{
				const char *pszGrenadeName = WeaponIdToClassname( pData->m_aGrenades[iGrenade] );
				pGrenade = (CTFWeaponBase *)GiveNamedItem( pszGrenadeName );

				if ( pGrenade )
				{
					pGrenade->DefaultTouch( this );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get preset from the vector
//-----------------------------------------------------------------------------
CEconItemView *CTFPlayer::GetLoadoutItem( int iClass, int iSlot )
{
	int iPreset = m_WeaponPreset[iClass][iSlot];

	if ( lfe_force_legacy.GetBool() )
		iPreset = 0;

	return GetTFInventory()->GetItem( iClass, iSlot, iPreset );
}

//-----------------------------------------------------------------------------
// Purpose: WeaponPreset command handle
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_WeaponPreset(int iSlotNum, int iPresetNum)
{
	int iClass = m_PlayerClass.GetClassIndex();

	if (!GetTFInventory()->CheckValidSlot(iClass, iSlotNum))
		return;

	if (!GetTFInventory()->CheckValidWeapon(iClass, iSlotNum, iPresetNum))
		return;

	m_WeaponPreset[iClass][iSlotNum] = iPresetNum;
}

//-----------------------------------------------------------------------------
// Purpose: WeaponPreset command handle
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_WeaponPreset(int iClass, int iSlotNum, int iPresetNum)
{
	if (!GetTFInventory()->CheckValidSlot(iClass, iSlotNum))
		return;

	if (!GetTFInventory()->CheckValidWeapon(iClass, iSlotNum, iPresetNum))
		return;

	m_WeaponPreset[iClass][iSlotNum] = iPresetNum;
}

//-----------------------------------------------------------------------------
// Purpose: Create and give the named item to the player, setting the item ID. Then return it.
//-----------------------------------------------------------------------------
CBaseEntity	*CTFPlayer::GiveNamedItem( const char *pszName, int iSubType, CEconItemView* pItem, bool bOwns )
{
	const char *pszEntName = TranslateWeaponEntForClass( pszName, m_PlayerClass.GetClassIndex() );

	// If I already own this type don't create one
	if ( bOwns )
	{
		if ( Weapon_OwnsThisType( pszEntName ) )
			return NULL;
	}

	CBaseEntity *pEntity = CreateEntityByName( pszEntName );
	
	if ( pEntity == NULL )
	{
		Msg( "NULL Ent in GiveNamedItem!\n" );
		return NULL;
	}

	CEconEntity *pEcon = dynamic_cast<CEconEntity *>( pEntity );
	if ( pEcon && pItem )
	{
		pEcon->SetItem( *pItem );
	}

	pEntity->SetLocalOrigin( GetLocalOrigin() );
	pEntity->AddSpawnFlags( SF_NORESPAWN );

	CBaseCombatWeapon *pWeapon = pEntity->MyCombatWeaponPointer();
	if ( pWeapon )
	{
		pWeapon->SetSubType( iSubType );
	}

	DispatchSpawn( pEntity );
	pEntity->Activate();

	if ( bOwns )
	{
		if ( pEntity && !pEntity->IsMarkedForDeletion() )
		{
			pEntity->Touch( this );
		}
	}

	return pEntity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::EndClassSpecialSkill( void )
{
	if ( Weapon_OwnsThisID( TF_WEAPON_PIPEBOMBLAUNCHER ) == NULL )
	{
		CTFWearableDemoShield *pShield = GetEquippedDemoShield( this );
		if ( pShield )
			pShield->EndSpecialAction( this );
	}
}

CBaseEntity *FindPlayerStart(const char *pszClassName);

//-----------------------------------------------------------------------------
// Purpose: Find a spawn point for the player.
//-----------------------------------------------------------------------------
CBaseEntity* CTFPlayer::EntSelectSpawnPoint()
{
	if ( m_hReviveSpawnSpot.Get() && ( m_hReviveSpawnSpot->GetHealth() >= m_hReviveSpawnSpot->GetMaxHealth() ) )
	{
		//return ( m_hReviveSpawnSpot.Get() );
		if ( m_hTempSpawnSpot )
			return ( m_hTempSpawnSpot.Get() );
	}

	// If we have a temp spawn point set up then use that.
	if ( m_hTempSpawnSpot )
	{
		return ( m_hTempSpawnSpot.Get() );
	}

	CBaseEntity *pSpot = g_pLastSpawnPoints[ GetTeamNumber() ];
	const char *pSpawnPointName = "";

	switch( GetTeamNumber() )
	{
	case TF_TEAM_RED:
	case TF_TEAM_BLUE:
	case TF_TEAM_GREEN:
	case TF_TEAM_YELLOW:
		{
			bool bSuccess = false;
			pSpawnPointName = "info_player_teamspawn";
			bSuccess = SelectSpawnSpot( pSpawnPointName, pSpot );

			if ( bSuccess )
				g_pLastSpawnPoints[ GetTeamNumber() ] = pSpot;

			// need to save this for later so we can apply and modifiers to the armor and grenades...after the call to InitClass()
			m_pSpawnPoint = dynamic_cast<CTFTeamSpawn*>( pSpot );
			break;
		}
	case TEAM_SPECTATOR:
	case TEAM_UNASSIGNED:
	default:
		{
			pSpot = CBaseEntity::Instance( INDEXENT(0) );
			break;
		}
	}

	if ( !pSpot )
	{
		pSpot = FindPlayerStart( "info_player_start" );
		if ( pSpot )
			return pSpot;

		return CBaseEntity::Instance( INDEXENT(0) );
	}

	return pSpot;
} 

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot )
{
	// Get an initial spawn point.
	pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	if ( !pSpot )
	{
		// Sometimes the first spot can be NULL????
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}

	if ( !pSpot )
	{
		// Still NULL? That means there're no spawn points at all, bail.
		return false;
	}


	// First we try to find a spawn point that is fully clear. If that fails,
	// we look for a spawnpoint that's clear except for another players. We
	// don't collide with our team members, so we should be fine.
	bool bIgnorePlayers = false;

	CBaseEntity *pFirstSpot = pSpot;
	do 
	{
		if ( pSpot )
		{
			// Check to see if this is a valid team spawn (player is on this team, etc.).
			if( TFGameRules()->IsSpawnPointValid( pSpot, this, bIgnorePlayers ) )
			{
				// Check for a bad spawn entity.
				if ( pSpot->GetAbsOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
					continue;
				}

				if ( pSpot->HasSpawnFlags( 1 ) && IsPlayerClass( TF_CLASS_SCOUT ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 2 ) && IsPlayerClass( TF_CLASS_SNIPER ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 4 ) && IsPlayerClass( TF_CLASS_SOLDIER ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 8 ) && IsPlayerClass( TF_CLASS_DEMOMAN ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 16 ) && IsPlayerClass( TF_CLASS_MEDIC ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 32 ) && IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 64 ) && IsPlayerClass( TF_CLASS_PYRO ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 128 ) && IsPlayerClass( TF_CLASS_SPY ) )
					return true;
				else if ( pSpot->HasSpawnFlags( 256 ) && IsPlayerClass( TF_CLASS_ENGINEER ) )
					return true;
				else
					return true;
			}
		}

		// Get the next spawning point to check.
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

		if ( pSpot == pFirstSpot && !bIgnorePlayers )
		{
			// Loop through again, ignoring players
			bIgnorePlayers = true;
			pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
		}
	} 
	// Continue until a valid spawn point is found or we hit the start.
	while ( pSpot != pFirstSpot ); 

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Search for a player to spawn near
//-----------------------------------------------------------------------------
void CTFPlayer::SearchCoopSpawnSpot( void )
{
	m_bSearchingSpawn = false;

	if ( !ShouldUseCoopSpawning() )
	{
		// No longer spawning this way.
		return;
	}

	CUtlVector<CBasePlayer *> vecPlayers;

	// Qualified players are teammates that are alive.
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer && InSameTeam( pPlayer ) && pPlayer->IsAlive() )
		{
			vecPlayers.AddToTail( pPlayer );
		}
	}

	vecPlayers.Shuffle();

	Vector vecSpawnOrigin = vec3_origin;
	QAngle vecSpawnAngles;

	int numPlayers = vecPlayers.Count();
	for ( int i = 0; i < numPlayers && vecSpawnOrigin == vec3_origin; i++ )
	{
		CBasePlayer *pPlayer = vecPlayers[i];
		if ( !pPlayer )
			continue;

		Vector vecOrigin = pPlayer->GetAbsOrigin();
		QAngle vecAngles = pPlayer->GetAbsAngles();
		vecAngles[PITCH] = 0.0;

		if ( IsSpaceToSpawnHere( vecOrigin ) )
		{
			vecSpawnOrigin = vecOrigin;
			vecSpawnAngles = vecAngles;
		}
	}

	if ( vecSpawnOrigin != vec3_origin )
	{
		CTFTeamSpawn *pSpot = static_cast<CTFTeamSpawn *>( CBaseEntity::Create( "info_player_teamspawn", vecSpawnOrigin, vecSpawnAngles ) );
		if ( pSpot )
		{
			pSpot->ChangeTeam( GetTeamNumber() );
			// Disable it so other players don't accidently snatch it.
			pSpot->SetDisabled( true );

			m_hTempSpawnSpot = pSpot;

			ForceRespawn();
			return;
		}
	}

	SetContextThink( &CTFPlayer::SearchCoopSpawnSpot, gpGlobals->curtime + 1.0, "SpawnSearchThink" );
	m_bSearchingSpawn = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldUseCoopSpawning( void )
{
	if ( TFGameRules()->CanPlayerSearchSpawn() )
		return ( !IsAlive() && IsOnStoryTeam() && TFGameRules()->IsCoOpGameRunning() || IsOnCombineTeam() && TFGameRules()->IsBluCoOpGameRunning() );

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::SelectFurthestSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot, bool bTelefrag /*= true*/ )
{
	// Get an initial spawn point.
	pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	if ( !pSpot )
	{
		// Sometimes the first spot can be NULL????
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}

	if ( !pSpot )
	{
		// Still NULL? That means there're no spawn points at all, bail.
		return false;
	}

	// Find spawn point that is furthest from all other players.
	CBaseEntity *pFirstSpot = pSpot;
	float flFurthest = 0.0f;
	CBaseEntity *pFurthest = NULL;
	do
	{
		if ( !pSpot )
		{
			pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
			continue;
		}

		// Check to see if this is a valid team spawn (player is on this team, etc.).
		if ( TFGameRules()->IsSpawnPointValid( pSpot, this, true ) )
		{
			// Check for a bad spawn entity.
			if ( pSpot->GetAbsOrigin() == vec3_origin )
			{
				pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
				continue;
			}

			// Check distance from other players.
			bool bOtherPlayersPresent = false;
			float flClosestPlayerDist = FLT_MAX;
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if ( !pPlayer || pPlayer == this || !pPlayer->IsAlive() || ( InSameTeam( pPlayer ) ) )
					continue;

				bOtherPlayersPresent = true;

				float flDistSqr = ( pPlayer->GetAbsOrigin() - pSpot->GetAbsOrigin() ).LengthSqr();
				if ( flDistSqr < flClosestPlayerDist )
				{
					flClosestPlayerDist = flDistSqr;
				}
			}

			// If there are no other players just pick the first valid spawn point.
			if ( !bOtherPlayersPresent )
			{
				pFurthest = pSpot;
				break;
			}

			if ( flClosestPlayerDist > flFurthest )
			{
				flFurthest = flClosestPlayerDist;
				pFurthest = pSpot;
			}
		}

		// Get the next spawning point to check.
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}
	// Continue until a valid spawn point is found or we hit the start.
	while ( pSpot != pFirstSpot );

	if ( pFurthest )
	{
		if ( bTelefrag )
		{
			// Kill off anyone occupying this spot if it's somehow busy.
			CBaseEntity *pList[MAX_PLAYERS];
			Vector vecMins = pSpot->GetAbsOrigin() + VEC_HULL_MIN;
			Vector vecMaxs = pSpot->GetAbsOrigin() + VEC_HULL_MAX;
			int count = UTIL_EntitiesInBox( pList, MAX_PLAYERS, vecMins, vecMaxs, FL_CLIENT );

			for ( int i = 0; i < count; i++ )
			{
				CBaseEntity *pEntity = pList[i];
				if ( pEntity != this && ( !InSameTeam( pEntity ) ) )
				{
					CTakeDamageInfo info( this, this, 1000, DMG_CRUSH, TF_DMG_CUSTOM_TELEFRAG );
					pEntity->TakeDamage( info );
				}
			}
		}

		pSpot = pFurthest;

		// Found a valid spawn point.
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTriggerAreaCapture *CTFPlayer::GetControlPointStandingOn( void )
{
	touchlink_t *root = (touchlink_t *)GetDataObject( TOUCHLINK );
	if (root)
	{
		touchlink_t *next = root->nextLink;
		while (next != root)
		{
			CBaseEntity *pEntity = next->entityTouched;
			if (!pEntity)
				return NULL;

			if ( pEntity->IsSolidFlagSet( FSOLID_TRIGGER ) && pEntity->IsBSPModel() )
			{
				CTriggerAreaCapture *pCapArea = dynamic_cast<CTriggerAreaCapture *>( pEntity );
				if ( pCapArea )
					return pCapArea;
			}

			next = next->nextLink;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsCapturingPoint( void )
{
	CTriggerAreaCapture *pCapArea = GetControlPointStandingOn();
	if ( pCapArea )
	{
		CTeamControlPoint *pPoint = pCapArea->GetControlPoint();
		if ( pPoint && TFGameRules()->TeamMayCapturePoint( GetTeamNumber(), pPoint->GetPointIndex() ) &&
			 TFGameRules()->PlayerMayCapturePoint( this, pPoint->GetPointIndex() ) )
		{
			return pPoint->GetOwner() != GetTeamNumber();
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( m_PlayerAnimState )
		m_PlayerAnimState->DoAnimationEvent( event, nData );

	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PhysObjectSleep()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Sleep();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PhysObjectWake()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Wake();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::GetAutoTeam( void )
{
	int iTeam = TEAM_SPECTATOR;

	CTFTeam *pBlue = TFTeamMgr()->GetTeam(TF_TEAM_BLUE);
	CTFTeam *pRed = TFTeamMgr()->GetTeam(TF_TEAM_RED);

		if (pBlue && pRed)
		{
			if ( TFGameRules()->IsCoOp() )
			{
				if ( TFGameRules()->IsVersus() )
				{
					// Check RED for min amount of players.
					if ( pRed->GetNumPlayers() < lfe_vs_min_red_players.GetInt() )
					{
						iTeam = TF_TEAM_RED;
					}
				}
				else
				{
					// Always join RED in co-op.
					iTeam = TF_TEAM_RED;
				}
			}

			if (pBlue->GetNumPlayers() < pRed->GetNumPlayers())
			{
				iTeam = TF_TEAM_BLUE;
			}
			else if (pRed->GetNumPlayers() < pBlue->GetNumPlayers())
			{
				iTeam = TF_TEAM_RED;
			}
			else
			{
				iTeam = RandomInt(0, 1) ? TF_TEAM_RED : TF_TEAM_BLUE;
			}
		}

	return iTeam;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_JoinTeam( const char *pTeamName )
{
	if ( TFGameRules()->IsInfectionMode() && ( stricmp( pTeamName, "spectate" ) != 0 ) )
	{
		if ( TFGameRules()->InSetup() || TFGameRules()->InSetup() || TFGameRules()->State_Get() == GR_STATE_PREROUND )
		{
			ChangeTeam( TF_TEAM_RED );
			SetDesiredPlayerClassIndex( TF_CLASS_CIVILIAN );
		}
		else
		{
			ChangeTeam( TF_TEAM_BLUE );
			SetDesiredPlayerClassIndex( TF_CLASS_ZOMBIEFAST );
		}
		return;
	}

	int iTeam = TF_TEAM_RED;
	if ( stricmp( pTeamName, "auto" ) == 0 )
	{
		iTeam = GetAutoTeam();
	}
	if (stricmp(pTeamName, "green") == 0)
	{
		return;
	}
	if (stricmp(pTeamName, "yellow") == 0)
	{
		return;
	}
	if (stricmp(pTeamName, "unassigned") == 0)
	{
		return;
	}
	else if ( stricmp( pTeamName, "spectate" ) == 0 )
	{
		iTeam = TEAM_SPECTATOR;
	}
	else
	{
		for ( int i = 0; i < TF_TEAM_COUNT; ++i )
		{
			if ( stricmp( pTeamName, g_aTeamNames[i] ) == 0 )
			{
				iTeam = i;
				break;
			}
		}
	}

	if (iTeam == GetTeamNumber())
		return;	// we wouldn't change the team

	if ( (iTeam == TF_TEAM_BLUE && TFGameRules()->IsCoOp()) || (iTeam == TF_TEAM_RED && TFGameRules()->IsBluCoOp()) || (TFGameRules()->IsHordeMode() && iTeam == TF_TEAM_BLUE) )
		return;


	if ( HasTheFlag() )
		DropFlag();

	if ( m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
		DropRune();

	if ( iTeam == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() && !IsHLTV() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return;
		}
		
		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			CommitSuicide( false, true );
		}

		ChangeTeam( TEAM_SPECTATOR );

		// do we have fadetoblack on? (need to fade their screen back in)
		if ( mp_fadetoblack.GetBool() )
		{
			color32_s clr = { 0,0,0,255 };
			UTIL_ScreenFade( this, clr, 0, 0, FFADE_IN | FFADE_PURGE );
		}
	}
	else
	{
		// if this join would unbalance the teams, refuse
		// come up with a better way to tell the player they tried to join a full team!
		if ( TFGameRules()->WouldChangeUnbalanceTeams( iTeam, GetTeamNumber() ) )
		{
			ShowViewPortPanel( PANEL_TEAM );
			return;
		}

		ChangeTeam( iTeam );

		switch ( iTeam )
		{
		case TF_TEAM_RED:
			ShowViewPortPanel( PANEL_CLASS_RED );
			break;

		case TF_TEAM_BLUE:
			ShowViewPortPanel( PANEL_CLASS_BLUE );
			break;
		}
	}
	CheckTeam();
}

//-----------------------------------------------------------------------------
// Purpose: Join a team without using the game menus
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_JoinTeam_NoMenus( const char *pTeamName )
{
	Assert( IsX360() );

	Msg( "Client command HandleCommand_JoinTeam_NoMenus: %s\n", pTeamName );

	// Only expected to be used on the 360 when players leave the lobby to start a new game
	if ( !IsInCommentaryMode() )
	{
		Assert( GetTeamNumber() == TEAM_UNASSIGNED );
		Assert( IsX360() );
	}

	if (stricmp(pTeamName, "red") != 0 && TFGameRules()->IsCoOp() || stricmp(pTeamName, "blue") != 0 && !TFGameRules()->IsBluCoOp() || stricmp(pTeamName, "green") == 0 || stricmp(pTeamName, "yellow") == 0 || stricmp(pTeamName, "unassigned") == 0)
	{
		return;
	}

	if ( !TFGameRules()->IsAnyCoOp() && !TFGameRules()->IsHordeMode() )
	{
		int iTeam = TEAM_SPECTATOR;
		if ( Q_stricmp( pTeamName, "spectate" ) )
		{
			for ( int i = 0; i < TF_TEAM_COUNT; ++i )
			{
				if ( stricmp( pTeamName, g_aTeamNames[i] ) == 0 )
				{
					iTeam = i;
					break;
				}
			}
		}

	ForceChangeTeam( iTeam );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Join a team without suiciding
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_JoinTeam_NoKill( const char *pTeamName )
{
	if ( TFGameRules()->IsInfectionMode() && ( stricmp( pTeamName, "spectate" ) != 0 ) )
	{
		if ( TFGameRules()->InSetup() || TFGameRules()->State_Get() == GR_STATE_PREROUND )
		{
			ChangeTeam( TF_TEAM_RED );
			SetDesiredPlayerClassIndex( TF_CLASS_CIVILIAN );
		}
		else
		{
			ChangeTeam( TF_TEAM_BLUE );
			SetDesiredPlayerClassIndex( TF_CLASS_ZOMBIEFAST );
		}
		return;
	}

	int iTeam = TF_TEAM_RED;

	if (stricmp(pTeamName, "green") == 0)
	{
		return;
	}
	if (stricmp(pTeamName, "yellow") == 0)
	{
		return;
	}
	if (stricmp(pTeamName, "unassigned") == 0)
	{
		return;
	}

	if ( sv_cheats->GetInt() == 0 && !IsDeveloper() )
		return;

	if ( stricmp( pTeamName, "auto" ) == 0 )
	{
		iTeam = GetAutoTeam();
	}
	else if ( stricmp( pTeamName, "spectate" ) == 0 )
	{
		iTeam = TEAM_SPECTATOR;
	}
	else
	{
		for ( int i = 0; i < TF_TEAM_COUNT; ++i )
		{
			if ( stricmp( pTeamName, g_aTeamNames[i] ) == 0 )
			{
				iTeam = i;
				break;
			}
		}
	}

	if (iTeam == GetTeamNumber())
	{
		return;	// we wouldn't change the team
	}

	BaseClass::ChangeTeam( iTeam );
}

//-----------------------------------------------------------------------------
// Purpose: Player has been forcefully changed to another team
//-----------------------------------------------------------------------------
void CTFPlayer::ForceChangeTeam( int iTeamNum )
{
	int iNewTeam = iTeamNum;

	if ( iNewTeam == TF_TEAM_AUTOASSIGN )
	{
		iNewTeam = GetAutoTeam();
	}

	if ( !GetGlobalTeam( iNewTeam ) )
	{
		Warning( "CTFPlayer::ForceChangeTeam( %d ) - invalid team index.\n", iNewTeam );
		return;
	}

	int iOldTeam = GetTeamNumber();

	// if this is our current team, just abort
	if ( iNewTeam == iOldTeam )
		return;

	RemoveAllOwnedEntitiesFromWorld( false );
	RemoveNemesisRelationships();

	BaseClass::ChangeTeam( iNewTeam );

	if ( iNewTeam == TEAM_UNASSIGNED )
	{
		StateTransition( TF_STATE_OBSERVER );
	}
	else if ( iNewTeam == TEAM_SPECTATOR )
	{
		m_bIsIdle = false;
		StateTransition( TF_STATE_OBSERVER );

		RemoveAllWeapons();
		DestroyViewModels();
	}

	// Don't modify living players in any way
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::HandleFadeToBlack( void )
{
	if ( mp_fadetoblack.GetBool() )
	{
		color32_s clr = { 0,0,0,255 };
		UTIL_ScreenFade( this, clr, 0.75, 0, FFADE_OUT | FFADE_STAYOUT );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ChangeTeam( int iTeamNum, bool bAutoTeam, bool bSilent )
{
	if ( !GetGlobalTeam( iTeamNum ) )
	{
		Warning( "CTFPlayer::ChangeTeam( %d ) - invalid team index.\n", iTeamNum );
		return;
	}

	int iOldTeam = GetTeamNumber();

	// if this is our current team, just abort
	if ( iTeamNum == iOldTeam )
		return;

	RemoveAllOwnedEntitiesFromWorld( false );
	RemoveNemesisRelationships();

	BaseClass::ChangeTeam( iTeamNum, false, bSilent );

	if ( iTeamNum == TEAM_UNASSIGNED )
	{
		StateTransition( TF_STATE_OBSERVER );
	}
	else if ( iTeamNum == TEAM_SPECTATOR )
	{
		m_bIsIdle = false;

		StateTransition( TF_STATE_OBSERVER );

		RemoveAllWeapons();
		DestroyViewModels();
	}
	else // active player
	{
		if ( !IsDead() && ( iOldTeam >= FIRST_GAME_TEAM ) )
		{
			// Kill player if switching teams while alive
			CommitSuicide( false, true );
		}
		else if ( IsDead() && iOldTeam < FIRST_GAME_TEAM )
		{
			SetObserverMode( OBS_MODE_CHASE );
			HandleFadeToBlack();
		}

		// let any spies disguising as me know that I've changed teams
		for ( int i = 1 ; i <= gpGlobals->maxClients ; i++ )
		{
			CTFPlayer *pTemp = ToTFPlayer( UTIL_PlayerByIndex( i ) );
			if ( pTemp && pTemp != this )
			{
				if ( ( pTemp->m_Shared.GetDisguiseTarget() == this ) || // they were disguising as me and I've changed teams
					 ( !pTemp->m_Shared.GetDisguiseTarget() && pTemp->m_Shared.GetDisguiseTeam() == iTeamNum ) ) // they don't have a disguise and I'm joining the team they're disguising as
				{
					// choose someone else...
					pTemp->m_Shared.FindDisguiseTarget();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: grave maker
//-----------------------------------------------------------------------------
void CTFPlayer::HandleCommand_JoinClass( const char *pClassName )
{
	if ( GetNextChangeClassTime() > gpGlobals->curtime )
		return;

	// can only join a class after you join a valid team
	if ( GetTeamNumber() <= LAST_SHARED_TEAM )
		return;

	// In case we don't get the class menu message before the spawn timer
	// comes up, fake that we've closed the menu.
	SetClassMenuOpen( false );

	if ( TFGameRules()->InStalemate() )
	{
		if ( IsAlive() && !TFGameRules()->CanChangeClassInStalemate() )
		{
			ClientPrint(this, HUD_PRINTTALK, "#game_stalemate_cant_change_class" );
			return;
		}
	}

	int iClass = TF_CLASS_UNDEFINED;
	bool bShouldNotRespawn = false;

	if ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN )
	{
		m_bAllowInstantSpawn = false;
		bShouldNotRespawn = true;
	}

	if ( stricmp( pClassName, "random" ) != 0 )
	{
		int iLastClass = TF_LAST_NORMAL_CLASS;
		if ( lfe_allow_special_classes.GetBool() || IsDeveloper() || TFGameRules()->IsInfectionMode() )
			iLastClass = TF_CLASS_COUNT;
		else if ( TFGameRules()->IsTFCAllowed() )
			iLastClass = TF_CLASS_CIVILIAN;

		int i = 0;

		for ( i = TF_FIRST_NORMAL_CLASS; i < TF_CLASS_COUNT_ALL; i++ )
		{
			if ( stricmp( pClassName, GetPlayerClassData( i )->m_szClassName ) == 0 )
			{
				iClass = i;
				break;
			}
		}
		
		if ( i > iLastClass )
		{
			ClientPrint( this, HUD_PRINTCONSOLE, UTIL_VarArgs( "Invalid class name \"%s\".\n", pClassName ) );
			return;
		}
	}
	else
	{
		// The player has selected Random class...so let's pick one for them.
		do
		{
			// Don't let them be the same class twice in a row
			iClass = random->RandomInt( TF_FIRST_NORMAL_CLASS, TF_LAST_NORMAL_CLASS );
		} while( iClass == GetPlayerClass()->GetClassIndex() );
	}

	if ( !TFGameRules()->CanPlayerChooseClass( this, iClass ) )
		return;

	// joining the same class?
	if ( iClass != TF_CLASS_RANDOM && iClass == GetDesiredPlayerClassIndex() )
	{
		// If we're dead, and we have instant spawn, respawn us immediately. Catches the case
		// were a player misses respawn wave because they're at the class menu, and then changes
		// their mind and reselects their current class.
		if ( m_bAllowInstantSpawn && !IsAlive() && ( m_Shared.GetLivesCount() != 0 ) && !m_bInTransition )
		{
			ForceRespawn();
		}
		return;
	}

	SetNextChangeClassTime(gpGlobals->curtime + 2.0f);

	if ( TFGameRules()->IsInfectionMode() )
	{
		if ( TFGameRules()->InSetup() || TFGameRules()->State_Get() == GR_STATE_PREROUND )
		{
			iClass = TF_CLASS_CIVILIAN;
			ChangeTeam( TF_TEAM_RED );
		}
		else
		{
			iClass = TF_CLASS_ZOMBIEFAST;
			ChangeTeam( TF_TEAM_BLUE );
		}
	}

	SetDesiredPlayerClassIndex( iClass );
	IGameEvent * event = gameeventmanager->CreateEvent( "player_changeclass" );
	if ( event )
	{
		event->SetInt( "userid", GetUserID() );
		event->SetInt( "class", iClass );

		gameeventmanager->FireEvent( event );
	}

	// are they TF_CLASS_RANDOM and trying to select the class they're currently playing as (so they can stay this class)?
	if ( iClass == GetPlayerClass()->GetClassIndex() )
	{
		// If we're dead, and we have instant spawn, respawn us immediately. Catches the case
		// were a player misses respawn wave because they're at the class menu, and then changes
		// their mind and reselects their current class.
		if ( m_bAllowInstantSpawn && !IsAlive() && ( m_Shared.GetLivesCount() != 0 ) && !m_bInTransition )
		{
			ForceRespawn();
		}
		return;
	}

	// We can respawn instantly if:
	//	- We're dead, and we're past the required post-death time
	//	- We're inside a respawn room
	//	- We're in the stalemate grace period
	bool bInRespawnRoom = PointInRespawnRoom( this, WorldSpaceCenter() );
	if ( bInRespawnRoom && !IsAlive() )
	{
		// If we're not spectating ourselves, ignore respawn rooms. Otherwise we'll get instant spawns
		// by spectating someone inside a respawn room.
		bInRespawnRoom = (GetObserverTarget() == this);
	}
	bool bDeadInstantSpawn = !IsAlive();
	if ( bDeadInstantSpawn && m_flDeathTime )
	{
		// In death mode, don't allow class changes to force respawns ahead of respawn waves
		float flWaveTime = TFGameRules()->GetNextRespawnWave( GetTeamNumber(), this );
		bDeadInstantSpawn = (gpGlobals->curtime > flWaveTime);
	}
	bool bInStalemateClassChangeTime = false;
	if ( TFGameRules()->InStalemate() )
	{
		// Stalemate overrides respawn rules. Only allow spawning if we're in the class change time.
		bInStalemateClassChangeTime = TFGameRules()->CanChangeClassInStalemate();
		bDeadInstantSpawn = false;
		bInRespawnRoom = false;
	}
	if ( bShouldNotRespawn == false && ( m_Shared.GetLivesCount() != 0 ) && ( m_bAllowInstantSpawn || bDeadInstantSpawn || bInRespawnRoom || bInStalemateClassChangeTime ) )
	{
		ForceRespawn();
		return;
	}

	if( iClass == TF_CLASS_RANDOM )
	{
		if( IsAlive() )
		{
			ClientPrint(this, HUD_PRINTTALK, "#game_respawn_asrandom" );
		}
		else
		{
			ClientPrint(this, HUD_PRINTTALK, "#game_spawn_asrandom" );
		}
	}
	else
	{
		if( IsAlive() )
		{
			ClientPrint(this, HUD_PRINTTALK, "#game_respawn_as", GetPlayerClassData( iClass )->m_szLocalizableName );
		}
		else
		{
			ClientPrint(this, HUD_PRINTTALK, "#game_spawn_as", GetPlayerClassData( iClass )->m_szLocalizableName );
		}
	}

	if ( IsAlive() && ( GetHudClassAutoKill() == true ) && bShouldNotRespawn == false )
	{
		CommitSuicide( false, true );
		RemoveAllObjects( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::ClientCommand( const CCommand &args )
{
	const char *pcmd = args[0];
	
	m_flLastAction = gpGlobals->curtime;

	if ( FStrEq( pcmd, "burn" ) ) 
	{
		if ( sv_cheats->GetInt() != 0 || IsDeveloper() )
		{
			m_Shared.Burn( this );
			return true;
		}
		return false;
	}
	else if ( FStrEq( pcmd, "jointeam" ) )
	{
		if (gpGlobals->curtime < GetNextChangeTeamTime())
			return false;

		SetNextChangeTeamTime(gpGlobals->curtime + 2.0f);

		if ( args.ArgC() >= 2 )
		{
			HandleCommand_JoinTeam( args[1] );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "jointeam_nomenus" ) )
	{
		if ( IsX360() )
		{
			if ( args.ArgC() >= 2 )
			{
				HandleCommand_JoinTeam_NoMenus( args[1] );
			}
			return true;
		}
		return false;
	}
	else if ( FStrEq( pcmd, "jointeam_nokill" ) )
	{
		if ( sv_cheats->GetInt() != 0 || IsDeveloper() )
		{
			if ( args.ArgC() >= 2 )
			{
				HandleCommand_JoinTeam_NoKill( args[1] );
			}
			return true;
		}
		return false;
	}
	else if ( FStrEq( pcmd, "closedwelcomemenu" ) )
	{
		if ( GetTeamNumber() == TEAM_UNASSIGNED )
		{
			ShowViewPortPanel( PANEL_TEAM, true );
		}
		else if ( IsPlayerClass( TF_CLASS_UNDEFINED ) )
		{
			switch( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				ShowViewPortPanel( PANEL_CLASS_RED, true );
				break;

			case TF_TEAM_BLUE:
				ShowViewPortPanel( PANEL_CLASS_BLUE, true );
				break;

			default:
				break;
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "joinclass" ) ) 
	{
		if ( args.ArgC() >= 2 )
		{
			HandleCommand_JoinClass( args[1] );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "weaponpreset" ) )
	{
		if ( args.ArgC() >= 3 )
		{
			HandleCommand_WeaponPreset(abs(atoi(args[1])), abs(atoi(args[2])));
		}
		return true;
	}
	else if ( FStrEq( pcmd, "weaponpresetclass" ) )
	{
		if ( args.ArgC() >= 4 )
		{
			HandleCommand_WeaponPreset(abs(atoi(args[1])), abs(atoi(args[2])), abs(atoi(args[3])));
		}
		return true;
	}
	else if ( FStrEq( pcmd, "getweaponinfos" ) )
	{
		for ( int iWeapon = 0; iWeapon < LOADOUT_POSITION_BUFFER; ++iWeapon )
		{
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetWeapon( iWeapon );

			if ( pWeapon && pWeapon->HasItemDefinition() )
			{
				CEconItemView *econItem = pWeapon->GetItem();
				CEconItemDefinition *itemdef = econItem->GetStaticData();

				if ( itemdef )
				{
					Msg( "ItemID %i:\nname %s\nitem_class %s\nitem_type_name %s\n",
						pWeapon->GetItemID(), itemdef->name, itemdef->item_class, itemdef->item_type_name );

					Msg( "Attributes:\n" );
					for ( int i = 0; i < itemdef->attributes.Count(); i++ )
					{
						static_attrib_t *pAttribute = &itemdef->attributes[i];
						const CEconAttributeDefinition *pStatic = pAttribute->GetStaticData();

						if ( pStatic )
						{
							float value = BitsToFloat( pAttribute->value.iVal );
							if ( pStatic->description_format == ATTRIB_FORMAT_PERCENTAGE || pStatic->description_format == ATTRIB_FORMAT_INVERTED_PERCENTAGE )
							{
								value *= 100.0f;
							}

							Msg( "%s %g\n", pStatic->description_string, value );
						}
					}
					Msg( "\n" );
				}
			}

		}
		return true;
	}
	else if ( FStrEq( pcmd, "disguise" ) ) 
	{
		if ( args.ArgC() >= 3 )
		{
			if ( CanDisguise() )
			{
				int nClass = atoi( args[ 1 ] );
				int nTeam = atoi( args[ 2 ] );
				
				// intercepting the team value and reassigning what gets passed into Disguise()
				// because the team numbers in the client menu don't match the #define values for the teams
					switch (nTeam)
					{
					case 0:
						m_Shared.Disguise( TF_TEAM_RED, nClass );
						break;
					case 1:
						m_Shared.Disguise( TF_TEAM_BLUE, nClass );
						break;
					case 2:
						m_Shared.Disguise( TF_TEAM_GREEN, nClass );
						break;
					case 3:
						m_Shared.Disguise( TF_TEAM_YELLOW, nClass );
						break;
					}

					//m_Shared.Disguise((nTeam == 1) ? TF_TEAM_BLUE : TF_TEAM_RED, nClass);
			}
		}
		return true;
	}
	else if (FStrEq( pcmd, "lastdisguise" ) )
	{
		// disguise as our last known disguise. desired disguise will be initted to something sensible
		if ( CanDisguise() )
		{
			// disguise as the previous class, if one exists
			int nClass = m_Shared.GetDesiredDisguiseClass();

			// PistonMiner: try and disguise as the previous team
			int nTeam = m_Shared.GetDesiredDisguiseTeam();

			//If we pass in "random" or whatever then just make it pick a random class.
			if ( args.ArgC() > 1 )
			{
				nClass = TF_CLASS_UNDEFINED;
			}

			if ( nClass == TF_CLASS_UNDEFINED )
			{
				// they haven't disguised yet, pick a nice one for them.
				// exclude some undesirable classes 

				// PistonMiner: Made it so it doesnt pick your own team.
				do
				{
					nClass = random->RandomInt( TF_FIRST_NORMAL_CLASS, TF_LAST_NORMAL_CLASS );
					
					//	nTeam = random->RandomInt( TF_TEAM_RED, TF_TEAM_YELLOW );
						GetTeamNumber() == TF_TEAM_BLUE ? nTeam = TF_TEAM_RED : nTeam = TF_TEAM_BLUE;

				} while( nClass == TF_CLASS_SCOUT || nClass == TF_CLASS_SPY || nTeam == GetTeamNumber() );
			}

			m_Shared.Disguise( nTeam, nClass );
		}

		return true;
	}
	else if ( FStrEq( pcmd, "mp_playgesture" ) )
	{
		if ( args.ArgC() == 1 )
		{
			Warning( "mp_playgesture: Gesture activity or sequence must be specified!\n" );
			return true;
		}

		if ( sv_cheats->GetInt() != 0 || IsDeveloper() )
		{
			if ( !PlayGesture( args[1] ) )
			{
				Warning( "mp_playgesture: unknown sequence or activity name \"%s\"\n", args[1] );
				return true;
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "mp_playanimation" ) )
	{
		if ( args.ArgC() == 1 )
		{
			Warning( "mp_playanimation: Activity or sequence must be specified!\n" );
			return true;
		}

		if ( sv_cheats->GetInt() != 0 || IsDeveloper() )
		{
			if ( !PlaySpecificSequence( args[1] ) )
			{
				Warning( "mp_playanimation: Unknown sequence or activity name \"%s\"\n", args[1] );
				return true;
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "menuopen" ) )
	{
		SetClassMenuOpen( true );
		return true;
	}
	else if ( FStrEq( pcmd, "menuclosed" ) )
	{
		SetClassMenuOpen( false );
		return true;
	}
	else if ( FStrEq( pcmd, "pda_click" ) )
	{
		// player clicked on the PDA, play attack animation

		CTFWeaponBase *pWpn = GetActiveTFWeapon();
		CTFWeaponPDA *pPDA = dynamic_cast<CTFWeaponPDA *>( pWpn );

		if ( pPDA && !m_Shared.InCond( TF_COND_DISGUISED ) )
		{
			DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
		}

		return true;
	}
	else if ( FStrEq( pcmd, "taunt" ) )
	{
		HandleTauntCommand( 0 );
		return true;
	}
	else if ( FStrEq( pcmd, "build" ) )
	{
		int iBuilding = 0;
		int iMode = 0;

		if ( args.ArgC() == 2 )
		{
			// player wants to build something
			iBuilding = atoi( args[ 1 ] );
			iMode = 0;

			if ( iBuilding == 3 )
				iBuilding = iMode = 1;

			StartBuildingObjectOfType( iBuilding, iMode );
		}
		else if ( args.ArgC() == 3 )
		{
			// player wants to build something
			iBuilding = atoi( args[ 1 ] );
			iMode = atoi( args[ 2 ] );

			StartBuildingObjectOfType( iBuilding, iMode );
		}
		else
		{
			ClientPrint( this, HUD_PRINTCONSOLE, "Usage: build <building> <mode>\n" );
			return true;
		}

		return true;
	}
	else if ( FStrEq( pcmd, "destroy" ) )
	{
		int iBuilding = 0;
		int iMode = 0;

		if ( args.ArgC() == 2 )
		{
			// player wants to destroy something
			iBuilding = atoi( args[ 1 ] );
			iMode = 0;

			if ( iBuilding == 3 )
				iBuilding = iMode = 1;

			DetonateOwnedObjectsOfType( iBuilding, iMode );
		}
		else if ( args.ArgC() == 3 )
		{
			// player wants to destroy something
			iBuilding = atoi( args[ 1 ] );
			iMode = atoi( args[ 2 ] );

			DetonateOwnedObjectsOfType( iBuilding, iMode );
		}
		else
		{
			ClientPrint( this, HUD_PRINTCONSOLE, "Usage: destroy <building> <mode>\n" );
			return true;
		}

		return true;
	}
	else if ( FStrEq( pcmd, "eureka_teleport" ) )
	{
		int iMode = 0;

		if ( args.ArgC() == 2 )
		{
			if ( iMode == 0 )
			{
				// spawn
			}
			else if ( iMode == 1 )
			{
				// teleport exit
			}
		}
		else
		{
			ClientPrint( this, HUD_PRINTCONSOLE, "Usage: eureka_teleport <mode>\n" );
			return true;
		}

		return true;
	}
	else if ( FStrEq( pcmd, "extendfreeze" ) )
	{
		m_flDeathTime += 2.0f;
		return true;
	}
	else if ( FStrEq( pcmd, "show_motd" ) )
	{
		KeyValues *data = new KeyValues( "data" );
		data->SetString( "title", "#TF_Welcome" );	// info panel title
		data->SetString( "type", "1" );				// show userdata from stringtable entry
		data->SetString( "msg",	"motd" );			// use this stringtable entry
		data->SetString( "cmd", "mapinfo" );		// exec this command if panel closed

		ShowViewPortPanel( PANEL_INFO, true, data );

		data->deleteThis();
	}
	else if ( FStrEq( pcmd, "loot_response" ) )
	{
		if ( args.ArgC() >= 2 )
		{
			if ( FStrEq( args[1], "common" ) )
				SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_COMMON );
			else if ( FStrEq( args[1], "rare" ) )
				SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_RARE );
			else if ( FStrEq( args[1], "ultra_rare" ) )
				SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_ULTRARARE );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cheatgun_build" ) )
	{
		CTFCheatGun *pKitchenGun = dynamic_cast<CTFCheatGun *>( Weapon_OwnsThisID( TF_WEAPON_CHEATGUN ) );
		if ( pKitchenGun )
		{
			MDLCACHE_CRITICAL_SECTION();

			if ( !Q_stricmp( args[1], "point_servercommand" ) || !Q_stricmp( args[1], "point_clientcommand" ) )
			{
				if ( engine->IsDedicatedServer() )
				{
					if ( !IsAutoKickDisabled() )
						return true;
				}
				else if ( gpGlobals->maxClients > 1 )
				{
					CBasePlayer *pHostPlayer = UTIL_GetListenServerHost();
					if ( this != pHostPlayer )
						return true;
				}
			}
			else if ( FStrEq( args[1], "npc_combine" ) || FStrEq( args[1], "npc_advisor" ) )
			{
				Msg( "Unable to create %s.\n", args[1] );
				return true;
			}

			bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
			CBaseEntity::SetAllowPrecache( true );

			// Try to create entity
			CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName(args[1]) );
			if ( entity )
			{
				entity->Precache();

				// Pass in any additional parameters.
				for ( int i = 2; i + 1 < args.ArgC(); i += 2 )
				{
					const char *pKeyName = args[i];
					const char *pValue = args[i+1];
					entity->KeyValue( pKeyName, pValue );
				}

				DispatchSpawn( entity );

				// Now attempt to drop into the world
				trace_t tr;
				Vector forward;
				EyeVectors( &forward );
				Ray_t ray;
				ray.Init( EyePosition(), EyePosition() + forward * MAX_TRACE_LENGTH );
				UTIL_Portal_TraceRay( ray, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

				if ( tr.fraction != 1.0 )
				{
					entity->Teleport( &tr.endpos, NULL, NULL );
					UTIL_SetOrigin( entity, tr.endpos );
					UTIL_DropToFloor( entity, MASK_SOLID );
				}

				entity->Activate();
				pKitchenGun->SetSpawnEnt( entity );
			}
			CBaseEntity::SetAllowPrecache( allowPrecache );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "dispenserhtml" ) )
	{
		if ( args.ArgC() >= 2 )
		{
			for ( int i = GetObjectCount()-1; i >= 0; i-- )
			{
				CBaseObject *pObj = GetObject( i );

				if ( pObj && pObj->GetType() == OBJ_DISPENSER )
				{
					CObjectDispenser *pDispenser = dynamic_cast< CObjectDispenser* >( pObj );
					if ( pDispenser )
						pDispenser->SetCustomHTML( args[1] );
				}
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "dev_noclip" ) )
	{
		if ( !IsDeveloper() )
		{
			return false;
		}
		else 
		{
			if ( !m_Shared.InCond( LFE_COND_NOCLIP ) )
			{
				m_Shared.AddCond( LFE_COND_NOCLIP );
				return true;
			}

			if ( m_Shared.InCond( LFE_COND_NOCLIP ) )
			{
				m_Shared.RemoveCond( LFE_COND_NOCLIP );
				return true;
			}

			return false;
		}
	}
	else if ( FStrEq( pcmd, "dev_cvar" ) )
	{
		if ( !IsDeveloper() )
		{
			return false;
		}
		else 
		{
			if ( args.ArgC() == 3 )
			{
				const char *pszCvarName = args[1];
				const char *pszCvarValue = args[2];
				ConVarRef var( pszCvarName );
				var.SetValue( pszCvarValue );
			}
			else
			{
				ClientPrint( this, HUD_PRINTCONSOLE, "Usage: dev_cvar <cvar> <value>\n" );
			}
			return true;
		}
	}
	else if ( FStrEq( pcmd, "dev_message" ) )
	{
		if ( !IsDeveloper() )
		{
			return false;
		}
		else 
		{
			if ( args.ArgC() >= 2 )
			{
				const char *pszMessage = args[1];
				int nTeam = atoi( args[2] );
				const char *pszIcon = args[3];

				CBroadcastRecipientFilter filter;
				TFGameRules()->SendHudNotification( filter, pszMessage, pszIcon, nTeam );
			}
			else
			{
				ClientPrint( this, HUD_PRINTCONSOLE, "Usage: dev_message <text> <team> <icon>\n" );
			}
			return true;
		}
	}
	else if ( FStrEq( pcmd, "dev_deathnotice" ) )
	{
		if ( !IsDeveloper() )
		{
			return false;
		}
		else 
		{
			if ( args.ArgC() >= 2 )
			{
				const char *pszMessage = args[1];
				int nTeam = atoi( args[2] );
				const char *pszIcon = args[3];
				int nCrit = atoi( args[4] );

				IGameEvent *event = gameeventmanager->CreateEvent( "lfe_deathnotice_text" );
				if ( event )
				{
					event->SetInt( "id", entindex() );
					event->SetString( "text", pszMessage );
					event->SetString( "icon", pszIcon );
					event->SetInt( "team", nTeam );
					event->SetInt( "crit", nCrit );
					event->SetInt( "priority", 9 );

					gameeventmanager->FireEvent( event );
				}
			}
			else
			{
				ClientPrint( this, HUD_PRINTCONSOLE, "Usage: dev_deathnotice <text> <team> <icon> <crit>\n" );
			}
			return true;
		}
	}
	else if ( FStrEq( pcmd, "regenerate" ) )
	{
		if ( sv_cheats->GetInt() != 0 || IsDeveloper() )
		{
			Regenerate();
			return true;
		}
		return false;
	}

	return BaseClass::ClientCommand( args );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SetClassMenuOpen( bool bOpen )
{
	m_bIsClassMenuOpen = bOpen;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsClassMenuOpen( void )
{
	return m_bIsClassMenuOpen;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::PlayGesture( const char *pGestureName )
{
	Activity nActivity = (Activity)LookupActivity( pGestureName );
	if ( nActivity != ACT_INVALID )
	{
		DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, nActivity );
		return true;
	}

	int nSequence = LookupSequence( pGestureName );
	if ( nSequence != -1 )
	{
		DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE_SEQUENCE, nSequence );
		return true;
	} 

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::PlaySpecificSequence( const char *pAnimationName )
{
	Activity nActivity = (Activity)LookupActivity( pAnimationName );
	if ( nActivity != ACT_INVALID )
	{
		DoAnimationEvent( PLAYERANIMEVENT_CUSTOM, nActivity );
		return true;
	}

	int nSequence = LookupSequence( pAnimationName );
	if ( nSequence != -1 )
	{
		DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_SEQUENCE, nSequence );
		return true;
	} 

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::CanDisguise( void )
{
	if ( !IsAlive() )
		return false;

	if ( GetPlayerClass()->GetClassIndex() != TF_CLASS_SPY )
		return false;

	if ( IsInAVehicle() )
		return false;

	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		HintMessage( HINT_CANNOT_DISGUISE_WITH_FLAG );
		return false;
	}

	int nCannotDisguise = 0;
	CALL_ATTRIB_HOOK_INT( nCannotDisguise, set_cannot_disguise );
	int nConsumeCloak = 0;
	CALL_ATTRIB_HOOK_INT(nConsumeCloak, mod_disguise_consumes_cloak);
	if ( nCannotDisguise == 1 || nConsumeCloak == 1 && m_Shared.GetSpyCloakMeter() < 99.0f )
	{
		// Not allowed
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::CanDisguise_OnKill( void )
{
	if ( !IsAlive() )
		return false;

	if ( GetPlayerClass()->GetClassIndex() != TF_CLASS_SPY )
		return false;

	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		HintMessage( HINT_CANNOT_DISGUISE_WITH_FLAG );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DetonateOwnedObjectsOfType( int iType, int iMode )
{
	int i;
	int iNumObjects = GetObjectCount();
	for ( i=0;i<iNumObjects;i++ )
	{
		CBaseObject *pObj = GetObject(i);

		if ( pObj && pObj->GetType() == iType && pObj->GetObjectMode() == iMode )
		{
			SpeakConceptIfAllowed( MP_CONCEPT_DETONATED_OBJECT, pObj->GetResponseRulesModifier() );
			pObj->DetonateObject();

			const CObjectInfo *pInfo = GetObjectInfo( iType );

			if ( pInfo )
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"killedobject\" (object \"%s\") (weapon \"%s\") (objectowner \"%s<%i><%s><%s>\") (attacker_position \"%d %d %d\")\n",   
					GetPlayerName(),
					GetUserID(),
					GetNetworkIDString(),
					GetTeam()->GetName(),
					pInfo->m_pObjectName,
					"pda_engineer",
					GetPlayerName(),
					GetUserID(),
					GetNetworkIDString(),
					GetTeam()->GetName(),
					(int)GetAbsOrigin().x, 
					(int)GetAbsOrigin().y,
					(int)GetAbsOrigin().z );
			}

			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StartBuildingObjectOfType( int iType, int iMode )
{
	// early out if we can't build this type of object
	if ( CanBuild( iType, iMode ) != CB_CAN_BUILD )
		return;

	for ( int i = 0; i < WeaponCount(); i++) 
	{
		CTFWeaponBase *pWpn = ( CTFWeaponBase *)GetWeapon(i);

		if ( pWpn == NULL )
			continue;

		if ( pWpn->GetWeaponID() != TF_WEAPON_BUILDER )
			continue;

		CTFWeaponBuilder *pBuilder = dynamic_cast< CTFWeaponBuilder * >( pWpn );

		// Is this the builder that builds the object we're looking for?
		if ( pBuilder )
		{
			pBuilder->SetSubType( iType );
			pBuilder->SetObjectMode( iMode );

			if ( GetActiveTFWeapon() == pBuilder )
			{
				SetActiveWeapon( NULL );
			}

			// try to switch to this weapon
			if ( Weapon_Switch( pBuilder ) )
			{
				break;
			}
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	if (m_takedamage != DAMAGE_YES)
		return;

	CTakeDamageInfo info_modified = info;
	bool bNPC = info_modified.GetAttacker()->IsNPC();
	if (bNPC && InSameTeam(info_modified.GetAttacker()))
	{
		return;
	}

	CTFPlayer *pTFAttacker = ToTFPlayer(info.GetAttacker());
	if ( pTFAttacker )
	{
		// Prevent team damage here so blood doesn't appear
		if ( !g_pGameRules->FPlayerCanTakeDamage( this, pTFAttacker, info ) && friendlyfire.GetBool() == false )
			return;
	}

	// Save this bone for the ragdoll.
	m_nForceBone = ptr->physicsbone;
	SetLastHitGroup( ptr->hitgroup );

	// Ignore hitboxes for all weapons except the sniper rifle
	if ( info_modified.GetDamageType() & DMG_USE_HITLOCATIONS )
	{
		switch ( ptr->hitgroup )
		{
		case HITGROUP_HEAD:
		{
			CTFWeaponBase *pWpn = pTFAttacker->GetActiveTFWeapon();
			float flDamage = info_modified.GetDamage();
			bool bCritical = true;

			if ( pWpn && !pWpn->CanFireCriticalShot( true ) )
				bCritical = false;

			if ( bCritical )
			{
				info_modified.AddDamageType( DMG_CRITICAL );
				info_modified.SetDamageCustom( TF_DMG_CUSTOM_HEADSHOT );

				// play the critical shot sound to the shooter	
				if ( pWpn )
				{
					if ( pWpn->IsWeapon( TF_WEAPON_SNIPERRIFLE ) || pWpn->IsWeapon( TF_WEAPON_SNIPERRIFLE_CLASSIC ) || pWpn->IsWeapon( TF_WEAPON_SNIPERRIFLE_DECAP ) || pWpn->IsWeapon( TF_WEAPON_REVOLVER ) )
						pWpn->WeaponSound( BURST );
				}
			}

			info_modified.SetDamage( flDamage );
			break;
		}

		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			/*if ( pWpn )
			{
				float flBodyshotModifer = 1.0f;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pWpn, flBodyshotModifer, bodyshot_damage_modify );
				if ( flBodyshotModifer != 1.0f ) // We have an attribute changing damage, modify it.
				{
					float flDamage = info_modified.GetDamage();
					flDamage *= flBodyshotModifer;
					info_modified.SetDamage( flDamage );
				}
			}*/
		default:
			break;
		}
	}

	if ( bNPC )
	{
		switch ( ptr->hitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			info_modified.SetDamageCustom( TF_DMG_CUSTOM_HEADSHOT );
			info_modified.ScaleDamage( sk_player_head.GetInt() );
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			info_modified.ScaleDamage( sk_player_arm.GetInt() );
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			info_modified.ScaleDamage( sk_player_leg.GetInt() );
			break;
		case HITGROUP_CHEST:
			info_modified.ScaleDamage( sk_player_chest.GetInt() );
			break;
		case HITGROUP_STOMACH:
			info_modified.ScaleDamage( sk_player_stomach.GetInt() );
			break;
		default:
			break;
		}
	}

	if ( m_Shared.InCond( TF_COND_DISGUISED ) )
	{
		// no impact effects
	}
	else if ( m_Shared.IsInvulnerable() )
	{ 
		// Make bullet impacts
		g_pEffects->Ricochet( ptr->endpos - (vecDir * 8), -vecDir );
	}
	else if( m_Shared.InCond( TF_COND_PHASE ) )
	{
		CEffectData	data;
		data.m_nHitBox = GetParticleSystemIndex( "miss_text" );
		data.m_vOrigin = WorldSpaceCenter() + Vector(0,0,32);
		data.m_vAngles = vec3_angle;
		data.m_nEntIndex = 0;

		CSingleUserRecipientFilter filter( ToBasePlayer( info.GetAttacker() ) );
		te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );

		SpeakConceptIfAllowed( MP_CONCEPT_DODGE_SHOT );
	}
	else
	{
		if ( !m_Shared.InCond( TF_COND_PHASE ) )
		{
			// Since this code only runs on the server, make sure it shows the tempents it creates.
			CDisablePredictionFiltering disabler;

			// This does smaller splotches on the guy and splats blood on the world.
			TraceBleed( info_modified.GetDamage(), vecDir, ptr, info_modified.GetDamageType() );
		}
	}

	/*if (TFGameRules()->IsCoOp() && GetTeamNumber() == TF_TEAM_RED && GetGlobalTFTeam(TF_TEAM_RED)->GetNumPlayers() > 1)
	{
		info_modified.SetDamage(info_modified.GetDamage() + info_modified.GetDamage() * (0.20 * GetGlobalTFTeam(TF_TEAM_RED)->GetNumPlayers()));
	}
	else if (TFGameRules()->IsBluCoOp() && GetTeamNumber() == TF_TEAM_BLUE && GetGlobalTFTeam(TF_TEAM_BLUE)->GetNumPlayers() > 1)
	{
		info_modified.SetDamage(info_modified.GetDamage() + info_modified.GetDamage() * (0.20 * GetGlobalTFTeam(TF_TEAM_BLUE)->GetNumPlayers()));
	}*/

	AddMultiDamage( info_modified, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::TakeHealth( float flHealth, int bitsDamageType )
{
	int iResult = false;

	// If the bit's set, add over the max health
	if ( bitsDamageType & DMG_IGNORE_MAXHEALTH )
	{
		int iTimeBasedDamage = g_pGameRules->Damage_GetTimeBased();
		m_bitsDamageType &= ~(bitsDamageType & ~iTimeBasedDamage);
		m_iHealth += flHealth;
		iResult = (int)flHealth;
	}
	else
	{
		float flHealthToAdd = flHealth;
		float flMaxHealth = GetMaxHealth();
		
		// don't want to add more than we're allowed to have
		if ( flHealthToAdd > flMaxHealth - m_iHealth )
		{
			flHealthToAdd = flMaxHealth - m_iHealth;
		}

		if ( flHealthToAdd <= 0 )
		{
			iResult = 0;
		}
		else
		{
			iResult = BaseClass::TakeHealth( flHealthToAdd, bitsDamageType );
		}
	}

	return iResult;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::TFWeaponRemove( int iWeaponID )
{
	// find the weapon that matches the id and remove it
	int i;
	for (i = 0; i < WeaponCount(); i++) 
	{
		CTFWeaponBase *pWeapon = ( CTFWeaponBase *)GetWeapon( i );
		if ( !pWeapon )
			continue;

		if ( pWeapon->GetWeaponID() != iWeaponID )
			continue;

		DevMsg("Removing weapon %s with ID %i", pWeapon->GetName(), iWeaponID);

		RemovePlayerItem( pWeapon );
		UTIL_Remove( pWeapon );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::DropCurrentWeapon( void )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropFlag( void )
{
	if ( HasItem() )
	{
		CCaptureFlag *pFlag = dynamic_cast<CCaptureFlag*>( GetItem() );
		if ( pFlag )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_flag_event" );
			if ( event )
			{
				event->SetInt( "player", entindex() );
				event->SetString( "player_name", GetPlayerName() );
				event->SetInt( "carrier", entindex() );
				event->SetInt( "eventtype", TF_FLAGEVENT_DROPPED );
				event->SetInt( "home", 0 ); // whether or not the flag was home (only set for TF_FLAGEVENT_PICKUP) 
				event->SetInt( "team", GetItem()->GetTeamNumber() );
				event->SetInt( "priority", 8 );

				gameeventmanager->FireEvent( event );
			}
			pFlag->Drop( this, true, true );
			RemoveGlowEffect();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
EHANDLE CTFPlayer::TeamFortress_GetDisguiseTarget( int nTeam, int nClass )
{
	if ( nTeam == GetTeamNumber() || nTeam == TF_SPY_UNDEFINED )
	{
		// we're not disguised as the enemy team
		return NULL;
	}

	CBaseEntity *pLastTarget = m_Shared.GetDisguiseTarget(); // don't redisguise self as this person
	
	// Find a player on the team the spy is disguised as to pretend to be
	CTFPlayer *pPlayer = NULL;

	// Loop through players
	int i;
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			if ( pPlayer == pLastTarget )
			{
				// choose someone else, we're trying to rid ourselves of a disguise as this one
				continue;
			}

			// First, try to find a player with the same color AND skin
			if ( pPlayer->GetTeamNumber() == nTeam && pPlayer->GetPlayerClass()->GetClassIndex() == nClass )
			{
				return pPlayer;
			}
		}
	}

	// we didn't find someone with the same skin, so just find someone with the same color
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			if ( pPlayer->GetTeamNumber() == nTeam )
			{
				return pPlayer;
			}
		}
	}

	// we didn't find anyone
	return NULL;
}

static float DamageForce( const Vector &size, float damage, float scale )
{ 
	float force;

	// Adjust for HL2 hull size if it's enabled.
	if ( lfe_use_hl2_player_hull.GetBool() )
		force = damage * ((24 * 24 * 64.0) / (size.x * size.y * size.z)) * scale;
	else
		force = damage * ((48 * 48 * 82.0) / (size.x * size.y * size.z)) * scale;
	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}

ConVar tf_debug_damage( "tf_debug_damage", "0", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	IServerVehicle *pVehicle = GetVehicle();
	if ( pVehicle )
	{
		// Let the vehicle decide if we should take this damage or not
		if ( pVehicle->PassengerShouldReceiveDamage( info ) == false )
			return 0;
	}

	if ( GetFlags() & FL_GODMODE )
		return 0;

	if ( IsInCommentaryMode() )
		return 0;

	// Early out if there's no damage
	if ( !info.GetDamage() )
		return 0;

	if ( !IsAlive() )
		return 0;

	CBaseEntity *pAttacker = info.GetAttacker();
	CBaseEntity *pInflictor = info.GetInflictor();
	CTFWeaponBase *pWeapon = NULL;
	bool bObject = false;

	if ( pAttacker && pAttacker->IsNPC() && InSameTeam( pAttacker ) )
	{
		return 0;
	}

	if ( inputInfo.GetWeapon() )
	{
		pWeapon = dynamic_cast<CTFWeaponBase *>( inputInfo.GetWeapon() );
	}
	else if ( pAttacker && pAttacker->IsPlayer() )
	{
		// Assume that player used his currently active weapon.
		pWeapon = ToTFPlayer( pAttacker )->GetActiveTFWeapon();
	}

	// If this is a base object get the builder
	if ( pAttacker && pAttacker->IsBaseObject() )
	{
		CBaseObject *pObject = static_cast< CBaseObject * >( pAttacker );
		pAttacker = pObject->GetBuilder();
		bObject = true;
	}

	int iHealthBefore = GetHealth();

	bool bDebug = tf_debug_damage.GetBool();
	if ( bDebug )
	{
		Warning( "%s taking damage from %s, via %s. Damage: %.2f\n", GetDebugName(), pInflictor ? pInflictor->GetDebugName() : "Unknown Inflictor", pAttacker ? pAttacker->GetDebugName() : "Unknown Attacker", info.GetDamage() );
	}

	// Make sure the player can take damage from the attacking entity
	if ( !g_pGameRules->FPlayerCanTakeDamage( this, pAttacker, info ) )
	{
		if ( bDebug )
		{
			Warning( "    ABORTED: Player can't take damage from that attacker.\n" );
		}
		return 0;
	}

	if ( info.GetDamage() > 0.0f )
	{
		if ( info.GetAttacker() )
			NotifyFriendsOfDamage( pAttacker );
	}

	if ( info.GetDamageType() & DMG_FALL )
	{
		CBaseCombatCharacter *pLandingPad = ToBaseCombatCharacter( GetGroundEntity() );
		if ( pLandingPad && !pLandingPad->IsBaseObject() && m_Shared.CanFallStomp() && !InSameTeam( pLandingPad ) )
		{
			float flDamage = info.GetDamage() * 3 + 10.f; // Minimum 10 damage
			CBaseEntity *pBoots = GetWearableForLoadoutSlot( LOADOUT_POSITION_SECONDARY );
			CTakeDamageInfo stompInfo( this, this, pBoots, flDamage, DMG_FALL, TF_DMG_CUSTOM_BOOTS_STOMP );
			pLandingPad->TakeDamage( stompInfo );

			m_Local.m_flFallVelocity = 0;
			info.SetDamage( 0 );

			EmitSound( "Weapon_Mantreads.Impact" );
			EmitSound( "Player.FallDamageDealt" );
			UTIL_ScreenShake( WorldSpaceCenter(), 15.0, 150.0, 1.0, 500.0, SHAKE_START );
			DispatchParticleEffect( "stomp_text", pLandingPad->WorldSpaceCenter() + Vector(0,0,32), vec3_angle );
		}
	}

	AddDamagerToHistory( pAttacker );

	// keep track of amount of damage last sustained
	m_lastDamageAmount = info.GetDamage();
	m_LastDamageType = info.GetDamageType();

	// Speak if this is a fall damage just like how it is on live tf2
	if ( !( info.GetDamageType() & DMG_FALL ) )
	{
		SpeakConceptIfAllowed( MP_CONCEPT_HURT );
		m_Shared.NoteLastDamageTime( m_lastDamageAmount );
	}

	// if this is our own rocket and we're in mid-air, scale down the damage
	if ( IsPlayerClass( TF_CLASS_SOLDIER ) || IsPlayerClass( TF_CLASS_DEMOMAN ) )
	{
		if ( ( info.GetDamageType() & DMG_BLAST ) && pAttacker == this && GetGroundEntity() == NULL )
		{
			float flDamage = info.GetDamage();
			int iJumpType = 0;

			if ( !IsPlayerClass( TF_CLASS_DEMOMAN ) )
			{
				flDamage *= tf_damagescale_self_soldier.GetFloat();
				iJumpType = TF_JUMP_ROCKET;
			}
			else
			{
				iJumpType = TF_JUMP_STICKY;
			}

			if ( m_Shared.InCond( TF_COND_RUNE_PRECISION ) )
				flDamage = 0;

			info.SetDamage( flDamage );

			int iPlaySound = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iPlaySound, no_self_blast_dmg );

			// Set blast jumping state. It will be cleared once we land.
			SetBlastJumpState( iJumpType, iPlaySound != 0 );
		}
	}

	// Save damage force for ragdolls.
	m_vecTotalBulletForce = info.GetDamageForce();
	m_vecTotalBulletForce.x = clamp( m_vecTotalBulletForce.x, -15000.0f, 15000.0f );
	m_vecTotalBulletForce.y = clamp( m_vecTotalBulletForce.y, -15000.0f, 15000.0f );
	m_vecTotalBulletForce.z = clamp( m_vecTotalBulletForce.z, -15000.0f, 15000.0f );

	int bTookDamage = 0;
 
	int bitsDamage = inputInfo.GetDamageType();

	CTFPlayer *pTFAttacker = ToTFPlayer( pAttacker );
	//CTFPlayer *pTFInflictor = ToTFPlayer( pInflictor );

	if ( pTFAttacker && !pTFAttacker->InSameTeam( this ) && TFGameRules()->IsTruceActive() )
		return 0;

	// If we're invulnerable or bonk, force ourselves to only take damage events only, so we still get pushed
	if ( m_Shared.IsInvulnerable() || m_Shared.InCond( TF_COND_PHASE ) )
	{
		bool bAllowDamage = false;

		// check to see if our attacker is a trigger_hurt entity (and allow it to kill us even if we're invuln)
		if ( pAttacker && pAttacker->IsSolidFlagSet( FSOLID_TRIGGER ) )
		{
			CTriggerHurt *pTrigger = dynamic_cast<CTriggerHurt *>( pAttacker );
			if ( pTrigger )
				bAllowDamage = true;
		}

		// Does not save from telefrags.
		if ( info.GetDamageCustom() == TF_DMG_CUSTOM_TELEFRAG )
			bAllowDamage = true;

		// Does not save from crocodiles.
		if ( info.GetDamageCustom() == TF_DMG_CUSTOM_CROC )
			bAllowDamage = true;

		// Disallow Canal Slime
		if ( info.GetDamageType() == DMG_RADIATION )
			bAllowDamage = false;

		if ( !bAllowDamage )
		{
			int iOldTakeDamage = m_takedamage;
			m_takedamage = DAMAGE_EVENTS_ONLY;
			// NOTE: Deliberately skip base player OnTakeDamage, because we don't want all the stuff it does re: suit voice
			CBaseCombatCharacter::OnTakeDamage( info );
			m_takedamage = iOldTakeDamage;

			// Burn sounds are handled in ConditionThink()
			if ( !(bitsDamage & DMG_BURN ) )
				SpeakConceptIfAllowed( MP_CONCEPT_HURT );

			if ( m_Shared.InCond( TF_COND_PHASE ) )
			{
				CEffectData	data;
				data.m_nHitBox = GetParticleSystemIndex( "miss_text" );
				data.m_vOrigin = WorldSpaceCenter() + Vector(0,0,32);
				data.m_vAngles = vec3_angle;
				data.m_nEntIndex = 0;

				CSingleUserRecipientFilter filter( (CBasePlayer*)pAttacker );
				te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );

				SpeakConceptIfAllowed( MP_CONCEPT_DODGE_SHOT );
			}
			return 0;
		}
	}

	if ( m_Shared.InCond( TF_COND_URINE ) || m_Shared.InCond( TF_COND_MARKEDFORDEATH ) || m_Shared.InCond( TF_COND_MARKEDFORDEATH_SILENT ) )
	{
		// Jarated players take mini crits
		bitsDamage |= DMG_MINICRITICAL;
		info.AddDamageType( DMG_MINICRITICAL );
	}

	if ( GetActiveTFWeapon() )
	{
		float flBecomeFireProof = 0.0f;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetActiveTFWeapon(), flBecomeFireProof, become_fireproof_on_hit_by_fire );
		if ( ( flBecomeFireProof > 0.0f ) && info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING )
		{
			m_Shared.AddCond( TF_COND_AFTERBURN_IMMUNE, flBecomeFireProof );
		}
	}

	// Handle on-hit effects.
	if ( pWeapon && pAttacker != this )
	{
		int nCritOnCond = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCritOnCond, or_crit_vs_playercond );
		if ( nCritOnCond )
		{
			for ( int i = 0; condition_to_attribute_translation[i] != TF_COND_LAST; i++ )
			{
				int nCond = condition_to_attribute_translation[i];
				int nFlag = ( 1 << i );
				if ( ( nCritOnCond & nFlag ) && m_Shared.InCond( nCond ) )
				{
					bitsDamage |= DMG_CRITICAL;
					info.AddDamageType( DMG_CRITICAL );
					break;
				}
			}
		}

		int nMiniCritOnCond = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nMiniCritOnCond, or_minicrit_vs_playercond_burning );
		if ( nMiniCritOnCond )
		{
			for ( int i = 0; condition_to_attribute_translation[i] != TF_COND_LAST; i++ )
			{
				int nCond = condition_to_attribute_translation[i];
				int nFlag = ( 1 << i );
				if ( ( nMiniCritOnCond & nFlag ) && m_Shared.InCond( nCond ) )
				{
					bitsDamage |= DMG_MINICRITICAL;
					info.AddDamageType( DMG_MINICRITICAL );
					break;
				}
			}
		}

		float flPenaltyNonBurning = info.GetDamage();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flPenaltyNonBurning, mult_dmg_vs_nonburning );
		if ( !m_Shared.InCond( TF_COND_BURNING ) )
			info.SetDamage( flPenaltyNonBurning );

		float flPenaltyHealth = info.GetDamage();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flPenaltyHealth, mult_dmg_penalty_while_half_alive );
		if ( pAttacker->GetHealth() >= ( pAttacker->GetMaxHealth() / 2 ) )
			info.SetDamage( flPenaltyHealth );

		float flBonusHealth = info.GetDamage();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flBonusHealth, mult_dmg_bonus_while_half_dead );
		if ( pAttacker->GetHealth() < ( pAttacker->GetMaxHealth() / 2 ) )
			info.SetDamage( flBonusHealth );

		int iCritVsWet = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iCritVsWet, crit_vs_wet_players );
		if ( iCritVsWet > 0 )
		{
			if ( GetWaterLevel() >= WL_Feet )
			{
				bitsDamage |= DMG_CRITICAL;
				info.AddDamageType( DMG_CRITICAL );
			}
		}

		if ( pTFAttacker )
		{
			if ( pTFAttacker->m_Shared.InCond( TF_COND_ENERGY_BUFF ) )
			{
				bitsDamage |= DMG_MINICRITICAL;
				info.AddDamageType( DMG_MINICRITICAL );
			}

			int nMarkAttackerForDeath = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFAttacker, nMarkAttackerForDeath, mod_mark_attacker_for_death );
			if ( nMarkAttackerForDeath && pTFAttacker->m_Shared.InCond( TF_COND_ENERGY_BUFF ) )
			{
				pTFAttacker->m_Shared.AddCond( TF_COND_MARKEDFORDEATH, 5 );
			}

			int nCritWhileAirborne = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCritWhileAirborne, crit_while_airborne );
			if ( nCritWhileAirborne && pTFAttacker->m_Shared.InCond( TF_COND_BLASTJUMPING ) )
			{
				bitsDamage |= DMG_CRITICAL;
				info.AddDamageType( DMG_CRITICAL );
			}

			int nMiniCritOnAirborne = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nMiniCritOnAirborne, mini_crit_airborne );
			if ( nMiniCritOnAirborne && m_Shared.InCond( TF_COND_BLASTJUMPING ) )
			{
				bitsDamage |= DMG_MINICRITICAL;
				info.AddDamageType( DMG_MINICRITICAL );
			}

			if ( pTFAttacker->m_Shared.InCond( TF_COND_DISGUISED ) )
			{
				float flDisguisedMod = info.GetDamage();
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flDisguisedMod, mult_dmg_disguised );
				info.SetDamage( flDisguisedMod );
			}



			// Notify the damaging weapon.
			pWeapon->ApplyOnHitAttributes( this, pTFAttacker, info );

			if ( GetActiveTFWeapon() )
				GetActiveTFWeapon()->ApplyOnInjuredAttributes( this, pTFAttacker, info );

			// Build rage
			bool bBannerHealed = false;
			CTFBuffItem *pBanner = ( CTFBuffItem * )pTFAttacker->Weapon_GetSlot( LOADOUT_POSITION_SECONDARY );
			if ( pBanner )
			{
				int iBannerMode = pBanner->GetBuffType();
				if ( iBannerMode == TF_BUFF_REGENONDAMAGE ) // conch
				{
					if ( pTFAttacker->m_Shared.InCond( TF_COND_REGENONDAMAGEBUFF ) )
					{
						pTFAttacker->TakeHealth( info.GetDamage() / 6.0f, DMG_GENERIC );
						bBannerHealed = true;
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", info.GetDamage() / 6.0f );
							event->SetInt( "entindex", pTFAttacker->entindex() );

							gameeventmanager->FireEvent( event );
						}
					}
					else
					{
						pTFAttacker->m_Shared.SetRageMeter( info.GetDamage() / (480 / 100), iBannerMode );
					}
				}
				else // buff banner, battalions
				{
					pTFAttacker->m_Shared.SetRageMeter( info.GetDamage() / (600 / 100), iBannerMode );
				}
			}

			// build mmmph
			int nSurpriseMotherFucker = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nSurpriseMotherFucker, burn_damage_earns_rage );
			if ( nSurpriseMotherFucker )
				pTFAttacker->m_Shared.SetRageMeter( info.GetDamage() / (600 / 100), TF_BUFF_CRITBOOSTED ); // live = 300, mvm = 1200

			if ( !bBannerHealed && m_Shared.InCond( TF_COND_MAD_MILK ) )
			{
				pTFAttacker->TakeHealth( info.GetDamage() / 2.0f, DMG_GENERIC );
				IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
				if ( event )
				{
					event->SetInt( "amount", info.GetDamage() / 2.0f );
					event->SetInt( "entindex", pTFAttacker->entindex() );

					gameeventmanager->FireEvent( event );
				}
			}

			// build hype
			int nHypeOnDamage = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nHypeOnDamage, hype_on_damage );
			if ( nHypeOnDamage && !pTFAttacker->m_Shared.InCond( TF_COND_SODAPOPPER_HYPE ) )
				pTFAttacker->m_Shared.SetScoutHypeMeter( info.GetDamage() / (350 / 100) );

			// build boost
			int nBoostOnDamage = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nBoostOnDamage, boost_on_damage );
			if ( nBoostOnDamage )
				pTFAttacker->m_Shared.SetScoutHypeMeter( info.GetDamage() );

			float flAddChargeShield = 0.0f;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFAttacker, flAddChargeShield, charge_meter_on_hit );
			if ( flAddChargeShield )
				pTFAttacker->m_Shared.SetShieldChargeMeter( min( ( pTFAttacker->m_Shared.GetShieldChargeMeter() + ( flAddChargeShield * 100 ) ), 100.0f ) );
		}

		if ( pAttacker && pAttacker->IsNPC() )
		{
			CAI_BaseNPC *pAttacker2 = (CAI_BaseNPC*)pAttacker;
			if ( pAttacker2 && (pAttacker2->InCond( TF_COND_REGENONDAMAGEBUFF ) || m_Shared.InCond( TF_COND_MAD_MILK ) )  )
			{
				if ( !pAttacker2->InCond( TF_COND_REGENONDAMAGEBUFF ) )
				{
					pAttacker2->TakeHealth(info.GetDamage() / 2.0f, DMG_GENERIC);
				}
				else
				{
					pAttacker2->TakeHealth(info.GetDamage() / 6.0f, DMG_GENERIC);
				}
			}
		}

		int nMiniCritBecomeCrits = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nMiniCritBecomeCrits, minicrits_become_crits );
		if ( nMiniCritBecomeCrits && ( bitsDamage & DMG_MINICRITICAL ) )
		{
			bitsDamage |= DMG_CRITICAL;
			info.AddDamageType( DMG_CRITICAL );
		}

		bool bAttackBehind = false;

		// Get the forward view vector of the target, ignore Z
		Vector vecVictimForward;
		AngleVectors( EyeAngles(), &vecVictimForward );
		vecVictimForward.z = 0.0f;
		vecVictimForward.NormalizeInPlace();

		// Get a vector from my origin to my targets origin
		Vector vecToTarget;
		vecToTarget = WorldSpaceCenter() - pAttacker->WorldSpaceCenter();
		vecToTarget.z = 0.0f;
		vecToTarget.NormalizeInPlace();

		// Get a forward vector of the attacker.
		Vector vecOwnerForward;
		AngleVectors( pAttacker->EyeAngles(), &vecOwnerForward );
		vecOwnerForward.z = 0.0f;
		vecOwnerForward.NormalizeInPlace();

		float flDotOwner = DotProduct( vecOwnerForward, vecToTarget );
		float flDotVictim = DotProduct( vecVictimForward, vecToTarget );

		// Make sure they're actually facing the target.
		// This needs to be done because lag compensation can place target slightly behind the attacker.
		if ( flDotOwner > 0.5 && ( flDotVictim > -0.1 ) )
		{
			bAttackBehind = true;
		}

		int nCritFromBehind = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCritFromBehind, crit_from_behind );
		if ( nCritFromBehind && bAttackBehind )
		{
			bitsDamage |= DMG_CRITICAL;
			info.AddDamageType( DMG_CRITICAL );
		}

		int nBackAttackMiniCrit = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nBackAttackMiniCrit, closerange_backattack_minicrits );
		if ( nBackAttackMiniCrit && bAttackBehind )
		{
			bitsDamage |= DMG_MINICRITICAL;
			info.AddDamageType( DMG_MINICRITICAL );
		}

		int nCritForceLaugh = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCritForceLaugh, crit_forces_victim_to_laugh );
		if ( nCritForceLaugh && ( bitsDamage & DMG_CRITICAL ) )
		{
			Taunt( TAUNTATK_NONE, MP_CONCEPT_TAUNT_LAUGH );
		}

		int nCritNoDamage = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nCritNoDamage, crit_does_no_damage );
		if ( nCritNoDamage && ( bitsDamage & DMG_CRITICAL ) )
		{
			bitsDamage &= ~( DMG_CRITICAL );
			info.SetDamage( 0 );
		}

		int nSameWeapon = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nSameWeapon, tickle_enemies_wielding_same_weapon );
		if ( nSameWeapon && ( GetActiveTFWeapon() && GetActiveTFWeapon()->GetItemID() == pWeapon->GetItemID() ) )
		{
			Taunt( TAUNTATK_NONE, MP_CONCEPT_TAUNT_LAUGH );
		}

		int iCritBecomeMiniCrits = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iCritBecomeMiniCrits, crits_become_minicrits );
		if ( iCritBecomeMiniCrits && ( bitsDamage & DMG_CRITICAL ) )
		{
			bitsDamage &= ~( DMG_CRITICAL );
			bitsDamage |= DMG_MINICRITICAL;
			info.AddDamageType( DMG_MINICRITICAL );
		}

		// Check if we're stunned and should have reduced damage taken
		/*if ( m_Shared.InCond( TF_COND_STUNNED ) && ( m_Shared.GetStunFlags() & TF_STUNFLAG_RESISTDAMAGE ) )
		{
			// Reduce our damage
			info.SetDamage( info.GetDamage() * m_Shared.m_flStunResistance );
		}*/
	}

	// If we're not damaging ourselves, apply randomness
	if ( pAttacker != this && !( bitsDamage & ( DMG_DROWN | DMG_FALL ) ) )
	{
		float flDamage = 0;
		if ( info.GetAmmoType() == TF_AMMO_PRIMARY && pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_LASER_POINTER ) )
		{
			// Wrangled shots should have damage falloff
			bitsDamage |= DMG_USEDISTANCEMOD;

			// Distance should be calculated from sentry
			pAttacker = info.GetAttacker();
		}

		if ( ( bitsDamage & DMG_BLAST ) && ( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_CANNON ) ) )
		{
			float flTimeSinceDamage = gpGlobals->curtime - GetLastDamageTime();
			if ( flTimeSinceDamage < tf_double_donk_window.GetFloat() )
			{
				// Show the attacker, unless the target is a disguised spy
				if ( pAttacker && pAttacker->IsPlayer() && !m_Shared.InCond( TF_COND_DISGUISED ) )
				{
					CEffectData	data;
					data.m_nHitBox = GetParticleSystemIndex( "doubledonk_text" );
					data.m_vOrigin = WorldSpaceCenter() + Vector(0,0,32);
					data.m_vAngles = vec3_angle;
					data.m_nEntIndex = 0;

					CSingleUserRecipientFilter filter( (CBasePlayer*)pAttacker );
					te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );

					EmitSound_t params;
					params.m_flSoundTime = 0;
					params.m_pSoundName = "TFPlayer.DoubleDonk";
					EmitSound( filter, pAttacker->entindex(), params );

					bitsDamage |= DMG_MINICRITICAL;
				}
			}
		}
		if (bitsDamage & DMG_DROWN){
			float flDrownDamageMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT(flDrownDamageMult, mult_dmg_drown);
			if (flDrownDamageMult == 0)
			{
				flDamage = 0;
				return 0;
			}
			else
				flDamage = info.GetDamage() * flDrownDamageMult;
		}
		if ( bitsDamage & DMG_CRITICAL )
		{
			if ( bDebug )
			{
				Warning( "    CRITICAL!\n");
			}

			flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER;
			if ( pAttacker && pAttacker->IsNPC() )
			{
				//TESTING: remove weird skill level damage scaling to make weapons feel consistent across difficulties!
				if (TFGameRules()->IsSkillLevel(SKILL_EASY))
					flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER / 2;
				else if ( TFGameRules()->IsSkillLevel( SKILL_MEDIUM ) )
					flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER / 1.5;
				else if ( TFGameRules()->IsSkillLevel( SKILL_HARD ) )
					flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER / 1.2;
				// I'm stupid, this is just for NPCS
			}

			// Show the attacker, unless the target is a disguised spy
			if ( pAttacker && pAttacker->IsPlayer() && !m_Shared.InCond( TF_COND_DISGUISED ) )
			{
				CEffectData	data;
				data.m_nHitBox = GetParticleSystemIndex( "crit_text" );
				data.m_vOrigin = WorldSpaceCenter() + Vector(0,0,32);
				data.m_vAngles = vec3_angle;
				data.m_nEntIndex = 0;

				CSingleUserRecipientFilter filter( (CBasePlayer*)pAttacker );
				te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );

				EmitSound_t params;
				params.m_flSoundTime = 0;
				params.m_pSoundName = "TFPlayer.CritHit";
				EmitSound( filter, pAttacker->entindex(), params );
			}

			// Burn sounds are handled in ConditionThink()
			if ( !(bitsDamage & DMG_BURN ) )
			{
				SpeakConceptIfAllowed( MP_CONCEPT_HURT, "damagecritical:1" );
			}
		}
		else if ( bitsDamage & DMG_MINICRITICAL )
		{
			if ( bDebug )
			{
				Warning( "    MINI CRITICAL!\n" );
			}
			
			if ( DMG_USEDISTANCEMOD )
			{
				// Do any calculations regarding bonus ramp up/falloff compensation.
				float flRandomDamage = info.GetDamage() * tf_damage_range.GetFloat();			
				if ( tf_damage_lineardist.GetBool() )
				{
					float flBaseDamage = info.GetDamage() - flRandomDamage;
					if ( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ) )
					{
						// If we're a crossbow, invert our damage formula.
						flDamage = flBaseDamage - RandomFloat( 0, flRandomDamage * 2 );
					}
					else
						flDamage = flBaseDamage + RandomFloat( 0, flRandomDamage * 2 );
					
					// Negate any potential damage falloff.
					if ( flDamage < info.GetDamage() )
						flDamage = info.GetDamage();
				}
				else  // If we have damage falloff, compensate falloff and add ramp up damage to our minicrit.
				{
					// Distance should be calculated from sentry
					pAttacker = info.GetAttacker();
					
					float flCenter = 0.5;
					float flCenVar = ( 10 / 100 ) ;	
					float flMin = flCenter - flCenVar;
					float flMax = flCenter + flCenVar;


					float flDistance = Max( 1.0f, ( WorldSpaceCenter() - pAttacker->WorldSpaceCenter() ).Length() );
					float flOptimalDistance = 512.0;

					// We have damage ramp up, but no damage falloff for minicrit.
					flCenter = RemapValClamped( flDistance / flOptimalDistance, 0.0, 1.0, 1.0, 0.5 );
					if ( bitsDamage & DMG_NOCLOSEDISTANCEMOD )
					{
						if ( flCenter > 0.5 )
						{
							// Reduce the damage bonus at close range
							flCenter = RemapVal( flCenter, 0.5, 1.0, 0.5, 0.65 );
						}
					}
					
					if ( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ) )
					{
						// Unlike other weapons, we don't get any compensation here.
						flCenter = RemapVal( flDistance / flOptimalDistance, 0.0, 2.0, 0.0, 0.5 );
					}

					flMin = ( 0.0 > (flCenter + flCenVar) ? 0.0 : (flCenter + flCenVar) ); // Our version of MAX.
					flMax = ( 1.0 < (flCenter + flCenVar) ? 1.0 : (flCenter - flCenVar) ); // Our version of MIN.

					if ( bDebug )
					{
						Warning( "    RANDOM: Dist %.2f, Ctr: %.2f, Min: %.2f, Max: %.2f\n", flDistance, flCenter, flMin, flMax );
					}

					//Msg("Range: %.2f - %.2f\n", flMin, flMax );
					float flRandomVal;

					if ( tf_damage_disablespread.GetBool() )
					{
						flRandomVal = flCenter;
					}
					else
					{
						flRandomVal = RandomFloat( flMin, flMax );
					}

					if ( flRandomVal > 0.5 )
					{
						// Rocket launcher, Sticky launcher and Scattergun have different short range bonuses
						if ( pWeapon )
						{
							switch ( pWeapon->GetWeaponID() )
							{
								case TF_WEAPON_ROCKETLAUNCHER:
								case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
								case TF_WEAPON_PIPEBOMBLAUNCHER:
									// Rocket launcher and sticky launcher only have half the bonus of the other weapons at short range
									flRandomDamage *= 0.5;
									break;
								case TF_WEAPON_SCATTERGUN:
									// Scattergun gets 50% bonus of other weapons at short range
									flRandomDamage *= 1.5;
									break;

								case TF_WEAPON_SHOTGUN_PRIMARY:
								case TF_WEAPON_SHOTGUN_SOLDIER:
								case TF_WEAPON_SHOTGUN_HWG:
								case TF_WEAPON_SHOTGUN_PYRO:
								case TF_WEAPON_SENTRY_REVENGE:
									// Shotguns gets 10% bonus of other weapons at short range
									flRandomDamage *= 1.1;
									break;
							}
						}	
					}

					float flOut = SimpleSplineRemapValClamped( flRandomVal, 0, 1, -flRandomDamage, flRandomDamage );
					
					flDamage = ( info.GetDamage() + flOut );
				}
			}
			else
				flDamage = info.GetDamage(); // This is kind of anticlimatic.

			// Now we can finally add on the minicrit multiplier.
			flDamage *= TF_DAMAGE_MINICRIT_MULTIPLIER;
			if ( pAttacker && pAttacker->IsNPC() )
			{
				if ( TFGameRules()->IsSkillLevel( SKILL_EASY ) )
					flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER / 1.25;
				else if ( TFGameRules()->IsSkillLevel( SKILL_MEDIUM ) )
					flDamage = info.GetDamage() * TF_DAMAGE_CRIT_MULTIPLIER / 1.2;
			}

			// Show the attacker, unless the target is a disguised spy
			if ( pAttacker && pAttacker->IsPlayer() && !m_Shared.InCond( TF_COND_DISGUISED ) )
			{
				CEffectData	data;
				data.m_nHitBox = GetParticleSystemIndex( "minicrit_text" );
				data.m_vOrigin = WorldSpaceCenter() + Vector(0,0,32);
				data.m_vAngles = vec3_angle;
				data.m_nEntIndex = 0;

				CSingleUserRecipientFilter filter( (CBasePlayer*)pAttacker );
				te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );

				EmitSound_t params;
				params.m_flSoundTime = 0;
				params.m_pSoundName = "TFPlayer.CritHitMini";
				EmitSound( filter, pAttacker->entindex(), params );
			}

			// Burn sounds are handled in ConditionThink()
			if ( !(bitsDamage & DMG_BURN ) )
			{
				SpeakConceptIfAllowed( MP_CONCEPT_HURT, "damagecritical:1" );
			}
		}
		else if ( DMG_USEDISTANCEMOD )
		{
			float flRandomDamage = info.GetDamage() * tf_damage_range.GetFloat();
			if ( tf_damage_lineardist.GetBool() )
			{
				float flBaseDamage = info.GetDamage() - flRandomDamage;
				if ( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ) )
				{
					// If we're a crossbow, invert our damage formula.
					flDamage = flBaseDamage - RandomFloat( 0, flRandomDamage * 2 );
				}
				else
					flDamage = flBaseDamage + RandomFloat( 0, flRandomDamage * 2 );
			}
			else
			{
				float flCenter = 0.5;
				float flCenVar = ( 10 / 100 ) ;
				float flMin = flCenter - flCenVar;
				float flMax = flCenter + flCenVar;

				if ( bitsDamage & DMG_USEDISTANCEMOD )
				{
					float flDistance = Max( 1.0f, ( WorldSpaceCenter() - pAttacker->WorldSpaceCenter() ).Length() );
					float flOptimalDistance = 512.0;

					flCenter = RemapValClamped( flDistance / flOptimalDistance, 0.0, 2.0, 1.0, 0.0 );
					if ( bitsDamage & DMG_NOCLOSEDISTANCEMOD )
					{
						if ( flCenter > 0.5 )
						{
							// Reduce the damage bonus at close range
							flCenter = RemapVal( flCenter, 0.5, 1.0, 0.5, 0.65 );
						}
					}
					
					if ( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ) )
					{
						// If we're a crossbow, change our falloff band so that our 100% is at long range.
						flCenter = RemapVal( flDistance / flOptimalDistance, 0.0, 2.0, 0.0, 0.5 );
					}
					
					flMin = ( 0.0 > (flCenter + flCenVar) ? 0.0 : (flCenter + flCenVar) ); // Our version of MAX.
					flMax = ( 1.0 < (flCenter + flCenVar) ? 1.0 : (flCenter - flCenVar) ); // Our version of MIN.

					if ( bDebug )
					{
						Warning( "    RANDOM: Dist %.2f, Ctr: %.2f, Min: %.2f, Max: %.2f\n", flDistance, flCenter, flMin, flMax );
					}
				}

				//Msg("Range: %.2f - %.2f\n", flMin, flMax );
				float flRandomVal;

				if ( tf_damage_disablespread.GetBool() )
				{
					flRandomVal = flCenter;
				}
				else
				{
					flRandomVal = RandomFloat( flMin, flMax );
				}

				if ( flRandomVal > 0.5 )
				{
					// Rocket launcher, Sticky launcher and Scattergun have different short range bonuses
					if ( pWeapon )
					{
						switch ( pWeapon->GetWeaponID() )
						{
							case TF_WEAPON_ROCKETLAUNCHER:
							case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
							case TF_WEAPON_PIPEBOMBLAUNCHER:
								// Rocket launcher and sticky launcher only have half the bonus of the other weapons at short range
								flRandomDamage *= 0.5;
								break;
							case TF_WEAPON_SCATTERGUN:
								// Scattergun gets 50% bonus of other weapons at short range
								flRandomDamage *= 1.5;
								break;

							case TF_WEAPON_SHOTGUN_PRIMARY:
							case TF_WEAPON_SHOTGUN_SOLDIER:
							case TF_WEAPON_SHOTGUN_HWG:
							case TF_WEAPON_SHOTGUN_PYRO:
							case TF_WEAPON_SENTRY_REVENGE:
								// Shotguns gets 10% bonus of other weapons at short range
								flRandomDamage *= 1.1;
								break;
						}
					}
				}

				float flOut = SimpleSplineRemapValClamped( flRandomVal, 0, 1, -flRandomDamage, flRandomDamage );
				
				flDamage = info.GetDamage() + flOut;

				/*
				for ( float flVal = flMin; flVal <= flMax; flVal += 0.05 )
				{
					float flOut = SimpleSplineRemapValClamped( flVal, 0, 1, -flRandomDamage, flRandomDamage );
					Msg("Val: %.2f, Out: %.2f, Dmg: %.2f\n", flVal, flOut, info.GetDamage() + flOut );
				}
				*/
			}


			// Burn sounds are handled in ConditionThink()
			if ( !( bitsDamage & DMG_BURN ) )
			{
				SpeakConceptIfAllowed( MP_CONCEPT_HURT );
			}
		}
		else
			flDamage = info.GetDamage(); // This is kind of anticlimatic.

		info.SetDamage( flDamage );
	}

	// god
	if ( m_debugOverlays & OVERLAY_BUDDHA_MODE ) 
	{
		if ((m_iHealth - info.GetDamage()) <= 0)
		{
			m_iHealth = 1;
			return 0;
		}
	}

	// wait a minute
	if ( m_Shared.InCond( TF_COND_PREVENT_DEATH ) ) 
	{
		if ((m_iHealth - info.GetDamage()) <= 0)
		{
			m_iHealth = 1;
			m_Shared.RemoveCond( TF_COND_PREVENT_DEATH );
			return 0;
		}
	}

	// helltower stuff
	if ( m_Shared.InCond( TF_COND_HALLOWEEN_IN_HELL ) ) 
	{
		if ( (m_iHealth - info.GetDamage()) <= 0 )
		{
			m_iHealth = 1;
			m_Shared.AddCond( TF_COND_HALLOWEEN_GHOST_MODE );
			return 0;
		}
	}

	// some buff before death
	if ( m_Shared.GetLivesCount() == 1 ) 
	{
		if ( ( m_iHealth - info.GetDamage()) <= 0 )
		{
			m_Shared.SetLivesCount( 0 );
			m_iHealth = 15;
			m_Shared.AddCond( TF_COND_INVULNERABLE, 3 );

			CSingleUserRecipientFilter filter( this );
			EmitSound_t params;
			params.m_flSoundTime = 0;
			params.m_pSoundName = "Player.KillSoundDefaultDing";
			EmitSound( filter, entindex(), params );
			return 0;
		}
	}

	// infection
	if ( TFGameRules()->IsInfectionMode() && ( GetTeamNumber() == TF_TEAM_RED ) )
	{
		if ( (m_iHealth - info.GetDamage()) <= 0 )
		{
			m_iHealth = 1;
			Vector vecOrigin = GetAbsOrigin();
			QAngle angAngle = GetAbsAngles();

			ChangeTeam( TF_TEAM_BLUE );
			SetDesiredPlayerClassIndex( TF_CLASS_ZOMBIEFAST );
			ForceRespawn();
			Teleport( &vecOrigin, &angAngle, NULL );
			return 0;
		}
	}

	if ( m_Shared.InCond( TF_COND_RUNE_WARLOCK ) )
	{
		if ( pAttacker && ( pAttacker->GetHealth() - ( info.GetDamage() / 1.2 ) ) >= 1 )
		{
			CTakeDamageInfo newInfo( this, this, info.GetDamage() / 1.2, DMG_GENERIC, TF_DMG_CUSTOM_RUNE_REFLECT );
			pAttacker->TakeDamage( info );
			EmitSound( "Powerup.Reflect.Reflect" );
		}
	}

	if ( pWeapon && pTFAttacker )
	{
		int nAddCloakOnHit = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nAddCloakOnHit, add_cloak_on_hit );
		if ( nAddCloakOnHit > 0 )
			pTFAttacker->m_Shared.AddToSpyCloakMeter( nAddCloakOnHit );
	}

	float flDamageTaken = info.GetDamage();
	// Vintage Battalion's Backup resists
	if ( m_Shared.InCond( TF_COND_DEFENSEBUFF ) )
	{
		// Battalion's Backup negates all crit damage
		bitsDamage &= ~( DMG_CRITICAL | DMG_MINICRITICAL );
		if ( bObject )
		{
			// 50% resistance to sentry damage
			info.SetDamage( flDamageTaken * 0.50f );
		}
		else
		{
			// 35% to all other sources
			info.SetDamage( flDamageTaken * 0.65f );
		}
	}

	if ( m_Shared.InCond( TF_COND_RUNE_RESIST ) )
	{
		bitsDamage &= ~( DMG_CRITICAL | DMG_MINICRITICAL );
		info.SetDamage( flDamageTaken * 0.50f );
		EmitSound( "Powerup.ReducedDamage" );
	}

	// NOTE: Deliberately skip base player OnTakeDamage, because we don't want all the stuff it does re: suit voice
	bTookDamage = CBaseCombatCharacter::OnTakeDamage( info );

	// If we have an attribute reducing boost/hype, reduce our level.
	if ( pWeapon )
	{
		int nLoseHypeOnDamage = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nLoseHypeOnDamage, lose_hype_on_take_damage );
		if ( nLoseHypeOnDamage != 0 )
		pTFAttacker->m_Shared.SetScoutHypeMeter( ( ( info.GetDamage() * nLoseHypeOnDamage ) * -1 ) );

	}

	// Early out if the base class took no damage
	if ( !bTookDamage )
	{
		if ( bDebug )
		{
			Warning( "    ABORTED: Player failed to take the damage.\n" );
		}
		return 0;
	}

	if ( bDebug )
	{
		Warning( "    DEALT: Player took %.2f damage.\n", info.GetDamage() );
		Warning( "    HEALTH LEFT: %d\n", GetHealth() );
	}

	// Send the damage message to the client for the hud damage indicator
	// Don't do this for damage types that don't use the indicator
	if ( !(bitsDamage & (DMG_DROWN | DMG_FALL | DMG_BURN) ) )
	{
		// Try and figure out where the damage is coming from
		Vector vecDamageOrigin = info.GetReportedPosition();

		// If we didn't get an origin to use, try using the attacker's origin
		if ( vecDamageOrigin == vec3_origin && pInflictor )
		{
			vecDamageOrigin = pInflictor->GetAbsOrigin();
		}

		if ( info.GetDamage() != 0.0f )
		{
			CSingleUserRecipientFilter user( this );
			UserMessageBegin( user, "Damage" );
				WRITE_BYTE( clamp( (int)info.GetDamage(), 0, 255 ) );
				WRITE_VEC3COORD( vecDamageOrigin );
			MessageEnd();
		}
	}

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( pInflictor && pInflictor->edict() )
	{
		m_DmgOrigin = pInflictor->GetAbsOrigin();
	}

	m_DmgTake += (int)info.GetDamage();

	// Reset damage time countdown for each type of time based damage player just sustained
	for (int i = 0; i < CDMG_TIMEBASED; i++)
	{
		// Make sure the damage type is really time-based.
		// This is kind of hacky but necessary until we setup DamageType as an enum.
		int iDamage = ( DMG_PARALYZE << i );
		if ( ( info.GetDamageType() & iDamage ) && g_pGameRules->Damage_IsTimeBased( iDamage ) )
		{
			m_rgbTimeBasedDamage[i] = 0;
		}
	}

	// Display any effect associate with this damage type
	DamageEffect( info.GetDamage(),bitsDamage );

	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	int iAimingNoFlinch = 0;
	CALL_ATTRIB_HOOK_INT( iAimingNoFlinch, aiming_no_flinch );

	if( iAimingNoFlinch != 1 && !m_Shared.InCond( TF_COND_AIMING ) )
		m_Local.m_vecPunchAngle.SetX( -2 );

	// Do special explosion damage effect
	if ( bitsDamage & DMG_BLAST )
		OnDamagedByExplosion( info );


		

	PainSound( info );

	if( iAimingNoFlinch != 1 && !m_Shared.InCond( TF_COND_AIMING ) )
		PlayFlinch( info );

	// Detect drops below 25% health and restart expression, so that characters look worried.
	int iHealthBoundary = (GetMaxHealth() * 0.25);
	if ( GetHealth() <= iHealthBoundary && iHealthBefore > iHealthBoundary )
	{
		ClearExpression();
	}

	CTF_GameStats.Event_PlayerDamage( this, info, iHealthBefore - GetHealth() );

	// Post modified weapon effects
	if ( pWeapon )
		pWeapon->ApplyPostHitEffects( inputInfo, this );

	return bTookDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DamageEffect(float flDamage, int fDamageType)
{
	bool bDisguised = m_Shared.InCond( TF_COND_DISGUISED );

	if (fDamageType & DMG_CRUSH)
	{
		//Red damage indicator
		color32 red = {128,0,0,128};
		UTIL_ScreenFade( this, red, 1.0f, 0.1f, FFADE_IN );
	}
	else if (fDamageType & DMG_DROWN)
	{
		//Blue damage indicator
		float flDrownDamageMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT(flDrownDamageMult, mult_dmg_drown);
		if (flDrownDamageMult >= 0)
		{
			return;
		}
		else{
			color32 blue = { 0, 0, 128, 128 };
			UTIL_ScreenFade(this, blue, 1.0f, 0.1f, FFADE_IN);
		}
	}
	else if (fDamageType & DMG_SLASH)
	{
		if ( !bDisguised )
		{
			// bots don't have blood
			if ( TFGameRules() && TFGameRules()->IsMvMModelsAllowed() )
			{
				g_pEffects->Sparks( EyePosition() );
				if ( random->RandomFloat( 0, 2 ) >= 1 )
					UTIL_Smoke( EyePosition(), random->RandomInt( 10, 15 ), 10 );
			}
			else
			{
				// If slash damage shoot some blood
				SpawnBlood(EyePosition(), g_vecAttackDir, BloodColor(), flDamage);
			}

		}
	}
	/*else if (fDamageType & DMG_PLASMA) // cremator effect shouldn't be on pyro
	{
		// Blue screen fade
		color32 blue = {0,0,255,100};
		UTIL_ScreenFade( this, blue, 0.2, 0.4, FFADE_MODULATE );

		// Very small screen shake
		// Both -0.1 and 0.1 map to 0 when converted to integer, so all of these RandomInt
		// calls are just expensive ways of returning zero. This code has always been this
		// way and has never had any value. clang complains about the conversion from a
		// literal floating-point number to an integer.
		//ViewPunch(QAngle(random->RandomInt(-0.1,0.1), random->RandomInt(-0.1,0.1), random->RandomInt(-0.1,0.1)));

		// Burn sound 
		EmitSound( "Player.PlasmaDamage" );
	}*/
	else if (fDamageType & DMG_SONIC)
	{
		// Sonic damage sound 
		if ( !bDisguised )
			EmitSound( "Player.SonicDamage" );
	}
	else if ( fDamageType & DMG_BULLET )
	{
		if ( !bDisguised )
		{
			if ( TFGameRules() && TFGameRules()->IsMvMModelsAllowed() )
				EmitSound( "MVM_Weapon_Default.HitFlesh" );
			else
				EmitSound( "Flesh.BulletImpact" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( ( ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT ) && tf_avoidteammates.GetBool() ) ||
		collisionGroup == TFCOLLISION_GROUP_ROCKETS ||
		/*collisionGroup == COLLISION_GROUP_NPC_ACTOR ||
		collisionGroup == COLLISION_GROUP_NPC ||*/
		collisionGroup == HL2COLLISION_GROUP_COMBINE_BALL ||
		collisionGroup == HL2COLLISION_GROUP_COMBINE_BALL_NPC )
	{

		switch( GetTeamNumber() )
		{
		case TF_TEAM_RED:
			if ( !( contentsMask & CONTENTS_REDTEAM ) )
				return false;
			break;

		case TF_TEAM_BLUE:
			if ( !( contentsMask & CONTENTS_BLUETEAM ) )
				return false;
			break;

		case TF_TEAM_GREEN:
			if ( !(contentsMask & CONTENTS_GREENTEAM ) )
				return false;
			break;

		case TF_TEAM_YELLOW:
			if ( !(contentsMask & CONTENTS_YELLOWTEAM ) )
				return false;
			break;
		}
	}

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

//---------------------------------------
// Is the player the passed player class?
//---------------------------------------
bool CTFPlayer::IsPlayerClass( int iClass ) const
{
	const CTFPlayerClass *pClass = &m_PlayerClass;

	if ( !pClass )
		return false;

	return ( pClass->IsClass( iClass ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsPlayerNPCClass( void ) const
{
	return ( IsPlayerClass( TF_CLASS_COMBINE ) || IsPlayerClass( TF_CLASS_ZOMBIEFAST ) || IsPlayerClass( TF_CLASS_WILDCARD ) );
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the player is currently able to enter the vehicle
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFPlayer::CanEnterVehicle( IServerVehicle *pVehicle, int nRole )
{
	// Must not have a passenger there already
	if ( pVehicle->GetPassenger( nRole ) )
		return false;

	// Must be alive
	if ( !IsAlive() )
		return false;

	// Can't be pulled by a barnacle
	if ( IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Put this player in a vehicle 
//-----------------------------------------------------------------------------
bool CTFPlayer::GetInVehicle( IServerVehicle *pVehicle, int nRole )
{
	Assert( NULL == m_hVehicle.Get() );

	// Make sure we can enter the vehicle
	if ( CanEnterVehicle( pVehicle, nRole ) == false )
		return false;

	CBaseEntity *pEnt = pVehicle->GetVehicleEnt();
	Assert( pEnt );

	// Try to stow weapons
	if ( pVehicle->IsPassengerUsingStandardWeapons( nRole ) == false )
	{
		CBaseCombatWeapon *pWeapon = GetActiveWeapon();
		if ( pWeapon != NULL )
		{
			pWeapon->Lower();
			//pWeapon->SetWeaponVisible( false );
		}

		if ( nRole != VEHICLE_ROLE_DRIVER )
			m_Local.m_iHideHUD |= HIDEHUD_WEAPONSELECTION;

		m_Local.m_iHideHUD |= HIDEHUD_INVEHICLE;
	}

	// Put us in the vehicle
	pVehicle->SetPassenger( nRole, this );

	ViewPunchReset();

	// Setting the velocity to 0 will cause the IDLE animation to play
	SetAbsVelocity( vec3_origin );
	SetMoveType( MOVETYPE_NOCLIP );

	// This is a hack to fixup the player's stats since they really didn't "cheat" and enter noclip from the console
	gamestats->Event_DecrementPlayerEnteredNoClip( this );

	// Get the seat position we'll be at in this vehicle
	Vector vSeatOrigin;
	QAngle qSeatAngles;
	pVehicle->GetPassengerSeatPoint( nRole, &vSeatOrigin, &qSeatAngles );
	
	// Set us to that position
	SetAbsOrigin( vSeatOrigin );
	SetAbsAngles( qSeatAngles );
	
	// Parent to the vehicle
	SetParent( pEnt );

	SetCollisionGroup( COLLISION_GROUP_IN_VEHICLE );
	
	// We cannot be ducking -- do all this before SetPassenger because it
	// saves our view offset for restoration when we exit the vehicle.
	RemoveFlag( FL_DUCKING );
	SetViewOffset( VEC_VIEW_SCALED( this ) );
	m_Local.m_bDucked = false;
	m_Local.m_bDucking  = false;
	m_Local.m_flDucktime = 0.0f;
	m_Local.m_flDuckJumpTime = 0.0f;
	m_Local.m_flJumpTime = 0.0f;

	// Turn our toggled duck off
	if ( GetToggledDuckState() )
		ToggleDuck();

	m_hVehicle = pEnt;

	// Throw an event indicating that the player entered the vehicle.
	g_pNotify->ReportNamedEvent( this, "PlayerEnteredVehicle" );

	m_iVehicleAnalogBias = VEHICLE_ANALOG_BIAS_NONE;

	RemoveInvisibility();
	RemoveDisguise();
	m_Shared.m_bFeignDeathReady = false;
	HolsterOffHandWeapon();

	OnVehicleStart();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	BaseClass::LeaveVehicle( vecExitPoint, vecExitAngles );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::CommitSuicide( bool bExplode /* = false */, bool bForce /*= false*/ )
{
	// Don't suicide if we haven't picked a class for the first time, or we're not in active state
	if ( IsPlayerClass( TF_CLASS_UNDEFINED ) || !m_Shared.InState( TF_STATE_ACTIVE ) )
		return;

	// Don't suicide during the "bonus time" if we're not on the winning team
	if ( !bForce && TFGameRules()->State_Get() == GR_STATE_TEAM_WIN && 
		 GetTeamNumber() != TFGameRules()->GetWinningTeam() )
	{
		return;
	}

	m_iSuicideCustomKillFlags = TF_DMG_CUSTOM_SUICIDE;

	BaseClass::CommitSuicide( bExplode, bForce );

	if ( TFGameRules()->IsInfectionMode() && ( GetTeamNumber() == TF_TEAM_RED ) )
	{
		ChangeTeam( TF_TEAM_BLUE );
		SetDesiredPlayerClassIndex( TF_CLASS_ZOMBIEFAST );
		ForceRespawn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : int
//-----------------------------------------------------------------------------
int CTFPlayer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// Grab the vector of the incoming attack. 
	// (Pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	CBaseEntity *pAttacker = info.GetAttacker();
	CTFPlayer 	*pTFAttacker = ToTFPlayer( pAttacker );
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pWeapon = info.GetWeapon();
	CTFWeaponBase *pTFWeapon = dynamic_cast<CTFWeaponBase *>( pWeapon );
	CTFPlayer *pScorer = ToTFPlayer( TFGameRules()->GetDeathScorer( pAttacker, pInflictor, this ) );
	CBaseEntity *pAssister =  TFGameRules()->GetAssister( this, pScorer, pInflictor );
	CTFPlayer *pTFAssister = ToTFPlayer( pAssister );

	int iOldHealth = m_iHealth;

	// If this is an object get the builder
	if ( pAttacker->IsBaseObject() )
	{
		CBaseObject *pObject = static_cast< CBaseObject * >( pAttacker );
		pAttacker = pObject->GetBuilder();
	}

	Vector vecDir = vec3_origin;
	if ( pInflictor )
	{
		// Huntsman should not do too much knockback
		if ( pTFWeapon && ( pTFWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pTFWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ) )
		{
			vecDir = -info.GetDamageForce();
		}
		else
		{
			vecDir = pInflictor->WorldSpaceCenter() - Vector( 0.0f, 0.0f, 10.0f ) - WorldSpaceCenter();
		}
		VectorNormalize( vecDir );
	}
	g_vecAttackDir = vecDir;

	if ( m_Shared.m_bFeignDeathReady && m_Shared.GetSpyCloakMeter() == 100.0f && !m_Shared.InCond( TF_COND_STEALTHED ) )
		FeignDeath( info );

	TeamFortress_SetSpeed();

	// Do the damage.
	// NOTE: None of the damage modifiers used here should affect the knockback force.
	m_bitsDamageType |= info.GetDamageType();
	float flDamage = info.GetDamage();

	if ( flDamage == 0.0f )
		return 0;

	// Self-damage modifiers.
	if ( pTFAttacker == this )
	{
		if ( ( info.GetDamageType() & DMG_BLAST ) && !info.GetDamagedOtherPlayers() )
		{
			CALL_ATTRIB_HOOK_FLOAT( flDamage, rocket_jump_dmg_reduction );
		}

		if ( pWeapon )
		{
			int iNoSelfDmg = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iNoSelfDmg, no_self_blast_dmg );
			if ( pTFAttacker->m_Shared.InCond( TF_COND_RUNE_PRECISION ) )
				iNoSelfDmg = 2;

			if ( iNoSelfDmg )
			{
				flDamage = 0.0f;
			}
			else
			{
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flDamage, blast_dmg_to_self );
			}
		}
	}

	int nIgnoreResists = 0;
	if ( pTFAttacker != this && pWeapon )
	{
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nIgnoreResists, mod_pierce_resists_absorbs );
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flDamage, mult_dmg_vs_players );
	}

	if ( info.GetDamageType() & DMG_CRITICAL )
	{
		float flDamageCritMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageCritMult, mult_dmgtaken_from_crit );
		if ( (nIgnoreResists == 1) && ( flDamageCritMult < 1.0f ) )
			flDamageCritMult = 1.0f;
		flDamage *= flDamageCritMult;
	}

	if ( info.GetDamageType() & DMG_BLAST )
	{
		float flDamageBlastMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageBlastMult, mult_dmgtaken_from_explosions );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_SMALL_BLAST_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageBlastMult, medigun_blast_resist_passive );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_UBER_BLAST_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageBlastMult, medigun_blast_resist_deployed );

		if ( (nIgnoreResists == 1) && ( flDamageBlastMult < 1.0f ) )
			flDamageBlastMult = 1.0f;

		flDamage *= flDamageBlastMult;

		if ( m_Shared.InCond( TF_COND_BLAST_IMMUNE ) )
			flDamage = 0.0;
	}

	if ( info.GetDamageType() & DMG_BURN|DMG_IGNITE )
	{
		float flDamageFireMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageFireMult, mult_dmgtaken_from_fire );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_SMALL_FIRE_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageFireMult, medigun_fire_resist_passive );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_UBER_FIRE_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageFireMult, medigun_fire_resist_deployed );

		if ( (nIgnoreResists == 1) && ( flDamageFireMult < 1.0f ) )
			flDamageFireMult = 1.0f;

		flDamage *= flDamageFireMult;

		if ( m_Shared.InCond( TF_COND_FIRE_IMMUNE ) )
			flDamage = 0.0f;
	}

	if ( info.GetDamageType() & DMG_BULLET|DMG_BUCKSHOT )
	{
		float flDamageBulletMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageBulletMult, mult_dmgtaken_from_bullets );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_SMALL_BULLET_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageBulletMult, medigun_bullet_resist_passive );

		if ( m_Shared.InCond( TF_COND_MEDIGUN_UBER_BULLET_RESIST ) )
			CALL_ATTRIB_HOOK_FLOAT( flDamageBulletMult, medigun_bullet_resist_deployed );

		flDamage *= flDamageBulletMult;

		if ( m_Shared.InCond( TF_COND_BULLET_IMMUNE ) )
			flDamage = 0.0f;
	}

	if (info.GetDamageType() & DMG_DROWN)
	{
		float flDamageDrownMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT(flDamageDrownMult, mult_dmg_drown);
		if (flDamageDrownMult <= 0.01f)
			flDamage = 0.0f;
		else
			flDamage *= flDamageDrownMult;
	}
	if ( info.GetDamageType() & DMG_CLUB|DMG_SLASH )
	{
		float flDamageMeleeMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageMeleeMult, mult_dmgtaken_from_melee );
		if ( (nIgnoreResists == 1) && ( flDamageMeleeMult < 1.0f ) )
			flDamageMeleeMult = 1.0f;
		flDamage *= flDamageMeleeMult;
	}
	if (info.GetDamageType() & DMG_BULLET | DMG_BUCKSHOT | DMG_IGNITE | DMG_BLAST | DMG_SONIC)
	{
		float flDamageRangedMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT(flDamageRangedMult, dmg_from_ranged);
		if ((nIgnoreResists == 1) && (flDamageRangedMult < 1.0f))
			flDamageRangedMult = 1.0f;
		flDamage *= flDamageRangedMult;
	}
	float flDamageAllMult = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flDamageAllMult, mult_dmgtaken );
	if ( (nIgnoreResists == 1) && ( flDamageAllMult < 1.0f ) )
		flDamageAllMult = 1.0f;
	flDamage *= flDamageAllMult;

	if ( pInflictor && pInflictor->IsBaseObject() )
	{
		float flDamageSentryMult = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamageSentryMult, dmg_from_sentry_reduced );
		if ( (nIgnoreResists == 1) && ( flDamageSentryMult < 1.0f ) )
			flDamageSentryMult = 1.0f;
		flDamage *= flDamageSentryMult;
	}

	if ( GetActiveTFWeapon() )
	{
		if ( info.GetDamageType() & (DMG_CLUB | DMG_SLASH) )
		{
			float flDamageClubMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetActiveTFWeapon(), flDamageClubMult, dmg_from_melee );
			CALL_ATTRIB_HOOK_FLOAT(flDamageClubMult, dmg_from_melee);
			if ( (nIgnoreResists == 1) && ( flDamageClubMult < 1.0f ) )
				flDamageClubMult = 1.0f;
			flDamage *= flDamageClubMult;
		}

		if ( info.GetDamageType() & DMG_BULLET|DMG_BUCKSHOT|DMG_IGNITE|DMG_BLAST|DMG_SONIC )
		{
			float flDamageRangedMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetActiveTFWeapon(), flDamageRangedMult, dmg_from_ranged );
			CALL_ATTRIB_HOOK_FLOAT(flDamageRangedMult, dmg_from_ranged);
			if ( (nIgnoreResists == 1) && ( flDamageRangedMult < 1.0f ) )
				flDamageRangedMult = 1.0f;
			flDamage *= flDamageRangedMult;
		}
		
		if ( info.GetDamageType() & DMG_BURN|DMG_IGNITE )
		{
			float flDamageFireActiveMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetActiveTFWeapon(), flDamageFireActiveMult, mult_dmgtaken_from_fire_active );
			CALL_ATTRIB_HOOK_FLOAT(flDamageFireActiveMult, mult_dmgtaken_from_fire_active);
			if ( (nIgnoreResists == 1) && ( flDamageFireActiveMult < 1.0f ) )
				flDamageFireActiveMult = 1.0f;
			flDamage *= flDamageFireActiveMult;
		}

		if ( m_Shared.InCond( TF_COND_AIMING ) )
		{
			if ( ( GetHealth() < ( GetMaxHealth() / 2 ) ) )
			{
				float flDamageAimMult = 1.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetActiveTFWeapon(), flDamageAimMult, spunup_damage_resistance );
				if ( (nIgnoreResists == 1) && ( flDamageAimMult < 1.0f ) )
					flDamageAimMult = 1.0f;
				flDamage *= flDamageAimMult;
			}
		}
	}

	if ( info.GetDamageCustom() != TF_DMG_CUSTOM_TELEFRAG )
	{
		if ( m_Shared.InCond( TF_COND_FEIGN_DEATH ) )
		{
			flDamage *= tf_feign_death_damage_scale.GetFloat();
		}
		else if ( m_Shared.m_bFeignDeathReady )
		{
			flDamage *= tf_feign_death_activate_damage_scale.GetFloat();
		}
	}

	bool bIgniting = false;
	float flBleeding = 0.0f;

	if ( m_takedamage != DAMAGE_EVENTS_ONLY )
	{
		// Start burning if we took ignition damage
		bIgniting = ( ( info.GetDamageType() & DMG_IGNITE ) && ( GetWaterLevel() < WL_Waist ) );
		if ( !bIgniting )
		{
			int iIgniting = 0;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, iIgniting, set_dmgtype_ignite );
			bIgniting = ( iIgniting > 0 );

			if ( !bIgniting )
				bIgniting = m_Shared.InCond( TF_COND_GAS );
		}

		if ( m_Shared.InCond( TF_COND_GRAPPLINGHOOK_LATCHED ) )
			flBleeding = 0.5f;

		if ( info.GetDamageCustom() != TF_DMG_CUSTOM_BLEEDING )
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFWeapon, flBleeding, bleeding_duration );

		// Take damage - round to the nearest integer.
		m_iHealth -= ( flDamage + 0.5f );
	}

	m_flLastDamageTime = gpGlobals->curtime;

	// Apply a damage force.
	if ( !pAttacker )
		return 0;

	ApplyPushFromDamage( info, vecDir );

	if ( bIgniting )
		m_Shared.Burn( pTFAttacker, pTFWeapon );

	if ( flBleeding > 0.0f )
	{
		int flBleedDamage = TF_BLEEDING_DAMAGE;
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFWeapon, flBleedDamage, mult_wpn_bleeddmg );
		m_Shared.MakeBleed( pTFAttacker, pTFWeapon, flBleeding, flBleedDamage );
	}

	if ( pTFWeapon )
	{
		// Special case for Sydney, we base it on charge
		if ( pTFWeapon->IsWeapon( TF_WEAPON_SNIPERRIFLE ) )
		{
			CTFSniperRifle *pSniper = assert_cast<CTFSniperRifle *>( pTFWeapon );

			float flJarateDuration = pSniper->GetJarateTime();
			if ( flJarateDuration > 0.0f )
				m_Shared.AddCond( TF_COND_URINE, flJarateDuration );
		}
		else
		{
			float flJarateDuration = 0.0f;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pTFWeapon, flJarateDuration, jarate_duration );
			if ( flJarateDuration > 0.0f )
				m_Shared.AddCond( TF_COND_URINE, flJarateDuration );
		}
	}

	// Fire a global game event - "player_hurt"
	IGameEvent *event = gameeventmanager->CreateEvent( "player_hurt" );
	if ( event )
	{
		event->SetInt( "userid", GetUserID() );
		event->SetInt( "health", max( 0, m_iHealth ) );
		event->SetInt( "damageamount", ( iOldHealth - m_iHealth ) );
		event->SetBool( "crit", ( info.GetDamageType() & DMG_CRITICAL ) != 0 );
		event->SetBool( "minicrit", ( info.GetDamageType() & DMG_MINICRITICAL ) != 0 );

		// HLTV event priority, not transmitted
		event->SetInt( "priority", 5 );	

		CBasePlayer *pPlayer = ToBasePlayer( pAttacker );
		event->SetInt( "attacker", pPlayer ? pPlayer->GetUserID() : 0 );

		event->SetInt( "victim_index", entindex() );
		event->SetInt( "attacker_index", pAttacker->entindex() );

        gameeventmanager->FireEvent( event );
	}

	if ( pTFWeapon && pTFWeapon->IsWeapon( TF_WEAPON_BAT_FISH ) )
	{
		int iFishOverride = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFWeapon, iFishOverride, fish_damage_override );

		const char *pszFishEvent = "fish_notice";
		if ( iFishOverride == 1 )
			pszFishEvent = "fish_notice__arm";

		IGameEvent *eventfish = gameeventmanager->CreateEvent( pszFishEvent );
		if ( eventfish )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetInt( "victim_index", entindex() );
			event->SetInt( "attacker", pTFAttacker ? pTFAttacker->GetUserID() : 0 );
			event->SetInt( "attacker_index", pAttacker->entindex() );
			event->SetString( "attacker_name", pAttacker ? pAttacker->GetClassname() : NULL );
			event->SetInt( "attacker_team", pAttacker ? pAttacker->GetTeamNumber() : 0 );
			event->SetInt( "assister", pTFAssister ? pTFAssister->GetUserID() : -1 );
			event->SetInt( "assister_index", pAssister ? pAssister->entindex() : -1 );
			event->SetString( "assister_name", pAssister ? pAssister->GetClassname() : NULL );
			event->SetInt( "assister_team", pAssister ? pAssister->GetTeamNumber() : 0 );
			event->SetString( "weapon", ( iFishOverride == 1 ) ? "unarmed_combat" : "holymackerel" );
			event->SetInt( "weaponid", pTFWeapon->GetWeaponID() );
			event->SetString( "weapon_logclassname", ( iFishOverride == 1 ) ? "unarmed_combat" : "holymackerel" );
			event->SetInt( "damagebits", info.GetDamageType() );
			event->SetInt( "customkill", info.GetDamageCustom() );
			event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted
			event->SetInt( "death_flags", GetDeathFlags() );

			gameeventmanager->FireEvent( event );
		}
	}

	if ( pAttacker != this && pAttacker->IsPlayer() )
	{
		pTFAttacker->RecordDamageEvent( info, (m_iHealth <= 0) );
	}

	//No bleeding while invul or disguised.
	bool bBleed = ( ( m_Shared.InCond( TF_COND_DISGUISED ) == false || m_Shared.InCond( TF_COND_PHASE ) == false || m_Shared.GetDisguiseTeam() == GetTeamNumber() ) &&
		m_Shared.IsInvulnerable() == false );
	if ( bBleed && pTFAttacker )
	{
		CTFWeaponBase *pWeapon = pTFAttacker->GetActiveTFWeapon();
		if ( pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER )
		{
			bBleed = false;
		}
	}

	if ( bBleed )
	{
		Vector vDamagePos = info.GetDamagePosition();

		if ( vDamagePos == vec3_origin )
		{
			vDamagePos = WorldSpaceCenter();
		}

		CPVSFilter filter( vDamagePos );
		TE_TFBlood( filter, 0.0, vDamagePos, -vecDir, entindex() );
	}

	// Done.
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::ApplyPushFromDamage( const CTakeDamageInfo &info, Vector &vecDir )
{
	CBaseEntity *pAttacker = info.GetAttacker();
	CBaseEntity *pInflictor = info.GetInflictor();

	if ( info.GetDamageType() & DMG_PREVENT_PHYSICS_FORCE )
		return;

	if( !pInflictor ||
		( GetMoveType() != MOVETYPE_WALK ) ||
		( pAttacker->IsSolidFlagSet( FSOLID_TRIGGER ) ) ||
		( m_Shared.InCond( TF_COND_DISGUISED ) ) )
		return;

	float flDamage = info.GetDamage();

	float flForceMultiplier = 1.0f;
	Vector vecForce;
	vecForce.Init();
	if ( pAttacker == this )
	{
		if ( IsPlayerClass( TF_CLASS_SOLDIER ) && ( info.GetDamageType() & DMG_BLAST ) )
		{
			// Since soldier only takes reduced self-damage while in mid-air we have to accomodate for that.
			float flScale = 1.0f;

			if ( GetFlags() & FL_ONGROUND )
			{
				flScale = tf_damageforcescale_self_soldier_badrj.GetFloat();
				SetBlastJumpState( TF_JUMP_ROCKET, false );
			}
			else
			{
				// If we're in mid-air then the code in OnTakeDamage should have already set blast jumping state.
				flScale = tf_damageforcescale_self_soldier_rj.GetFloat();
			}

			vecForce = vecDir * -DamageForce( WorldAlignSize(), flDamage, flScale );
		}
		else if ( IsPlayerClass( TF_CLASS_PYRO ) && ( m_Shared.InCond( TF_COND_ROCKETPACK ) ) )
		{
			vecForce = vecDir * -DamageForce( WorldAlignSize(), flDamage, tf_damageforcescale_pyro_jump.GetFloat() );
		}
		else
		{
			vecForce = vecDir * -DamageForce( WorldAlignSize(), flDamage, DAMAGE_FORCE_SCALE_SELF );
		}

		// If we damaged ourselves, check the force modifier attribute.
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( ToTFPlayer(pAttacker), flForceMultiplier, mult_dmgself_push_force );
		if ( flForceMultiplier != 1.0f )
			vecForce *= flForceMultiplier;
	}
	else
	{
		// Sentryguns push a lot harder
		if ( ( info.GetDamageType() & DMG_BULLET ) && pInflictor->IsBaseObject() )
		{
			vecForce = vecDir * -DamageForce( WorldAlignSize(), flDamage, 16 );
		}
		else
		{
			vecForce = vecDir * -DamageForce( WorldAlignSize(), flDamage, tf_damageforcescale_other.GetFloat() );

			if ( IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
			{
				// Heavies take less push from non sentryguns
				vecForce *= 0.5;
			}
		}

		if ( info.GetDamageType() & DMG_BLAST )
		{
			m_bBlastLaunched = true;
		}

		// If we were damaged by someone else, reduce the amount of force with an attribute.
		// Try moving this below as it weirdly isn't noticeable when damaged by NPCs. Maybe I'm just blind, though.
	//	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pAttacker, flForceMultiplier, damage_force_reduction );
	//	if ( flForceMultiplier != 1.0f )
	//		vecForce *= flForceMultiplier;

		if ( m_Shared.InCond( TF_COND_MEGAHEAL ) )
			vecForce *= 0.25f;
	}

	ApplyAbsVelocityImpulse( vecForce );

	for ( int i = 0; i < m_Shared.m_aHealers.Count(); i++ )
	{
		CTFPlayer *pHealer = ToTFPlayer( static_cast<CBaseEntity *>( m_Shared.GetHealerByIndex( i ) ) );
		if ( pHealer && pHealer->GetMedigun() && ( pHealer->GetMedigun()->GetMedigunType() == TF_MEDIGUN_QUICKFIX ) )
			pHealer->ApplyAbsVelocityImpulse( vecForce );
	}
}

void CTFPlayer::ApplyAbsVelocityImpulse( const Vector &inVecImpulse )
{
	if ( inVecImpulse != vec3_origin )
	{
		Vector vecImpulse = inVecImpulse;

		// Safety check against receive a huge impulse, which can explode physics
		switch ( CheckEntityVelocity( vecImpulse ) )
		{
			case -1:
				Warning( "Discarding ApplyAbsVelocityImpulse(%f,%f,%f) on %s\n", vecImpulse.x, vecImpulse.y, vecImpulse.z, GetDebugName() );
				Assert( false );
				return;
			case 0:
				if ( CheckEmitReasonablePhysicsSpew() )
				{
					Warning( "Clamping ApplyAbsVelocityImpulse(%f,%f,%f) on %s\n", inVecImpulse.x, inVecImpulse.y, inVecImpulse.z, GetDebugName() );
				}
				break;
		}

		Vector vecResult;

		Vector vecModImpulse = vecImpulse;
		float flImpulseScale = 1.0f;
		if ( m_Shared.InCond( TF_COND_AIMING ) )
			CALL_ATTRIB_HOOK_FLOAT( flImpulseScale, mult_aiming_knockback_resistance );

		CALL_ATTRIB_HOOK_FLOAT(flImpulseScale, damage_force_reduction);

		if ( m_Shared.InCond( TF_COND_HALLOWEEN_TINY ) )
			flImpulseScale *= 2.0f;

		if ( m_Shared.InCond( TF_COND_PARACHUTE_ACTIVE ) )
		{
			vecModImpulse.x *= 1.5f;
			vecModImpulse.y *= 1.5f;
		}

		VectorAdd( GetAbsVelocity(), vecModImpulse * flImpulseScale, vecResult );
		SetAbsVelocity( vecResult );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::ApplyGenericPushbackImpulse( const Vector &vecDir )
{
	if ( m_Shared.GetCarryingRuneType() == TF_RUNE_KNOCKOUT ) 
		return;

	float flAirblastVulMulti = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flAirblastVulMulti, airblast_vulnerability_multiplier );

	float flAirblastVerVulMulti = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flAirblastVerVulMulti, airblast_vertical_vulnerability_multiplier );

	Vector dp = vecDir;

	/*float hull_size_factor = ( ( WorldAlignSize().x / 48) * ( WorldAlignSize().y / 48) * ( WorldAlignSize().z / 82) );
	if ( lfe_use_hl2_player_hull.GetBool() )
		hull_size_factor = ( ( WorldAlignSize().x / 24) * ( WorldAlignSize().y / 24) * ( WorldAlignSize().z / 64) );

	float flImpulse = 360 / hull_size_factor;
	flImpulse = MIN( 1000.0, flImpulse );

	dp *= flImpulse;*/

	// do the z-minning BEFORE applying the regular vulnerability multiplier
	if ( tf_airblast_cray_ground_reflect.GetBool() )
	{
		if (( GetFlags() & FL_ONGROUND) != 0) {
			dp.z = Min(dp.z, tf_airblast_cray_ground_minz.GetFloat());
		}
	}

	dp   *= flAirblastVulMulti;
	dp.z *= flAirblastVerVulMulti;

	SetGroundEntity( NULL );
	SetAirblastState( true );

	ApplyAbsVelocityImpulse(dp);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::FeignDeath( const CTakeDamageInfo &info )
{
	if ( !IsAlive() || !IsPlayerClass( TF_CLASS_SPY ) )
		return;

	SetDeathFlags( TF_DEATH_FEIGN_DEATH );
	m_Shared.AddCond( TF_COND_FEIGN_DEATH, tf_feign_death_duration.GetFloat() );
	TFGameRules()->DeathNotice( this, info );

	if ( GetActiveWeapon() )
	{
		int nDropsHealthPack = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetActiveWeapon(), nDropsHealthPack, drop_health_pack_on_kill );
		if ( nDropsHealthPack == 1 )
			DropHealthPack( info );
	}

	SpeakConceptIfAllowed( MP_CONCEPT_DIED );

	bool bDisguised = m_Shared.InCond( TF_COND_DISGUISED );
	bool bFriendly = false;
	if ( bDisguised )
	{
		int iDisguiseTeam = m_Shared.GetDisguiseTeam();
		bFriendly = GetTeamNumber() == iDisguiseTeam;
	}

	bool bGib = ShouldGib( info );

	bool bBurning = m_Shared.InCond( TF_COND_BURNING );
	if ( bFriendly && !m_Shared.InCond( TF_COND_DISGUISED_AS_DISPENSER ) )
		bBurning &= GetPlayerClass()->GetClassIndex() != 7;

	if ( HasTheFlag() )
		DropFlag();

	if ( m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
		DropRune();

	RemoveTeleportEffect();

	// Reset our model if we were disguised
	if ( bDisguised )
		UpdateModel();

	RemoveTeleportEffect();

	m_Shared.RemoveCond( TF_COND_BURNING );
	m_Shared.RemoveCond( TF_COND_BLEEDING );

	EmitSound( "BaseCombatCharacter.StopWeaponSounds" );

	DropAmmoPack( info, false, true );

	CreateFeignDeathRagdoll( info, bGib, bBurning, bFriendly );
}

//-----------------------------------------------------------------------------
// Purpose: mini-crits for reserve shooter
//-----------------------------------------------------------------------------
void CTFPlayer::SetBlastJumpState( int iJumpType, bool bPlaySound )
{
	m_nBlastJumpFlags |= iJumpType;

	const char *pszEventName = NULL;

	switch ( iJumpType )
	{
	case TF_JUMP_ROCKET:
		pszEventName = "rocket_jump";
		break;
	case TF_JUMP_STICKY:
		pszEventName = "sticky_jump";
		break;
	}

	if ( pszEventName )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( pszEventName );

		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetBool( "playsound", bPlaySound );
			gameeventmanager->FireEvent( event );
		}
	}

	m_Shared.AddCond( TF_COND_BLASTJUMPING );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::ClearBlastJumpState( void )
{
	const char *pszEventName = NULL;

	if ( m_nBlastJumpFlags & TF_JUMP_ROCKET )
	{
		pszEventName = "rocket_jump_landed";
	}
	else if ( m_nBlastJumpFlags & TF_JUMP_STICKY )
	{
		pszEventName = "sticky_jump_landed";
	}

	if ( pszEventName )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( pszEventName );

		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			gameeventmanager->FireEvent( event );
		}
	}

	m_nBlastJumpFlags = 0;
	m_bJumpEffect = false;
	m_Shared.RemoveCond( TF_COND_BLASTJUMPING );
}

//-----------------------------------------------------------------------------
// Purpose: no mini-crits for reserve shooter
//-----------------------------------------------------------------------------
void CTFPlayer::SetAirblastState( bool bAirblastState )
{
	if ( bAirblastState )
		m_Shared.AddCond( TF_COND_KNOCKED_INTO_AIR );
	else
		m_Shared.RemoveCond( TF_COND_KNOCKED_INTO_AIR );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::ClearAirblastState( void )
{
	m_Shared.RemoveCond( TF_COND_KNOCKED_INTO_AIR );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::SetForceByNature( bool bForced )
{
	if ( bForced && ( GetGroundEntity() == NULL ) )
		m_bForceByNature = true;
	else
		m_bForceByNature = false;
}

//-----------------------------------------------------------------------------
// Purpose: Adds this damager to the history list of people who damaged player
//-----------------------------------------------------------------------------
void CTFPlayer::AddDamagerToHistory( EHANDLE hDamager )
{
	// sanity check: ignore damager if it is on our team.  (Catch-all for 
	// damaging self in rocket jumps, etc.)
	if ( !hDamager || ( !hDamager->IsPlayer() && !hDamager->IsNPC() ) || ( InSameTeam( hDamager ) && !TFGameRules()->IsMPFriendlyFire() ) )
		return;

	// If this damager is different from the most recent damager, shift the
	// damagers down and drop the oldest damager.  (If this damager is already
	// the most recent, we will just update the damage time but not remove
	// other damagers from history.)
	if ( m_DamagerHistory[0].hDamager != hDamager )
	{
		for ( int i = 1; i < ARRAYSIZE( m_DamagerHistory ); i++ )
		{
			m_DamagerHistory[i] = m_DamagerHistory[i-1];
		}		
	}	
	// set this damager as most recent and note the time
	m_DamagerHistory[0].hDamager = hDamager;
	m_DamagerHistory[0].flTimeDamage = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Clears damager history
//-----------------------------------------------------------------------------
void CTFPlayer::ClearDamagerHistory()
{
	for ( int i = 0; i < ARRAYSIZE( m_DamagerHistory ); i++ )
	{
		m_DamagerHistory[i].Reset();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldGib( const CTakeDamageInfo &info )
{
	// Check to see if we should allow players to gib.
	int nGibCvar = tf_playergib.GetInt();
	if ( nGibCvar == 0 )
		return false;

	if ( nGibCvar == 2 )
		return true;

	if ( info.GetDamageType() & DMG_NEVERGIB )
		return false;

	if ( info.GetDamageType() & DMG_ALWAYSGIB )
		return true;

	if ( info.GetDamageCustom() == TF_DMG_CUSTOM_TELEFRAG )
		return true;

	if ( info.GetDamageCustom() == TF_DMG_CUSTOM_FLARE_EXPLOSION )
		return true;

	if ( info.GetDamageCustom() == TF_DMG_CUSTOM_CROC )
		return true;

	if ( ( info.GetDamageType() & DMG_BLAST ) || ( info.GetDamageType() & DMG_HALF_FALLOFF ) )
	{
		if ( ( info.GetDamageType() & DMG_CRITICAL ) || m_iHealth < -9 )
			return true;
	}

	int iCritKillGib = 0;
	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>( info.GetWeapon() );
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iCritKillGib, crit_kill_will_gib );
	if ( iCritKillGib )
	{
		if ( ( info.GetDamageType() & DMG_CRITICAL ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	BaseClass::Event_KilledOther( pVictim, info );
	if ( pVictim->IsPlayer() && pVictim->GetTeamNumber() == 3 && (TFGameRules()->IsAnyCoOp() || TFGameRules()->IsVersus()) && TFGameRules()->IsDynamicNPCAllowed() )
		TFGameRules()->m_iDirectorAnger = TFGameRules()->m_iDirectorAnger + 3;

	CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( info.GetWeapon() );

	if ( pVictim->IsPlayer() )
	{
		CTFPlayer *pTFVictim = ToTFPlayer( pVictim );
		bool bInflictor = false;

		// Custom death handlers
		const char *pszCustomDeath = "customdeath:none";
		if ( info.GetAttacker() && info.GetAttacker()->IsBaseObject() )
		{
			pszCustomDeath = "customdeath:sentrygun";
			bInflictor = true;
		}
		else if ( info.GetInflictor() && info.GetInflictor()->IsBaseObject() )
		{
			pszCustomDeath = "customdeath:sentrygun";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_HEADSHOT )
		{				
			pszCustomDeath = "customdeath:headshot";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB )
		{
			pszCustomDeath = "customdeath:backstab";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING_ARROW
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE )
		{
			pszCustomDeath = "customdeath:burning";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING_FLARE 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_FLARE_EXPLOSION
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_FLARE_PELLET )
		{
			pszCustomDeath = "customdeath:flareburn";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_HADOUKEN 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_FENCING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GRENADE
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_UBERSLICE
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GASBLAST )
		{
			pszCustomDeath = "customdeath:taunt";
		}

		// special responses for mini-sentry
		if ( V_strcmp( pszCustomDeath, "customdeath:sentrygun" ) == 0 )
		{
			if ( HasGunslinger() )
				pszCustomDeath = "customdeath:minisentrygun";
		}

		// Revenge handler
		const char *pszDomination = "domination:none";
		if ( pTFVictim->GetDeathFlags() & (TF_DEATH_REVENGE|TF_DEATH_ASSISTER_REVENGE) )
		{
			pszDomination = "domination:revenge";
		}
		else if ( pTFVictim->GetDeathFlags() & TF_DEATH_DOMINATION )
		{
			pszDomination = "domination:dominated";
		}

		if ( IsAlive() )
		{
			m_Shared.IncKillstreak();

			// Apply on-kill effects.
			if ( pWeapon )
			{
				if ( !pWeapon->IsSilentKiller() )
				{
					CFmtStrN<128> modifiers( "%s,%s,victimclass:%s", pszCustomDeath, pszDomination, g_aPlayerClassNames_NonLocalized[pTFVictim->GetPlayerClass()->GetClassIndex()] );
					SpeakConceptIfAllowed( MP_CONCEPT_KILLED_PLAYER, modifiers );

					if ( pTFVictim->IsMiniBoss() )
						SpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_KILLED );
				}

				// Apply on-kill effects.
				float flCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flCritOnKill, add_onkill_critboost_time );
				if ( flCritOnKill )
					m_Shared.AddCond( TF_COND_CRITBOOSTED_ON_KILL, flCritOnKill );

				float flMiniCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flMiniCritOnKill, add_onkill_minicritboost_time );
				if ( flMiniCritOnKill )
					m_Shared.AddCond( TF_COND_MINICRITBOOSTED_ON_KILL, flMiniCritOnKill );

				float flSpeedBoostOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeedBoostOnKill, speed_boost_on_kill );
				if ( flSpeedBoostOnKill )
					m_Shared.AddCond( TF_COND_SPEED_BOOST, flSpeedBoostOnKill );

				float flHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flHealthOnKill, heal_on_kill );
				if( flHealthOnKill )
				{
					int iHealthRestored = TakeHealth( flHealthOnKill, DMG_GENERIC );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flRestoreHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flRestoreHealthOnKill, restore_health_on_kill );
				CALL_ATTRIB_HOOK_FLOAT(flRestoreHealthOnKill, restore_health_on_kill);
				if( ( flRestoreHealthOnKill > 0 ) && ( m_Shared.GetMaxBuffedHealth() > GetHealth() ) )
				{
					int iHealthRestored = TakeHealth( ( ( flRestoreHealthOnKill/100 ) * GetMaxHealth() ), DMG_IGNORE_MAXHEALTH );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flAddChargeShieldKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT( flAddChargeShieldKill, kill_refills_meter );
				if ( flAddChargeShieldKill )
				{
					m_Shared.m_flChargeMeter = min( ( m_Shared.m_flChargeMeter + ( flAddChargeShieldKill * 100 ) ), 100.0f );
					IGameEvent *event = gameeventmanager->CreateEvent( "kill_refills_meter" );
					if ( event )
					{
						event->SetInt( "index", entindex() );
						gameeventmanager->FireEvent( event );
					}
				}

				int nAddCloakOnKill = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nAddCloakOnKill, add_cloak_on_kill );
				if ( nAddCloakOnKill > 0 )
					m_Shared.AddToSpyCloakMeter( nAddCloakOnKill );
			}

			if ( pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_SWORD )
			{
				m_Shared.SetMaxHealth( GetMaxHealth() );
			}

			int nDisguiseOnStab = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER(pWeapon, nDisguiseOnStab, set_disguise_on_backstab);
			if (nDisguiseOnStab != 0 && info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB)
			{
				int iClassIdx = TF_CLASS_SCOUT;
				iClassIdx = pTFVictim->GetPlayerClass()->GetClassIndex();
				m_Shared.Disguise(pTFVictim->GetTeamNumber(), iClassIdx);
				m_Shared.CompleteDisguise(true);
				int rand = random->RandomInt(pTFVictim->GetMaxHealth() / 2, pTFVictim->GetMaxHealth());
				m_Shared.SetDisguiseHealth(rand);
			}

			int iKillsCollectCrit = 0;
			CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
			if ( iKillsCollectCrit != 0 && info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB )
			{
				m_Shared.SetDecapitationCount( Min( m_Shared.GetDecapitationCount() + 1, 35 ) );
			}
		}
	}
	else if ( pVictim->IsBaseObject() )
	{
		CBaseObject *pObject = dynamic_cast<CBaseObject *>( pVictim );
		SpeakConceptIfAllowed( MP_CONCEPT_KILLED_OBJECT, pObject->GetResponseRulesModifier() );
	}
	else if ( pVictim->IsNPC() )
	{
		// Custom death handlers
		const char *pszCustomDeath = "customdeath:none";
		if ( info.GetAttacker() && info.GetAttacker()->IsBaseObject() )
		{
			pszCustomDeath = "customdeath:sentrygun";
		}
		else if ( info.GetInflictor() && info.GetInflictor()->IsBaseObject() )
		{
			pszCustomDeath = "customdeath:sentrygun";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_HEADSHOT )
		{				
			pszCustomDeath = "customdeath:headshot";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB )
		{
			pszCustomDeath = "customdeath:backstab";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING_ARROW
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE )
		{
			pszCustomDeath = "customdeath:burning";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BURNING_FLARE 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_FLARE_EXPLOSION
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_FLARE_PELLET )
		{
			pszCustomDeath = "customdeath:flareburn";
		}
		else if ( info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_HADOUKEN 
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_FENCING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GRENADE
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_UBERSLICE
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF
		|| info.GetDamageCustom() == TF_DMG_CUSTOM_TAUNTATK_GASBLAST )
		{
			pszCustomDeath = "customdeath:taunt";
		}

		// special responses for mini-sentry
		if ( V_strcmp( pszCustomDeath, "customdeath:sentrygun" ) == 0 )
		{
			if ( HasGunslinger() )
				pszCustomDeath = "customdeath:minisentrygun";
		}

		const char *pszDomination = "domination:none";
		if ( RandomInt(1,10) == 3 )
			pszDomination = "domination:dominated";

		if ( IsAlive() )
		{
			m_Shared.IncKillstreak();

			// Apply on-kill effects.
			if ( pWeapon )
			{
				const char *pszClassname = g_aPlayerClassNames_NonLocalized[TF_CLASS_UNDEFINED];
				if ( pVictim->ClassMatches( "npc_combine_s" ) || pVictim->ClassMatches( "npc_metropolice" ) || pVictim->ClassMatches( "npc_antlion" ) || pVictim->ClassMatches( "monster_human_grunt" ) || pVictim->ClassMatches( "monster_alien_grunt" ) )
					pszClassname = g_aPlayerClassNames_NonLocalized[TF_CLASS_SOLDIER];
				else if ( pVictim->ClassMatches( "npc_cremator" ) )
					pszClassname = g_aPlayerClassNames_NonLocalized[TF_CLASS_PYRO];
				else if ( pVictim->ClassMatches( "npc_sniper" ) )
					pszClassname = g_aPlayerClassNames_NonLocalized[TF_CLASS_SNIPER];
				else if ( pVictim->ClassMatches( "monster_human_assassin" ) || pVictim->ClassMatches( "monster_male_assassin" ) )
					pszClassname = g_aPlayerClassNames_NonLocalized[TF_CLASS_SPY];

				if ( !pWeapon->IsSilentKiller() )
				{
					CFmtStrN<128> modifiers( "%s,%s,victimclass:%s", pszCustomDeath, pszDomination, pszClassname );
					SpeakConceptIfAllowed( MP_CONCEPT_KILLED_PLAYER, modifiers );

					if ( pVictim->MyNPCPointer()->IsMiniBoss() )
						SpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_KILLED );
				}

				// Apply on-kill effects.
				float flCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flCritOnKill, add_onkill_critboost_time );
				if ( flCritOnKill )
					m_Shared.AddCond( TF_COND_CRITBOOSTED_ON_KILL, flCritOnKill );

				float flMiniCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flMiniCritOnKill, add_onkill_minicritboost_time );
				if ( flMiniCritOnKill )
					m_Shared.AddCond( TF_COND_MINICRITBOOSTED_ON_KILL, flMiniCritOnKill );

				float flSpeedBoostOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeedBoostOnKill, speed_boost_on_kill );
				if ( flSpeedBoostOnKill )
					m_Shared.AddCond( TF_COND_SPEED_BOOST, flSpeedBoostOnKill );

				float flHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flHealthOnKill, heal_on_kill );
				if( flHealthOnKill )
				{
					int iHealthRestored = TakeHealth( flHealthOnKill, DMG_GENERIC );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flRestoreHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flRestoreHealthOnKill, restore_health_on_kill );
				if( ( flRestoreHealthOnKill > 0 ) && ( m_Shared.GetMaxBuffedHealth() > GetHealth() ) )
				{
					int iHealthRestored = TakeHealth( ( ( flRestoreHealthOnKill/100 ) * GetMaxHealth() ), DMG_IGNORE_MAXHEALTH );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flAddChargeShieldKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT( flAddChargeShieldKill, kill_refills_meter );
				if ( flAddChargeShieldKill )
				{
					m_Shared.m_flChargeMeter = min( ( m_Shared.m_flChargeMeter + ( flAddChargeShieldKill * 100 ) ), 100.0f );
					IGameEvent *event = gameeventmanager->CreateEvent( "kill_refills_meter" );
					if ( event )
					{
						event->SetInt( "index", entindex() );
						gameeventmanager->FireEvent( event );
					}
				}

				int nAddCloakOnKill = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nAddCloakOnKill, add_cloak_on_kill );
				if ( nAddCloakOnKill > 0 )
					m_Shared.AddToSpyCloakMeter( nAddCloakOnKill );

				int nDisguiseOnStab = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER(pWeapon, nDisguiseOnStab, set_disguise_on_backstab);
				CAI_BaseNPC *pNPCVictim = pVictim->MyNPCPointer();
				if (pNPCVictim && nDisguiseOnStab != 0 && info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB)
				{
					int iClassIdx = TF_CLASS_SCOUT;
					if (pNPCVictim->ClassMatches("npc_combine_s") || pNPCVictim->ClassMatches("npc_metropolice"))
					{
						iClassIdx = TF_CLASS_COMBINE;
					}
					if (pNPCVictim->ClassMatches("npc_zombie") || pNPCVictim->ClassMatches("npc_poisonzombie") || pNPCVictim->ClassMatches("npc_zombine") || pNPCVictim->ClassMatches("npc_fastzombie"))
					{
						iClassIdx = TF_CLASS_ZOMBIEFAST;
					}
					if (pNPCVictim->ClassMatches("npc_antlion"))
					{
						iClassIdx = TF_CLASS_WILDCARD;
					}

					m_Shared.Disguise(pNPCVictim->GetTeamNumber(), iClassIdx);
					m_Shared.CompleteDisguise(true);
				}

				int iKillsCollectCrit = 0;
				CALL_ATTRIB_HOOK_INT( iKillsCollectCrit, sapper_kills_collect_crits );
				if ( iKillsCollectCrit != 0 && info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB )
				{
					m_Shared.SetDecapitationCount( Min( m_Shared.GetDecapitationCount() + 1, 35 ) );
				}
			}
		}
	}
	else if ( pVictim->ClassMatches( "prop_vehicle_apc" ) )
	{
		if ( IsAlive() )
		{
			m_Shared.IncKillstreak();

			// Apply on-kill effects.
			if ( pWeapon )
			{
				if ( !pWeapon->IsSilentKiller() )
					SpeakConceptIfAllowed( MP_CONCEPT_MVM_TANK_DEAD );

				// Apply on-kill effects.
				float flCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flCritOnKill, add_onkill_critboost_time );
				if ( flCritOnKill )
					m_Shared.AddCond( TF_COND_CRITBOOSTED_ON_KILL, flCritOnKill );

				float flMiniCritOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flMiniCritOnKill, add_onkill_minicritboost_time );
				if ( flMiniCritOnKill )
					m_Shared.AddCond( TF_COND_MINICRITBOOSTED_ON_KILL, flMiniCritOnKill );

				float flSpeedBoostOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flSpeedBoostOnKill, speed_boost_on_kill );
				if ( flSpeedBoostOnKill )
					m_Shared.AddCond( TF_COND_SPEED_BOOST, flSpeedBoostOnKill );

				float flHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flHealthOnKill, heal_on_kill );
				if( flHealthOnKill )
				{
					int iHealthRestored = TakeHealth( flHealthOnKill, DMG_GENERIC );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flRestoreHealthOnKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flRestoreHealthOnKill, restore_health_on_kill );
				if( ( flRestoreHealthOnKill > 0 ) && ( m_Shared.GetMaxBuffedHealth() > GetHealth() ) )
				{
					int iHealthRestored = TakeHealth( ( ( flRestoreHealthOnKill/100 ) * GetMaxHealth() ), DMG_IGNORE_MAXHEALTH );
					if ( iHealthRestored )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
						if ( event )
						{
							event->SetInt( "amount", iHealthRestored );
							event->SetInt( "entindex", entindex() );
					
							gameeventmanager->FireEvent( event );
						}
					}
				}

				float flAddChargeShieldKill = 0.0f;
				CALL_ATTRIB_HOOK_FLOAT( flAddChargeShieldKill, kill_refills_meter );
				if ( flAddChargeShieldKill )
				{
					m_Shared.m_flChargeMeter = min( ( m_Shared.m_flChargeMeter + ( flAddChargeShieldKill * 100 ) ), 100.0f );
					IGameEvent *event = gameeventmanager->CreateEvent( "kill_refills_meter" );
					if ( event )
					{
						event->SetInt( "index", entindex() );
						gameeventmanager->FireEvent( event );
					}
				}

				int nAddCloakOnKill = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nAddCloakOnKill, add_cloak_on_kill );
				if ( nAddCloakOnKill > 0 )
					m_Shared.AddToSpyCloakMeter( nAddCloakOnKill );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	if (GetTeamNumber() == TF_TEAM_RED && (info.GetAttacker()->IsNPC() || info.GetAttacker()->IsPlayer() && info.GetAttacker()->GetTeamNumber() == TF_TEAM_BLUE)
		&& TFGameRules()->IsDynamicNPCAllowed() )
	{
		TFGameRules()->m_iDirectorAnger = TFGameRules()->m_iDirectorAnger - 3;
	}

	StateTransition( TF_STATE_DYING );	// Transition into the dying state.

	CBaseEntity *pAttacker = info.GetAttacker();
	CBaseEntity *pInflictor = info.GetInflictor();
	CTFPlayer *pTFAttacker = ToTFPlayer( pAttacker );

	CTFWeaponBase *pKillerWeapon = NULL;
	if ( info.GetWeapon() )
	{
		pKillerWeapon = dynamic_cast<CTFWeaponBase *>( info.GetWeapon() );
	}
	else if ( pAttacker && pAttacker->IsPlayer() )
	{
		// Assume that player used his currently active weapon.
		pKillerWeapon = pTFAttacker->GetActiveTFWeapon();
	}

	// Deplete lives counter.
	int iLives = m_Shared.GetLivesCount();
	if ( ( TFGameRules()->State_Get() == GR_STATE_RND_RUNNING && !TFGameRules()->IsInWaitingForPlayers() ) && iLives > 0 )
	{
		m_Shared.SetLivesCount( --iLives );
		if ( iLives > 1 )
		{
			char szLivesCount[16];
			V_snprintf( szLivesCount, sizeof( szLivesCount ), "%d", iLives );
			ClientPrint( this, HUD_PRINTTALK, "#game_lives_left", szLivesCount );
		}
		else if ( iLives == 1 )
		{
			ClientPrint( this, HUD_PRINTTALK, "#game_lives_one" );
		}
		else
		{
			ClientPrint( this, HUD_PRINTTALK, "#game_lives_zero" );
		}
	}

	bool bDisguised = m_Shared.InCond( TF_COND_DISGUISED );
	// we want the rag doll to burn if the player was burning and was not a pryo (who only burns momentarily)
	bool bBurning = m_Shared.InCond( TF_COND_BURNING ) && ( TF_CLASS_PYRO != GetPlayerClass()->GetClassIndex() );
	bool bOnGround = ( GetFlags() & FL_ONGROUND ) != 0;

	m_Shared.RemoveCond( TF_COND_TAUNTING );

	if ( m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
		DropRune();

	// Remove all conditions...
	m_Shared.RemoveAllCond();

	// Reset our model if we were disguised
	if ( bDisguised )
		UpdateModel();

	if (pKillerWeapon && pKillerWeapon->IsSilentKiller())
	{
		//Player was silently killed
		
	}
	else{
		// test test test test test test test test test test test test test test test test test test test test
		if ((TFGameRules()->IsTFCAllowed() || TFGameRules()->IsHolidayActive(kHoliday_AprilFools))){
			EmitSound("TFCPlayer.Death");
		}
		int iDeathConcept = MP_CONCEPT_PLRDEATH;
		if (m_LastDamageType & DMG_BLAST){
			iDeathConcept = MP_CONCEPT_PLRDEATHEXPLOSION;
			
		}
		if (m_LastDamageType & DMG_CLUB){
			iDeathConcept = MP_CONCEPT_PLRDEATHMELEE;
			
		}
		if (m_LastDamageType & DMG_CRITICAL){
			iDeathConcept = MP_CONCEPT_PLRDEATHCRIT;
			
		}
		DevMsg("DEATH!!!! Concept to speak is: %i \n", iDeathConcept);

		SpeakConceptIfAllowed(iDeathConcept);
	}
	

	if ( pTFAttacker )
	{
		int nDropHealthOnKill = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pTFAttacker, nDropHealthOnKill, drop_health_pack_on_kill );
		if ( nDropHealthOnKill != 0 )
			DropHealthPack( info );
	}

	RemoveTeleportEffect();

	// Drop our weapon and ammo box
	if ( lfe_allow_tf_dropped_weapon.GetBool() && !lfe_force_legacy.GetBool() )
		DropWeapon( GetActiveTFWeapon(), true );

	bool bLunchbox = false;
	CTFWeaponBase *pActiveWeapon = GetActiveTFWeapon();
	if ( pActiveWeapon )
	{
		// If the player died holding the sandvich drop a special health kit that heals 50hp (75hp for scout)
		if ( pActiveWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX )
			bLunchbox = true;
	}

	CTFGrapplingHook *pHook = dynamic_cast< CTFGrapplingHook * >( Weapon_OwnsThisID( TF_WEAPON_GRAPPLINGHOOK ) );
	if ( pHook )
		pHook->RemoveHookProjectile( true );

	CTFWeaponBase *pPortalGun = dynamic_cast<CTFWeaponBase*>( Weapon_OwnsThisID( TF_WEAPON_PORTALGUN ) );
	if ( pPortalGun )
		pPortalGun->Drop( vec3_origin );

	DropAmmoPack( info, bLunchbox, false );

	if ( lfe_allow_revive_marker.GetBool() && !m_bInTransition && !lfe_force_legacy.GetBool() )
	{
		if ( TFGameRules() && TFGameRules()->IsAnyCoOp() || TFGameRules()->IsVersus() )
			DropReviveMarker();
	}

	if ( !lfe_force_legacy.GetBool() )
	{
		if ( pAttacker->ClassMatches( "npc_headcrab" ) || pAttacker->ClassMatches( "npc_headcrab_fast" ) || pAttacker->ClassMatches( "npc_headcrab_poison" ) || pAttacker->ClassMatches( "monster_headcrab" ) )
			BecomeZombie( pAttacker );
	}

	m_Shared.SetDesiredWeaponIndex( -1 );
	RemoveAllWeapons();

	// If the player has a capture flag and was killed by another player, award that player a defense
	if ( HasItem() && pAttacker && ( pAttacker != this ) )
	{
		CCaptureFlag *pCaptureFlag = dynamic_cast<CCaptureFlag *>( GetItem() );
		if ( pCaptureFlag )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_flag_event" );
			if ( event )
			{
				event->SetInt( "player", pAttacker->entindex() );
				event->SetString( "player_name", pAttacker->IsPlayer() ? pTFAttacker->GetPlayerName() : pAttacker->GetClassname() );
				event->SetInt( "carrier", entindex() );
				event->SetInt( "eventtype", TF_FLAGEVENT_DEFEND );
				event->SetInt( "home", 0 ); // whether or not the flag was home (only set for TF_FLAGEVENT_PICKUP) 
				event->SetInt( "team", GetTeamNumber() );
				event->SetInt( "priority", 8 );
				gameeventmanager->FireEvent( event );
			}

			if ( pTFAttacker )
				CTF_GameStats.Event_PlayerDefendedPoint( pTFAttacker );
		}
	}

	bool bElectrocute = false;
	if ( IsPlayerClass( TF_CLASS_MEDIC ) && MedicGetChargeLevel() == 1.0f && pTFAttacker )
	{
		// Bonus points.
		IGameEvent *event_bonus = gameeventmanager->CreateEvent( "player_bonuspoints" );
		if ( event_bonus )
		{
			event_bonus->SetInt( "player_entindex", entindex() );
			event_bonus->SetInt( "source_entindex", pAttacker->entindex() );
			event_bonus->SetInt( "points", 2 );
			gameeventmanager->FireEvent( event_bonus );
		}

		CTF_GameStats.Event_PlayerAwardBonusPoints( pTFAttacker, this, 2 );

		bElectrocute = true;
	}

	if ( IsPlayerClass( TF_CLASS_ENGINEER ) && m_Shared.GetCarriedObject() )
	{
		// Blow it up at our position.
		CBaseObject *pObject = m_Shared.GetCarriedObject();
		pObject->Teleport( &WorldSpaceCenter(), &GetAbsAngles(), &vec3_origin );
		pObject->DropCarriedObject( this );

		CTakeDamageInfo newInfo( pInflictor, pAttacker, (float)pObject->GetHealth(), DMG_GENERIC, TF_DMG_CUSTOM_CARRIED_BUILDING );
		pObject->Killed( newInfo );
	}

	// Remove all items...
	RemoveAllItems( true );

	for ( int iWeapon = 0; iWeapon < LOADOUT_POSITION_BUFFER; ++iWeapon )
	{
		CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetWeapon( iWeapon );

		if ( pWeapon )
		{
			pWeapon->WeaponReset();
		}
	}

	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->SendViewModelAnim( ACT_IDLE );
		GetActiveWeapon()->Holster();
		SetActiveWeapon( NULL );
	}

	ClearZoomOwner();

	m_vecLastDeathPosition = GetAbsOrigin();
	int iDamageCustom = info.GetDamageCustom();

	CTakeDamageInfo info_modified = info;

	// Ragdoll, gib, or death animation.
	bool bRagdoll = true;
	bool bGib = false;

	// See if we should gib.
	if ( ShouldGib( info ) )
	{
		bGib = true;
		bRagdoll = false;

		if ( pKillerWeapon && pKillerWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC )
		{
			CEffectData	data;
			data.m_nHitBox = GetParticleSystemIndex( "tfc_sniper_mist" );
			data.m_vOrigin = WorldSpaceCenter();
			data.m_vAngles = vec3_angle;
			data.m_nEntIndex = 0;

			CPVSFilter filter( WorldSpaceCenter() );
			te->DispatchEffect( filter, 0.0, data.m_vOrigin, "ParticleEffect", data );
		}
	}
	else
	// See if we should play a custom death animation.
	{
		if ( PlayDeathAnimation( info, info_modified ) )
		{
			bRagdoll = false;
		}
	}

	// show killer in death cam mode
	// chopped down version of SetObserverTarget without the team check
	if( pTFAttacker )
	{
		// See if we were killed by a sentrygun. If so, look at that instead of the player
		if ( pInflictor && pInflictor->IsBaseObject() )
		{
			// Catches the case where we're killed directly by the sentrygun (i.e. bullets)
			// Look at the sentrygun
			m_hObserverTarget.Set( pInflictor ); 
		}
		// See if we were killed by a projectile emitted from a base object. The attacker
		// will still be the owner of that object, but we want the deathcam to point to the 
		// object itself.
		else if ( pInflictor && pInflictor->GetOwnerEntity() && 
					pInflictor->GetOwnerEntity()->IsBaseObject() )
		{
			m_hObserverTarget.Set( pInflictor->GetOwnerEntity() );
		}
		else
		{
			// Look at the player
			m_hObserverTarget.Set( pAttacker ); 
		}

		// reset fov to default
		SetFOV( this, 0 );
	}
	else if ( pAttacker && pAttacker->IsBaseObject() )
	{
		// Catches the case where we're killed by entities spawned by the sentrygun (i.e. rockets)
		// Look at the sentrygun. 
		m_hObserverTarget.Set( pAttacker ); 
	}
	else if ( pAttacker && pAttacker->IsNPC() )
	{
		m_hObserverTarget.Set( pAttacker );
	}
	else
	{
		m_hObserverTarget.Set( NULL );
	}

	if ( bElectrocute && bGib )
	{
		const char *pszEffect = ConstructTeamParticle( "electrocuted_gibbed_%s", GetTeamNumber() );
		DispatchParticleEffect( pszEffect, GetAbsOrigin(), vec3_angle );
		EmitSound( "TFPlayer.MedicChargedDeath" );
	}

	bool bCritOnHardHit = false, bTurnToAsh = false, bTurnToGold = false;
	if ( pKillerWeapon && pAttacker != this )
	{
		int nRagdollsBecomeAsh = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nRagdollsBecomeAsh, ragdolls_become_ash );
		bTurnToAsh = nRagdollsBecomeAsh != 0;

		int nRagdollPlasmaEffect = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nRagdollPlasmaEffect, ragdolls_plasma_effect );
		if ( nRagdollPlasmaEffect != 0 )
			iDamageCustom = TF_DMG_CUSTOM_PLASMA;

		int nCritOnHardHit = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nCritOnHardHit, crit_on_hard_hit );
		bCritOnHardHit = nCritOnHardHit != 0;

		int nTurnToGold = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nTurnToGold, set_turn_to_gold );
		if ( nTurnToGold != 0 )
			iDamageCustom = TF_DMG_CUSTOM_GOLD_WRENCH;
	}

	if ( iDamageCustom == TF_DMG_CUSTOM_GOLD_WRENCH )
		bTurnToGold = true;

	if ( iDamageCustom == TF_DMG_CUSTOM_SUICIDE )
	{
		// if this was suicide, recalculate attacker to see if we want to award the kill to a recent damager
		info_modified.SetAttacker( TFGameRules()->GetDeathScorer( info.GetAttacker(), pInflictor, this ) );
	}
	else if ( !pAttacker || pAttacker == this || pAttacker->IsBSPModel() )
	{
		// Recalculate attacker if player killed himself or this was environmental death.
		CBaseEntity *pDamager = TFGameRules()->GetRecentDamager( this, 0, TF_TIME_ENV_DEATH_KILL_CREDIT );
		if (pDamager)
		{
			info_modified.SetAttacker( pDamager );
			info_modified.SetInflictor( NULL );
			info_modified.SetWeapon( NULL );
			info_modified.SetDamageType( DMG_GENERIC );
			iDamageCustom = TF_DMG_CUSTOM_SUICIDE;
		}
	}

	bool bDisguiseOnStab = false, bTurnToIce = false;
	if ( pTFAttacker )
	{
		if ( TF_DMG_CUSTOM_HEADSHOT == iDamageCustom )
		{
			CTF_GameStats.Event_Headshot( pTFAttacker );
		}
		else if ( TF_DMG_CUSTOM_BACKSTAB == iDamageCustom )
		{
			CTF_GameStats.Event_Backstab( pTFAttacker );
			if ( pKillerWeapon && pAttacker != this )
			{
				int nTurnToIce = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nTurnToIce, freeze_backstab_victim );
				bTurnToIce = nTurnToIce != 0;

				int nDisguiseOnBackstab = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pKillerWeapon, nDisguiseOnBackstab, set_disguise_on_backstab );
				bDisguiseOnStab = nDisguiseOnBackstab != 0;
			}
		}
	}

	// mega already made server ragdoll
	if ( iDamageCustom == LFE_DMG_CUSTOM_PHYSCANNON_MEGA )
		bRagdoll = false;

	info_modified.SetDamageCustom( iDamageCustom );

	m_OnDeath.FireOutput( this, this );

	BaseClass::Event_Killed( info_modified );

	// Create the ragdoll entity.
	if ( bGib || bRagdoll )
	{
		CreateRagdollEntity( bGib, bBurning, bElectrocute, bOnGround, bDisguiseOnStab, bTurnToGold, bTurnToIce, bTurnToAsh, iDamageCustom, bCritOnHardHit );
	}

	// Don't overflow the value for this.
	m_iHealth = 0;

	// If we died in sudden death and we're an engineer, explode our buildings
	if ( IsPlayerClass( TF_CLASS_ENGINEER ) && TFGameRules()->InStalemate() )
	{
		for (int i = GetObjectCount()-1; i >= 0; i--)
		{
			CBaseObject *obj = GetObject(i);
			Assert( obj );

			if ( obj )
			{
				obj->DetonateObject();
			}		
		}
	}

	m_Shared.SetKillstreak( 0 );

	if ( pAttacker && pAttacker == pInflictor && pAttacker->IsBSPModel() )
	{
		CBaseEntity *pDamager = TFGameRules()->GetRecentDamager( this, 0, TF_TIME_ENV_DEATH_KILL_CREDIT );
		CTFPlayer *pTFDamager = ToTFPlayer( pDamager );

		if ( pTFDamager )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "environmental_death" );

			if ( event )
			{
				event->SetInt( "killer", pTFDamager->GetUserID() );
				event->SetInt( "victim", GetUserID() );
				event->SetInt( "priority", 9 ); // HLTV event priority, not transmitted
				
				gameeventmanager->FireEvent( event );
			}
		}
	}

	CTFRagdoll *pRagdoll = dynamic_cast<CTFRagdoll*>( m_hRagdoll.Get() );
	if ( ( info.GetDamageType() & ( DMG_ALWAYSGIB | DMG_LASTGENERICFLAG | DMG_CRUSH )) == ( DMG_ALWAYSGIB | DMG_LASTGENERICFLAG | DMG_CRUSH ) )
	{
		if( pRagdoll )
			pRagdoll->GetBaseAnimating()->AddEffects( EF_NODRAW );
	}

	if ( info.GetDamageType() & DMG_REMOVENORAGDOLL )
	{
		if( pRagdoll )
			pRagdoll->GetBaseAnimating()->AddEffects( EF_NODRAW );
	}

	if (info.GetDamageType() & DMG_DISSOLVE)
	{
		if( pRagdoll )
			pRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
	}

	// Handle on-kill effects.
	if ( pKillerWeapon && pAttacker != this )
	{
		// Notify the damaging weapon.
		pKillerWeapon->OnPlayerKill( this, info );
	}

	if ( IsGlowEffectActive() )
	{
		RemoveGlowEffect();
		SetGlowEffectColor(0, 0, 0, 0);
	}

	StopWaterDeathSounds();

	if ( !lfe_force_legacy.GetBool() )
	{
		// IS DEAD!
		if ( m_Shared.GetLivesCount() == 0 || ( GetGlobalTFTeam( GetTeamNumber() )->GetNumPlayers() <= 16 ) )
		{
			CFmtStrN<128> modifiers( "victimclass:%s", g_aPlayerClassNames_NonLocalized[GetPlayerClass()->GetClassIndex()] );
			TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_DEFENDER_DIED, GetTeamNumber(), modifiers );
		}

		if ( pTFAttacker && pTFAttacker->IsMiniBoss() )
			TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_KILLED_TEAMMATE, GetTeamNumber() );

		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pAttacker );
		if ( pNPC && pNPC->IsMiniBoss() )
			TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_KILLED_TEAMMATE, GetTeamNumber() );
	}

	// instant spawn
	if ( mp_disable_respawn_times.GetInt() == 2 )
		ForceRespawn();
}

bool CTFPlayer::Event_Gibbed( const CTakeDamageInfo &info )
{
	// CTFRagdoll takes care of gibbing.
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector )
{
	if ( CanBecomeRagdoll() )
	{
		VPhysicsDestroyObject();
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_nRenderFX = kRenderFxRagdoll;

		// Have to do this dance because m_vecForce is a network vector
		// and can't be sent to ClampRagdollForce as a Vector *
		Vector vecClampedForce;
		ClampRagdollForce( forceVector, &vecClampedForce );
		m_vecForce = vecClampedForce;

		SetParent( NULL );

		AddFlag( FL_TRANSRAGDOLL );

		SetMoveType( MOVETYPE_NONE );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::PlayDeathAnimation( const CTakeDamageInfo &info, CTakeDamageInfo &info_modified )
{
	if ( SelectWeightedSequence( ACT_DIESIMPLE ) == -1 )
		return false;

	// Get the attacking player.
	CTFPlayer *pAttacker = (CTFPlayer*)ToTFPlayer( info.GetAttacker() );
	if ( !pAttacker )
		return false;

	bool bPlayDeathAnim = false;

	// Check for a sniper headshot. (Currently only on Heavy.)
	if ( pAttacker->GetPlayerClass()->IsClass( TF_CLASS_SNIPER ) && ( info.GetDamageCustom() == TF_DMG_CUSTOM_HEADSHOT ) )
	{
		bPlayDeathAnim = true;
	}
	// Check for a spy backstab. (Currently only on Sniper.)
	else if ( pAttacker->GetPlayerClass()->IsClass( TF_CLASS_SPY ) && ( info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB ) )
	{
		bPlayDeathAnim = true;
	}
	// Check for a sword kill. (Currently only on Demo.)
	else if ( pAttacker->GetPlayerClass()->IsClass( TF_CLASS_DEMOMAN ) && ( info.GetDamageCustom() == TF_DMG_CUSTOM_DECAPITATION ) )
	{
		bPlayDeathAnim = true;
	}

	// Play death animation?
	if ( bPlayDeathAnim )
	{
		info_modified.SetDamageType( info_modified.GetDamageType() | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE );

		SetAbsVelocity( vec3_origin );
		DoAnimationEvent( PLAYERANIMEVENT_DIE );

		// No ragdoll yet.
		if ( m_hRagdoll.Get() )
		{
			UTIL_Remove( m_hRagdoll );
		}
	}

	return bPlayDeathAnim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StopRagdollDeathAnim( void )
{
	CTFRagdoll *pRagdoll = dynamic_cast<CTFRagdoll *>( m_hRagdoll.Get() );
	if ( pRagdoll )
		pRagdoll->m_iDamageCustom = 0; //??
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pWeapon - 
//			&vecOrigin - 
//			&vecAngles - 
//-----------------------------------------------------------------------------
bool CTFPlayer::CalculateAmmoPackPositionAndAngles( CTFWeaponBase *pWeapon, Vector &vecOrigin, QAngle &vecAngles )
{
	// Look up the hand and weapon bones.
	int iHandBone = LookupBone( "weapon_bone" );
	if ( iHandBone == -1 )
		return false;

	GetBonePosition( iHandBone, vecOrigin, vecAngles );

	// Draw the position and angles.
	Vector vecDebugForward2, vecDebugRight2, vecDebugUp2;
	AngleVectors( vecAngles, &vecDebugForward2, &vecDebugRight2, &vecDebugUp2 );

	VectorAngles( vecDebugUp2, vecAngles );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// NOTE: If we don't let players drop ammo boxes, we don't need this code..
//-----------------------------------------------------------------------------
void CTFPlayer::AmmoPackCleanUp( void )
{
	// If we have more than 3 ammo packs out now, destroy the oldest one.
	int iNumPacks = 0;
	CTFAmmoPack *pOldestBox = NULL;

	// Cycle through all ammobox in the world and remove them
	CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, "tf_ammo_pack" );
	while ( pEnt )
	{
		CBaseEntity *pOwner = pEnt->GetOwnerEntity();
		if (pOwner == this)
		{
			CTFAmmoPack *pThisBox = dynamic_cast<CTFAmmoPack *>( pEnt );
			Assert( pThisBox );
			if ( pThisBox )
			{
				iNumPacks++;

				// Find the oldest one
				if ( pOldestBox == NULL || pOldestBox->GetCreationTime() > pThisBox->GetCreationTime() )
				{
					pOldestBox = pThisBox;
				}
			}
		}

		pEnt = gEntList.FindEntityByClassname( pEnt, "tf_ammo_pack" );
	}

	// If they have more than 3 packs active, remove the oldest one
	if ( iNumPacks > 3 && pOldestBox )
	{
		UTIL_Remove( pOldestBox );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Clean up dropped weapons to avoid overpopulation.
//-----------------------------------------------------------------------------
void CTFPlayer::DroppedWeaponCleanUp( void )
{
	// If we have more than 3 dropped weapons out now, destroy the oldest one.
	int iNumWeps = 0;
	CTFDroppedWeapon *pOldestWeapon = NULL;

	// Cycle through all weapons in the world and remove them
	CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, "tf_dropped_weapon" );
	while ( pEnt )
	{
		CBaseEntity *pOwner = pEnt->GetOwnerEntity();
		if ( pOwner == this )
		{
			CTFDroppedWeapon *pThisWeapon = dynamic_cast<CTFDroppedWeapon *>( pEnt );
			Assert( pThisWeapon );
			if ( pThisWeapon )
			{
				iNumWeps++;

				// Find the oldest one
				if ( pOldestWeapon == NULL || pOldestWeapon->GetCreationTime() > pThisWeapon->GetCreationTime() )
				{
					pOldestWeapon = pThisWeapon;
				}
			}
		}

		pEnt = gEntList.FindEntityByClassname( pEnt, "tf_dropped_weapon" );
	}

	// If they have more than 3 weapons active, remove the oldest one
	if ( iNumWeps > 3 && pOldestWeapon )
	{
		UTIL_Remove( pOldestWeapon );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropAmmoPack( CTakeDamageInfo const &info, bool bLunchbox, bool bFeigning )
{
	// Since weapon is hidden in loser state don't drop ammo pack.
	if ( m_Shared.IsLoser() )
		return;

	// We want the ammo packs to look like the player's weapon model they were carrying.
	// except if they are melee or building weapons
	CTFWeaponBase *pWeapon = NULL;
	CTFWeaponBase *pActiveWeapon = m_Shared.GetActiveTFWeapon();

	if ( !pActiveWeapon || pActiveWeapon->GetTFWpnData().m_bDontDrop ||
		( pActiveWeapon->IsWeapon( TF_WEAPON_BUILDER ) && m_Shared.m_bCarryingObject ) )
	{
		// Don't drop this one, find another one to drop
		int iWeight = -1;

		// find the highest weighted weapon
		for (int i = 0;i < WeaponCount(); i++) 
		{
			CTFWeaponBase *pWpn = ( CTFWeaponBase *)GetWeapon(i);
			if ( !pWpn )
				continue;

			if ( pWpn->GetTFWpnData().m_bDontDrop )
				continue;

			int iThisWeight = pWpn->GetTFWpnData().iWeight;

			if ( iThisWeight > iWeight )
			{
				iWeight = iThisWeight;
				pWeapon = pWpn;
			}
		}
	}
	else
	{
		pWeapon = pActiveWeapon;
	}

	// If we didn't find one, bail
	if ( !pWeapon )
		return;

	// We need to find bones on the world model, so switch the weapon to it.
	const char *pszWorldModel = "models/items/ammopack_medium.mdl";

	pWeapon->SetModel( pszWorldModel );

	// Find the position and angle of the weapons so the "ammo box" matches.
	Vector vecPackOrigin;
	QAngle vecPackAngles;
	if( !CalculateAmmoPackPositionAndAngles( pWeapon, vecPackOrigin, vecPackAngles ) )
		return;

	// Fill the ammo pack with unused player ammo, if out add a minimum amount.
	int iPrimary = max( 5, GetAmmoCount( TF_AMMO_PRIMARY ) );
	int iSecondary = max( 5, GetAmmoCount( TF_AMMO_SECONDARY ) );
	int iMetal = max( 5, GetAmmoCount( TF_AMMO_METAL ) );
	int iGrenade1 = max( 5, GetAmmoCount( LFE_AMMO_GRENADES1 ) );
	int iGrenade2 = max( 5, GetAmmoCount( LFE_AMMO_GRENADES2 ) );

	// Create the ammo pack.
	CTFAmmoPack *pAmmoPack = CTFAmmoPack::Create( vecPackOrigin, vecPackAngles, this, pszWorldModel );
	Assert( pAmmoPack );
	if ( pAmmoPack )
	{
		bool bHolidayPack = false;
		if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) || TFGameRules()->IsHolidayActive( kHoliday_Christmas ) )
		{
			// Only make a holiday pack 3 times out of 10
			if ( rand() / float( VALVE_RAND_MAX ) < 0.3 )
			{
				pAmmoPack->MakeHolidayPack();
				bHolidayPack = true;
			}
		}

		if ( !bFeigning )
		{
			// Remove all of the players ammo.
			RemoveAllAmmo();
		}

		if ( bLunchbox && !bHolidayPack )
		{
			// No ammo for sandviches
			pAmmoPack->SetIsLunchbox( true );
		}
		else if ( !bFeigning )
		{
			// Fill up the ammo pack.
			pAmmoPack->GiveAmmo( iPrimary, TF_AMMO_PRIMARY );
			pAmmoPack->GiveAmmo( iSecondary, TF_AMMO_SECONDARY );
			pAmmoPack->GiveAmmo( iMetal, TF_AMMO_METAL );
			pAmmoPack->GiveAmmo( iGrenade1, LFE_AMMO_GRENADES1 );
			pAmmoPack->GiveAmmo( iGrenade2, LFE_AMMO_GRENADES2 );
		}

		Vector vecRight, vecUp;
		AngleVectors( EyeAngles(), NULL, &vecRight, &vecUp );

		// Calculate the initial impulse on the weapon.
		Vector vecImpulse( 0.0f, 0.0f, 0.0f );
		vecImpulse += vecUp * random->RandomFloat( -0.25, 0.25 );
		vecImpulse += vecRight * random->RandomFloat( -0.25, 0.25 );
		VectorNormalize( vecImpulse );
		vecImpulse *= random->RandomFloat( tf_weapon_ragdoll_velocity_min.GetFloat(), tf_weapon_ragdoll_velocity_max.GetFloat() );
		vecImpulse += GetAbsVelocity();

		// Cap the impulse.
		float flSpeed = vecImpulse.Length();
		if ( flSpeed > tf_weapon_ragdoll_maxspeed.GetFloat() )
		{
			VectorScale( vecImpulse, tf_weapon_ragdoll_maxspeed.GetFloat() / flSpeed, vecImpulse );
		}

		if ( pAmmoPack->VPhysicsGetObject() )
		{
			// We can probably remove this when the mass on the weapons is correct!
			pAmmoPack->VPhysicsGetObject()->SetMass( 25.0f );
			AngularImpulse angImpulse( 0, random->RandomFloat( 0, 100 ), 0 );
			pAmmoPack->VPhysicsGetObject()->SetVelocityInstantaneous( &vecImpulse, &angImpulse );
		}

		pAmmoPack->SetInitialVelocity( vecImpulse );

		// Give the ammo pack some health, so that trains can destroy it.
		pAmmoPack->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
		pAmmoPack->m_takedamage = DAMAGE_YES;		
		pAmmoPack->SetHealth( 900 );
		
		pAmmoPack->SetBodygroup( 1, 1 );
	
		// Clean up old ammo packs if they exist in the world
		AmmoPackCleanUp();	
	}	
	pWeapon->SetModel( pWeapon->GetViewModel() );
}

//-----------------------------------------------------------------------------
// Purpose: Creates tf_dropped_weapon based on selected weapon
//-----------------------------------------------------------------------------
void CTFPlayer::DropWeapon( CTFWeaponBase *pWeapon, bool bKilled /*= false*/ )
{
	if ( !pWeapon )
		return;

	// Can't drop this weapon
	if ( pWeapon->GetTFWpnData().m_bDontDrop || !pWeapon->HasItemDefinition() || ( pWeapon->IsWeapon( TF_WEAPON_BUILDER ) || pWeapon->IsWeapon( TF_WEAPON_PHYSCANNON ) || pWeapon->IsWeapon( TF_WEAPON_PHYSGUN ) || pWeapon->IsWeapon( TF_WEAPON_PORTALGUN ) ) )
		return;

	int iClip = pWeapon->UsesClipsForAmmo1() ? pWeapon->Clip1() : WEAPON_NOCLIP;
	int iAmmo = GetAmmoCount( pWeapon->GetPrimaryAmmoType() );

	// Find the position and angle of the weapons so the dropped entity matches.
	Vector vecPackOrigin;
	QAngle vecPackAngles;
	if ( !CalculateAmmoPackPositionAndAngles( pWeapon, vecPackOrigin, vecPackAngles ) )
		return;

	// Create dropped weapon entity.
	CTFDroppedWeapon *pDroppedWeapon = CTFDroppedWeapon::Create( vecPackOrigin, vecPackAngles, this, pWeapon );
	Assert( pDroppedWeapon );

	if ( pDroppedWeapon )
	{
		// Give the dropped weapon entity our ammo.
		pDroppedWeapon->SetClip( iClip );
		pDroppedWeapon->SetAmmo( iAmmo );
		pDroppedWeapon->SetMaxAmmo( GetMaxAmmo( pWeapon->GetPrimaryAmmoType() ) );

		// Randomize velocity if we dropped weapon upon being killed.
		if ( bKilled )
		{
			// Remove all of the player's ammo.
			RemoveAllAmmo();

			Vector vecRight, vecUp;
			AngleVectors( EyeAngles(), NULL, &vecRight, &vecUp );

			// Calculate the initial impulse on the weapon.
			Vector vecImpulse( 0.0f, 0.0f, 0.0f );
			vecImpulse += vecUp * random->RandomFloat( -0.25, 0.25 );
			vecImpulse += vecRight * random->RandomFloat( -0.25, 0.25 );
			VectorNormalize( vecImpulse );
			vecImpulse *= random->RandomFloat( tf_weapon_ragdoll_velocity_min.GetFloat(), tf_weapon_ragdoll_velocity_max.GetFloat() );
			vecImpulse += GetAbsVelocity();

			// Cap the impulse.
			float flSpeed = vecImpulse.Length();
			if ( flSpeed > tf_weapon_ragdoll_maxspeed.GetFloat() )
			{
				VectorScale( vecImpulse, tf_weapon_ragdoll_maxspeed.GetFloat() / flSpeed, vecImpulse );
			}

			if ( pDroppedWeapon->VPhysicsGetObject() )
			{
				AngularImpulse angImpulse( 0, random->RandomFloat( 0, 100 ), 0 );
				pDroppedWeapon->VPhysicsGetObject()->SetVelocityInstantaneous( &vecImpulse, &angImpulse );
			}
		}

		// Give the ammo pack some health, so that trains can destroy it.
		//pDroppedWeapon->m_takedamage = DAMAGE_YES;
		//pDroppedWeapon->SetHealth( 900 );

		pDroppedWeapon->SetBodygroup( 1, 1 );

		// Clean up old dropped weapons if they exist in the world
		DroppedWeaponCleanUp();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropHealthPack( CTakeDamageInfo const &info, bool bSomething )
{
	CTFPowerup *pPack = (CTFPowerup *)CBaseEntity::Create( "item_healthkit_small", WorldSpaceCenter(), vec3_angle );
	if ( !pPack )
		return;

	// Investigate for constant expression
	Vector vecRand;
	vecRand.x = ( rand() * 0.000061037019 ) + -1.0f;
	vecRand.y = ( rand() * 0.000061037019 ) + -1.0f;
	vecRand.z = rand();

	vecRand.AsVector2D().NormalizeInPlace();

	pPack->DropSingleInstance( 250 * vecRand, this, 0.0f, 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropDeathCallingCard( CTFPlayer *pKiller, CTFPlayer *pVictim )
{
	CTFPowerup *pPack = (CTFPowerup *)CBaseEntity::Create( "item_healthkit_small", WorldSpaceCenter(), vec3_angle );
	if ( !pPack )
		return;

	pPack->DropSingleInstance( vec3_origin, this, 0.0f, 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropRune( void )
{
	int iTeam = GetTeamNumber();
	if ( GetTeamNumber() == TF_TEAM_RED )
		iTeam = TEAM_UNASSIGNED;
	if ( GetTeamNumber() == TF_TEAM_BLUE || GetTeamNumber() == TF_TEAM_GREEN || GetTeamNumber() == TF_TEAM_YELLOW )
		iTeam = TF_TEAM_RED;

	CTFRune *pRune = CTFRune::CreateRune( WorldSpaceCenter(), m_Shared.GetCarryingRuneType(), iTeam );
	Vector vecForward;
	QAngle angAngs;

	angAngs = GetAbsAngles();
	angAngs.x -= 30;
	AngleVectors( angAngs, &vecForward );
	pRune->SetAbsVelocity( vecForward * 250 );
	pRune->SetAbsAngles( vec3_angle );
	pRune->SetOwnerEntity( this );
	pRune->m_flNextCollideTime = gpGlobals->curtime + 0.5f;

	switch ( m_Shared.GetCarryingRuneType() )
	{
	case TF_RUNE_NONE:
		break;
	case TF_RUNE_STRENGTH:
		m_Shared.RemoveCond( TF_COND_RUNE_STRENGTH );
		break;
	case TF_RUNE_HASTE:
		m_Shared.RemoveCond( TF_COND_RUNE_HASTE );
		break;
	case TF_RUNE_REGEN:
		m_Shared.RemoveCond( TF_COND_RUNE_REGEN );
		break;
	case TF_RUNE_RESIST:
		m_Shared.RemoveCond( TF_COND_RUNE_RESIST );
		break;
	case TF_RUNE_VAMPIRE:
		m_Shared.RemoveCond( TF_COND_RUNE_VAMPIRE );
		break;
	case TF_RUNE_WARLOCK:
		m_Shared.RemoveCond( TF_COND_RUNE_WARLOCK );
		break;
	case TF_RUNE_PRECISION:
		m_Shared.RemoveCond( TF_COND_RUNE_PRECISION );
		break;
	case TF_RUNE_AGILITY:
		m_Shared.RemoveCond( TF_COND_RUNE_AGILITY );
		break;
	case TF_RUNE_KNOCKOUT:
		m_Shared.RemoveCond( TF_COND_RUNE_KNOCKOUT );
		break;
	case TF_RUNE_KING:
		m_Shared.RemoveCond( TF_COND_RUNE_KING );
		break;
	case TF_RUNE_PLAGUE:
		m_Shared.RemoveCond( TF_COND_RUNE_PLAGUE );
		break;
	case TF_RUNE_SUPERNOVA:
		m_Shared.RemoveCond( TF_COND_RUNE_SUPERNOVA );
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DropReviveMarker( void )
{
	if ( m_Shared.IsLoser() )
		return;

	CTFReviveMarker *pMarker = CTFReviveMarker::Create( this );
	m_hReviveSpawnSpot = pMarker;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::BecomeZombie( CBaseEntity *pKiller )
{
	if ( !pKiller )
		return;

	if ( IsPlayerClass( TF_CLASS_SPY ) )
		return;

	CTFZombie *pZombie = CTFZombie::CreateZombie( GetAbsOrigin(), this, pKiller );
	if ( pZombie )
	{
		Vector vecOrigin = GetAbsOrigin() + Vector( 0, 0, 16 );
		if ( IsSpaceToSpawnHere( vecOrigin ) )
		{
			pZombie->Teleport( &vecOrigin, &GetAbsAngles(), NULL );
			DestroyRagdoll();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayerDeathThink( void )
{
	//overridden, do nothing
}

//-----------------------------------------------------------------------------
// Purpose: Remove the tf items from the player then call into the base class
//          removal of items.
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveAllItems( bool removeSuit )
{
	// If the player has a capture flag, drop it.
	if ( HasItem() )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_flag_event" );
		if ( event )
		{
			event->SetInt( "player", entindex() );
			event->SetString( "player_name", GetPlayerName() );
			event->SetInt( "carrier", entindex() );
			event->SetInt( "eventtype", TF_FLAGEVENT_DROPPED );
			event->SetInt( "home", 0 ); // whether or not the flag was home (only set for TF_FLAGEVENT_PICKUP) 
			event->SetInt( "team", GetItem()->GetTeamNumber() );
			event->SetInt( "priority", 8 );
			gameeventmanager->FireEvent( event );
		}

		GetItem()->Drop( this, true );
	}

	if ( m_hOffHandWeapon.Get() )
	{ 
		HolsterOffHandWeapon();

		// hide the weapon model
		// don't normally have to do this, unless we have a holster animation
		CTFViewModel *vm = dynamic_cast<CTFViewModel*>( GetViewModel( 1 ) );
		if ( vm )
		{
			vm->SetWeaponModel( NULL, NULL );
		}

		m_hOffHandWeapon = NULL;
	}

	Weapon_SetLast( NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveAllWeapons( void )
{
	BaseClass::RemoveAllWeapons();

	// Remove all wearables.
	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CEconWearable *pWearable = GetWearable( i );
		if ( !pWearable )
			continue;

		RemoveWearable( pWearable );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Mapmaker input to force this player to speak a response rules concept
//-----------------------------------------------------------------------------
void CTFPlayer::InputSetForcedTauntCam( inputdata_t &inputdata )
{
	 m_nForceTauntCam = clamp( inputdata.value.Int(), 0, 1 );
}

//-----------------------------------------------------------------------------
// Purpose: Ignite a player
//-----------------------------------------------------------------------------
void CTFPlayer::InputIgnitePlayer( inputdata_t &inputdata )
{
	m_Shared.Burn( ToTFPlayer( inputdata.pActivator ), NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Extinguish a player
//-----------------------------------------------------------------------------
void CTFPlayer::InputExtinguishPlayer( inputdata_t &inputdata )
{
	if ( m_Shared.InCond( TF_COND_BURNING ) )
	{
		EmitSound( "TFPlayer.FlameOut" );
		m_Shared.RemoveCond( TF_COND_BURNING );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Bleed a player
//-----------------------------------------------------------------------------
void CTFPlayer::InputBleedPlayer( inputdata_t &inputdata )
{
	m_Shared.MakeBleed( ToTFPlayer( inputdata.pActivator ), NULL, inputdata.value.Float(), 4 );
}

//-----------------------------------------------------------------------------
// Purpose: Set Model
//-----------------------------------------------------------------------------
void CTFPlayer::InputSetCustomModel( inputdata_t &inputdata )
{
	char *szModel = (char*)inputdata.value.String();
	Q_snprintf( m_iszCustomModel.GetForModify(), MAX_PATH, szModel );

	if ( Q_stricmp( m_iszCustomModel, "" ) )
	{
		PrecacheModel( szModel );
		SetModel( m_iszCustomModel );
	}
	else
	{
		SetModel( GetPlayerClass()->GetModelName() );
	}
}


void CTFPlayer::ClientHearVox( const char *pSentence )
{
	//TFTODO: implement this.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateModel( void )
{
	if (Q_stricmp(m_iszCustomModel, "")){
		SetModel(m_iszCustomModel);
		float flCustomScale = 1;
		CALL_ATTRIB_HOOK_FLOAT(flCustomScale, player_model_scale);
		SetModelScale(flCustomScale);
		DevMsg("ModelScale is %s", flCustomScale);
	}
	else
		SetModel( GetPlayerClass()->GetModelName() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iSkin - 
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateSkin( int iTeam )
{
	// The player's skin is team - 2.
	//int iSkin = iTeam - 2;
	int iSkin;

	switch (iTeam)
	{
	case TF_TEAM_RED:
		iSkin = 0;
		break;
	case TF_TEAM_BLUE:
		iSkin = 1;
		break;
	default:
		iSkin = 0;
		break;
	}

	//iSkin = iTeam - 2;

	// Check to see if the skin actually changed.
	if ( iSkin != m_iLastSkin )
	{
		m_nSkin = iSkin;
		m_iLastSkin = iSkin;
	}
}

//=========================================================================
// Displays the state of the items specified by the Goal passed in
void CTFPlayer::DisplayLocalItemStatus( CTFGoal *pGoal )
{
#if 0
	for (int i = 0; i < 4; i++)
	{
		if (pGoal->display_item_status[i] != 0)
		{
			CTFGoalItem *pItem = Finditem(pGoal->display_item_status[i]);
			if (pItem)
				DisplayItemStatus(pGoal, this, pItem);
			else
				ClientPrint( this, HUD_PRINTTALK, "#Item_missing" );
		}
	}
#endif
}

//=========================================================================
// Called when the player disconnects from the server.
void CTFPlayer::TeamFortress_ClientDisconnected( void )
{
	RemoveAllOwnedEntitiesFromWorld( false );
	RemoveNemesisRelationships();
	m_OnDeath.FireOutput( this, this );
	CTFWeaponBase *pMyPortalGun = static_cast<CTFWeaponBase*>(Weapon_OwnsThisID(TF_WEAPON_PORTALGUN));
	if (entindex() == 0 && TFGameRules()->IsInPortalMap() && pMyPortalGun)
	{
		UTIL_Remove(pMyPortalGun);
		CBaseEntity *pPortalGun = gEntList.FindEntityByClassname(NULL, "tf_weapon_portalgun");
		if (pPortalGun)
		{
			CWeaponPortalgun *pPortalGunToModify = dynamic_cast<CWeaponPortalgun*>(pPortalGun);
			if (pPortalGunToModify)
			{
				pPortalGunToModify->Reload();
				pPortalGunToModify->m_iPortalLinkageGroupID = 0;
			}
		}
	}
	RemoveAllWeapons();
}

//=========================================================================
// Removes everything this player has (buildings, grenades, etc.) from the world
void CTFPlayer::RemoveAllOwnedEntitiesFromWorld( bool bSilent /* = true */ )
{
	RemoveOwnedProjectiles();
	
	// Destroy any buildables - this should replace TeamFortress_RemoveBuildings
	RemoveAllObjects( bSilent );
}

//=========================================================================
// Removes all projectiles player has fired into the world.
void CTFPlayer::RemoveOwnedProjectiles( void )
{
	for ( int i = 0; i < IBaseProjectileAutoList::AutoList().Count(); i++ )
	{
		CBaseProjectile *pProjectile = static_cast<CBaseProjectile *>( IBaseProjectileAutoList::AutoList()[i] );

		// If the player owns this entity, remove it.
		CBaseEntity *pOwner = pProjectile->GetOwnerEntity();

		if ( !pOwner )
		{
			// Might be a grenade.
			CBaseGrenade *pGrenade = dynamic_cast<CBaseGrenade *>( pProjectile );
			if ( pGrenade )
			{
				pOwner = pGrenade->GetThrower();
			}
		}

		if ( pOwner == this )
		{
			pProjectile->SetThink( &CBaseEntity::SUB_Remove );
			pProjectile->SetNextThink( gpGlobals->curtime );
			pProjectile->SetTouch( NULL );
			pProjectile->AddEffects( EF_NODRAW );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::NoteWeaponFired()
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

//=============================================================================
//
// Player state functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CPlayerStateInfo *CTFPlayer::StateLookupInfo( int nState )
{
	// This table MUST match the 
	static CPlayerStateInfo playerStateInfos[] =
	{
		{ TF_STATE_ACTIVE,				"TF_STATE_ACTIVE",				&CTFPlayer::StateEnterACTIVE,				NULL,	NULL },
		{ TF_STATE_WELCOME,				"TF_STATE_WELCOME",				&CTFPlayer::StateEnterWELCOME,				NULL,	&CTFPlayer::StateThinkWELCOME },
		{ TF_STATE_OBSERVER,			"TF_STATE_OBSERVER",			&CTFPlayer::StateEnterOBSERVER,				NULL,	&CTFPlayer::StateThinkOBSERVER },
		{ TF_STATE_DYING,				"TF_STATE_DYING",				&CTFPlayer::StateEnterDYING,				NULL,	&CTFPlayer::StateThinkDYING },
	};

	for ( int iState = 0; iState < ARRAYSIZE( playerStateInfos ); ++iState )
	{
		if ( playerStateInfos[iState].m_nPlayerState == nState )
			return &playerStateInfos[iState];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateEnter( int nState )
{
	m_Shared.m_nPlayerState = nState;
	m_pStateInfo = StateLookupInfo( nState );

	if ( tf_playerstatetransitions.GetInt() == -1 || tf_playerstatetransitions.GetInt() == entindex() )
	{
		if ( m_pStateInfo )
			Msg( "ShowStateTransitions: entering '%s'\n", m_pStateInfo->m_pStateName );
		else
			Msg( "ShowStateTransitions: entering #%d\n", nState );
	}

	// Initialize the new state.
	if ( m_pStateInfo && m_pStateInfo->pfnEnterState )
	{
		(this->*m_pStateInfo->pfnEnterState)();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateLeave( void )
{
	if ( m_pStateInfo && m_pStateInfo->pfnLeaveState )
	{
		(this->*m_pStateInfo->pfnLeaveState)();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateTransition( int nState )
{
	StateLeave();
	StateEnter( nState );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateEnterWELCOME( void )
{
	PickWelcomeObserverPoint();  
	
	StartObserverMode( OBS_MODE_FIXED );

	// Important to set MOVETYPE_NONE or our physics object will fall while we're sitting at one of the intro cameras.
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );
	AddEffects( EF_NODRAW | EF_NOSHADOW );		

	PhysObjectSleep();

	if ( TFGameRules()->IsCoOp() )
	{
		mp_waitingforplayers_cancel.SetValue( 1 );
		m_bSeenRoundInfo = true;
		m_bIsIdle = false;

		ShowViewPortPanel( PANEL_TEAM, false );
		ShowViewPortPanel( PANEL_MAPINFO, false );
		ShowViewPortPanel( PANEL_CLASS_RED, true );

		ChangeTeam( TF_STORY_TEAM, false, true );
		return;
	}
	else if ( TFGameRules()->IsBluCoOp() )
	{
		mp_waitingforplayers_cancel.SetValue( 1 );
		m_bSeenRoundInfo = true;
		m_bIsIdle = false;

		ShowViewPortPanel( PANEL_TEAM, false );
		ShowViewPortPanel( PANEL_MAPINFO, false );
		ShowViewPortPanel( PANEL_CLASS_BLUE, true );

		ChangeTeam( TF_COMBINE_TEAM, false, true );
		return;
	}

	if ( gpGlobals->eLoadType == MapLoad_Background )
	{
		m_bSeenRoundInfo = true;

		ChangeTeam( TEAM_SPECTATOR );
	}
	else if ( ( TFGameRules() && TFGameRules()->IsLoadingBugBaitReport()) )
	{
		m_bSeenRoundInfo = true;
		
		ChangeTeam( TF_TEAM_BLUE );
		SetDesiredPlayerClassIndex( TF_CLASS_SCOUT );
		ForceRespawn();
	}
	else if ( IsInCommentaryMode() )
	{
		m_bSeenRoundInfo = true;
	}
	else
	{
		KeyValues *data = new KeyValues( "data" );
		data->SetString( "title", "#TF_Welcome" );	// info panel title
		data->SetString( "type", "1" );				// show userdata from stringtable entry
		data->SetString( "msg",	"motd" );			// use this stringtable entry
		data->SetString( "cmd", "mapinfo" );		// exec this command if panel closed

		ShowViewPortPanel( PANEL_INFO, true, data );

		data->deleteThis();

		m_bSeenRoundInfo = false;
	}

	m_bIsIdle = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StateThinkWELCOME( void )
{
	if ( IsInCommentaryMode() && !IsFakeClient() )
	{
		ChangeTeam( TF_TEAM_BLUE );
		SetDesiredPlayerClassIndex( TF_CLASS_SCOUT );
		ForceRespawn();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateEnterACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	RemoveEffects( EF_NODRAW | EF_NOSHADOW );
	RemoveSolidFlags( FSOLID_NOT_SOLID );
	m_Local.m_iHideHUD = 0;
	PhysObjectWake();

	m_flLastAction = gpGlobals->curtime;
	m_bIsIdle = false;

	SetContextThink( &CTFPlayer::RegenThink, gpGlobals->curtime + TF_MEDIC_REGEN_TIME, "RegenThink" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::SetObserverMode(int mode)
{
	if ( mode < OBS_MODE_NONE || mode >= NUM_OBSERVER_MODES )
		return false;

	// Skip OBS_MODE_POI as we're not using that.
	if ( mode == OBS_MODE_POI )
	{
		mode++;
	}

	// Skip over OBS_MODE_ROAMING for dead players
	if( GetTeamNumber() > TEAM_SPECTATOR )
	{
		if ( IsDead() && ( mode > OBS_MODE_FIXED ) && mp_fadetoblack.GetBool() )
		{
			mode = OBS_MODE_CHASE;
		}
		else if ( mode == OBS_MODE_ROAMING )
		{
			mode = OBS_MODE_IN_EYE;
		}
	}

	if ( m_iObserverMode > OBS_MODE_DEATHCAM )
	{
		// remember mode if we were really spectating before
		m_iObserverLastMode = m_iObserverMode;
	}

	m_iObserverMode = mode;
	m_flLastAction = gpGlobals->curtime;

	switch ( mode )
	{
	case OBS_MODE_NONE:
	case OBS_MODE_FIXED :
	case OBS_MODE_DEATHCAM :
		SetFOV( this, 0 );	// Reset FOV
		SetViewOffset( vec3_origin );
		SetMoveType( MOVETYPE_NONE );
		break;

	case OBS_MODE_CHASE :
	case OBS_MODE_IN_EYE :	
		// udpate FOV and viewmodels
		SetObserverTarget( m_hObserverTarget );	
		SetMoveType( MOVETYPE_OBSERVER );
		break;

	case OBS_MODE_ROAMING :
		SetFOV( this, 0 );	// Reset FOV
		SetObserverTarget( m_hObserverTarget );
		SetViewOffset( vec3_origin );
		SetMoveType( MOVETYPE_OBSERVER );
		break;
		
	case OBS_MODE_FREEZECAM:
		SetFOV( this, 0 );	// Reset FOV
		SetObserverTarget( m_hObserverTarget );
		SetViewOffset( vec3_origin );
		SetMoveType( MOVETYPE_OBSERVER );
		break;
	}

	CheckObserverSettings();

	return true;	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateEnterOBSERVER( void )
{
	// Drop flag when switching to spec.
	if ( HasTheFlag() )
		DropFlag();

	if ( m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
		DropRune();

	// Always start a spectator session in chase mode
	m_iObserverLastMode = OBS_MODE_CHASE;

	if( m_hObserverTarget == NULL )
	{
		// find a new observer target
		CheckObserverSettings();
	}

	if ( !m_bAbortFreezeCam )
	{
		FindInitialObserverTarget();
	}

	StartObserverMode( m_iObserverLastMode );

	PhysObjectSleep();

	m_bIsIdle = false;

	if ( GetTeamNumber() != TEAM_SPECTATOR )
	{
		HandleFadeToBlack();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StateThinkOBSERVER()
{
	// Make sure nobody has changed any of our state.
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::StateEnterDYING( void )
{
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_bPlayedFreezeCamSound = false;
	m_bAbortFreezeCam = false;
}

//-----------------------------------------------------------------------------
// Purpose: Move the player to observer mode once the dying process is over
//-----------------------------------------------------------------------------
void CTFPlayer::StateThinkDYING( void )
{
	// If we have a ragdoll, it's time to go to deathcam
	if ( !m_bAbortFreezeCam && m_hRagdoll && 
		(m_lifeState == LIFE_DYING || m_lifeState == LIFE_DEAD) && 
		GetObserverMode() != OBS_MODE_FREEZECAM )
	{
		if ( GetObserverMode() != OBS_MODE_DEATHCAM )
		{
			StartObserverMode( OBS_MODE_DEATHCAM );	// go to observer mode
		}
		RemoveEffects( EF_NODRAW | EF_NOSHADOW );	// still draw player body
	}

	float flTimeInFreeze = spec_freeze_traveltime.GetFloat() + spec_freeze_time.GetFloat();
	float flFreezeEnd = (m_flDeathTime + TF_DEATH_ANIMATION_TIME + flTimeInFreeze );
	if ( !m_bPlayedFreezeCamSound  && GetObserverTarget() && GetObserverTarget() != this )
	{
		// Start the sound so that it ends at the freezecam lock on time
		float flFreezeSoundLength = 0.3;
		float flFreezeSoundTime = (m_flDeathTime + TF_DEATH_ANIMATION_TIME ) + spec_freeze_traveltime.GetFloat() - flFreezeSoundLength;
		if ( gpGlobals->curtime >= flFreezeSoundTime )
		{
			CSingleUserRecipientFilter filter( this );
			EmitSound_t params;
			params.m_flSoundTime = 0;
			params.m_pSoundName = "TFPlayer.FreezeCam";
			EmitSound( filter, entindex(), params );

			m_bPlayedFreezeCamSound = true;
		}
	}

	if ( gpGlobals->curtime >= (m_flDeathTime + TF_DEATH_ANIMATION_TIME ) )	// allow x seconds death animation / death cam
	{
		if ( GetObserverTarget() && GetObserverTarget() != this )
		{
			if ( !m_bAbortFreezeCam && gpGlobals->curtime < flFreezeEnd )
			{
				if ( GetObserverMode() != OBS_MODE_FREEZECAM )
				{
					StartObserverMode( OBS_MODE_FREEZECAM );
					PhysObjectSleep();
				}
				return;
			}
		}

		if ( GetObserverMode() == OBS_MODE_FREEZECAM )
		{
			// If we're in freezecam, and we want out, abort.  (only if server is not using mp_fadetoblack)
			if ( m_bAbortFreezeCam && !mp_fadetoblack.GetBool() )
			{
				if ( m_hObserverTarget == NULL )
				{
					// find a new observer target
					CheckObserverSettings();
				}

				FindInitialObserverTarget();
				SetObserverMode( OBS_MODE_CHASE );
				ShowViewPortPanel( "specgui" , ModeWantsSpectatorGUI(OBS_MODE_CHASE) );
			}
		}

		// Don't allow anyone to respawn until freeze time is over, even if they're not
		// in freezecam. This prevents players skipping freezecam to spawn faster.
		if ( gpGlobals->curtime < flFreezeEnd )
			return;

		m_lifeState = LIFE_RESPAWNABLE;

		StopAnimation();

		AddEffects( EF_NOINTERP );

		if ( GetMoveType() != MOVETYPE_NONE && (GetFlags() & FL_ONGROUND) )
			SetMoveType( MOVETYPE_NONE );

		StateTransition( TF_STATE_OBSERVER );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::AttemptToExitFreezeCam( void )
{
	float flFreezeTravelTime = (m_flDeathTime + TF_DEATH_ANIMATION_TIME ) + spec_freeze_traveltime.GetFloat() + 0.5;
	if ( gpGlobals->curtime < flFreezeTravelTime )
		return;

	m_bAbortFreezeCam = true;
}

class CIntroViewpoint : public CPointEntity
{
	DECLARE_CLASS( CIntroViewpoint, CPointEntity );
public:
	DECLARE_DATADESC();

	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	int			m_iIntroStep;
	float		m_flStepDelay;
	string_t	m_iszMessage;
	string_t	m_iszGameEvent;
	float		m_flEventDelay;
	int			m_iGameEventData;
	float		m_flFOV;
};

BEGIN_DATADESC( CIntroViewpoint )
	DEFINE_KEYFIELD( m_iIntroStep,	FIELD_INTEGER,	"step_number" ),
	DEFINE_KEYFIELD( m_flStepDelay,	FIELD_FLOAT,	"time_delay" ),
	DEFINE_KEYFIELD( m_iszMessage,	FIELD_STRING,	"hint_message" ),
	DEFINE_KEYFIELD( m_iszGameEvent,	FIELD_STRING,	"event_to_fire" ),
	DEFINE_KEYFIELD( m_flEventDelay,	FIELD_FLOAT,	"event_delay" ),
	DEFINE_KEYFIELD( m_iGameEventData,	FIELD_INTEGER,	"event_data_int" ),
	DEFINE_KEYFIELD( m_flFOV,	FIELD_FLOAT,	"fov" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( game_intro_viewpoint, CIntroViewpoint );

int CTFPlayer::GiveAmmo( int iCount, int iAmmoIndex, bool bSuppressSound, EAmmoSource ammosource )
{
	if ( iCount <= 0 )
	{
		return 0;
	}

	if ( iAmmoIndex == TF_AMMO_METAL )
	{
		if ( ammosource != TF_AMMO_SOURCE_RESUPPLY )
		{
			CALL_ATTRIB_HOOK_INT( iCount, mult_metal_pickup );
		}
	}

	int nAmmoBecomesHealth = 0;
	CALL_ATTRIB_HOOK_INT( nAmmoBecomesHealth, ammo_becomes_health );
	if ( nAmmoBecomesHealth != 0 )
	{
		if ( ammosource == TF_AMMO_SOURCE_AMMOPACK )
		{
			if ( TakeHealth( iCount, DMG_GENERIC ) > 0 )
			{
				if ( !bSuppressSound )
					EmitSound( "BaseCombatCharacter.AmmoPickup" );
				m_Shared.HealthKitPickupEffects( iCount );
			}
			return 0;
		}

		if ( ammosource == TF_AMMO_SOURCE_DISPENSER )
			return 0;
	}

	if ( !g_pGameRules->CanHaveAmmo( this, iAmmoIndex ) )
	{
		// game rules say I can't have any more of this ammo type.
		return 0;
	}

	int iMaxAmmo = GetMaxAmmo( iAmmoIndex );
	int iAmmoCount = GetAmmoCount( iAmmoIndex );
	int iAdd = min( iCount, iMaxAmmo - iAmmoCount );

	if ( iAdd < 1 )
	{
		return 0;
	}

	CBaseCombatCharacter::GiveAmmo( iAdd, iAmmoIndex, bSuppressSound );
	return iAdd;
}

//-----------------------------------------------------------------------------
// Purpose: Give the player some ammo.
// Input  : iCount - Amount of ammo to give.
//			iAmmoIndex - Index of the ammo into the AmmoInfoArray
//			iMax - Max carrying capability of the player
// Output : Amount of ammo actually given
//-----------------------------------------------------------------------------
int CTFPlayer::GiveAmmo( int iCount, int iAmmoIndex, bool bSuppressSound )
{
	return GiveAmmo( iCount, iAmmoIndex, bSuppressSound, TF_AMMO_SOURCE_AMMOPACK );
}

//-----------------------------------------------------------------------------
// Purpose: Reset player's information and force him to spawn
//-----------------------------------------------------------------------------
void CTFPlayer::ForceRespawn( void )
{
	if ( ShouldUseCoopSpawning() && !m_hTempSpawnSpot.Get() )
	{
		if ( m_nButtons & IN_USE_ACTION )
		{
			m_bSearchingSpawn = false;
		}
		else
		{
			// In co-op, we respawn near a living teammate.
			if ( !m_bSearchingSpawn )
			{
				SetContextThink( &CTFPlayer::SearchCoopSpawnSpot, gpGlobals->curtime, "SpawnSearchThink" );
				m_bSearchingSpawn = true;
			}
			return;
		}
	}

	// Stop searching for spawn point in case we're still doing this.
	SetContextThink( NULL, 0, "SpawnSearchThink" );

	CTF_GameStats.Event_PlayerForceRespawn( this );

	m_flSpawnTime = gpGlobals->curtime;

	int iDesiredClass = GetDesiredPlayerClassIndex();

	if ( iDesiredClass == TF_CLASS_UNDEFINED )
	{
		return;
	}

	if ( iDesiredClass == TF_CLASS_RANDOM )
	{
		// Don't let them be the same class twice in a row
		do{
			iDesiredClass = random->RandomInt( TF_FIRST_NORMAL_CLASS, TF_LAST_NORMAL_CLASS );
		} while( iDesiredClass == GetPlayerClass()->GetClassIndex() );
	}

	if ( HasTheFlag() )
		DropFlag();

	if ( m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
		DropRune();

	if ( GetPlayerClass()->GetClassIndex() != iDesiredClass )
	{
		// clean up any pipebombs/buildings in the world (no explosions)
		RemoveAllOwnedEntitiesFromWorld();

		GetPlayerClass()->Init( iDesiredClass );

		CTF_GameStats.Event_PlayerChangedClass( this );
	}

	m_Shared.RemoveAllCond();

	RemoveAllItems( true );

	// Reset ground state for airwalk animations
	SetGroundEntity( NULL );

	// TODO: move this into conditions
	RemoveTeleportEffect();

	// remove invisibility very quickly	
	m_Shared.FadeInvis( 0.1 );

	// Stop any firing that was taking place before respawn.
	m_nButtons = 0;

	DestroyRagdoll();

	StateTransition( TF_STATE_ACTIVE );
	Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Do nothing multiplayer_animstate takes care of animation.
//-----------------------------------------------------------------------------
void CTFPlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Handle impulse commands
//-----------------------------------------------------------------------------
void CTFPlayer::ImpulseCommands( void )
{
	BaseClass::ImpulseCommands();
}

//-----------------------------------------------------------------------------
// Purpose: Handle cheat commands
//-----------------------------------------------------------------------------
void CTFPlayer::CheatImpulseCommands( int iImpulse )
{
	switch( iImpulse )
	{
	case 101:
		{
			if( sv_cheats->GetInt() != 0 || IsDeveloper() )
			{
				extern int gEvilImpulse101;
				gEvilImpulse101 = true;

				for ( int i = TF_AMMO_PRIMARY; i < TF_AMMO_COUNT; i++ )
				{
					GiveAmmo( GetMaxAmmo( i ), i, false, TF_AMMO_SOURCE_RESUPPLY );
				}

				SetHealth( GetMaxHealth() );

				// Refill clip in all weapons.
				for ( int i = 0; i < WeaponCount(); i++ )
				{
					CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
					if ( !pWeapon )
						continue;

					pWeapon->GiveDefaultAmmo();
					pWeapon->WeaponRegenerate();

					// Refill charge meters
					if ( pWeapon->HasChargeBar() )
						pWeapon->EffectBarRegenFinished();

					m_Shared.m_flEffectBarProgress = 100.0f;

					if ( m_Shared.HasDemoShieldEquipped() )
						m_Shared.SetShieldChargeMeter( 100.0f );

					m_Shared.SetScoutHypeMeter( 100.0f );

					CWeaponMedigun *pMedigun = GetMedigun();
					if ( pMedigun )
					{
						int nRageOnHeal = 0;
						CALL_ATTRIB_HOOK_INT_ON_OTHER( pMedigun, nRageOnHeal, generate_rage_on_heal );
						if ( nRageOnHeal != 0 )
							pMedigun->SetEnergyMeter( 100.0f );
					}

					m_Shared.SetFlashlightBattery( 100.0f );
				}

			// buildings
			for ( int i = GetObjectCount()-1; i >= 0; i-- )
			{
				CBaseObject *pObj = GetObject( i );

				if ( pObj )
					pObj->FinishUpgrading();
			}

				gEvilImpulse101 = false;
			}
		}
		break;

	default:
		{
			BaseClass::CheatImpulseCommands( iImpulse );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SetWeaponBuilder( CTFWeaponBuilder *pBuilder )
{
	m_hWeaponBuilder = pBuilder;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWeaponBuilder *CTFPlayer::GetWeaponBuilder( void )
{
	Assert( 0 );
	return m_hWeaponBuilder;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this player is building something
//-----------------------------------------------------------------------------
bool CTFPlayer::IsBuilding( void )
{
	/*
	CTFWeaponBuilder *pBuilder = GetWeaponBuilder();
	if ( pBuilder )
		return pBuilder->IsBuilding();
		*/

	return false;
}

void CTFPlayer::RemoveBuildResources( int iAmount )
{
	RemoveAmmo( iAmount, TF_AMMO_METAL );
}

void CTFPlayer::AddBuildResources( int iAmount )
{
	GiveAmmo( iAmount, TF_AMMO_METAL );	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseObject	*CTFPlayer::GetObject( int index )
{
	return (CBaseObject *)( m_aObjects[index].Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFPlayer::GetObjectCount( void )
{
	return m_aObjects.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseObject	*CTFPlayer::GetObjectOfType( int iType, int iMode )
{
	FOR_EACH_VEC( m_aObjects, i )
	{
		CBaseObject *obj = (CBaseObject *)m_aObjects[i].Get();
		if (obj->ObjectType() == iType && obj->GetObjectMode() == iMode)
			return obj;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Remove all the player's objects
//			If bForceAll is set, remove all of them immediately.
//			Otherwise, make them all deteriorate over time.
//			If iClass is passed in, don't remove any objects that can be built 
//			by that class. If bReturnResources is set, the cost of any destroyed 
//			objects will be returned to the player.
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveAllObjects( bool bSilent )
{
	// Remove all the player's objects
	for (int i = GetObjectCount()-1; i >= 0; i--)
	{
		CBaseObject *obj = GetObject(i);
		Assert( obj );

		if ( obj )
		{
			bSilent ? UTIL_Remove(obj) : obj->DetonateObject();
		}		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StopPlacement( void )
{
	/*
	// Tell our builder weapon
	CTFWeaponBuilder *pBuilder = GetWeaponBuilder();
	if ( pBuilder )
	{
		pBuilder->StopPlacement();
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Player has started building an object
//-----------------------------------------------------------------------------
int	CTFPlayer::StartedBuildingObject( int iObjectType )
{
	// Deduct the cost of the object
	int iCost = CalculateObjectCost( iObjectType, HasGunslinger() );
	if ( iCost > GetBuildResources() )
	{
		// Player must have lost resources since he started placing
		return 0;
	}

	RemoveBuildResources( iCost );

	// If the object costs 0, we need to return non-0 to mean success
	if ( !iCost )
		return 1;

	return iCost;
}

//-----------------------------------------------------------------------------
// Purpose: Player has aborted building something
//-----------------------------------------------------------------------------
void CTFPlayer::StoppedBuilding( int iObjectType )
{
	/*
	int iCost = CalculateObjectCost( iObjectType );

	AddBuildResources( iCost );

	// Tell our builder weapon
	CTFWeaponBuilder *pBuilder = GetWeaponBuilder();
	if ( pBuilder )
	{
		pBuilder->StoppedBuilding( iObjectType );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Object has been built by this player
//-----------------------------------------------------------------------------
void CTFPlayer::FinishedObject( CBaseObject *pObject )
{
	AddObject( pObject );
	CTF_GameStats.Event_PlayerCreatedBuilding( this, pObject );

	/*
	// Tell our builder weapon
	CTFWeaponBuilder *pBuilder = GetWeaponBuilder();
	if ( pBuilder )
	{
		pBuilder->FinishedObject();
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Add the specified object to this player's object list.
//-----------------------------------------------------------------------------
void CTFPlayer::AddObject( CBaseObject *pObject )
{
	TRACE_OBJECT( UTIL_VarArgs( "%0.2f CBaseTFPlayer::AddObject adding object %p:%s to player %s\n", gpGlobals->curtime, pObject, pObject->GetClassname(), GetPlayerName() ) );

	// Make a handle out of it
	CHandle<CBaseObject> hObject;
	hObject = pObject;

	bool alreadyInList = PlayerOwnsObject( pObject );
	Assert( !alreadyInList );
	if ( !alreadyInList )
	{
		m_aObjects.AddToTail( hObject );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Object built by this player has been destroyed
//-----------------------------------------------------------------------------
void CTFPlayer::OwnedObjectDestroyed( CBaseObject *pObject )
{
	TRACE_OBJECT( UTIL_VarArgs( "%0.2f CBaseTFPlayer::OwnedObjectDestroyed player %s object %p:%s\n", gpGlobals->curtime, 
		GetPlayerName(),
		pObject,
		pObject->GetClassname() ) );

	RemoveObject( pObject );

	// Tell our builder weapon so it recalculates the state of the build icons
	/*
	CTFWeaponBuilder *pBuilder = GetWeaponBuilder();
	if ( pBuilder )
	{
		pBuilder->RecalcState();
	}
	*/
}


//-----------------------------------------------------------------------------
// Removes an object from the player
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveObject( CBaseObject *pObject )
{
	TRACE_OBJECT( UTIL_VarArgs( "%0.2f CBaseTFPlayer::RemoveObject %p:%s from player %s\n", gpGlobals->curtime, 
		pObject,
		pObject->GetClassname(),
		GetPlayerName() ) );

	Assert( pObject );

	int i;
	for ( i = m_aObjects.Count(); --i >= 0; )
	{
		// Also, while we're at it, remove all other bogus ones too...
		if ( (!m_aObjects[i].Get()) || (m_aObjects[i] == pObject))
		{
			m_aObjects.FastRemove(i);
		}
	}
}

//-----------------------------------------------------------------------------
// See if the player owns this object
//-----------------------------------------------------------------------------
bool CTFPlayer::PlayerOwnsObject( CBaseObject *pObject )
{
	return ( m_aObjects.Find( pObject ) != -1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayFlinch( const CTakeDamageInfo &info )
{
	// Don't play flinches if we just died. 
	if ( !IsAlive() )
		return;

	// No pain flinches while disguised, our man has supreme discipline
	if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		return;

	PlayerAnimEvent_t flinchEvent;

	switch ( LastHitGroup() )
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchEvent = PLAYERANIMEVENT_FLINCH_HEAD;
		break;
	case HITGROUP_LEFTARM:
		flinchEvent = PLAYERANIMEVENT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchEvent = PLAYERANIMEVENT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchEvent = PLAYERANIMEVENT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchEvent = PLAYERANIMEVENT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_STOMACH:
	case HITGROUP_CHEST:
	case HITGROUP_GEAR:
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchEvent = PLAYERANIMEVENT_FLINCH_CHEST;
		break;
	}

	DoAnimationEvent( flinchEvent );
}

//-----------------------------------------------------------------------------
// Purpose: Plays the crit sound that players that get crit hear
//-----------------------------------------------------------------------------
float CTFPlayer::PlayCritReceivedSound( void )
{
	float flCritPainLength = 0;
	// Play a custom pain sound to the guy taking the damage
	CSingleUserRecipientFilter receiverfilter( this );
	EmitSound_t params;
	params.m_flSoundTime = 0;
	params.m_pSoundName = "TFPlayer.CritPain";
	params.m_pflSoundDuration = &flCritPainLength;
	EmitSound( receiverfilter, entindex(), params );

	return flCritPainLength;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PainSound( const CTakeDamageInfo &info )
{
	// Don't make sounds if we just died. DeathSound will handle that.
	if ( !IsAlive() )
		return;

	// no pain sounds while disguised, our man has supreme discipline
	if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		return;

	if ( m_flNextPainSoundTime > gpGlobals->curtime )
		return;

	// Don't play falling pain sounds, they have their own system
	if ( info.GetDamageType() & DMG_FALL )
		return;

	// Don't play sounds for 0 dmg
	if ( info.GetDamage() == 0.0f )
		return;

	if ( info.GetDamageType() & DMG_DROWN )
	{
			float flDrownDamageMult = 1.0f;
			CALL_ATTRIB_HOOK_FLOAT(flDrownDamageMult, mult_dmg_drown);
			if (flDrownDamageMult >= 0)
			{
				return;
			}
			else{
				EmitSound("TFPlayer.Drown");
				return;
			}
	}

	if ( info.GetDamageType() & DMG_BURN )
	{
		// Looping fire pain sound is done in CTFPlayerShared::ConditionThink
		return;
	}

	float flPainLength = 0;

	bool bAttackerIsPlayer = ( info.GetAttacker() && info.GetAttacker()->IsPlayer() );

	CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
	Assert( pExpresser );

	pExpresser->AllowMultipleScenes();

	// speak a pain concept here, send to everyone but the attacker
	CPASFilter filter( GetAbsOrigin() );

	if ( bAttackerIsPlayer )
	{
		filter.RemoveRecipient( ToBasePlayer( info.GetAttacker() ) );
	}

	// play a crit sound to the victim ( us )
	if ( info.GetDamageType() & DMG_CRITICAL || info.GetDamageType() & DMG_MINICRITICAL )
	{
		flPainLength = PlayCritReceivedSound();

		// remove us from hearing our own pain sound if we hear the crit sound
		filter.RemoveRecipient( this );
	}

	char szResponse[AI_Response::MAX_RESPONSE_NAME];

	if ( SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_PAIN, "damagecritical:1", szResponse, AI_Response::MAX_RESPONSE_NAME, &filter ) )
	{
		flPainLength = max( GetSceneDuration( szResponse ), flPainLength );
	}

	// speak a louder pain concept to just the attacker
	if ( bAttackerIsPlayer )
	{
		CSingleUserRecipientFilter attackerFilter( ToBasePlayer( info.GetAttacker() ) );
		SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_ATTACKER_PAIN, "damagecritical:1", szResponse, AI_Response::MAX_RESPONSE_NAME, &attackerFilter );
	}

	pExpresser->DisallowMultipleScenes();

	m_flNextPainSoundTime = gpGlobals->curtime + flPainLength;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DeathSound( const CTakeDamageInfo &info )
{
	// Don't make death sounds when choosing a class
	if ( IsPlayerClass( TF_CLASS_UNDEFINED ) )
		return;

	TFPlayerClassData_t *pData = GetPlayerClass()->GetData();
	if ( !pData )
		return;

	if ( m_LastDamageType & DMG_FALL ) // Did we die from falling?
	{
		// They died in the fall. Play a splat sound.
		EmitSound( "Player.FallGib" );
	}

	// Disable this for now, test overriding via response system

//	else if ( m_LastDamageType & DMG_BLAST )
//	{
//		EmitSound( pData->m_szExplosionDeathSound );
//	}
//	else if ( m_LastDamageType & DMG_CRITICAL )
//	{
//		EmitSound( pData->m_szCritDeathSound );
//
//		PlayCritReceivedSound();
//	}
//	else if ( m_LastDamageType & (DMG_CLUB | DMG_SLASH) )
//	{
//		EmitSound( pData->m_szMeleeDeathSound );
//	}
//	else
//	{
//		EmitSound( pData->m_szDeathSound );
//	}
}

//-----------------------------------------------------------------------------
// Purpose: called when this player burns another player
//-----------------------------------------------------------------------------
void CTFPlayer::OnBurnOther( CBaseEntity *pTFPlayerVictim )
{
#define ACHIEVEMENT_BURN_TIME_WINDOW	30.0f
#define ACHIEVEMENT_BURN_VICTIMS	5
	// add current time we burned another player to head of vector
	m_aBurnOtherTimes.AddToHead( gpGlobals->curtime );

	// remove any burn times that are older than the burn window from the list
	float flTimeDiscard = gpGlobals->curtime - ACHIEVEMENT_BURN_TIME_WINDOW;
	for ( int i = 1; i < m_aBurnOtherTimes.Count(); i++ )
	{
		if ( m_aBurnOtherTimes[i] < flTimeDiscard )
		{
			m_aBurnOtherTimes.RemoveMultiple( i, m_aBurnOtherTimes.Count() - i );
			break;
		}
	}

	// see if we've burned enough players in time window to satisfy achievement
	if ( m_aBurnOtherTimes.Count() >= ACHIEVEMENT_BURN_VICTIMS )
	{
		CSingleUserRecipientFilter filter( this );
		UserMessageBegin( filter, "AchievementEvent" );
		WRITE_BYTE( ACHIEVEMENT_TF_BURN_PLAYERSINMINIMIMTIME );
		MessageEnd();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFTeam *CTFPlayer::GetTFTeam( void )
{
	CTFTeam *pTeam = dynamic_cast<CTFTeam *>( GetTeam() );
	Assert( pTeam );
	return pTeam;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFTeam *CTFPlayer::GetOpposingTFTeam( void )
{
	int iTeam = GetTeamNumber();
	if ( iTeam == TF_TEAM_RED )
	{
		return TFTeamMgr()->GetTeam( TF_TEAM_BLUE );
	}
	else
	{
		return TFTeamMgr()->GetTeam( TF_TEAM_RED );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Give this player the "i just teleported" effect for 12 seconds
//-----------------------------------------------------------------------------
void CTFPlayer::TeleportEffect( void )
{
	m_Shared.AddCond( TF_COND_TELEPORTED );

	// Also removed on death
	SetContextThink( &CTFPlayer::RemoveTeleportEffect, gpGlobals->curtime + 12, "TFPlayer_TeleportEffect" );
}

//-----------------------------------------------------------------------------
// Purpose: Remove the teleporter effect
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveTeleportEffect( void )
{
	m_Shared.RemoveCond( TF_COND_TELEPORTED );
	m_Shared.SetTeleporterEffectColor( TEAM_UNASSIGNED );
}

bool CTFPlayer::UseFoundEntity( CBaseEntity *pUseEntity )
{
	bool usedSomething = false;

	//!!!UNDONE: traceline here to prevent +USEing buttons through walls			
	int caps = pUseEntity->ObjectCaps();
	variant_t emptyVariant;

	if ( m_afButtonPressed & IN_USE )
	{
		// Robin: Don't play sounds for NPCs, because NPCs will allow respond with speech.
		if ( !pUseEntity->MyNPCPointer() )
		{
			CSingleUserRecipientFilter filter( this );
			EmitSound_t params;
			params.m_flSoundTime = 0;
			params.m_pSoundName = "HL2Player.Use";
			EmitSound( filter, entindex(), params );
		}
	}

	if ( ( (m_nButtons & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) || ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
	{
		if ( caps & FCAP_CONTINUOUS_USE )
			m_afPhysicsFlags |= PFLAG_USING;

		if ( pUseEntity->ObjectCaps() & FCAP_ONOFF_USE )
		{
			pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_ON );
		}
		else
		{
			pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );
		}

		usedSomething = true;
	}
	// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
	else if ( (m_afButtonReleased & IN_USE) && (pUseEntity->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
	{
		pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );

		usedSomething = true;
	}

	return usedSomething;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayerUse( void )
{
	// both hand are in use, no use allow. 
	if ( !tf_allow_player_use.GetBool() )
		return;

	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	if ( IsObserver() )
	{
		// do special use operation in oberserver mode
		if ( m_afButtonPressed & IN_USE )
			ObserverUse( true );
		else if ( m_afButtonReleased & IN_USE )
			ObserverUse( false );
		
		return;
	}

	// push objects in turbo physics mode
	/*if ( (m_nButtons & IN_USE) && sv_turbophysics.GetBool() )
	{
		Vector forward, up;
		EyeVectors( &forward, NULL, &up );

		trace_t tr;
		// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
		Vector searchCenter = EyePosition();

		CUsePushFilter filter;

		UTIL_TraceLine( searchCenter, searchCenter + forward * 96.0f, MASK_SOLID, &filter, &tr );

		// try the hit entity if there is one, or the ground entity if there isn't.
		CBaseEntity *entity = tr.m_pEnt;

		if ( entity )
		{
			IPhysicsObject *pObj = entity->VPhysicsGetObject();

			if ( pObj )
			{
				Vector vPushAway = (entity->WorldSpaceCenter() - WorldSpaceCenter());
				vPushAway.z = 0;

				float flDist = VectorNormalize( vPushAway );
				flDist = MAX( flDist, 1 );

				float flForce = sv_pushaway_force.GetFloat() / flDist;
				flForce = MIN( flForce, sv_pushaway_max_force.GetFloat() );

				pObj->ApplyForceOffset( vPushAway * flForce, WorldSpaceCenter() );
			}
		}
	}*/

	if ( m_afButtonPressed & IN_USE )
	{
		// Controlling some latched entity?
		if ( ClearUseEntity() )
		{
			return;
		}
		else
		{
			if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
			{
				m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = GetGroundEntity();
				if ( pTrain && !(m_nButtons & IN_JUMP) && (GetFlags() & FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(this) )
				{
					m_afPhysicsFlags |= PFLAG_DIROVERRIDE;
					m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
					m_iTrain |= TRAIN_NEW;
					EmitSound( "HL2Player.TrainUse" );
					return;
				}
			}
		}

		// Tracker 3926:  We can't +USE something if we're climbing a ladder
		if ( GetMoveType() == MOVETYPE_LADDER )
		{
			return;
		}
	}

	CBaseEntity *pUseEntity = FindUseEntity();

	bool usedSomething = false;

	// Found an object
	if ( pUseEntity )
	{
		SetHeldObjectOnOppositeSideOfPortal( false );

		// TODO: Removed because we no longer have ghost animatings. May need to rework this code.
		//// If we found a ghost animating then it needs to be held across a portal
		//CGhostAnimating *pGhostAnimating = dynamic_cast<CGhostAnimating*>( pUseEntity );
		//if ( pGhostAnimating )
		//{
		//	CProp_Portal *pPortal = NULL;

		//	CPortalSimulator *pPortalSimulator = CPortalSimulator::GetSimulatorThatOwnsEntity( pGhostAnimating->GetSourceEntity() );

		//	//HACKHACK: This assumes all portal simulators are a member of a prop_portal
		//	pPortal = (CProp_Portal *)(((char *)pPortalSimulator) - ((int)&(((CProp_Portal *)0)->m_PortalSimulator)));
		//	Assert( (&(pPortal->m_PortalSimulator)) == pPortalSimulator ); //doublechecking the hack

		//	if ( pPortal )
		//	{
		//		SetHeldObjectPortal( pPortal->m_hLinkedPortal );
		//		SetHeldObjectOnOppositeSideOfPortal( true );
		//	}
		//}
		usedSomething = UseFoundEntity( pUseEntity );
	}
	else 
	{
		Vector forward;
		EyeVectors( &forward, NULL, NULL );
		Vector start = EyePosition();

		Ray_t rayPortalTest;
		rayPortalTest.Init( start, start + forward * PLAYER_USE_RADIUS );

		float fMustBeCloserThan = 2.0f;

		CProp_Portal *pPortal = UTIL_Portal_FirstAlongRay( rayPortalTest, fMustBeCloserThan );

		if ( pPortal )
		{
			SetHeldObjectPortal( pPortal );
			pUseEntity = FindUseEntityThroughPortal();
		}

		if ( pUseEntity )
		{
			SetHeldObjectOnOppositeSideOfPortal( true );
			usedSomething = UseFoundEntity( pUseEntity );
		}
		else if ( m_afButtonPressed & IN_USE )
		{
			// Signal that we want to play the deny sound, unless the user is +USEing on a ladder!
			// The sound is emitted in ItemPostFrame, since that occurs after GameMovement::ProcessMove which
			// lets the ladder code unset this flag.
			//m_bPlayUseDenySound = true;
		}
	}

	// Debounce the use key
	if ( usedSomething && pUseEntity )
	{
		m_Local.m_nOldButtons |= IN_USE;
		m_afButtonPressed &= ~IN_USE;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::CreateRagdollEntity( void )
{
	CreateRagdollEntity( false, false, false, (GetFlags() & FL_ONGROUND), false, false, false, false, 0, false );
}

//-----------------------------------------------------------------------------
// Purpose: Create a ragdoll entity to pass to the client.
//-----------------------------------------------------------------------------
void CTFPlayer::CreateRagdollEntity( bool bGibbed, bool bBurning, bool bElectrocute, bool bOnGround, bool bCloak, bool bGoldStatue, bool bIceStatue, bool bDisintigrate, int iDamageCustom, bool bCreatePhysics )
{
	// If we already have a ragdoll destroy it.
	CTFRagdoll *pRagdoll = dynamic_cast<CTFRagdoll*>( m_hRagdoll.Get() );
	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
		pRagdoll = NULL;
	}
	Assert( pRagdoll == NULL );

	Vector vecVelocity = GetAbsVelocity();
	Vector vecTotalBulletForce = m_vecTotalBulletForce;
	if ( TFGameRules()->IsTFCAllowed() )
	{
		vecVelocity = vec3_origin;
		vecTotalBulletForce = vec3_origin;
	}

	// Create a ragdoll.
	pRagdoll = dynamic_cast<CTFRagdoll*>( CreateEntityByName( "tf_ragdoll" ) );
	if ( pRagdoll )
	{
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = vecVelocity;
		pRagdoll->m_vecForce = vecTotalBulletForce;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_iPlayerIndex.Set( entindex() );
		pRagdoll->m_bGib = bGibbed;
		pRagdoll->m_bBurning = bBurning;
		pRagdoll->m_bElectrocuted = bElectrocute;
		pRagdoll->m_bCloaked = bCloak;
		pRagdoll->m_bBecomeAsh = bDisintigrate;
		pRagdoll->m_bCritOnHardHit = bCreatePhysics;
		pRagdoll->m_bGoldRagdoll = bGoldStatue;
		pRagdoll->m_bIceRagdoll = bIceStatue;
		pRagdoll->m_iDamageCustom = iDamageCustom;
		pRagdoll->m_bOnGround = bOnGround;
		pRagdoll->m_iTeam = GetTeamNumber();
		pRagdoll->m_iClass = GetPlayerClass()->GetClassIndex();
	}

	// Turn off the player.
	AddSolidFlags(FSOLID_NOT_SOLID);
	AddEffects(EF_NODRAW | EF_NOSHADOW);
	SetMoveType(MOVETYPE_NONE);

	// Add additional gib setup.
	if ( bGibbed )
	{
		EmitSound( "BaseCombatCharacter.CorpseGib" ); // Squish!
		m_nRenderFX = kRenderFxRagdoll;
	}

	CSoundEnt::InsertSound( SOUND_CARCASS, GetAbsOrigin(), 384, 5 );

	// Save ragdoll handle.
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
// Purpose: Create a ragdoll to fake our death
//-----------------------------------------------------------------------------
void CTFPlayer::CreateFeignDeathRagdoll( CTakeDamageInfo const &info, bool bGibbed, bool bBurning, bool bFriendlyDisguise )
{
	// If we already have a ragdoll destroy it.
	CTFRagdoll *pRagdoll = dynamic_cast<CTFRagdoll *>( m_hRagdoll.Get() );
	if ( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
		pRagdoll = NULL;
	}
	Assert( pRagdoll == NULL );

	// Create a ragdoll.
	pRagdoll = dynamic_cast<CTFRagdoll *>( CreateEntityByName( "tf_ragdoll" ) );
	if ( pRagdoll )
	{
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = m_vecTotalBulletForce;
		pRagdoll->m_iPlayerIndex.Set( entindex() );
		pRagdoll->m_vecForce = CalcDamageForceVector( info );
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_iDamageCustom = info.GetDamageCustom();
		pRagdoll->m_bGib = bGibbed;
		pRagdoll->m_bBurning = bBurning;
		pRagdoll->m_bWasDisguised = bFriendlyDisguise;
		pRagdoll->m_bFeignDeath = true;
		pRagdoll->m_bCloaked = true;
		pRagdoll->m_bOnGround = ( GetFlags() & FL_ONGROUND ) != 0;
		pRagdoll->m_iTeam = GetTeamNumber();
		pRagdoll->m_iClass = GetPlayerClass()->GetClassIndex();
	
		if ( info.GetInflictor() )
		{
			if ( info.GetDamageCustom() == TF_DMG_CUSTOM_BACKSTAB )
			{
				int nTurnToIce = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( info.GetInflictor(), nTurnToIce, freeze_backstab_victim );
				pRagdoll->m_bIceRagdoll = nTurnToIce != 0;
			}

			int nTurnToGold = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( info.GetInflictor(), nTurnToGold, set_turn_to_gold );
			pRagdoll->m_bGoldRagdoll = nTurnToGold != 0;

			int nRagdollsBecomeAsh = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( info.GetInflictor(), nRagdollsBecomeAsh, ragdolls_become_ash );
			pRagdoll->m_bBecomeAsh = nRagdollsBecomeAsh != 0;

			int nRagdollPlasmaEffect = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( info.GetInflictor(), nRagdollPlasmaEffect, ragdolls_plasma_effect );
			if ( nRagdollPlasmaEffect != 0 )
				pRagdoll->m_iDamageCustom = TF_DMG_CUSTOM_PLASMA;

			int nCritOnHardHit = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( info.GetInflictor(), nCritOnHardHit, crit_on_hard_hit );
			pRagdoll->m_bCritOnHardHit = nCritOnHardHit != 0;
		}

		if ( !bGibbed && info.GetDamageType() & DMG_BLAST )
		{
			Vector vecForce = info.GetDamageForce();
			pRagdoll->m_vecForce = vecForce * 1.5;
		}
	}

	m_hRagdoll = pRagdoll;
}


//-----------------------------------------------------------------------------
// Purpose: Destroy's a ragdoll, called with a player is disconnecting.
//-----------------------------------------------------------------------------
void CTFPlayer::DestroyRagdoll( void )
{
	CTFRagdoll *pRagdoll = dynamic_cast<CTFRagdoll*>( m_hRagdoll.Get() );	
	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Weapon_FrameUpdate( void )
{
	BaseClass::Weapon_FrameUpdate();

	if ( m_hOffHandWeapon.Get() && m_hOffHandWeapon->IsWeaponVisible() )
	{
		m_hOffHandWeapon->Operator_FrameUpdate( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTFPlayer::Weapon_HandleAnimEvent( animevent_t *pEvent )
{
	BaseClass::Weapon_HandleAnimEvent( pEvent );

	if ( m_hOffHandWeapon.Get() )
	{
		m_hOffHandWeapon->Operator_HandleAnimEvent( pEvent, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTFPlayer::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget , const Vector *pVelocity ) 
{
	
}

//-----------------------------------------------------------------------------
// Purpose: Remove invisibility, called when player attacks
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveInvisibility( void )
{
	if ( !m_Shared.IsStealthed() )
		return;

	if ( m_Shared.InCond( TF_COND_STEALTHED ) )
	{
		// remove quickly
		m_Shared.RemoveCond( TF_COND_STEALTHED );
		m_Shared.FadeInvis( 0.5f );
	}
	else
	{
		// Blink cloak.
		m_Shared.OnSpyTouchedByEnemy();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove disguise
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveDisguise( void )
{
	// remove quickly
	if ( m_Shared.InCond( TF_COND_DISGUISED ) || m_Shared.InCond( TF_COND_DISGUISING ) )
	{
		m_Shared.RemoveDisguise();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SaveMe( void )
{
	if ( !IsAlive() || IsPlayerClass( TF_CLASS_UNDEFINED ) || GetTeamNumber() < TF_TEAM_RED )
		return;

	int nWeaponBlocksHealing = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( GetActiveTFWeapon(), nWeaponBlocksHealing, weapon_blocks_healing );
	if ( nWeaponBlocksHealing == 1 )
		return;

	m_bSaveMeParity = !m_bSaveMeParity;
	CallForNPCMedic();
}

//-----------------------------------------------------------------------------
// Purpose: GIMME THE MEDKIT
//-----------------------------------------------------------------------------
void CTFPlayer::CallForNPCMedic( void )
{
	if ( !IsAlive() )
		return;

	if ( GetNumSquadCommandableMedics() == 0 )
		return;

	CBaseEntity *pCitizen = GetPlayerSquad()->GetAnyMember();
	if ( pCitizen )
	{
		CNPC_Citizen *pMedic = (CNPC_Citizen*)pCitizen;
		if ( pMedic && pMedic->IsMedic() && pMedic->m_NPCState != NPC_STATE_SCRIPT && pMedic->m_flTimeNextHealStare == 0 )
		{
			 pMedic->TossHealthKit( this, Vector( 48.0f, 0.0f, 0.0f ) );
		}
		if ( pMedic && !pMedic->IsMedic() )
		{
			CallForNPCMedic();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: drops the flag
//-----------------------------------------------------------------------------
void CC_DropItem( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() ); 
	if ( pPlayer )
	{
		if ( pPlayer->HasTheFlag() )
		{
			if ( pPlayer->m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
				pPlayer->DropRune();
			else
				pPlayer->DropFlag();
		}
		else
		{
			if ( pPlayer->m_Shared.GetCarryingRuneType() != TF_RUNE_NONE )
				pPlayer->DropRune();
		}
	}
}
static ConCommand dropitem( "dropitem", CC_DropItem, "Drop the flag." );

static void CC_CreateVehicle( CTFPlayer *pPlayer )
{
	if ( !pPlayer )
		return;

	if ( pPlayer->m_pVehicle == NULL && TFGameRules()->IsVehicleSpawningAllowed() )
	{
		if ( TFGameRules()->GetSpawningVehicleType() == 3 )
		{
			Vector vecForward;
			AngleVectors(pPlayer->EyeAngles(), &vecForward);
			CBaseEntity *pJeep = (CBaseEntity *)CreateEntityByName("prop_vehicle_jeep");
			if (pJeep)
			{
				Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 80);
				QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
				if (IsEntityPositionReasonable(vecOrigin) && IsEntityQAngleReasonable(vecAngles))
				{
					pJeep->SetAbsOrigin(vecOrigin);
					pJeep->SetAbsAngles(vecAngles);
					pJeep->KeyValue("model", "models/vehicle.mdl");
					pJeep->KeyValue("solid", "6");
					pJeep->KeyValue("vehiclescript", "scripts/vehicles/jalopy.txt");
					DispatchSpawn(pJeep);
					pJeep->Activate();
					pJeep->Teleport(&vecOrigin, &vecAngles, NULL);
					pPlayer->m_pVehicle = pJeep;
				}
			}
		}
		else if  ( TFGameRules()->GetSpawningVehicleType() == 1 )
		{
			Vector vecForward;
			AngleVectors(pPlayer->EyeAngles(), &vecForward);
			CBaseEntity *pBoat = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
			if (pBoat)
			{
				Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 80);
				QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
				if (IsEntityPositionReasonable(vecOrigin) && IsEntityQAngleReasonable(vecAngles))
				{
					pBoat->SetAbsOrigin(vecOrigin);
					pBoat->SetAbsAngles(vecAngles);
					pBoat->KeyValue("model", "models/airboat.mdl");
					pBoat->KeyValue("solid", "6");
					pBoat->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
					DispatchSpawn(pBoat);
					pBoat->Activate();
					pPlayer->m_pVehicle = pBoat;
				}
			}
		}
		else if ( TFGameRules()->GetSpawningVehicleType() == 2 )
		{
			Vector vecForward;
			AngleVectors(pPlayer->EyeAngles(), &vecForward);
			CBaseEntity *pJeep = (CBaseEntity *)CreateEntityByName("prop_vehicle_jeep");
			if (pJeep)
			{
				Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 80);
				QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
				if (IsEntityPositionReasonable(vecOrigin) && IsEntityQAngleReasonable(vecAngles))
				{
					pJeep->SetAbsOrigin(vecOrigin);
					pJeep->SetAbsAngles(vecAngles);
					pJeep->KeyValue("model", "models/buggy.mdl");
					pJeep->KeyValue("solid", "6");
					pJeep->KeyValue("vehiclescript", "scripts/vehicles/jeep_test.txt");
					DispatchSpawn(pJeep);
					pJeep->Activate();
					pJeep->Teleport(&vecOrigin, &vecAngles, NULL);
					pPlayer->m_pVehicle = pJeep;
				}
			}
		}
	}
	else
	{
		if ( pPlayer->m_pVehicle )
		{
			UTIL_Remove( pPlayer->m_pVehicle );
			pPlayer->m_pVehicle = NULL;
		}
	}
}

void CC_CH_LFE_CreateVehicle( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer || !pPlayer->IsAlive())
		return;

	CC_CreateVehicle( pPlayer );
}

static ConCommand lfe_createvehicle( "lfe_createvehicle", CC_CH_LFE_CreateVehicle, "Create a vehicle in front of the player." );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CC_CH_CreateCrane(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	Vector vecForward;
	AngleVectors( pPlayer->EyeAngles(), &vecForward );

	CBaseEntity *magnet = (CBaseEntity*)CreateEntityByName("phys_magnet");
	if ( magnet )
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 256);
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
		magnet->SetAbsOrigin( vecOrigin );
		magnet->SetAbsAngles( vecAngles );
		magnet->KeyValue( "model", "models/props_wasteland/cranemagnet01a.mdl" );
		magnet->KeyValue( "massScale", "1000" );
		magnet->KeyValue( "targetname", "cranemagnet2" );
		magnet->KeyValue( "overridescript", "damping,0.2,rotdamping,0.2,inertia,0.3" );
		DispatchSpawn( magnet );
		magnet->Activate();
		magnet->Teleport( &vecOrigin, &vecAngles, NULL );

		CBaseEntity *crane = (CBaseEntity*)CreateEntityByName( "prop_vehicle_crane" );
		if ( crane )
		{
			Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 64);
			QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
			crane->SetAbsOrigin( vecOrigin );
			crane->SetAbsAngles( vecAngles );
			crane->KeyValue( "model", "models/cranes/crane_docks.mdl" );
			crane->KeyValue( "targetname", "crane" );
			crane->KeyValue( "vehiclescript", "scripts/vehicles/crane.txt" );
			crane->KeyValue( "magnetname", "cranemagnet2" );
			DispatchSpawn( crane );
			crane->Activate();
			crane->Teleport( &vecOrigin, &vecAngles, NULL );
		}
	}
}

static ConCommand ch_createcrane("ch_createcrane", CC_CH_CreateCrane, "Spawn crane in front of the player.", FCVAR_CHEAT);

class CObserverPoint : public CPointEntity
{
	DECLARE_CLASS( CObserverPoint, CPointEntity );
public:
	DECLARE_DATADESC();

	virtual void Activate( void )
	{
		BaseClass::Activate();

		if ( m_iszAssociateTeamEntityName != NULL_STRING )
		{
			m_hAssociatedTeamEntity = gEntList.FindEntityByName( NULL, m_iszAssociateTeamEntityName );
			if ( !m_hAssociatedTeamEntity )
			{
				Warning("info_observer_point (%s) couldn't find associated team entity named '%s'\n", GetDebugName(), STRING(m_iszAssociateTeamEntityName) );
			}
		}
	}

	bool CanUseObserverPoint( CTFPlayer *pPlayer )
	{
		if ( m_bDisabled )
			return false;

		if ( m_hAssociatedTeamEntity && ( mp_forcecamera.GetInt() == OBS_ALLOW_TEAM ) )
		{
			// If we don't own the associated team entity, we can't use this point
			if ( m_hAssociatedTeamEntity->GetTeamNumber() != pPlayer->GetTeamNumber() && pPlayer->GetTeamNumber() >= FIRST_GAME_TEAM )
				return false;
		}

		// Only spectate observer points on control points in the current miniround
		if ( g_pObjectiveResource->PlayingMiniRounds() && m_hAssociatedTeamEntity )
		{
			CTeamControlPoint *pPoint = dynamic_cast<CTeamControlPoint*>(m_hAssociatedTeamEntity.Get());
			if ( pPoint )
			{
				bool bInRound = g_pObjectiveResource->IsInMiniRound( pPoint->GetPointIndex() );
				if ( !bInRound )
					return false;
			}
		}

		return true;
	}

	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	void InputEnable( inputdata_t &inputdata )
	{
		m_bDisabled = false;
	}
	void InputDisable( inputdata_t &inputdata )
	{
		m_bDisabled = true;
	}
	bool IsDefaultWelcome( void ) { return m_bDefaultWelcome; }

public:
	bool		m_bDisabled;
	bool		m_bDefaultWelcome;
	EHANDLE		m_hAssociatedTeamEntity;
	string_t	m_iszAssociateTeamEntityName;
	float		m_flFOV;
};

BEGIN_DATADESC( CObserverPoint )
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_KEYFIELD( m_bDefaultWelcome, FIELD_BOOLEAN, "defaultwelcome" ),
	DEFINE_KEYFIELD( m_iszAssociateTeamEntityName,	FIELD_STRING,	"associated_team_entity" ),
	DEFINE_KEYFIELD( m_flFOV,	FIELD_FLOAT,	"fov" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
END_DATADESC()

LINK_ENTITY_TO_CLASS(info_observer_point,CObserverPoint);

//-----------------------------------------------------------------------------
// Purpose: Builds a list of entities that this player can observe.
//			Returns the index into the list of the player's current observer target.
//-----------------------------------------------------------------------------
int CTFPlayer::BuildObservableEntityList( void )
{
	m_hObservableEntities.Purge();
	int iCurrentIndex = -1;

	// Add all the map-placed observer points
	CBaseEntity *pObserverPoint = gEntList.FindEntityByClassname( NULL, "info_observer_point" );
	while ( pObserverPoint )
	{
		m_hObservableEntities.AddToTail( pObserverPoint );

		if ( m_hObserverTarget.Get() == pObserverPoint )
		{
			iCurrentIndex = (m_hObservableEntities.Count()-1);
		}

		pObserverPoint = gEntList.FindEntityByClassname( pObserverPoint, "info_observer_point" );
	}

	// Add all the players
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
		{
			m_hObservableEntities.AddToTail( pPlayer );

			if ( m_hObserverTarget.Get() == pPlayer )
			{
				iCurrentIndex = (m_hObservableEntities.Count()-1);
			}
		}
	}

	// Add all my objects
	int iNumObjects = GetObjectCount();
	for ( int i = 0; i < iNumObjects; i++ )
	{
		CBaseObject *pObj = GetObject(i);
		if ( pObj )
		{
			m_hObservableEntities.AddToTail( pObj );

			if ( m_hObserverTarget.Get() == pObj )
			{
				iCurrentIndex = (m_hObservableEntities.Count()-1);
			}
		}
	}

	// Add all NPCs.
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		
		if ( pNPC )
		{
			m_hObservableEntities.AddToTail( pNPC );

			if ( m_hObserverTarget.Get() == pNPC )
			{
				iCurrentIndex = ( m_hObservableEntities.Count() - 1 );
			}
		}
	}

	return iCurrentIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::GetNextObserverSearchStartPoint( bool bReverse )
{
	int iDir = bReverse ? -1 : 1; 
	int startIndex = BuildObservableEntityList();
	int iMax = m_hObservableEntities.Count()-1;

	startIndex += iDir;
	if (startIndex > iMax)
		startIndex = 0;
	else if (startIndex < 0)
		startIndex = iMax;

	return startIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFPlayer::FindNextObserverTarget(bool bReverse)
{
	int startIndex = GetNextObserverSearchStartPoint( bReverse );

	int	currentIndex = startIndex;
	int iDir = bReverse ? -1 : 1; 

	int iMax = m_hObservableEntities.Count()-1;

	// Make sure the current index is within the max. Can happen if we were previously
	// spectating an object which has been destroyed.
	if ( startIndex > iMax )
	{
		currentIndex = startIndex = 1;
	}

	do
	{
		CBaseEntity *nextTarget = m_hObservableEntities[currentIndex];

		if ( IsValidObserverTarget( nextTarget ) )
			return nextTarget;	
 
		currentIndex += iDir;

		// Loop through the entities
		if (currentIndex > iMax)
		{
			currentIndex = 0;
		}
		else if (currentIndex < 0)
		{
			currentIndex = iMax;
		}
	} while ( currentIndex != startIndex );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsValidObserverTarget(CBaseEntity * target)
{
	if ( target && !target->IsPlayer() )
	{
		CObserverPoint *pObsPoint = dynamic_cast<CObserverPoint *>(target);
		if ( pObsPoint && !pObsPoint->CanUseObserverPoint( this ) )
			return false;

		if ( target->IsNPC() )
		{
			if ( lfe_allow_spectate_npc.GetBool() == false )
				return false;

			if ( target->IsEffectActive( EF_NODRAW ) ) // don't watch invisible NPC
				return false;

			if ( target->Classify() == CLASS_BULLSEYE ) // don't watch bullseyes
				return false;
		}

		if ( GetTeamNumber() == TEAM_SPECTATOR )
			return true;

		switch ( mp_forcecamera.GetInt() )	
		{
		case OBS_ALLOW_ALL		:	break;
		case OBS_ALLOW_TEAM		:	if (target->GetTeamNumber() != TEAM_UNASSIGNED && GetTeamNumber() != target->GetTeamNumber())
										return false;
									break;
		case OBS_ALLOW_NONE		:	return false;
		}

		return true;
	}

	return BaseClass::IsValidObserverTarget( target );
}


void CTFPlayer::PickWelcomeObserverPoint( void )
{
	//Don't just spawn at the world origin, find a nice spot to look from while we choose our team and class.
	CObserverPoint *pObserverPoint = (CObserverPoint *)gEntList.FindEntityByClassname( NULL, "info_observer_point" );

	while ( pObserverPoint )
	{
		if ( IsValidObserverTarget( pObserverPoint ) )
		{
			SetObserverTarget( pObserverPoint );
		}

		if ( pObserverPoint->IsDefaultWelcome() )
			break;

		pObserverPoint = (CObserverPoint *)gEntList.FindEntityByClassname( pObserverPoint, "info_observer_point" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::SetObserverTarget(CBaseEntity *target)
{
	ClearZoomOwner();
	SetFOV( this, 0 );
		
	if ( !BaseClass::SetObserverTarget(target) )
		return false;

	CObserverPoint *pObsPoint = dynamic_cast<CObserverPoint *>(target);
	if ( pObsPoint )
	{
		SetViewOffset( vec3_origin );
		JumptoPosition( target->GetAbsOrigin(), target->EyeAngles() );
		SetFOV( pObsPoint, pObsPoint->m_flFOV );
	}

	m_flLastAction = gpGlobals->curtime;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Find the nearest team member within the distance of the origin.
//			Favor players who are the same class.
//-----------------------------------------------------------------------------
CBaseEntity *CTFPlayer::FindNearestObservableTarget( Vector vecOrigin, float flMaxDist )
{
	CTeam *pTeam = GetTeam();
	CBaseEntity *pReturnTarget = NULL;
	bool bFoundClass = false;
	float flCurDistSqr = (flMaxDist * flMaxDist);
	int iNumPlayers = pTeam->GetNumPlayers();

	if ( pTeam->GetTeamNumber() == TEAM_SPECTATOR )
	{
		iNumPlayers = gpGlobals->maxClients;
	}


	for ( int i = 0; i < iNumPlayers; i++ )
	{
		CTFPlayer *pPlayer = NULL;

		if ( pTeam->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		}
		else
		{
			pPlayer = ToTFPlayer( pTeam->GetPlayer(i) );
		}

		if ( !pPlayer )
			continue;

		if ( !IsValidObserverTarget(pPlayer) )
			continue;

		float flDistSqr = ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr();

		if ( flDistSqr < flCurDistSqr )
		{
			// If we've found a player matching our class already, this guy needs
			// to be a matching class and closer to boot.
			if ( !bFoundClass || pPlayer->IsPlayerClass( GetPlayerClass()->GetClassIndex() ) )
			{
				pReturnTarget = pPlayer;
				flCurDistSqr = flDistSqr;

				if ( pPlayer->IsPlayerClass( GetPlayerClass()->GetClassIndex() ) )
				{
					bFoundClass = true;
				}
			}
		}
		else if ( !bFoundClass )
		{
			if ( pPlayer->IsPlayerClass( GetPlayerClass()->GetClassIndex() ) )
			{
				pReturnTarget = pPlayer;
				flCurDistSqr = flDistSqr;
				bFoundClass = true;
			}
		}
	}

	if ( !bFoundClass && IsPlayerClass( TF_CLASS_ENGINEER ) )
	{
		// let's spectate our sentry instead, we didn't find any other engineers to spec
		int iNumObjects = GetObjectCount();
		for ( int i = 0; i < iNumObjects; i++ )
		{
			CBaseObject *pObj = GetObject(i);

			if ( pObj && pObj->GetType() == OBJ_SENTRYGUN )
			{
				pReturnTarget = pObj;
			}
		}
	}		

	// Lastly, look for an allied NPC.
	if ( !pReturnTarget && lfe_allow_spectate_npc.GetBool() == true )
	{
		CTFTeam *pTFTeam = GetTFTeam();
		flCurDistSqr = ( flMaxDist * flMaxDist );
		int iNumNPC = pTFTeam->GetNumNPCs();
		bool bFoundVital = false;
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();

		if ( pTeam->GetTeamNumber() == TEAM_SPECTATOR )
		{
			iNumNPC = g_AI_Manager.NumAIs();
		}

		for ( int i = 0; i < iNumNPC; i++ )
		{
			CAI_BaseNPC *pNPC = NULL;

			if ( pTeam->GetTeamNumber() == TEAM_SPECTATOR )
			{
				pNPC = ppAIs[i];
			}
			else
			{
				pNPC = pTFTeam->GetNPC(i);
			}

			if ( !pNPC )
				continue;

			if ( !IsValidObserverTarget( pNPC ) )
				continue;

			float flDistSqr = ( pNPC->GetAbsOrigin() - vecOrigin ).LengthSqr();

			// Prioritize vital allies (Alyx, Barney) over other NPC.
			if ( flDistSqr < flCurDistSqr )
			{
				if ( !bFoundVital || pNPC->Classify() == CLASS_PLAYER_ALLY_VITAL )
				{
					pReturnTarget = pNPC;
					flCurDistSqr = flDistSqr;

					if ( pNPC->Classify() == CLASS_PLAYER_ALLY_VITAL )
					{
						bFoundVital = true;
					}
				}
			}
			else if ( !bFoundVital )
			{
				if ( pNPC->Classify() == CLASS_PLAYER_ALLY_VITAL )
				{
					pReturnTarget = pNPC;
					flCurDistSqr = flDistSqr;
					bFoundVital = true;
				}
			}
		}
	}

	return pReturnTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::FindInitialObserverTarget( void )
{
	// If we're on a team (i.e. not a pure observer), try and find
	// a target that'll give the player the most useful information.
	if ( GetTeamNumber() >= FIRST_GAME_TEAM )
	{
		CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
		if ( pMaster )
		{
			// Has our forward cap point been contested recently?
			int iFarthestPoint = TFGameRules()->GetFarthestOwnedControlPoint( GetTeamNumber(), false );
			if ( iFarthestPoint != -1 )
			{
				float flTime = pMaster->PointLastContestedAt( iFarthestPoint );
				if ( flTime != -1 && flTime > (gpGlobals->curtime - 30) )
				{
					// Does it have an associated viewpoint?
					CBaseEntity *pObserverPoint = gEntList.FindEntityByClassname( NULL, "info_observer_point" );
					while ( pObserverPoint )
					{
						CObserverPoint *pObsPoint = assert_cast<CObserverPoint *>(pObserverPoint);
						if ( pObsPoint && pObsPoint->m_hAssociatedTeamEntity == pMaster->GetControlPoint(iFarthestPoint) )
						{
							if ( IsValidObserverTarget( pObsPoint ) )
							{
								m_hObserverTarget.Set( pObsPoint );
								return;
							}
						}

						pObserverPoint = gEntList.FindEntityByClassname( pObserverPoint, "info_observer_point" );
					}
				}
			}

			// Has the point beyond our farthest been contested lately?
			iFarthestPoint += (ObjectiveResource()->GetBaseControlPointForTeam( GetTeamNumber() ) == 0 ? 1 : -1);
			if ( iFarthestPoint >= 0 && iFarthestPoint < MAX_CONTROL_POINTS )
			{
				float flTime = pMaster->PointLastContestedAt( iFarthestPoint );
				if ( flTime != -1 && flTime > (gpGlobals->curtime - 30) )
				{
					// Try and find a player near that cap point
					CBaseEntity *pCapPoint = pMaster->GetControlPoint(iFarthestPoint);
					if ( pCapPoint )
					{
						CBaseEntity *pTarget = FindNearestObservableTarget( pCapPoint->GetAbsOrigin(), 1500 );
						if ( pTarget )
						{
							m_hObserverTarget.Set( pTarget );
							return;
						}
					}
				}
			}
		}
	}

	// Find the nearest guy near myself
	CBaseEntity *pTarget = FindNearestObservableTarget( GetAbsOrigin(), FLT_MAX );
	if ( pTarget )
	{
		m_hObserverTarget.Set( pTarget );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ValidateCurrentObserverTarget( void )
{
	// If our current target is a dead player who's gibbed / died, refind as if 
	// we were finding our initial target, so we end up somewhere useful.
	if ( m_hObserverTarget && m_hObserverTarget->IsPlayer() )
	{
		CBasePlayer *player = ToBasePlayer( m_hObserverTarget );

		if ( player->m_lifeState == LIFE_DEAD || player->m_lifeState == LIFE_DYING )
		{
			// Once we're past the pause after death, find a new target
			if ( ( player->GetDeathTime() + DEATH_ANIMATION_TIME ) < gpGlobals->curtime )
			{
				FindInitialObserverTarget();
			}

			return;
		}
	}

	if ( m_hObserverTarget && ( m_hObserverTarget->IsBaseObject() || m_hObserverTarget->IsNPC() ) )
	{
		if ( m_iObserverMode == OBS_MODE_IN_EYE )
		{
			ForceObserverMode( OBS_MODE_CHASE );
		}
	}

	BaseClass::ValidateCurrentObserverTarget();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Touch( CBaseEntity *pOther )
{
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pOther );

	if ( pPlayer )
	{
		CheckUncoveringSpies( pPlayer );

		if ( m_Shared.InCond( TF_COND_RUNE_PLAGUE ) && !pPlayer->m_Shared.InCond( TF_COND_RUNE_RESIST ) )
		{
			pPlayer->m_Shared.AddCond( TF_COND_PLAGUE );
			pPlayer->m_Shared.MakeBleed( this, GetActiveTFWeapon(), 999, 4 );
			pPlayer->EmitSound( "Powerup.PickUpPlagueInfected" );
		}

		if ( m_Shared.InCond( TF_COND_HALLOWEEN_BOMB_HEAD ) )
			MerasmusPlayerBombExplode( false );

		if ( m_Shared.InCond( LFE_COND_ZOMBIE_LEAP ) )
		{
			CTakeDamageInfo info( this, this, vec3_origin, GetAbsOrigin(), 5, DMG_SLASH );
			pPlayer->TakeDamage( info );
		}
	}

	if ( pNPC )
	{
		if ( m_Shared.InCond( TF_COND_RUNE_PLAGUE ) && !pNPC->InCond( TF_COND_RUNE_RESIST ) )
		{
			pNPC->AddCond( TF_COND_PLAGUE );
			pNPC->MakeBleed( this, NULL, 999, 4 );
			pNPC->EmitSound( "Powerup.PickUpPlagueInfected" );
		}

		if ( m_Shared.InCond( TF_COND_HALLOWEEN_BOMB_HEAD ) )
			MerasmusPlayerBombExplode( false );

		if ( m_Shared.InCond( LFE_COND_ZOMBIE_LEAP ) )
		{
			CTakeDamageInfo info( this, this, vec3_origin, GetAbsOrigin(), 5, DMG_SLASH );
			pNPC->TakeDamage( info );
		}
	}

	BaseClass::Touch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: Check to see if this player has seen through an enemy spy's disguise
//-----------------------------------------------------------------------------
void CTFPlayer::CheckUncoveringSpies( CTFPlayer *pTouchedPlayer )
{
	// Only uncover enemies
	if ( m_Shared.IsAlly( pTouchedPlayer ) )
		return;

	// Only uncover if they're stealthed
	if ( !pTouchedPlayer->m_Shared.IsStealthed() )
		return;

	if ( !pTouchedPlayer->m_Shared.InCond( TF_COND_FEIGN_DEATH ) )
		return;

	// pulse their invisibility
	pTouchedPlayer->m_Shared.OnSpyTouchedByEnemy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::Taunt( taunts_t eTaunt, int iConcept )
{
	if ( GetActiveTFWeapon() && ( GetActiveTFWeapon()->GetTauntItem() != nullptr ) )
	{
		PlayTauntSceneFromItem( GetActiveTFWeapon()->GetTauntItem() );
		return;
	}

	if ( !IsAllowedToTaunt() )
		return;

	// Allow voice commands, etc to be interrupted.
	CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
	Assert( pExpresser );
	pExpresser->AllowMultipleScenes();

	m_bInitTaunt = true;

	char szResponse[AI_Response::MAX_RESPONSE_NAME];

	if ( SpeakConceptIfAllowed( iConcept, NULL, szResponse, AI_Response::MAX_RESPONSE_NAME ) )
	{
		// Get the duration of the scene.
		float flDuration = GetSceneDuration( szResponse ) + 0.2f;

		// Clear disguising state.
		if ( m_Shared.InCond( TF_COND_DISGUISING ) )
			m_Shared.RemoveCond( TF_COND_DISGUISING );

		// Set player state as taunting.
		m_Shared.AddCond( TF_COND_TAUNTING );
		m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + flDuration;
		if ( FlashlightIsOn() )
			FlashlightTurnOff();

		m_angTauntCamera = EyeAngles();
		m_vecTauntCamera = BodyDirection2D();

		// Slam velocity to zero.
		if ( !tf_allow_sliding_taunt.GetBool() )
			SetAbsVelocity( vec3_origin );

		CAttribute_String strCosmeticTauntSound;
		CALL_ATTRIB_HOOK_STRING( strCosmeticTauntSound, cosmetic_taunt_sound );
		if ( strCosmeticTauntSound && *strCosmeticTauntSound )
			EmitSound( strCosmeticTauntSound );

		CAttribute_String strTauntSucessSound;
		CALL_ATTRIB_HOOK_STRING( strTauntSucessSound, taunt_success_sound );
		if ( strTauntSucessSound && *strTauntSucessSound )
			EmitSound( strTauntSucessSound );

		CAttribute_String strTauntSucessSoundLoop ;
		CALL_ATTRIB_HOOK_STRING( strTauntSucessSoundLoop, taunt_success_sound_loop );
		if ( strTauntSucessSoundLoop && *strTauntSucessSoundLoop )
		{
			CPASAttenuationFilter filter( this );
			if ( !m_sndTauntLoop )
			{
				m_sndTauntLoop = ( CSoundEnvelopeController::GetController() ).SoundCreate( filter, entindex(), CHAN_STATIC, strTauntSucessSoundLoop , ATTN_NORM );
				( CSoundEnvelopeController::GetController() ).Play( m_sndTauntLoop, 1.0f, 100 );
			}
		}

		// Setup a taunt attack if necessary.
		CTFWeaponBase *pWeapon = GetActiveTFWeapon();
		if ( pWeapon )	
		{
			if ( pWeapon->IsWeapon( TF_WEAPON_BONESAW ) )
			{
				if ( !V_strnicmp( szResponse, "scenes/player/medic/low/taunt08.vcd", 32 ) == 0 )
				{
					pWeapon->WeaponSound( TAUNT );

					int iAoeHeal = 0;
					CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iAoeHeal, enables_aoe_heal );
					if ( iAoeHeal != 0 )
					{
						m_flTauntAttackTime = gpGlobals->curtime + 0.1f;
						m_iTauntAttack = TAUNTATK_MEDIC_INHALE;
					}
				}
			}
			else if ( pWeapon->IsWeapon( TF_WEAPON_LUNCHBOX ) )
			{
				if ( IsPlayerClass( TF_CLASS_SCOUT ) )
				{
					m_flTauntAttackTime = gpGlobals->curtime + 1.2f;
					m_iTauntAttack = TAUNTATK_SCOUT_DRINK;
				}
				else
				{
					m_flTauntAttackTime = gpGlobals->curtime + 1.0f;
					m_iTauntAttack = TAUNTATK_HEAVY_EAT;
				}
				pWeapon->DepleteAmmo();
			}
			else if ( pWeapon->IsWeapon( TF_WEAPON_FLAMETHROWER ) && ( CAttributeManager::AttribHookValue<int>( 0, "burn_damage_earns_rage", pWeapon ) == 1 ) && m_Shared.GetRageProgress() >= 100.0f )
			{
				m_Shared.AddCond( TF_COND_INVULNERABLE_USER_BUFF, flDuration );
				m_Shared.ActivateRageBuff( pWeapon, TF_BUFF_CRITBOOSTED );
			}
		}

		if ( V_stricmp( szResponse, "scenes/player/pyro/low/taunt02.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 2.0f;
			m_iTauntAttack = TAUNTATK_PYRO_HADOUKEN;
		}
		else if ( V_stricmp( szResponse, "scenes/player/heavy/low/taunt03_v1.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 1.8f;
			m_iTauntAttack = TAUNTATK_HEAVY_HIGH_NOON;
		}
		else if ( V_strnicmp( szResponse, "scenes/player/spy/low/taunt03", 29 ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 1.8f;
			m_iTauntAttack = TAUNTATK_SPY_FENCING_SLASH_A;
		}
		else if ( V_strnicmp( szResponse, "scenes/player/sniper/low/taunt04", 32 ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 0.85f;
			m_iTauntAttack = TAUNTATK_SNIPER_ARROW_STAB_IMPALE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/medic/low/taunt08.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 2.2f;
			m_iTauntAttack = TAUNTATK_MEDIC_UBERSLICE_IMPALE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/engineer/low/taunt09.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 3.2;
			m_iTauntAttack = TAUNTATK_ENGINEER_ARM_IMPALE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/scout/low/taunt05_v1.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 4.03f;
			m_iTauntAttack = TAUNTATK_SCOUT_GRAND_SLAM;
		}
		else if ( V_stricmp( szResponse, "scenes/player/medic/low/taunt06.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 0.35;
			m_iTauntAttack = TAUNTATK_MEDIC_INHALE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/soldier/low/taunt05.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 3.55f;
			m_iTauntAttack = TAUNTATK_SOLDIER_GRENADE_KILL;
		}
		else if ( V_stricmp( szResponse, "scenes/player/pyro/low/taunt_bubbles.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 4.0f;
			m_iTauntAttack = TAUNTATK_PYRO_ARMAGEDDON;
		}
		else if ( V_stricmp( szResponse, "scenes/player/pyro/low/taunt_scorch_shot.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 2.0f;
			m_iTauntAttack = TAUNTATK_PYRO_SCORCHSHOT;
		}
		else if ( V_stricmp( szResponse, "scenes/player/demoman/low/taunt09.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 2.55f;
			m_iTauntAttack = TAUNTATK_DEMOMAN_BARBARIAN_SWING;
		}
		else if ( V_stricmp( szResponse, "scenes/player/engineer/low/taunt07.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 3.69f;
			m_iTauntAttack = TAUNTATK_ENGINEER_GUITAR_SMASH;
		}
		else if ( V_strnicmp( szResponse, "scenes/player/combinesoldier/low/taunt01", 40 ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 0.45f;
			m_iTauntAttack = TAUNTATK_COMBINES_BASH;
		}
		else if ( V_stricmp( szResponse, "scenes/player/combinesoldier/low/taunt02.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 0.50f;
			m_iTauntAttack = TAUNTATK_COMBINES_THROW_GRENADE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/antlion/low/taunt03.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 0.60f;
			m_iTauntAttack = TAUNTATK_ANTLION_IMPALE;
		}
		else if ( V_stricmp( szResponse, "scenes/player/demoman//low/taunt04.vcd" ) == 0 )
		{
			m_flTauntAttackTime = gpGlobals->curtime + 3.0f;
			m_iTauntAttack = TAUNTATK_DEMOMAN_CABER;
		}
	}

	pExpresser->DisallowMultipleScenes();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DoTauntAttack( void )
{
	int iTauntType = m_iTauntAttack;

	if ( !iTauntType )
		return;

	m_iTauntAttack = TAUNTATK_NONE;

	switch ( iTauntType )
	{
		case TAUNTATK_SCOUT_GRAND_SLAM:
		case TAUNTATK_PYRO_HADOUKEN:
		case TAUNTATK_SPY_FENCING_SLASH_A:
		case TAUNTATK_SPY_FENCING_SLASH_B:
		case TAUNTATK_SPY_FENCING_STAB:
		case TAUNTATK_DEMOMAN_BARBARIAN_SWING:
		case TAUNTATK_ENGINEER_GUITAR_SMASH:
		{
			Vector vecAttackDir = m_vecTauntCamera;
			Vector vecOrigin = WorldSpaceCenter() + vecAttackDir * 64;
			Vector mins = vecOrigin - Vector( 24, 24, 24 );
			Vector maxs = vecOrigin + Vector( 24, 24, 24 );

			QAngle angForce( -45.0f, m_angTauntCamera[YAW], 0 );
			Vector vecForce;
			AngleVectors( angForce, &vecForce );
			float flDamage = 0.0f;
			int nDamageType = DMG_GENERIC;
			int iDamageCustom = 0;

			switch ( iTauntType )
			{
			case TAUNTATK_SCOUT_GRAND_SLAM:
				vecForce *= 130000.0f;
				flDamage = 500.0f;
				nDamageType = DMG_CLUB;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM;
				break;
			case TAUNTATK_PYRO_HADOUKEN:
				vecForce *= 25000.0f;
				flDamage = 500.0f;
				nDamageType = DMG_IGNITE;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_HADOUKEN;
				break;
			case TAUNTATK_SPY_FENCING_STAB:
				vecForce *= 20000.0f;
				flDamage = 500.0f;
				nDamageType = DMG_SLASH;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_FENCING;
				break;
			case TAUNTATK_DEMOMAN_BARBARIAN_SWING:
				vecForce.Zero();
				flDamage = 500.0f;
				nDamageType = DMG_SLASH;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING;
				break;
			case TAUNTATK_ENGINEER_GUITAR_SMASH:
				vecForce *= 100.0f;
				flDamage = 500.0f;
				nDamageType = DMG_CLUB;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH;
				break;
			default:
				vecForce *= 100.0f;
				flDamage = 25.0f;
				nDamageType = DMG_SLASH | DMG_PREVENT_PHYSICS_FORCE;
				iDamageCustom = TF_DMG_CUSTOM_TAUNTATK_FENCING;
				break;
			}

			// Spy taunt has 3 hits, set up the next one.
			if ( iTauntType == TAUNTATK_SPY_FENCING_SLASH_A )
			{
				m_flTauntAttackTime = gpGlobals->curtime + 0.47;
				m_iTauntAttack = TAUNTATK_SPY_FENCING_SLASH_B;
			}
			else if ( iTauntType == TAUNTATK_SPY_FENCING_SLASH_B )
			{
				m_flTauntAttackTime = gpGlobals->curtime + 1.73;
				m_iTauntAttack = TAUNTATK_SPY_FENCING_STAB;
			}

			CBaseEntity *pList[256];

			bool bHomerun = false;
			int count = UTIL_EntitiesInBox( pList, 256, mins, maxs, 0 );

			if ( tf_debug_damage.GetBool() )
			{
				NDebugOverlay::Box( vecOrigin, -Vector( 24, 24, 24 ), Vector( 24, 24, 24 ), 0, 255, 0, 40, 10.0f );
			}

			for ( int i = 0; i < count; i++ )
			{
				CBaseEntity *pEntity = pList[i];

				if ( pEntity == this || InSameTeam( pEntity ) || !FVisible( pEntity, MASK_SOLID ) )
					continue;

				CTFPlayer *pVictim = ToTFPlayer( pEntity );
				CAI_BaseNPC *pNPCVictim = dynamic_cast<CAI_BaseNPC *>( pEntity );
				CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( pEntity );

				// Only play the stun sound one time
				if ( iTauntType == TAUNTATK_SCOUT_GRAND_SLAM && !bHomerun )
				{
					if ( pVictim || pNPCVictim )
					{
						bHomerun = true;
						EmitSound(  "TFPlayer.StunImpactRange" );
					}
				}

				if ( iTauntType == TAUNTATK_PYRO_HADOUKEN )
				{
					if ( pProp && pProp->m_takedamage == DAMAGE_YES )
						pProp->IgniteLifetime( TF_BURNING_FLAME_LIFE );
				}

				Vector vecDamagePos = WorldSpaceCenter();
				vecDamagePos += ( pEntity->WorldSpaceCenter() - vecDamagePos ) * 0.75f;

				CTakeDamageInfo info( this, this, GetActiveTFWeapon(), vecForce, vecDamagePos, flDamage, nDamageType, iDamageCustom );
				pEntity->TakeDamage( info );

				if ( pEntity && pEntity->VPhysicsGetObject() )
					pEntity->VPhysicsGetObject()->ApplyForceOffset( vecForce, vecDamagePos );
			}

			break;
		}
		case TAUNTATK_HEAVY_HIGH_NOON:
		{
			// Fire a bullet in the direction player was looking at.
			Vector vecSrc, vecShotDir, vecEnd;
			QAngle angShot = m_angTauntCamera;
			AngleVectors( angShot, &vecShotDir );
			vecSrc = Weapon_ShootPosition() - Vector( 0, 0, 6 );
			vecEnd = vecSrc + vecShotDir * 500;

			trace_t tr;
			Ray_t ray;
			ray.Init( vecSrc, vecEnd );
			UTIL_Portal_TraceRay( ray, MASK_SOLID|CONTENTS_HITBOX, this, COLLISION_GROUP_PLAYER|COLLISION_GROUP_NPC, &tr );

			if ( tf_debug_damage.GetBool() )
				NDebugOverlay::Line( vecSrc, tr.endpos, 0, 255, 0, true, 10.0f );

			FX_TFTracer( "tfc_sniper_distortion_trail", vecSrc, tr.endpos, entindex(), true );

			if ( tr.fraction < 1.0f )
			{
				CBaseEntity *pEntity = tr.m_pEnt;
				if ( pEntity && !InSameTeam( pEntity ) )
				{
					Vector vecForce, vecDamagePos;
					QAngle angForce( -45.0, angShot[YAW], 0.0 );
					AngleVectors( angForce, &vecForce );
					vecForce *= 25000.0f;

					vecDamagePos = tr.endpos;

					CTakeDamageInfo info( this, this, GetActiveTFWeapon(), vecForce, vecDamagePos, 500, DMG_BULLET, TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON );
					pEntity->TakeDamage( info );

					if ( pEntity && pEntity->VPhysicsGetObject() )
						pEntity->VPhysicsGetObject()->ApplyForceOffset( vecForce, vecDamagePos );
				}
			}

			break;
		}

		case TAUNTATK_SOLDIER_GRENADE_KILL:
		case TAUNTATK_SOLDIER_GRENADE_KILL_WORMSIGN:
		{
			Vector where = GetAbsOrigin();
			
			CPVSFilter filter(where);
			TE_TFExplosion(filter, 0.0f, where, Vector(0.0f, 0.0f, 1.0f),
				TF_WEAPON_ROCKETLAUNCHER, ENTINDEX(this));

			CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

			CTakeDamageInfo dmginfo( this, this, this, where, where, 450.0f, DMG_BLAST | DMG_USEDISTANCEMOD, TF_DMG_CUSTOM_TAUNTATK_GRENADE, &where );
			
			CTFRadiusDamageInfo radius;
			radius.info       = &dmginfo;
			radius.m_vecSrc   = where;
			radius.m_flRadius = 100.0f;
			TFGameRules()->RadiusDamage( radius );

			break;
		}
		case TAUNTATK_PYRO_ARMAGEDDON:
		{
			if ( GetActiveTFWeapon() )
			{
				CPVSFilter filter( GetAbsOrigin() );
				TE_TFExplosion( filter, 0.0, GetAbsOrigin(), Vector( 0, 0, 1.0f ), TF_WEAPON_ROCKETLAUNCHER, entindex() );

				CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 1.0 );

				CTakeDamageInfo info( this, this, GetActiveWeapon(), vec3_origin, GetAbsOrigin(), 400.0f, DMG_IGNITE, TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON );

				CTFRadiusDamageInfo radius;
				radius.info       = &info;
				radius.m_vecSrc   = GetAbsOrigin();
				radius.m_flRadius = 100.0f;
				TFGameRules()->RadiusDamage( radius );
			}
			break;
		}
		case TAUNTATK_HEAVY_EAT:
		case TAUNTATK_SCOUT_DRINK:
		{
			CTFWeaponBase *pWeapon = GetActiveTFWeapon();
			if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_LUNCHBOX ) )
			{
				if ( pWeapon->ClassMatches( "tf_weapon_lunchbox_drink" ) )
				{
					m_iTauntAttack = TAUNTATK_SCOUT_DRINK;
					m_flTauntAttackTime = gpGlobals->curtime + 0.1f;
				}
				else
				{
					CTFLunchBox *pLunch = static_cast<CTFLunchBox *>( pWeapon );
					pLunch->ApplyBiteEffects( true );

					m_iTauntAttack = TAUNTATK_HEAVY_EAT;
					m_flTauntAttackTime = gpGlobals->curtime + 1.0f;
				}
			}
			break;
			SelectLastItem();
		}
		case TAUNTATK_SNIPER_ARROW_STAB_IMPALE:
		case TAUNTATK_SNIPER_ARROW_STAB_KILL:
		case TAUNTATK_MEDIC_UBERSLICE_IMPALE:
		case TAUNTATK_MEDIC_UBERSLICE_KILL:
		case TAUNTATK_ENGINEER_ARM_IMPALE:
		case TAUNTATK_ENGINEER_ARM_KILL:
		case TAUNTATK_ENGINEER_ARM_BLEND:
		case TAUNTATK_ANTLION_IMPALE:
		case TAUNTATK_ANTLION_KILL:
		{
			// Trace a bit ahead.
			Vector vecSrc, vecEnd;
			Vector vecShotDir = m_vecTauntCamera;
			vecSrc = Weapon_ShootPosition() - Vector( 0, 0, 6 );
			vecEnd = vecSrc + vecShotDir * 128;

			trace_t tr;
			Ray_t ray;
			ray.Init( vecSrc, vecEnd );
			UTIL_Portal_TraceRay( ray, MASK_SOLID|CONTENTS_HITBOX, this, COLLISION_GROUP_PLAYER|COLLISION_GROUP_NPC, &tr );

			if ( tf_debug_damage.GetBool() )
			{
				NDebugOverlay::Line( vecSrc, vecEnd, 0, 255, 0, true, 10.0f );
			}

			if ( tr.fraction < 1.0f )
			{
				CTFPlayer *pPlayer = ToTFPlayer( tr.m_pEnt );
				CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( tr.m_pEnt );
				CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( tr.m_pEnt );

				// First hit stuns, next hit kills.
				bool bStun = ( iTauntType == TAUNTATK_SNIPER_ARROW_STAB_IMPALE || iTauntType == TAUNTATK_MEDIC_UBERSLICE_IMPALE || iTauntType == TAUNTATK_ENGINEER_ARM_IMPALE || iTauntType == TAUNTATK_ENGINEER_ARM_BLEND || iTauntType == TAUNTATK_ANTLION_IMPALE );
				Vector vecForce, vecDamagePos;

				float flDamage = bStun ? 1.0f : 500.0f;
				int nDamageType = ( iTauntType == TAUNTATK_ENGINEER_ARM_KILL ) ? DMG_BLAST : DMG_SLASH | DMG_PREVENT_PHYSICS_FORCE;
				int iCustomDamage = 0;

				switch ( iTauntType )
				{
					case TAUNTATK_SNIPER_ARROW_STAB_IMPALE:
					case TAUNTATK_SNIPER_ARROW_STAB_KILL:
						iCustomDamage = TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB;
						break;
					case TAUNTATK_MEDIC_UBERSLICE_IMPALE:
					case TAUNTATK_MEDIC_UBERSLICE_KILL:
						iCustomDamage = TF_DMG_CUSTOM_TAUNTATK_UBERSLICE;
						break;
					case TAUNTATK_ENGINEER_ARM_IMPALE:
					case TAUNTATK_ENGINEER_ARM_BLEND:
					case TAUNTATK_ENGINEER_ARM_KILL:
						iCustomDamage = TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL;
						break;
					case TAUNTATK_ANTLION_IMPALE:
					case TAUNTATK_ANTLION_KILL:
						iCustomDamage = TF_DMG_CUSTOM_NONE;
						break;
				};

				vecDamagePos = tr.endpos;

				if ( tr.m_pEnt && !InSameTeam( tr.m_pEnt ) )
				{
					if ( pPlayer )
					{
						if ( bStun )
						{
							vecForce == vec3_origin;
						}
						else
						{
							// Pull them towards us.
							Vector vecDir = WorldSpaceCenter() - pPlayer->WorldSpaceCenter();
							VectorNormalize( vecDir );
							vecForce = vecDir * 12000;
						}

						if ( bStun )
						{
							// Stun the player
							pPlayer->m_Shared.StunPlayer( 3.0f, 0.0f, 0.0f, TF_STUNFLAGS_NORMALBONK | TF_STUNFLAG_NOSOUNDOREFFECT, this );
							pPlayer->m_iTauntAttack = TAUNTATK_NONE;
						}

						CTakeDamageInfo info( this, this, GetActiveTFWeapon(), vecForce, vecDamagePos, flDamage, nDamageType, iCustomDamage );
						pPlayer->TakeDamage( info );
						// bots don't have blood
						if ( TFGameRules() && TFGameRules()->IsMvMModelsAllowed() )
						{
							g_pEffects->Sparks( vecDamagePos );
							if ( random->RandomFloat( 0, 2 ) >= 1 )
								UTIL_Smoke( vecDamagePos, random->RandomInt( 10, 15 ), 10 );
						}
						else
						{
							// If slash damage shoot some blood
							SpawnBlood( vecDamagePos, g_vecAttackDir, pPlayer->BloodColor(), info.GetDamage() );
						}
					}
					else if ( pNPC )
					{
						if ( bStun )
						{
							vecForce == vec3_origin;
						}
						else
						{
							// Pull them towards us.
							Vector vecDir = WorldSpaceCenter() - pNPC->WorldSpaceCenter();
							VectorNormalize( vecDir );
							vecForce = vecDir * 12000;
						}

						if ( bStun )
						{
							// Stun the npc
							pNPC->StunNPC( 3.0f, 0.0f, 0.0f, TF_STUNFLAGS_NORMALBONK | TF_STUNFLAG_NOSOUNDOREFFECT, this );
						}

						CTakeDamageInfo info( this, this, GetActiveTFWeapon(), vecForce, vecDamagePos, flDamage, nDamageType, iCustomDamage );
						pNPC->TakeDamage( info );
						SpawnBlood( vecDamagePos, g_vecAttackDir, pNPC->BloodColor(), info.GetDamage() );
					}
					else if ( pProp )
					{
						if ( bStun )
						{
							vecForce == vec3_origin;
						}
						else
						{
							// Pull them towards us.
							Vector vecDir = WorldSpaceCenter() - pProp->WorldSpaceCenter();
							VectorNormalize( vecDir );
							vecForce = vecDir * 12000;
						}

						CTakeDamageInfo info( this, this, GetActiveTFWeapon(), vecForce, vecDamagePos, flDamage, nDamageType, iCustomDamage );
						pProp->TakeDamage( info );

						if ( tr.m_pEnt && tr.m_pEnt->VPhysicsGetObject() )
							tr.m_pEnt->VPhysicsGetObject()->ApplyForceOffset( vecForce, vecDamagePos );
					}

					if ( iTauntType == TAUNTATK_SNIPER_ARROW_STAB_IMPALE )
					{
						m_flTauntAttackTime = gpGlobals->curtime + 1.3f;
						m_iTauntAttack = TAUNTATK_SNIPER_ARROW_STAB_KILL;
					}
					else if ( iTauntType == TAUNTATK_MEDIC_UBERSLICE_IMPALE )
					{
						m_flTauntAttackTime = gpGlobals->curtime + 0.75f;
						m_iTauntAttack = TAUNTATK_MEDIC_UBERSLICE_KILL;
					}
					else if ( iTauntType == TAUNTATK_ENGINEER_ARM_IMPALE )
					{
						// Reset the damage counter
						m_nTauntDamageCount = 0;
						m_flTauntAttackTime = gpGlobals->curtime + 0.05f;
						m_iTauntAttack = TAUNTATK_ENGINEER_ARM_BLEND;
					}
					else if ( iTauntType == TAUNTATK_ENGINEER_ARM_BLEND )
					{
						// Increment the damage counter
						m_nTauntDamageCount++;
						m_flTauntAttackTime = gpGlobals->curtime + 0.05f;
						if ( m_nTauntDamageCount == 13 )
						{
							m_iTauntAttack = TAUNTATK_ENGINEER_ARM_KILL;
						}
						else
						{
							m_iTauntAttack = TAUNTATK_ENGINEER_ARM_BLEND;
						}
					}
					else if ( iTauntType == TAUNTATK_MEDIC_UBERSLICE_KILL )
					{
						CWeaponMedigun *pMedigun = GetMedigun();

						if ( pMedigun && !pProp )
						{
							// Successful kills gain +50% ubercharge
							pMedigun->AddCharge( 0.50 );
						}
					}
					else if ( iTauntType == TAUNTATK_ANTLION_IMPALE )
					{
						m_flTauntAttackTime = gpGlobals->curtime + 0.8f;
						m_iTauntAttack = TAUNTATK_ANTLION_KILL;
					}
				}
			}

			break;
		}
		case TAUNTATK_MEDIC_HEROIC_TAUNT:
		case TAUNTATK_MEDIC_RELEASE_DOVES:
		{
			if ( iTauntType == TAUNTATK_MEDIC_HEROIC_TAUNT )
			{
				CDisablePredictionFiltering disabler;
				DispatchParticleEffect( "god_rays", GetAbsOrigin(), GetAbsAngles() );
				EmitSound( "Taunt.MedicHeroic" );
				m_flTauntAttackTime = gpGlobals->curtime + 1.3f;
				m_iTauntAttack = TAUNTATK_MEDIC_RELEASE_DOVES;
			}
			else if ( iTauntType == TAUNTATK_MEDIC_RELEASE_DOVES )
			{
			}

			break;
		}
		case TAUNTATK_HIGHFIVE_PARTICLE:
		{
			CDisablePredictionFiltering disabler;

			int iHandBone = LookupBone( "bip_hand_r" );
			if ( iHandBone != -1 )
			{
				Vector vecBoneOrigin; QAngle vecBoneAngles;
				GetBonePosition( iHandBone, vecBoneOrigin, vecBoneAngles );

				const char *pszHFEffect = ConstructTeamParticle( "highfive_%s", GetTeamNumber() );
				DispatchParticleEffect( pszHFEffect, vecBoneOrigin, vecBoneAngles );
			}

			break;
		}
		case TAUNTATK_ALLCLASS_GUITAR_RIFF:
		{
			CDisablePredictionFiltering disabler;
			DispatchParticleEffect( "bl_killtaunt", GetAbsOrigin(), GetAbsAngles() );
			EmitSound( "Taunt.GuitarRiff" );
			CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 256, 0.5, this );
			break;
		}
		case TAUNTATK_COMBINES_BASH:
		{
			// Does no damage, because damage is applied based upon whether the target can handle the interaction
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, -Vector(16,16,18), Vector(16,16,18), 0, DMG_CLUB );
			if ( pHurt )
			{
				Vector vecForward, vecUp;
				AngleVectors( m_angTauntCamera, &vecForward, NULL, &vecUp );

				CTFPlayer *pTFPlayer = ToTFPlayer( pHurt );
				if ( pTFPlayer )
					pTFPlayer->ViewPunch( QAngle(-12,-7,0) );

				pHurt->ApplyAbsVelocityImpulse( vecForward * 100 + vecUp * 50 );

				int iDmgType = DMG_CLUB;
				if ( GetActiveTFWeapon() )
				{
					GetActiveTFWeapon()->CalcIsAttackCritical();
					GetActiveTFWeapon()->CalcIsAttackMiniCritical();

					if ( GetActiveTFWeapon()->IsCurrentAttackACrit() )
					{
						iDmgType |= DMG_CRITICAL;
					}
					if ( GetActiveTFWeapon()->IsCurrentAttackAMiniCrit() )
					{
						iDmgType |= DMG_MINICRITICAL;
					}
				}

				CTakeDamageInfo info( this, this, 25, iDmgType );
				CalculateMeleeDamageForce( &info, vecForward, pHurt->GetAbsOrigin() );
				pHurt->TakeDamage( info );

				EmitSound( "NPC_Combine.WeaponBash" );
			}
			break;
		}
		case TAUNTATK_COMBINES_THROW_GRENADE:
		{
			Vector vecForward, vecRight, vecUp;
			AngleVectors( m_angTauntCamera, &vecForward, &vecRight, &vecUp );

			Vector vecVelocity = ( vecForward * 600 ) + ( vecUp * 150.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) + ( random->RandomFloat(-10.0f, 10.0f) * vecUp );

			Vector vecSpin;
			vecSpin.x = random->RandomFloat( -1000.0, 1000.0 );
			vecSpin.y = random->RandomFloat( -1000.0, 1000.0 );
			vecSpin.z = random->RandomFloat( -1000.0, 1000.0 );

			Fraggrenade_Create( Weapon_ShootPosition(), vec3_angle, vecVelocity, vecSpin, this, 3.5, true );
			//EmitSound( "NPC_Combine.GrenadeLaunch" );
			break;
		}
		case TAUNTATK_MEDIC_INHALE:
		{
			CTFWeaponBase *pWeapon = GetActiveTFWeapon();
			if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_BONESAW ) )	
			{
				// Melody heals 25 health over the duration
				m_Shared.PulseMedicRadiusHeal();
				m_flTauntAttackTime = gpGlobals->curtime + TF_MEDIC_REGEN_TIME;
				m_iTauntAttack = TAUNTATK_MEDIC_INHALE;
			}
			else
			{
				// Medigun heals 11 health over the duration
				DispatchParticleEffect( ConstructTeamParticle( "healhuff_%s", GetTeamNumber(), false, g_aTeamNamesShort ), PATTACH_POINT_FOLLOW, this, "eyes" );
				TakeHealth( 1.1f, DMG_GENERIC );
				m_flTauntAttackTime = gpGlobals->curtime + 0.38;
				m_iTauntAttack = TAUNTATK_MEDIC_INHALE;
			}
			break;
		}
		case TAUNTATK_DEMOMAN_CABER:
		{
			if ( GetActiveTFWeapon() )
			{
				CTFStickBomb *pCaber = dynamic_cast<CTFStickBomb*>( GetActiveTFWeapon() );
				if ( pCaber )
					pCaber->Smack();
			}
			break;
		}
		case TAUNTATK_WATCHANDLEARN:
		{
			CDisablePredictionFiltering disabler;
			DispatchParticleEffect( "crit_text", WorldSpaceCenter() + Vector(0,0,32), vec3_angle );
			EmitSound( "TFPlayer.CritHit" );
			EmitSound( "TFPlayer.Drown" );
			CommitSuicide( false, true );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ClearTauntAttack( void )
{
	if ( m_iTauntAttack == TAUNTATK_HEAVY_EAT )
	{
		CTFWeaponBase *pWeapon = GetActiveTFWeapon();
		if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_LUNCHBOX ) )
		{
			SpeakConceptIfAllowed( MP_CONCEPT_ATE_FOOD );
		}
	}
	else if ( m_iTauntAttack == TAUNTATK_SCOUT_DRINK )
	{
		CTFWeaponBase *pWeapon = GetActiveTFWeapon();
		if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_LUNCHBOX ) )
		{
			int iType = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, iType, set_weapon_mode );
			if ( iType == 0 ) // it's a bonk
			{
				m_Shared.AddCond( TF_COND_PHASE, 8.0f );
				SpeakConceptIfAllowed( MP_CONCEPT_DODGING, "started_dodging:1" );
				m_angTauntCamera = EyeAngles();
			}
			else if ( iType == 2 )
			{
				m_Shared.AddCond( TF_COND_ENERGY_BUFF, 8.0f );
			}
		}
	}

	m_flTauntAttackTime = 0.0f;
	m_iTauntAttack = TAUNTATK_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::EndLongTaunt( void )
{
	PlayTauntOutroScene();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StopTaunt( bool bForce )
{
	StopTauntSoundLoop();

	if( m_hTauntScene.Get() )
	{
		StopScriptedScene( this, m_hTauntScene );
		m_hTauntScene = NULL;
	}

	m_Shared.m_flTauntRemoveTime = 0.0f;

	if ( m_iTauntItemDefIndex != 0 )
		m_iTauntItemDefIndex = 0;

	CBaseEntity *pProp = gEntList.FindEntityByClassname( NULL, "tf_taunt_prop" );
	if ( pProp )
	{
		if ( pProp->GetOwnerEntity() && ( pProp->GetOwnerEntity() == this ) )
			UTIL_Remove( pProp );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::StopTauntSoundLoop( void )
{
	if ( m_sndTauntLoop )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy( m_sndTauntLoop );
		m_sndTauntLoop = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayTauntSceneFromItem( const CEconItemView* pItem )
{
	PlayTauntSceneFromItemID( pItem->GetItemDefIndex() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayTauntSceneFromItemID( int iItemID )
{
/*congaTaunt
pAttrDef_TauntSuccessSoundLoopOffset
pAttrDef_TauntSuccessSoundOffset*/

	if ( !IsAllowedToTaunt() )
		return;

	if ( iItemID == 463 )
	{
		Taunt( TAUNTATK_NONE, MP_CONCEPT_TAUNT_LAUGH );
		return;
	}

	CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iItemID );
	if ( !pItemDef )
		return;

	CAttribute_String strTauntForceWeaponSlot;
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntForceWeaponSlot( "taunt force weapon slot" );
	if ( pAttrDef_TauntForceWeaponSlot )
	{
		CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_TauntForceWeaponSlot, &strTauntForceWeaponSlot );
		pItemDef->IterateAttributes( &func );
	}

	if ( strTauntForceWeaponSlot && *strTauntForceWeaponSlot )
		Weapon_Switch( Weapon_GetSlot( UTIL_StringFieldToInt(strTauntForceWeaponSlot, g_LoadoutSlots, LOADOUT_POSITION_COUNT ) ) );

	if ( pItemDef->is_partner_taunt )
	{
		Vector vecForward, vecRight;
		QAngle angAngs;

		angAngs = GetAbsAngles();
		AngleVectors( angAngs, &vecForward, &vecRight, NULL );

		Vector vecMe = ( GetAbsOrigin() + vecForward * pItemDef->taunt_separation_forward_distance + vecRight * pItemDef->taunt_separation_right_distance );

		if ( !IsSpaceToSpawnHere( vecMe ) )
		{
			CSingleUserRecipientFilter filter( this );
			EmitSound_t params;
			params.m_flSoundTime = 0;
			params.m_pSoundName = "HL2Player.UseDeny";
			EmitSound( filter, entindex(), params );
			TFGameRules()->SendHudNotification( filter, "#TF_PartnerTaunt_Blocked", "ico_notify_flag_moving", GetTeamNumber() );
			return;
		}
	}

	// Allow voice commands, etc to be interrupted.
	CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
	Assert( pExpresser );
	pExpresser->AllowMultipleScenes();

	m_bInitTaunt = false;

	if ( pItemDef->custom_taunt_scene_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
	{
		InstancedScriptedScene( this, pItemDef->custom_taunt_scene_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );

		unsigned int iHoldTaunt = 0;
		static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntPressAndHold( "taunt is press and hold" );
		if ( pAttrDef_TauntPressAndHold )
		{
			CAttributeIterator_GetSpecificAttribute<unsigned int> func( pAttrDef_TauntPressAndHold, &iHoldTaunt );
			pItemDef->IterateAttributes( &func );
		}

		if ( pItemDef->custom_taunt_prop_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
		{
			CTFTauntProp *pProp = dynamic_cast<CTFTauntProp*>( CreateEntityByName( "tf_taunt_prop" ) );
			if ( pProp )
			{
				pProp->SetAbsOrigin( GetAbsOrigin() );
				pProp->SetAbsAngles( GetAbsAngles() );
				pProp->SetModel( pItemDef->custom_taunt_prop_per_class[GetDesiredPlayerClassIndex()] );
				pProp->SetOwnerEntity( this );
				DispatchSpawn( pProp );

				if ( pItemDef->custom_taunt_prop_scene_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
				{
					SetParent( this );
					InstancedScriptedScene( pProp, pItemDef->custom_taunt_prop_scene_per_class[GetDesiredPlayerClassIndex()], NULL, 0.0f, false, NULL, true );
				}
				else
				{
					pProp->FollowEntity( this );
				}

				if ( iHoldTaunt == 0 )
				{
					pProp->SetThink( &CBaseEntity::SUB_Remove );
					pProp->SetNextThink( gpGlobals->curtime + GetSceneDuration( pItemDef->custom_taunt_scene_per_class[GetDesiredPlayerClassIndex()] ) );
				}
			}
		}

		// Clear disguising state.
		if ( m_Shared.InCond( TF_COND_DISGUISING ) )
			m_Shared.RemoveCond( TF_COND_DISGUISING );

		// Set player state as taunting.
		m_Shared.AddCond( TF_COND_TAUNTING );

		if ( iHoldTaunt > 0 )
			m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + 999;
		else
			m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_taunt_scene_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;

		if ( FlashlightIsOn() )
			FlashlightTurnOff();

		m_angTauntCamera = EyeAngles();
		m_vecTauntCamera = BodyDirection2D();

		// Slam velocity to zero.
		if ( !tf_allow_sliding_taunt.GetBool() )
			SetAbsVelocity( vec3_origin );

		CAttribute_String strCosmeticTauntSound;
		CALL_ATTRIB_HOOK_STRING( strCosmeticTauntSound, cosmetic_taunt_sound );
		if ( strCosmeticTauntSound && *strCosmeticTauntSound )
			EmitSound( strCosmeticTauntSound );

		CAttribute_String strTauntSuccessSound;
		static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntSuccessSound( "taunt success sound" );
		if ( pAttrDef_TauntSuccessSound )
		{
			CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_TauntSuccessSound, &strTauntSuccessSound );
			pItemDef->IterateAttributes( &func );
		}

		if ( strTauntSuccessSound )
			EmitSound( strTauntSuccessSound );

		CAttribute_String strTauntSuccessSoundLoop;
		static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntSuccessSoundLoop( "taunt success sound loop" );
		if ( pAttrDef_TauntSuccessSoundLoop )
		{
			CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_TauntSuccessSoundLoop, &strTauntSuccessSoundLoop );
			pItemDef->IterateAttributes( &func );
		}

		if ( strTauntSuccessSoundLoop )
		{
			CPASAttenuationFilter filter( this );
			if ( !m_sndTauntLoop )
			{
				m_sndTauntLoop = ( CSoundEnvelopeController::GetController() ).SoundCreate( filter, entindex(), CHAN_STATIC, strTauntSuccessSoundLoop , ATTN_NORM );
				( CSoundEnvelopeController::GetController() ).Play( m_sndTauntLoop, 1.0f, 100 );
			}
		}

		float flTauntAttackTime = 0.0f;
		static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntAttackTime( "taunt attack time" );
		if ( pAttrDef_TauntAttackTime )
		{
			CAttributeIterator_GetSpecificAttribute<float> func( pAttrDef_TauntAttackTime, &flTauntAttackTime );
			pItemDef->IterateAttributes( &func );
		}

		m_flTauntAttackTime = gpGlobals->curtime + flTauntAttackTime;

		CAttribute_String strTauntAttackName;
		static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntAttackName( "taunt attack name" );
		if ( pAttrDef_TauntAttackName )
		{
			CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_TauntAttackName, &strTauntAttackName );
			pItemDef->IterateAttributes( &func );
		}

		int iTauntAttack = TAUNTATK_NONE;
		if ( strTauntAttackName )
			iTauntAttack = GetTauntAttackByName( strTauntAttackName );

		m_iTauntAttack = iTauntAttack;

		pExpresser->DisallowMultipleScenes();

		CSoundEnt::InsertSound( SOUND_PLAYER, GetAbsOrigin(), 512, 0.5, this );

		m_iTauntItemDefIndex = iItemID;
		ParseSharedTauntDataFromEconItemView( NULL );

		if ( GetActiveTFWeapon() )
			GetActiveTFWeapon()->SetWeaponVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayTauntOutroScene( void )
{
	if ( m_iTauntItemDefIndex == 0 )
		return;

	CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_iTauntItemDefIndex );
	if ( !pItemDef )
		return;

	// Allow voice commands, etc to be interrupted.
	CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
	Assert( pExpresser );
	pExpresser->AllowMultipleScenes();

	if ( pItemDef->custom_taunt_outro_scene_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
	{
		if( m_hTauntScene.Get() )
		{
			StopScriptedScene( this, m_hTauntScene );
			m_hTauntScene = NULL;
		}

		InstancedScriptedScene( this, pItemDef->custom_taunt_outro_scene_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );

		if ( pItemDef->custom_taunt_prop_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
		{
			CTFTauntProp *pProp = dynamic_cast<CTFTauntProp*>( gEntList.FindEntityByClassname( NULL, "tf_taunt_prop" ) );
			if ( pProp )
			{
				if ( pProp->GetOwnerEntity() && ( pProp->GetOwnerEntity() == this ) )
				{
					if ( pItemDef->custom_taunt_prop_outro_scene_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
						InstancedScriptedScene( pProp, pItemDef->custom_taunt_prop_outro_scene_per_class[GetDesiredPlayerClassIndex()], NULL, 0.0f, false, NULL, true );

					pProp->SetThink( &CBaseEntity::SUB_Remove );
					pProp->SetNextThink( gpGlobals->curtime + GetSceneDuration( pItemDef->custom_taunt_outro_scene_per_class[GetDesiredPlayerClassIndex()] ) );
				}
			}
		}

		m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_taunt_outro_scene_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;

		pExpresser->DisallowMultipleScenes();
	}
	else
	{
		m_Shared.RemoveCond( TF_COND_TAUNTING );
	}

	m_iTauntItemDefIndex = 0; // set to 0 to prevent funni spam
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayTauntRemapInputScene( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::AcceptTauntWithPartner( CTFPlayer *pPartner )
{
	if ( !pPartner )
		return;

	DevMsg( "%s Accepting Taunt with %s\n", GetPlayerName(), pPartner->GetPlayerName() );

	if ( pPartner->m_iTauntItemDefIndex == 0 )
		return;

	CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( pPartner->m_iTauntItemDefIndex );
	if ( !pItemDef )
		return;

	Vector vecForward, vecRight;
	QAngle angAngs;

	angAngs = pPartner->GetAbsAngles();
	AngleVectors( angAngs, &vecForward, &vecRight, NULL );

	Vector vecMe = ( pPartner->GetAbsOrigin() + vecForward * pItemDef->taunt_separation_forward_distance + vecRight * pItemDef->taunt_separation_right_distance );

	// Allow voice commands, etc to be interrupted.
	CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
	Assert( pExpresser );
	pExpresser->AllowMultipleScenes();

	float flTauntAttackTime = 0.0f;
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntAttackTime( "taunt attack time" );
	if ( pAttrDef_TauntAttackTime )
	{
		CAttributeIterator_GetSpecificAttribute<float> func( pAttrDef_TauntAttackTime, &flTauntAttackTime );
		pItemDef->IterateAttributes( &func );
	}

	int iTauntAttack = TAUNTATK_NONE;
	CAttribute_String strTauntAttackName;
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntAttackName( "taunt attack name" );
	if ( pAttrDef_TauntAttackName )
	{
		CAttributeIterator_GetSpecificAttribute<CAttribute_String> func( pAttrDef_TauntAttackName, &strTauntAttackName );
		pItemDef->IterateAttributes( &func );
	}

	if ( strTauntAttackName )
		iTauntAttack = GetTauntAttackByName( strTauntAttackName );

	PerTeamVisuals_t *pVisuals = pItemDef->GetVisuals( GetTeamNumber() );
	if ( pVisuals )
	{
		if ( pVisuals->custom_sound0[0] != '\0' )
			EmitSound( pVisuals->custom_sound0 );
	}

	if ( ( pItemDef->custom_partner_taunt_initiator_per_class[GetDesiredPlayerClassIndex()][0] != '\0' ) && ( pItemDef->custom_partner_taunt_receiver_per_class[GetDesiredPlayerClassIndex()][0] != '\0' ) )
	{
		if( m_hTauntScene.Get() )
		{
			StopScriptedScene( this, m_hTauntScene );
			m_hTauntScene = NULL;
		}

		InstancedScriptedScene( this, pItemDef->custom_partner_taunt_initiator_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );
		InstancedScriptedScene( pPartner, pItemDef->custom_partner_taunt_receiver_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );

		m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_partner_taunt_initiator_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;
		pPartner->m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_partner_taunt_receiver_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;

		SetAbsOrigin( vecMe );
		SetAbsAngles( -angAngs );

		m_flTauntAttackTime = gpGlobals->curtime + flTauntAttackTime;
		m_iTauntAttack = iTauntAttack;

		m_iTauntItemDefIndex = pPartner->m_iTauntItemDefIndex;

		pExpresser->DisallowMultipleScenes();

		DevMsg( "%s custom_partner_taunt_initiator_per_class with %s\n", GetPlayerName(), pPartner->GetPlayerName() );
		return;
	}

	if ( pItemDef->custom_partner_taunt_per_class[GetDesiredPlayerClassIndex()][0] != '\0' )
	{
		if( m_hTauntScene.Get() )
		{
			StopScriptedScene( this, m_hTauntScene );
			m_hTauntScene = NULL;
		}

		InstancedScriptedScene( this, pItemDef->custom_partner_taunt_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );
		InstancedScriptedScene( pPartner, pItemDef->custom_partner_taunt_per_class[GetDesiredPlayerClassIndex()], &m_hTauntScene, 0.0f, false, NULL, true );

		m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_partner_taunt_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;
		pPartner->m_Shared.m_flTauntRemoveTime = gpGlobals->curtime + GetSceneDuration( pItemDef->custom_partner_taunt_per_class[GetDesiredPlayerClassIndex()] ) + 0.2f;

		Vector vecForward, vecRight;
		QAngle angAngs;

		angAngs = GetAbsAngles();
		AngleVectors( angAngs, &vecForward, &vecRight, NULL );

		SetAbsOrigin( vecMe );
		SetAbsAngles( -angAngs );

		m_flTauntAttackTime = gpGlobals->curtime + flTauntAttackTime;
		m_iTauntAttack = iTauntAttack;

		m_iTauntItemDefIndex = pPartner->m_iTauntItemDefIndex;

		pExpresser->DisallowMultipleScenes();

		DevMsg( "%s custom_partner_taunt_per_class with %s\n", GetPlayerName(), pPartner->GetPlayerName() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::HandleTauntCommand( int iSlot )
{
	bool bFound = false;

	Vector forward, up;
	EyeVectors( &forward, NULL, &up );
	trace_t tr;
	Vector vecEyes = EyePosition();

	Ray_t ray;
	ray.Init( vecEyes, vecEyes + forward * 256 );
	UTIL_Portal_TraceRay( ray, MASK_SOLID|CONTENTS_HITBOX, this, COLLISION_GROUP_PLAYER, &tr );

	CTFPlayer *pMimicPlayer = ToTFPlayer( tr.m_pEnt );
	if ( ( pMimicPlayer && pMimicPlayer->IsTaunting() ) && !IsTaunting() )
	{
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( pMimicPlayer->m_iTauntItemDefIndex );
		if ( pItemDef )
		{
			unsigned int iTauntMimic = 0;
			static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntMimic( "taunt mimic" );
			if ( pAttrDef_TauntMimic )
			{
				CAttributeIterator_GetSpecificAttribute<unsigned int> func( pAttrDef_TauntMimic, &iTauntMimic );
				pItemDef->IterateAttributes( &func );
			}

			if ( iTauntMimic > 0 )
			{
				PlayTauntSceneFromItemID( pMimicPlayer->m_iTauntItemDefIndex );
				bFound = true;
			}

			if ( pItemDef->is_partner_taunt )
			{
				AcceptTauntWithPartner( pMimicPlayer );
				bFound = true;
			}
		}
	}

	if ( !bFound )
	{
		if ( ( m_iTauntItemDefIndex != 0 ) && IsTaunting() )
		{
			CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_iTauntItemDefIndex );
			if ( pItemDef )
			{
				unsigned int iHoldTaunt = 0;
				static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntPressAndHold( "taunt is press and hold" );
				if ( pAttrDef_TauntPressAndHold )
				{
					CAttributeIterator_GetSpecificAttribute<unsigned int> func( pAttrDef_TauntPressAndHold, &iHoldTaunt );
					pItemDef->IterateAttributes( &func );
				}

				if ( iHoldTaunt > 0 )
					EndLongTaunt();
			}
		}
		else
		{
			Taunt();
		}
	}

	m_afButtonPressed &= ~IN_TAUNT;
}

//-----------------------------------------------------------------------------
// Purpose: Play a one-shot scene
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CTFPlayer::PlayScene( const char *pszScene, float flDelay, AI_Response *response, IRecipientFilter *filter )
{
	// This is a lame way to detect a taunt!
	if ( m_bInitTaunt )
	{
		m_bInitTaunt = false;
		return InstancedScriptedScene( this, pszScene, &m_hTauntScene, flDelay, false, response, true, filter );
	}
	else
	{
		return InstancedScriptedScene( this, pszScene, NULL, flDelay, false, response, true, filter );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ModifyOrAppendCriteria( AI_CriteriaSet& criteriaSet )
{
	BaseClass::ModifyOrAppendCriteria( criteriaSet );

	CTFWeaponBase *pActiveWeapon = m_Shared.GetActiveTFWeapon();

	// Handle rage taunting
	if ( pActiveWeapon )
	{
		int nGeneratesRage = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nGeneratesRage, generates_rage_on_dmg );

		if ( nGeneratesRage && pActiveWeapon->GetWeaponID() == TF_WEAPON_MINIGUN )
		{
			if ( m_Shared.GetRageProgress() < 100.0f )
			{
				if ( m_Shared.IsRageActive() )
					return;
			}
			else
			{
				if ( !m_Shared.IsRageActive() )
					m_Shared.SetRageActive( true );

				EmitSound( "Heavy.Battlecry03" );
			}
		}
	}
//	int iClass = m_PlayerClass.GetClassIndex();

	for (int i = 0; i < GetNumWearables(); i++)
	{
		CTFWearable *pWearable = assert_cast<CTFWearable *>(GetWearable(i));
		if (pWearable == nullptr)
			continue;

		// Always remove extra wearables when initializing weapons
		if (pWearable->IsExtraWearable())
		{
			continue;
		}

		CEconItemDefinition *pItemDeff = pWearable->GetItem()->GetStaticData();
		if (pItemDeff)
		{
		//	DevMsg("ItemDef found! ");
			//int iSlot = pItemDef->GetLoadoutSlot(iClass);
			//CEconItemView *pLoadoutItem = GetLoadoutItem(iClass, iSlot);

				if (pItemDeff->response_criteria)
				{
					
					//const char *cstr_response = strResponseCriteria.ToCStr();
					//DevMsg(cstr_response);
					if (pItemDeff->response_criteria_value && strcmp(pItemDeff->response_criteria_value, "/0") != 0)
					{
						criteriaSet.AppendCriteria(pItemDeff->response_criteria, pItemDeff->response_criteria_value);
					}
					else if (pItemDeff->response_criteria_value && strcmp(pItemDeff->response_criteria_value, "/0") == 0)
					{
						DevWarning("Response_criteria_value is not being properly read somehow!");

					}
					else if (!pItemDeff->response_criteria_value)
					{
						criteriaSet.AppendCriteria(pItemDeff->response_criteria, "1");
					}
				}
				else
				{
				//	DevMsg("ResponseCriteria is null. ");
				}
		}
		else{
		//	DevMsg("Wow. Epic Fail....");

		}
	}
	// If we have 'disguiseclass' criteria, pretend that we are actually our
	// disguise class. That way we just look up the scene we would play as if 
	// we were that class.
	int disguiseIndex = criteriaSet.FindCriterionIndex( "disguiseclass" );

	if ( disguiseIndex != -1 )
	{
		criteriaSet.AppendCriteria( "playerclass", criteriaSet.GetValue(disguiseIndex) );
	}
	else
	{
		if ( GetPlayerClass() )
		{
			criteriaSet.AppendCriteria( "playerclass", g_aPlayerClassNames_NonLocalized[ GetPlayerClass()->GetClassIndex() ] );
			const char *plmodel = m_iszCustomModel.Get();
			if (strcmp(plmodel, "/0") != 0)
			{
				criteriaSet.AppendCriteria("playermodel", plmodel, 0);
				DevMsg(plmodel);
			}
		}
	}

	criteriaSet.AppendCriteria( "recentkills", UTIL_VarArgs("%d", m_Shared.GetNumKillsInTime(30.0)) );

	int iTotalKills = 0;
	PlayerStats_t *pStats = CTF_GameStats.FindPlayerStats( this );
	if ( pStats )
	{
		iTotalKills = pStats->statsCurrentLife.m_iStat[TFSTAT_KILLS] + pStats->statsCurrentLife.m_iStat[TFSTAT_KILLASSISTS]+ 
			pStats->statsCurrentLife.m_iStat[TFSTAT_BUILDINGSDESTROYED];
	}

	criteriaSet.AppendCriteria( "killsthislife", UTIL_VarArgs( "%d", iTotalKills ) );
	criteriaSet.AppendCriteria( "cloaked", m_Shared.IsStealthed() ? "1" : "0" );
	criteriaSet.AppendCriteria( "disguised", m_Shared.InCond( TF_COND_DISGUISED ) ? "1" : "0" );
	criteriaSet.AppendCriteria( "invulnerable", m_Shared.IsInvulnerable() ? "1" : "0" );
	criteriaSet.AppendCriteria( "beinghealed", m_Shared.InCond( TF_COND_HEALTH_BUFF ) ? "1" : "0" );
	criteriaSet.AppendCriteria( "waitingforplayers", (TFGameRules()->IsInWaitingForPlayers() || TFGameRules()->IsInPreMatch()) ? "1" : "0" );
	criteriaSet.AppendCriteria( "teamrole", GetTFTeam()->GetRole() ? "defense" : "offense" );
	criteriaSet.AppendCriteria( "stunned", m_Shared.InCond( TF_COND_STUNNED ) ? "1" : "0" );
	criteriaSet.AppendCriteria( "dodging", m_Shared.InCond( TF_COND_PHASE ) ? "1" : "0" );
	criteriaSet.AppendCriteria( "IsInHell", m_Shared.InCond( TF_COND_HALLOWEEN_IN_HELL ) ? "1" : "0" );
	criteriaSet.AppendCriteria( "DoubleJumping", ( m_Shared.GetAirDash() > 0 ) ? "1" : "0" );

	if ( GetTFTeam() )
	{
		int iTeamRole = GetTFTeam()->GetRole();

		if ( iTeamRole == 1 )
			criteriaSet.AppendCriteria( "teamrole", "defense" );
		else
			criteriaSet.AppendCriteria( "teamrole", "offense" );
	}

	// Current weapon role
	if ( pActiveWeapon )
	{
		int iWeaponRole = pActiveWeapon->GetTFWpnData().m_iWeaponType;
		if ( pActiveWeapon->HasItemDefinition() )
		{
			int iSchemaRole = pActiveWeapon->GetItem()->GetAnimationSlot();
			if ( iSchemaRole >= 0 )
			{
				iWeaponRole = iSchemaRole;
			}
		}

		switch( iWeaponRole )
		{
		case TF_WPN_TYPE_PRIMARY:
		case TF_WPN_TYPE_PRIMARY2:
		default:
			criteriaSet.AppendCriteria( "weaponmode", "primary" );
			break;
		case TF_WPN_TYPE_SECONDARY:
		case TF_WPN_TYPE_SECONDARY2:
			criteriaSet.AppendCriteria( "weaponmode", "secondary" );
			break;
		case TF_WPN_TYPE_MELEE:
		case TF_WPN_TYPE_MELEE_ALLCLASS:
			criteriaSet.AppendCriteria( "weaponmode", "melee" );
			break;
		case TF_WPN_TYPE_BUILDING:
			criteriaSet.AppendCriteria( "weaponmode", "building" );
			break;
		case TF_WPN_TYPE_PDA:
			criteriaSet.AppendCriteria( "weaponmode", "pda" );
			break;
		case TF_WPN_TYPE_ITEM1:
			criteriaSet.AppendCriteria( "weaponmode", "item1" );
			break;
		case TF_WPN_TYPE_ITEM2:
			criteriaSet.AppendCriteria( "weaponmode", "item2" );
			break;
		case TF_WPN_TYPE_ITEM3:
			criteriaSet.AppendCriteria( "weaponmode", "item3" );
			break;
		case TF_WPN_TYPE_ITEM4:
			criteriaSet.AppendCriteria( "weaponmode", "item4" );
			break;
		}

		if ( WeaponID_IsSniperRifle( pActiveWeapon->GetWeaponID() ) )
		{
			CTFSniperRifle *pRifle = dynamic_cast<CTFSniperRifle*>(pActiveWeapon);
			if ( pRifle && pRifle->IsZoomed() )
			{
				criteriaSet.AppendCriteria( "sniperzoomed", "1" );
			}
		}
		else if ( pActiveWeapon->GetWeaponID() == TF_WEAPON_MINIGUN )
		{
			CTFMinigun *pMinigun = dynamic_cast<CTFMinigun*>(pActiveWeapon);
			if ( pMinigun )
			{
				criteriaSet.AppendCriteria( "minigunfiretime", UTIL_VarArgs("%.1f", pMinigun->GetFiringTime() ) );
			}
		}

		CEconItemDefinition *pItemDef = pActiveWeapon->GetItem()->GetStaticData();
		if ( pItemDef )
		{
			criteriaSet.AppendCriteria( "item_name", pItemDef->name );
			criteriaSet.AppendCriteria( "item_type_name", pItemDef->item_type_name );
		}
	}

	// Player under crosshair
	trace_t tr;
	Vector forward;
	EyeVectors( &forward );
	Ray_t ray;
	ray.Init( EyePosition(), EyePosition() + (forward * MAX_TRACE_LENGTH) );
	UTIL_Portal_TraceRay( ray, MASK_BLOCKLOS_AND_NPCS, this, COLLISION_GROUP_NONE, &tr );
	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if ( pEntity ) 
		{
			if ( pEntity->IsPlayer() )
			{
				CTFPlayer *pTFPlayer = ToTFPlayer( pEntity );
				if ( pTFPlayer )
				{
					int iClass = pTFPlayer->GetPlayerClass()->GetClassIndex();
					if ( iClass == TF_CLASS_COMBINE )
						iClass = TF_CLASS_SOLDIER;

					if ( !InSameTeam( pTFPlayer ) )
					{
						// Prevent spotting stealthed enemies who haven't been exposed recently
						if ( pTFPlayer->m_Shared.IsStealthed() )
						{
							if ( pTFPlayer->m_Shared.GetLastStealthExposedTime() < (gpGlobals->curtime - 3.0) )
							{
								iClass = TF_CLASS_UNDEFINED;
							}
							else
							{
								iClass = TF_CLASS_SPY;
							}
						}
						else if ( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) )
						{
							iClass = pTFPlayer->m_Shared.GetDisguiseClass();
						}
					}

					bool bSameTeam = InSameTeam( pTFPlayer );
					criteriaSet.AppendCriteria( "crosshair_enemy", bSameTeam ? "No" : "Yes" );

					if ( iClass > TF_CLASS_UNDEFINED && iClass <= TF_CLASS_COUNT )
						criteriaSet.AppendCriteria( "crosshair_on", g_aPlayerClassNames_NonLocalized[iClass] );
				}
			}
			else if ( pEntity->IsNPC() )
			{
				CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( pEntity );
				if ( pNPC )
				{
					int iClass = TF_CLASS_UNDEFINED;

					if ( pNPC->ClassMatches( "npc_combine_s" ) || pNPC->ClassMatches( "npc_metropolice" ) || pNPC->ClassMatches( "npc_antlion" ) || pNPC->ClassMatches( "monster_human_grunt" ) || pNPC->ClassMatches( "monster_alien_grunt" ) )
						iClass = TF_CLASS_SOLDIER;
					else if ( pNPC->ClassMatches( "npc_cremator" ) )
						iClass = TF_CLASS_PYRO;
					else if ( pNPC->ClassMatches( "npc_sniper" ) )
						iClass = TF_CLASS_SNIPER;
					else if ( pNPC->ClassMatches( "monster_human_assassin" ) || pNPC->ClassMatches( "monster_male_assassin" ) )
						iClass = TF_CLASS_SPY;

					bool bSameTeam = InSameTeam( pNPC );
					criteriaSet.AppendCriteria( "crosshair_enemy", bSameTeam ? "No" : "Yes" );

					if ( iClass > TF_CLASS_UNDEFINED && iClass <= TF_CLASS_COUNT )
						criteriaSet.AppendCriteria( "crosshair_on", g_aPlayerClassNames_NonLocalized[iClass] );
				}
			}
			else if ( pEntity->GetServerVehicle() != NULL )
			{
				IServerVehicle *pVehicle = pEntity->GetServerVehicle();
				if ( pVehicle && pVehicle->GetVehicleEnt() )
				{
					CTFPlayer *pTFPlayer = ToTFPlayer( pVehicle->GetVehicleEnt()->GetOwnerEntity() );
					if ( pTFPlayer )
					{
						int iClass = pTFPlayer->GetPlayerClass()->GetClassIndex();
						if ( iClass == TF_CLASS_COMBINE )
							iClass = TF_CLASS_SOLDIER;

						if ( !InSameTeam( pTFPlayer ) )
						{
							// Prevent spotting stealthed enemies who haven't been exposed recently
							if ( pTFPlayer->m_Shared.IsStealthed() )
							{
								if ( pTFPlayer->m_Shared.GetLastStealthExposedTime() < (gpGlobals->curtime - 3.0) )
								{
									iClass = TF_CLASS_UNDEFINED;
								}
								else
								{
									iClass = TF_CLASS_SPY;
								}
							}
							else if ( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) )
							{
								iClass = pTFPlayer->m_Shared.GetDisguiseClass();
							}
						}

						bool bSameTeam = InSameTeam( pTFPlayer );
						criteriaSet.AppendCriteria( "crosshair_enemy", bSameTeam ? "No" : "Yes" );

						if ( iClass > TF_CLASS_UNDEFINED && iClass <= TF_CLASS_COUNT )
							criteriaSet.AppendCriteria( "crosshair_on", g_aPlayerClassNames_NonLocalized[iClass] );
					}
				}
			}
		}
	}

	// Previous round win
	bool bLoser = ( TFGameRules()->GetPreviousRoundWinners() != TEAM_UNASSIGNED && TFGameRules()->GetPreviousRoundWinners() != GetTeamNumber() );
	criteriaSet.AppendCriteria( "LostRound", UTIL_VarArgs("%d", bLoser) );

	// Control points
	touchlink_t *root = ( touchlink_t * )GetDataObject( TOUCHLINK );
	if ( root )
	{
		for ( touchlink_t *link = root->nextLink; link != root; link = link->nextLink )
		{
			CBaseEntity *pTouch = link->entityTouched;
			if ( pTouch && pTouch->IsSolidFlagSet( FSOLID_TRIGGER ) && pTouch->IsBSPModel() )
			{
				CTriggerAreaCapture *pAreaTrigger = dynamic_cast<CTriggerAreaCapture*>(pTouch);
				if ( pAreaTrigger )
				{
					CTeamControlPoint *pCP = pAreaTrigger->GetControlPoint();
					if ( pCP )
					{
						if ( pCP->GetOwner() == GetTeamNumber() )
						{
							criteriaSet.AppendCriteria( "OnFriendlyControlPoint", "1" );
						}
						else 
						{
							if ( TeamplayGameRules()->TeamMayCapturePoint( GetTeamNumber(), pCP->GetPointIndex() ) && 
								 TeamplayGameRules()->PlayerMayCapturePoint( this, pCP->GetPointIndex() ) )
							{
								criteriaSet.AppendCriteria( "OnCappableControlPoint", "1" );
							}
						}
					}
				}
			}
		}
	}

	int nSpecialTaunt = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nSpecialTaunt, special_taunt );

	if ( TFGameRules() )
	{
		criteriaSet.AppendCriteria( "OnWinningTeam", ( GetTeamNumber() == TFGameRules()->GetWinningTeam() ) ? "1" : "0" );
		criteriaSet.AppendCriteria( "IsCompWinner", ( GetTeamNumber() == TFGameRules()->GetWinningTeam() ) ? "1" : "0" );
		criteriaSet.AppendCriteria( "IsComp6v6", ( TFGameRules()->IsVersus() ) ? "1" : "0" );

		int iGameRoundState = TFGameRules()->State_Get();
		criteriaSet.AppendCriteria( "GameRound", UTIL_VarArgs( "%d", iGameRoundState ) );

		bool bIsRedTeam = GetTeamNumber() == TF_TEAM_RED;
		criteriaSet.AppendCriteria( "OnRedTeam", UTIL_VarArgs( "%d", bIsRedTeam ) );
		criteriaSet.AppendCriteria( "IsMvMDefender", UTIL_VarArgs( "%d", bIsRedTeam ) );


		if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) && nSpecialTaunt == 0 )
		{
			if ( pActiveWeapon )
			{
				int nGeneratesRage = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nGeneratesRage, burn_dmg_earns_rage );
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nGeneratesRage, generate_rage_on_dmg );

				if ( ( pActiveWeapon->GetWeaponID() != TF_WEAPON_LUNCHBOX ) && ( nGeneratesRage == 0 || m_Shared.GetRageProgress() < 100.0f ) )
				{
					const float flRand = rand() / VALVE_RAND_MAX;
					if ( flRand < 0.4f )
					{
						criteriaSet.AppendCriteria( "IsHalloweenTaunt", "1" );
						CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 256, 0.5, this );
					}
				}
			}
		}

		if ( TFGameRules()->IsHolidayActive( kHoliday_AprilFools ) && nSpecialTaunt == 0 )
		{
			if ( pActiveWeapon )
			{
				int nGeneratesRage = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nGeneratesRage, burn_dmg_earns_rage );
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pActiveWeapon, nGeneratesRage, generate_rage_on_dmg );

				if ( ( pActiveWeapon->GetWeaponID() != TF_WEAPON_LUNCHBOX ) && ( nGeneratesRage == 0 || m_Shared.GetRageProgress() < 100.0f ) )
				{
					const float flRand = rand() / VALVE_RAND_MAX;
					if ( flRand < 0.4f )
						criteriaSet.AppendCriteria( "IsAprilFoolsTaunt", "1" );
				}
			}
		}
	}

	if ( m_Shared.InCond( TF_COND_HALLOWEEN_THRILLER ) )
	{
		criteriaSet.AppendCriteria( "IsHalloweenTaunt", "1" );
		CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 256, 0.5, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return m_iIgnoreGlobalChat != CHAT_IGNORE_ALL;

	// check if we're ignoring all chat
	if ( m_iIgnoreGlobalChat == CHAT_IGNORE_ALL )
		return false;

	// check if we're ignoring all but teammates
	if ( m_iIgnoreGlobalChat == CHAT_IGNORE_TEAM && g_pGameRules->PlayerRelationship( this, pPlayer ) != GR_TEAMMATE )
		return false;

	if ( pPlayer->m_lifeState != LIFE_ALIVE && m_lifeState == LIFE_ALIVE )
	{
		// Everyone can chat like normal when the round/game ends
		if ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN || TFGameRules()->State_Get() == GR_STATE_GAME_OVER )
			return true;

		// Everyone can chat with alltalk enabled.
		if ( sv_alltalk.GetBool() )
			return true;

		// Can hear dead teammates with tf_gravetalk enabled.
		if ( tf_gravetalk.GetBool() )
			return InSameTeam( pPlayer );

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IResponseSystem *CTFPlayer::GetResponseSystem()
{
	int iClass = GetPlayerClass()->GetClassIndex();

	if ( m_bSpeakingConceptAsDisguisedSpy && m_Shared.InCond( TF_COND_DISGUISED ) )
	{
		iClass = m_Shared.GetDisguiseClass();
	}

	bool bValidClass = ( iClass >= TF_CLASS_SCOUT && iClass <= TF_CLASS_COUNT );
	bool bValidConcept = ( m_iCurrentConcept >= 0 && m_iCurrentConcept < MP_TF_CONCEPT_COUNT );
	Assert( bValidClass );
	Assert( bValidConcept );

	if ( !bValidClass || !bValidConcept )
	{
		return BaseClass::GetResponseSystem();
	}
	else
	{
		return TFGameRules()->m_ResponseRules[iClass].m_ResponseSystems[m_iCurrentConcept];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::SpeakConceptIfAllowed( int iConcept, const char *modifiers, char *pszOutResponseChosen, size_t bufsize, IRecipientFilter *filter )
{
	if ( !IsAlive() )
		return false;

	bool bReturn = false;

	if ( IsSpeaking() )
	{
		if (iConcept != MP_CONCEPT_DIED && iConcept != MP_CONCEPT_PLRDEATH && iConcept != MP_CONCEPT_PLRDEATHCRIT && iConcept != MP_CONCEPT_PLRDEATHMELEE && iConcept != MP_CONCEPT_PLRDEATHEXPLOSION)
			return false;
	}

	// Save the current concept.
	m_iCurrentConcept = iConcept;

	if ( m_Shared.InCond( TF_COND_DISGUISED ) && !filter )
	{
		CSingleUserRecipientFilter filter(this);

		int iEnemyTeam = ( GetTeamNumber() == TF_TEAM_RED ) ? TF_TEAM_BLUE : TF_TEAM_RED;

		// test, enemies and myself
		CTeamRecipientFilter disguisedFilter( iEnemyTeam );
		disguisedFilter.AddRecipient( this );

		CMultiplayer_Expresser *pExpresser = GetMultiplayerExpresser();
		Assert( pExpresser );

		pExpresser->AllowMultipleScenes();

		// play disguised concept to enemies and myself
		char buf[128];
		Q_snprintf( buf, sizeof(buf), "disguiseclass:%s", g_aPlayerClassNames_NonLocalized[ m_Shared.GetDisguiseClass() ] );

		if ( modifiers )
		{
			Q_strncat( buf, ",", sizeof(buf), 1 );
			Q_strncat( buf, modifiers, sizeof(buf), COPY_ALL_CHARACTERS );
		}

		m_bSpeakingConceptAsDisguisedSpy = true;

		bool bPlayedDisguised = SpeakIfAllowed( g_pszMPConcepts[iConcept], buf, pszOutResponseChosen, bufsize, &disguisedFilter );

		m_bSpeakingConceptAsDisguisedSpy = false;

		// test, everyone except enemies and myself
		CBroadcastRecipientFilter undisguisedFilter;
		undisguisedFilter.RemoveRecipientsByTeam( GetGlobalTFTeam(iEnemyTeam) );
		undisguisedFilter.RemoveRecipient( this );

		// play normal concept to teammates
		bool bPlayedNormally = SpeakIfAllowed( g_pszMPConcepts[iConcept], modifiers, pszOutResponseChosen, bufsize, &undisguisedFilter );

		pExpresser->DisallowMultipleScenes();

		bReturn = ( bPlayedDisguised && bPlayedNormally );
	}
	else
	{
		// play normally
		bReturn = SpeakIfAllowed( g_pszMPConcepts[iConcept], modifiers, pszOutResponseChosen, bufsize, filter );
	}

	//Add bubble on top of a player calling for medic.
	if ( bReturn )
	{
		switch ( iConcept )
		{
		case MP_CONCEPT_PLAYER_MEDIC:
			{
				SaveMe();
				break;
			}
		case MP_CONCEPT_PLAYER_NO:
			{
				break;
			}
		case MP_CONCEPT_PLAYER_GO:
			{
				if ( GetTeamNumber() == TF_TEAM_RED || m_Shared.GetDisguiseTeam() == TF_TEAM_RED )
				{
					// TF2 characters don't have telepathy skills so you have to use voice commands to command rebels.
					m_QueuedCommand = CC_SEND;

					// Send antlions if we're allied with them
					if ( GlobalEntity_GetState( "antlion_allied" ) == GLOBAL_ON )
					{
						trace_t tr;
						Vector eyeDir;
						EyeVectors( &eyeDir );
						Ray_t ray;
						ray.Init( EyePosition(), EyePosition() + eyeDir * 2000 );
						UTIL_Portal_TraceRay( ray, MASK_BLOCKLOS_AND_NPCS, this, COLLISION_GROUP_NONE, &tr );
						if ( tr.fraction < 1.0f )
						{
							//Make sure we want to call antlions
							if ( CGrenadeBugBait::ActivateBugbaitTargets( this, tr.endpos, false ) == false )
							{
								//Alert any antlions around
								CSoundEnt::InsertSound( SOUND_BUGBAIT, tr.endpos, bugbait_hear_radius.GetInt(), bugbait_distract_time.GetFloat(), this );
							}

							// Tell all spawners to now fight to this position
							g_AntlionMakerManager.BroadcastFightGoal( tr.endpos );
						}
					}
					m_Shared.CreatePingEffect( TF_PING_GO );
				}
				break;
			}
		case MP_CONCEPT_PLAYER_HELP:
			{
				if ( GetTeamNumber() == TF_TEAM_RED || m_Shared.GetDisguiseTeam() == TF_TEAM_RED )
				{
					// Re-call squad.
					m_QueuedCommand = CC_FOLLOW;

					// Also call in antlions.
					if ( GlobalEntity_GetState( "antlion_allied" ) == GLOBAL_ON )
					{
						if ( CGrenadeBugBait::ActivateBugbaitTargets( this, GetAbsOrigin(), true ) == false )
						{
							g_AntlionMakerManager.BroadcastFollowGoal( this );
						}
					}
					m_Shared.CreatePingEffect( TF_PING_HELP );
				}
				break;
			}
		case MP_CONCEPT_PLAYER_CLOAKEDSPY:
			{
				m_Shared.CreatePingEffect( TF_PING_SPY );
				break;
			}
		case MP_CONCEPT_PLAYER_SENTRYHERE:
			{
				m_Shared.CreatePingEffect( TF_PING_SENTRY );
				break;
			}
		case MP_CONCEPT_PLAYER_DISPENSERHERE:
			{
				m_Shared.CreatePingEffect( TF_PING_DISPENSER );
				break;
			}
		case MP_CONCEPT_PLAYER_TELEPORTERHERE:
			{
				m_Shared.CreatePingEffect( TF_PING_TELEPORTER );
				break;
			}
		case MP_CONCEPT_PLAYER_ASK_FOR_BALL:
			{
				EmitSound( "Passtime.AskForBall" );
				break;
			}
		}
	}

	// now the enemies npc should hear us spamming MEDIC.
	CSoundEnt::InsertSound ( SOUND_PLAYER, GetAbsOrigin(), 512, 0.5, this );

	return bReturn;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateExpression( void )
{
	char szScene[ MAX_PATH ];
	if ( !GetResponseSceneFromConcept( MP_CONCEPT_PLAYER_EXPRESSION, szScene, sizeof( szScene ) ) )
	{
		ClearExpression();
		m_flNextRandomExpressionTime = gpGlobals->curtime + RandomFloat(30,40);
		return;
	}
	
	// Ignore updates that choose the same scene
	if ( m_iszExpressionScene != NULL_STRING && stricmp( STRING(m_iszExpressionScene), szScene ) == 0 )
		return;

	if ( m_hExpressionSceneEnt )
	{
		ClearExpression();
	}

	m_iszExpressionScene = AllocPooledString( szScene );
	float flDuration = InstancedScriptedScene( this, szScene, &m_hExpressionSceneEnt, 0.0, true, NULL, true );
	m_flNextRandomExpressionTime = gpGlobals->curtime + flDuration;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ClearExpression( void )
{
	if ( m_hExpressionSceneEnt != NULL )
	{
		StopScriptedScene( this, m_hExpressionSceneEnt );
	}
	m_flNextRandomExpressionTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Only show subtitle to enemy if we're disguised as the enemy
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldShowVoiceSubtitleToEnemy( void )
{
	return ( m_Shared.InCond( TF_COND_DISGUISED ) && m_Shared.GetDisguiseTeam() != GetTeamNumber() );
}

//-----------------------------------------------------------------------------
// Purpose: Don't allow rapid-fire voice commands
//-----------------------------------------------------------------------------
bool CTFPlayer::CanSpeakVoiceCommand( void )
{
	return ( gpGlobals->curtime > m_flNextVoiceCommandTime );
}

//-----------------------------------------------------------------------------
// Purpose: Note the time we're allowed to next speak a voice command
//-----------------------------------------------------------------------------
void CTFPlayer::NoteSpokeVoiceCommand( const char *pszScenePlayed )
{
	Assert( pszScenePlayed );
	m_flNextVoiceCommandTime = gpGlobals->curtime + min( GetSceneDuration( pszScenePlayed ), tf_max_voice_speak_delay.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::WantsLagCompensationOnEntity( const CBaseEntity *pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && ( pCmd->command_number - m_iLastWeaponFireUsercmd > 5 ) )
		return false;
	
	return BaseClass::WantsLagCompensationOnEntity( pEntity,pCmd,pEntityTransmitBits );
	
	bool bIsMedic = false;

	CTFPlayer *pPlayer = ToTFPlayer( (CBaseEntity*)pEntity );

	//Do Lag comp on medics trying to heal team mates.
	if ( ( IsPlayerClass( TF_CLASS_MEDIC ) == true ) || ( GetActiveTFWeapon() && GetActiveTFWeapon()->GetWeaponID() == TF_WEAPON_MEDIGUN ) )
	{
		bIsMedic = true;

		if ( pPlayer->GetTeamNumber() == GetTeamNumber()  )
		{
			CWeaponMedigun *pWeapon = dynamic_cast <CWeaponMedigun*>( GetActiveWeapon() );

			if ( pWeapon && pWeapon->GetHealTarget() )
			{
				if ( pWeapon->GetHealTarget() == pPlayer )
					return true;
				else
					return false;
			}
		}
	}

	if ( pPlayer->GetTeamNumber() == GetTeamNumber() && bIsMedic == false )
		return false;
	
	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pEntity->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	//float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();
	float maxspeed;
	if ( pPlayer )
		maxspeed = pPlayer->MaxSpeed();
	else
		maxspeed = 600;
	float maxDistance = 1.5 * maxspeed * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );

	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Mapmaker input to force this player to speak a response rules concept
//-----------------------------------------------------------------------------
void CTFPlayer::InputSpeakResponseConcept( inputdata_t &inputdata )
{
	int iConcept = GetMPConceptIndexFromString( inputdata.value.String() );
	if ( iConcept != MP_CONCEPT_NONE )
	{
		SpeakConceptIfAllowed( iConcept );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SpeakWeaponFire( int iCustomConcept )
{
	if ( iCustomConcept == MP_CONCEPT_NONE )
	{
		if ( m_flNextSpeakWeaponFire > gpGlobals->curtime )
			return;

		iCustomConcept = MP_CONCEPT_FIREWEAPON;
	}

	m_flNextSpeakWeaponFire = gpGlobals->curtime + 5;

	// Don't play a weapon fire scene if we already have one
	if ( m_hWeaponFireSceneEnt )
		return;

	char szScene[ MAX_PATH ];
	if ( !GetResponseSceneFromConcept( iCustomConcept, szScene, sizeof( szScene ) ) )
		return;

	float flDuration = InstancedScriptedScene(this, szScene, &m_hExpressionSceneEnt, 0.0, true, NULL, true );
	m_flNextSpeakWeaponFire = gpGlobals->curtime + flDuration;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ClearWeaponFireScene( void )
{
	if ( m_hWeaponFireSceneEnt )
	{
		StopScriptedScene( this, m_hWeaponFireSceneEnt );
		m_hWeaponFireSceneEnt = NULL;
	}
	m_flNextSpeakWeaponFire = gpGlobals->curtime;
}

int CTFPlayer::DrawDebugTextOverlays( void ) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];

		Q_snprintf( tempstr, sizeof( tempstr ),"Health: %d / %d ( %.1f )", GetHealth(), GetMaxHealth(), (float)GetHealth() / (float)GetMaxHealth() );
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Get response scene corresponding to concept
//-----------------------------------------------------------------------------
bool CTFPlayer::GetResponseSceneFromConcept( int iConcept, char *chSceneBuffer, int numSceneBufferBytes )
{
	AI_Response result;
	bool bResult = SpeakConcept( result, iConcept);
	if (bResult)
	{
		const char *szResponse = result.GetResponsePtr();
		Q_strncpy(chSceneBuffer, szResponse, numSceneBufferBytes);
	}
	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose:calculate a score for this player. higher is more likely to be switched
//-----------------------------------------------------------------------------
int	CTFPlayer::CalculateTeamBalanceScore( void )
{
	int iScore = BaseClass::CalculateTeamBalanceScore();

	// switch engineers less often
	if ( IsPlayerClass( TF_CLASS_ENGINEER ) )
	{
		iScore -= 120;
	}

	return iScore;
}

// ----------------------------------------------------------------------------
// Vintage Purpose: Play the stun sound to nearby players of the victim and the attacker
//-----------------------------------------------------------------------------
void CTFPlayer::StunSound( CTFPlayer *pStunner, int nStunFlags/*, int nCurrentStunFlags*/ )
{
	// Don't play the stun sound if sounds are disabled or the stunner isn't a player
	if ( pStunner == nullptr || ( nStunFlags & TF_STUNFLAG_NOSOUNDOREFFECT ) != 0 )
		return;

	const char *pszStunSound = "\0";
	if ( nStunFlags & TF_STUNFLAG_CHEERSOUND )
	{
		// Moonshot/Grandslam
		pszStunSound = "TFPlayer.StunImpactRange";
	}
	else if( !( nStunFlags & TF_STUNFLAG_GHOSTEFFECT ) )
	{
		// Normal stun
		pszStunSound = "TFPlayer.StunImpact";
	}
	else
	{
		// Spookily spooked
		pszStunSound = "Halloween.PlayerScream";
	}

	CRecipientFilter filter;
	CSingleUserRecipientFilter filterAttacker( pStunner );
	filter.AddRecipientsByPAS( GetAbsOrigin() );
	filter.RemoveRecipient( pStunner );

	EmitSound( filter, entindex(), pszStunSound );
	EmitSound( filterAttacker, pStunner->entindex(), pszStunSound );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
// Debugging Stuff
extern CBaseEntity *FindPickerEntity( CBasePlayer *pPlayer );
void DebugParticles( const CCommand &args )
{
	CBaseEntity *pEntity = FindPickerEntity( UTIL_GetCommandClient() );

	if ( pEntity && pEntity->IsPlayer() )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pEntity );

		// print out their conditions
		pPlayer->m_Shared.DebugPrintConditions();	
	}
}

static ConCommand sv_debug_stuck_particles( "sv_debug_stuck_particles", DebugParticles, "Debugs particles attached to the player under your crosshair.", FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: Debug concommand to set the player on fire
//-----------------------------------------------------------------------------
void IgnitePlayer()
{
	CTFPlayer *pPlayer = ToTFPlayer( ToTFPlayer( UTIL_PlayerByIndex( 1 ) ) );
	pPlayer->m_Shared.Burn( pPlayer );
}
static ConCommand cc_IgnitePlayer( "tf_ignite_player", IgnitePlayer, "Sets you on fire", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TestVCD( const CCommand &args )
{
	CBaseEntity *pEntity = FindPickerEntity( UTIL_GetCommandClient() );

	if ( pEntity && pEntity->IsCombatCharacter() )
	{
		CBaseCombatCharacter *pCharacter = ToBaseCombatCharacter( pEntity );
		if ( pCharacter )
		{
			if ( args.ArgC() >= 2 )
			{
				InstancedScriptedScene( pCharacter, args[1], NULL, 0.0f, false, NULL, true );
			}
			else
			{
				InstancedScriptedScene( pCharacter, "scenes/player/heavy/low/270.vcd", NULL, 0.0f, false, NULL, true );
			}
		}
	}
}
static ConCommand tf_testvcd( "tf_testvcd", TestVCD, "Run a vcd on the player currently under your crosshair. Optional parameter is the .vcd name (default is 'scenes/heavy_test.vcd')", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TestRR( const CCommand &args )
{
	if ( args.ArgC() < 2 )
	{
		Msg("No concept specified. Format is tf_testrr <concept>\n");
		return;
	}

	CBaseEntity *pEntity = NULL;
	const char *pszConcept = args[1];

	if ( args.ArgC() == 3 )
	{
		pszConcept = args[2];
		pEntity = UTIL_PlayerByName( args[1] );
	}

	if ( !pEntity || !pEntity->IsPlayer() )
	{
		pEntity = FindPickerEntity( UTIL_GetCommandClient() );
		if ( !pEntity || !pEntity->IsPlayer() )
		{
			pEntity = ToTFPlayer( UTIL_GetCommandClient() ); 
		}
	}

	if ( pEntity && pEntity->IsPlayer() )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pEntity );
		if ( sv_cheats->GetInt() != 0 )
		{
			if ( pPlayer )
			{
				int iConcept = GetMPConceptIndexFromString( pszConcept );
				if ( iConcept != MP_CONCEPT_NONE )
				{
					pPlayer->SpeakConceptIfAllowed( iConcept );
				}
				else
				{
					Msg( "Attempted to speak unknown multiplayer concept: %s\n", pszConcept );
				}
			}
		}
		else
		{
			Msg( "Can't use cheat command tf_testrr in multiplayer, unless the server has sv_cheats set to 1.\n" );
			return;
		}
	}
}
static ConCommand tf_testrr( "tf_testrr", TestRR, "Force the player under your crosshair to speak a response rule concept. Format is tf_testrr <concept>, or tf_testrr <player name> <concept>", 0 );

//-----------------------------------------------------------------------------
// Purpose: weaker version of tf_testrr
//-----------------------------------------------------------------------------
void LFVocalize( const CCommand &args )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( args.ArgC() < 2 )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "No concept specified. Format is vocalize <concept>\n" );
		return;
	}

	const char *pszConcept = args[1];
	if ( FStrEq( args[1], "TLK_PLAYER_TAUNT" ) )
		return;

	int iConcept = GetMPConceptIndexFromString( pszConcept );
	if ( iConcept != MP_CONCEPT_NONE )
	{
		pPlayer->SpeakConceptIfAllowed( iConcept );
	}
	else
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Attempted to speak unknown multiplayer concept: %s\n", pszConcept);
	}
}
static ConCommand vocalize( "vocalize", LFVocalize, "Speak a response rule concept.\nFormat is vocalize <concept>", 0 );


CON_COMMAND_F( tf_crashclients, "testing only, crashes about 50 percent of the connected clients.", FCVAR_DEVELOPMENTONLY )
{
	for ( int i = 1; i < gpGlobals->maxClients; ++i )
	{
		if ( RandomFloat( 0.0f, 1.0f ) < 0.5f )
		{
			CBasePlayer *pl = UTIL_PlayerByIndex( i + 1 );
			if ( pl )
			{
				engine->ClientCommand( pl->edict(), "crash\n" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Dev commands
//-----------------------------------------------------------------------------
CBaseEntity *GetNextCommandEntity( CBasePlayer *pPlayer, const char *name, CBaseEntity *ent );

CON_COMMAND_F( give_econ, "Give ECON item with specified ID from item schema.\nFormat: <id> <classname> <attribute1ID> <value1> <attribute2ID> <value2> ... <attributeNID> <valueN>", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( !pPlayer )
			return;

		if ( args.ArgC() < 2 )
			return;

		int iItemID = atoi( args[1] );
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iItemID );
		if ( !pItemDef )
		{
			const char *pszItemName = args[1];
			CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinitionByName( pszItemName );
			if ( !pItemDef )
			{
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Unknown Item ID\n" );
				return;
			}
		}

		CEconItemView econItem( iItemID );

		bool bAddedAttributes = false;

		// Additonal params are attributes.
		for ( int i = 3; i + 1 < args.ArgC(); i += 2 )
		{
			int iAttribIndex = atoi( args[i] );
			float flValue = atof( args[i + 1] );

			CEconItemAttribute econAttribute( iAttribIndex, flValue );
			bAddedAttributes = econItem.AddAttribute( &econAttribute );
		}

		econItem.SkipBaseAttributes( bAddedAttributes );

		// Nuke whatever we have in this slot.
		int iClass = pPlayer->GetPlayerClass()->GetClassIndex();
		int iSlot = pItemDef->GetLoadoutSlot( iClass );
		CEconEntity *pEntity = pPlayer->GetEntityForLoadoutSlot( iSlot );

		if ( pEntity )
		{
			CBaseCombatWeapon *pWeapon = pEntity->MyCombatWeaponPointer();

			if ( pWeapon )
			{
				if ( pWeapon == pPlayer->GetActiveWeapon() )
					pWeapon->Holster();

				pPlayer->Weapon_Detach( pWeapon );
				UTIL_Remove( pWeapon );
			}
			else if ( pEntity->IsWearable() )
			{
				CEconWearable *pWearable = static_cast<CEconWearable *>( pEntity );
				pPlayer->RemoveWearable( pWearable );
			}
			else
			{
				AssertMsg( false, "Player has unknown entity in loadout slot %d.", iSlot );
				UTIL_Remove( pEntity );
			}
		}

		const char *pszClassname = args.ArgC() > 2 ? args[2] : pItemDef->item_class;
		if ( FStrEq( pszClassname, "no_entity" ) )
			return;

		CEconEntity *pEconEnt = dynamic_cast<CEconEntity *>( pPlayer->GiveNamedItem( pszClassname, 0, &econItem ) );
		if ( pEconEnt )
		{
			pEconEnt->GiveTo( pPlayer );

			CBaseCombatWeapon *pWeapon = pEconEnt->MyCombatWeaponPointer();
			if ( pWeapon )
			{
				// Give full ammo for this weapon.
				int iAmmoType = pWeapon->GetPrimaryAmmoType();
				pPlayer->SetAmmoCount( pPlayer->GetMaxAmmo( iAmmoType ), iAmmoType );
			}
		}
	}
	else
	{
		Msg( "Can't use cheat command give_econ in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( npc_give_econ, "Give ECON item with specified classname from item schema to npc.\nFormat: <npc classname> <item id> <classname> <attribute1ID> <value1> <attribute2ID> <value2> ... <attributeNID> <valueN>", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( !pPlayer )
			return;

		if ( args.ArgC() < 2 )
			return;

		int iItemID = atoi( args[2] );
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iItemID );
		if ( !pItemDef )
		{
			const char *pszItemName = args[2];
			CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinitionByName( pszItemName );
			if ( !pItemDef )
			{
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Unknown Item ID\n" );
				return;
			}
		}

		CEconItemView econItem( iItemID );

		bool bAddedAttributes = false;

		// Additonal params are attributes.
		for ( int i = 4; i + 1 < args.ArgC(); i += 2 )
		{
			int iAttribIndex = atoi( args[i] );
			float flValue = atof( args[i + 1] );

			CEconItemAttribute econAttribute( iAttribIndex, flValue );
			bAddedAttributes = econItem.AddAttribute( &econAttribute );
		}

		econItem.SkipBaseAttributes( bAddedAttributes );

		CBaseEntity *pEntity = NULL;
		const char *name = args[1];
		while ( (pEntity = GetNextCommandEntity( pPlayer, name, pEntity )) != NULL )
		{
			CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
			if ( pNPC )
			{
				const char *pszClassname = args.ArgC() > 3 ? args[3] : pItemDef->item_class;
				if ( FStrEq( pszClassname, "no_entity" ) )
					return;

				CEconEntity *pEconEnt = dynamic_cast<CEconEntity *>( pPlayer->GiveNamedItem( pszClassname, 0, &econItem, false ) );

				if ( pEconEnt )
				{
					CBaseCombatWeapon *pWeapon = pEconEnt->MyCombatWeaponPointer();
					if ( pWeapon )
					{
						pNPC->PickupWeapon( pWeapon );

						// Give full ammo for this weapon.
						//int iAmmoType = pWeapon->GetPrimaryAmmoType();
						//pNPC->SetAmmoCount( pNPC->GetMaxAmmo( iAmmoType ), iAmmoType );
					}
					else if ( pEntity->IsWearable() )
					{
						pEconEnt->GiveTo( pNPC );
					}
				}
			}
		}
	}
	else
	{
		Msg( "Can't use cheat command npc_give_econ in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( spawn_econ, "Spawn ECON item with specified ID from item schema.\nFormat: <id> <classname> <attribute1ID> <value1> <attribute2ID> <value2> ... <attributeNID> <valueN>\nBut this command is only for the devs", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() < 2 )
			return;

		int iItemID = atoi( args[1] );
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( iItemID );
		if ( !pItemDef )
			return;

		CEconItemView econItem( iItemID );

		bool bAddedAttributes = false;

		// Additonal params are attributes.
		for ( int i = 3; i + 1 < args.ArgC(); i += 2 )
		{
			int iAttribIndex = atoi( args[i] );
			float flValue = atof( args[i + 1] );

			CEconItemAttribute econAttribute( iAttribIndex, flValue );
			bAddedAttributes = econItem.AddAttribute( &econAttribute );
		}

		econItem.SkipBaseAttributes( bAddedAttributes );

		const char *pszClassname = args.ArgC() > 2 ? args[2] : pItemDef->item_class;
		CEconEntity *pEconEnt = dynamic_cast<CEconEntity *>( pPlayer->GiveNamedItem( pszClassname, 0, &econItem, false ) );
		if ( pEconEnt )
		{
			CTFWeaponBase *pWeapon = dynamic_cast<CTFWeaponBase *>( pEconEnt );
			if ( pWeapon )
			{
				CTFDroppedWeapon *pDroppedWeapon = CTFDroppedWeapon::Create( pPlayer->WorldSpaceCenter(), pPlayer->GetAbsAngles(), pPlayer, pWeapon );
				if ( pDroppedWeapon )
				{
					pDroppedWeapon->SetClip( pWeapon->GetMaxClip1() );
					pDroppedWeapon->SetAmmo( pWeapon->GetMaxClip1() );
					pDroppedWeapon->SetMaxAmmo( pWeapon->GetMaxClip1() );
					UTIL_Remove( pWeapon );
				}
			}
		}
	}
	else
	{
		Msg( "Can't use cheat command spawn_econ in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( give_particle, "Set weapon/wearable particle effect", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() < 2 )
			return;

		const char *pszParticleName = args[1];

		for ( int iWearable = 0; iWearable < pPlayer->GetNumWearables(); iWearable++ )
		{
			CEconWearable *pWearable = pPlayer->GetWearable( iWearable );
			if ( pWearable )
				pWearable->SetParticle( pszParticleName );
		}

		/*for ( int iWeapon = 0; iWeapon < TF_PLAYER_WEAPON_COUNT; iWeapon++ )
		{
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)pPlayer->Weapon_GetSlot( iWeapon );
			if ( pWeapon )
				pWeapon->SetParticle( pszParticleName );
		}*/
	}
	else
	{
		Msg( "Can't use cheat command give_particle in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( weaponpresetclass, "", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( args.ArgC() >= 4 )
		pPlayer->HandleCommand_WeaponPreset(abs(atoi(args[1])), abs(atoi(args[2])), abs(atoi(args[3])));
}

int AddCondAutoComplete( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
	// Find the first space in our input
	const char *firstSpace = V_strstr(partial, " ");
	if(!firstSpace)
		return 0;

	int commandLength = firstSpace - partial;

	// Extract the command name from the input
	char commandName[COMMAND_COMPLETION_ITEM_LENGTH];
	V_StrSlice(partial, 0, commandLength, commandName, sizeof(commandName));

	// Calculate the length of the command string (minus the command name)
	partial += commandLength + 1;
	int partialLength = V_strlen(partial);

	int count = 0;
	for( unsigned int i=0; g_aTFCondNames[i] && count < COMMAND_COMPLETION_MAXITEMS; ++i )
	{
		if ( !Q_strnicmp( g_aTFCondNames[i], partial, partialLength ) )
		{
			// Add to the autocomplete array
			Q_snprintf( commands[ count++ ], COMMAND_COMPLETION_ITEM_LENGTH, "%s %s", commandName, g_aTFCondNames[i] );
		}
	}

	return count;
}

CON_COMMAND_F_COMPLETION( addcond, "Add TF Conds", 0, AddCondAutoComplete )
{
	CTFPlayer *pTargetPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pTargetPlayer )
		return;

	if ( ( sv_cheats->GetInt() != 0 ) || pTargetPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			int iCond = clamp( atoi( args[1] ), 0, TF_COND_LAST-1 );
			const char *pszTFCondName = args[1];
			if ( !iCond )
			{
				iCond = GetTFCondId( pszTFCondName );
			}

			if ( args.ArgC() >= 4 )
			{
				// Find the matching netname
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex(i) );
					if ( pPlayer )
					{
						if ( Q_strstr( pPlayer->GetPlayerName(), args[3] ) )
						{
							pTargetPlayer = pPlayer;
						}
					}
				}
			}
			
			if ( iCond == LFE_COND_POWERPLAY && pTargetPlayer->IsDeveloper() )
			{
				CTFTeam *pTFTeam = pTargetPlayer->GetTFTeam();
				if ( pTFTeam )
					pTFTeam->AwardAchievement( ACHIEVEMENT_LFE_DEV_ABUSE );
			}

			if ( args.ArgC() >= 3 )
			{
				float flDuration = atof( args[2] );
				pTargetPlayer->m_Shared.AddCond( iCond, flDuration );
				DevMsg( "Added Cond: %s with %f second to %s\n", pszTFCondName, flDuration, pTargetPlayer->GetPlayerName() );
			}
			else
			{
				pTargetPlayer->m_Shared.AddCond( iCond );
				DevMsg( "Added Cond: %s to %s\n", pszTFCondName, pTargetPlayer->GetPlayerName() );
			}

			if ( FStrEq( args[1], "all" ) )
			{
				int i;
				for ( i = 0; i < TF_COND_LAST; i++ )
				{
					pTargetPlayer->m_Shared.AddCond( i );
					//DevMsg( "Added all condition to %s\n", pTargetPlayer->GetPlayerName() );
				}
			}
		}
		else
		{
			ClientPrint( pTargetPlayer, HUD_PRINTCONSOLE, "Usage: addcond <condition number|all> <duration>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command addcond in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F_COMPLETION( removecond, "Remove TF Conds", 0, AddCondAutoComplete )
{
	CTFPlayer *pTargetPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pTargetPlayer )
		return;

	if ( ( sv_cheats->GetInt() != 0 ) || pTargetPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			int iCond = clamp( atoi( args[1] ), 0, TF_COND_LAST-1 );
			const char *pszTFCondName = args[1];
			if ( !iCond )
			{
				iCond = GetTFCondId( pszTFCondName );
			}

			pTargetPlayer->m_Shared.RemoveCond( iCond );
			DevMsg( "Removed Cond: %s from %s\n", pszTFCondName, pTargetPlayer->GetPlayerName() );

			if ( FStrEq( args[1], "all" ) )
			{
				int i;
				for ( i = 0; i < TF_COND_LAST; i++ )
				{
					if ( pTargetPlayer->m_Shared.InCond( i ) )
					{
						pTargetPlayer->m_Shared.RemoveCond( i );
						//DevMsg( "Removed all condition from %s\n", pTargetPlayer->GetPlayerName() );
					}
				}
			}
		}
		else
		{
			ClientPrint( pTargetPlayer, HUD_PRINTCONSOLE, "Usage: removecond <condition number|all>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command removecond in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

int NPCAddCondAutoComplete( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
	// Find the first space in our input
	const char *firstSpace = V_strstr(partial, " ");
	if(!firstSpace)
		return 0;

	int commandLength = firstSpace - partial;

	// Extract the command name from the input
	char commandName[COMMAND_COMPLETION_ITEM_LENGTH];
	V_StrSlice(partial, 0, commandLength, commandName, sizeof(commandName));

	// Calculate the length of the command string (minus the command name)
	partial += commandLength + 1;
	int partialLength = V_strlen(partial);

	int count = 0;
	for( unsigned int i=0; g_aTFCondNames[i] && count < COMMAND_COMPLETION_MAXITEMS; ++i )
	{
		if ( !Q_strnicmp( g_aTFCondNames[i], partial, partialLength ) )
		{
			// Add to the autocomplete array
			Q_snprintf( commands[ count++ ], COMMAND_COMPLETION_ITEM_LENGTH, "%s %s", commandName, g_aTFCondNames[i] );
		}
	}

	return count;
}

CON_COMMAND_F_COMPLETION( npc_addcond, "Add TF Conds to NPC", 0, NPCAddCondAutoComplete )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			int iCond = clamp( atoi( args[2] ), 0, TF_COND_LAST-1 );
			const char *pszTFCondName = args[2];
			if ( !iCond )
			{
				iCond = GetTFCondId( pszTFCondName );
			}

			CBaseEntity *pEntity = NULL;
			const char *name = args[1];
			while ( (pEntity = GetNextCommandEntity( pPlayer, name, pEntity )) != NULL )
			{
				CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
				if ( pNPC )
				{
					if ( args.ArgC() >= 4 )
					{
						float flDuration = atof( args[3] );
						pNPC->AddCond( iCond, flDuration );
						DevMsg( "Added %s with %f second to npc %s\n", pszTFCondName, flDuration, pNPC->GetClassname() );
					}
					else
					{
						pNPC->AddCond( iCond );
						DevMsg( "Added %s to npc %s\n", pszTFCondName, pNPC->GetClassname() );
					}
				}

				if ( FStrEq( args[2], "all" ) )
				{
					int i;
					for ( i = 0; i < TF_COND_LAST; i++ )
					{
						pNPC->AddCond( i );
						//DevMsg( "Added all condition to npc %s\n", pNPC->GetClassname() );
					}
				}
			}
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage: npc_addcond <npc classname> <condition number|all> <duration>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command addcond in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F_COMPLETION( npc_removecond, "Remove TF Conds  to NPC", 0, NPCAddCondAutoComplete )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			int iCond = clamp( atoi( args[2] ), 0, TF_COND_LAST-1 );
			const char *pszTFCondName = args[2];
			if ( !iCond )
			{
				iCond = GetTFCondId( pszTFCondName );
			}

			CBaseEntity *pEntity = NULL;
			const char *name = args[1];
			while ( (pEntity = GetNextCommandEntity( pPlayer, name, pEntity )) != NULL )
			{
				CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
				if ( pNPC )
				{
					pNPC->RemoveCond( iCond );
					DevMsg( "Removed %s from npc %s\n", pszTFCondName, pNPC->GetClassname() );
				}
			
				if ( FStrEq( args[2], "all" ) )
				{
					int i;
					for ( i = 0; i < TF_COND_LAST; i++ )
					{
						if ( pNPC->InCond( i ) )
						{
							pNPC->RemoveCond( i );
							//DevMsg( "Removed all condition from %s\n", pNPC->GetClassname() );
						}
					}
				}
			}
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage: npc_removecond <condition number|all> <npc classname>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command removecond in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( addattrib, "Add TF Attributes", 0 )
{
	CTFPlayer *pTargetPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pTargetPlayer )
		return;

	if ( sv_cheats->GetInt() != 0 || pTargetPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			const char *pszAttribute = args[1];

			if ( args.ArgC() >= 4 )
			{
				// Find the matching netname
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
					if ( pPlayer )
					{
						if ( Q_strstr( pPlayer->GetPlayerName(), args[3] ) )
						{
							pTargetPlayer = pPlayer;
							break;
						}
					}
				}
			}

			if ( args.ArgC() >= 3 )
			{
				const char *pszValue = args[2];

				if ( !V_isdigit(pszValue[0]) )
				{
					CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pszValue );
					if ( pAttrib )
					{
						pTargetPlayer->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );

						for ( int i = 0; i < pTargetPlayer->WeaponCount(); i++ )
						{
							CTFWeaponBase *pWeapon = ( CTFWeaponBase * )pTargetPlayer->GetWeapon( i );
							if ( pWeapon )
							{
								pWeapon->GetItem()->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );
								pWeapon->WeaponReset();
							}
						}

						for ( int i = 0; i < pTargetPlayer->GetNumWearables(); i++ )
						{
							CTFWearable *pWearable = static_cast<CTFWearable *>( pTargetPlayer->GetWearable( i ) );
							if ( pWearable )
							{
								pWearable->GetItem()->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );
							}
						}

						DevMsg( "Added Attribute: %s with %s value to %s\n", pszAttribute, pszValue, pTargetPlayer->GetPlayerName() );
					}
				}
				else
				{
					float flValue = atof( pszValue );
					pTargetPlayer->AddCustomAttribute( pszAttribute, flValue );
					DevMsg( "Added Attribute: %s with %f value to %s\n", pszAttribute, flValue, pTargetPlayer->GetPlayerName() );
				}
			}
			else
			{
				pTargetPlayer->AddCustomAttribute( pszAttribute, 1 );
				DevMsg( "Added Attribute: %s with %d value to %s\n", pszAttribute, 1, pTargetPlayer->GetPlayerName() );
			}
		}
		else
		{
			ClientPrint( pTargetPlayer, HUD_PRINTCONSOLE, "Usage: addattrib <attribute name> <value>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command addattrib in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( npc_addattrib, "Add TF Attributes to an npc", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( ( sv_cheats->GetInt() != 0 ) || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			const char *pszAttribute = args[2];

			CBaseEntity *pEntity = NULL;
			const char *name = args[1];
			while ( (pEntity = GetNextCommandEntity( pPlayer, name, pEntity )) != NULL )
			{
				CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
				if ( pNPC )
				{
					if ( args.ArgC() >= 3 )
					{
						const char *pszValue = args[2];
						if ( !V_isdigit(pszValue[0]) )
						{
							CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pszValue );
							if ( pAttrib )
							{
								pNPC->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );

								for ( int i = 0; i < pNPC->WeaponCount(); i++ )
								{
									CBaseCombatWeapon *pWeapon = pNPC->GetWeapon( i );
									if ( pWeapon )
									{
										pWeapon->GetItem()->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );
									}
								}

								for ( int i = 0; i < pNPC->GetNumWearables(); i++ )
								{
									CTFWearable *pWearable = static_cast<CTFWearable *>( pNPC->GetWearable( i ) );
									if ( pWearable )
									{
										pWearable->GetItem()->GetAttributeList()->SetRuntimeAttributeValue( pAttrib, pszValue );
									}
								}

								DevMsg( "Added Attribute: %s with %s value to %s\n", pszAttribute, pszValue, pNPC->GetClassname() );
							}
						}
						else
						{
							float flValue = atof( pszValue );
							pNPC->AddCustomAttribute( pszAttribute, flValue );
							DevMsg( "Added Attribute: %s with %f value to %s\n", pszAttribute, flValue, pNPC->GetClassname() );
						}
					}
					else
					{
						pNPC->AddCustomAttribute( pszAttribute, 1 );
						DevMsg( "Added Attribute: %s with %d value to %s\n", pszAttribute, 1, pNPC->GetClassname() );
					}
				}
			}
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage: addattrib <npc classname> <attribute name> <value>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command npc_addattrib in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( removeattrib, "Remove TF Attributes", 0 )
{
	CTFPlayer *pTargetPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pTargetPlayer )
		return;

	if ( sv_cheats->GetInt() != 0 || pTargetPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			const char *pszAttribute = args[1];
			pTargetPlayer->RemoveCustomAttribute( pszAttribute );
			DevMsg( "Removed Attribute: %s from %s\n", pszAttribute, pTargetPlayer->GetPlayerName() );
		}
		else
		{
			ClientPrint( pTargetPlayer, HUD_PRINTCONSOLE, "Usage: removeattrib <attribute name>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command removeattrib in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND_F( currency_give, "", 0 )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( sv_cheats->GetInt() != 0 || pPlayer->IsDeveloper() )
	{
		if ( args.ArgC() >= 2 )
		{
			pPlayer->m_Shared.SetCurrency( pPlayer->m_Shared.GetCurrency() + atoi( args[1] ) );
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage: currency_give <value>\n" );
			return;
		}
	}
	else
	{
		Msg( "Can't use cheat command currency_give in multiplayer, unless the server has sv_cheats set to 1.\n" );
		return;
	}
}

CON_COMMAND( taunt_by_name, "Use equipped taunt by id." )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( !pPlayer )
		return;

	if ( args.ArgC() == 2 )
	{
		if ( FStrEq( args[1], "default" ) )
		{
			pPlayer->HandleTauntCommand( 0 );
			return;
		}

		pPlayer->PlayTauntSceneFromItemID( atoi( args[1] ) );
	}
	else
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage: taunt_by_name <taunt id>\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::SetPowerplayEnabled( bool bOn )
{
	if ( bOn )
	{
		m_flPowerPlayTime = gpGlobals->curtime + 99999;
		PowerplayThink();
	}
	else
	{
		m_flPowerPlayTime = 0.0;
	}
	return true;
}

uint64 powerplaymask = 0xFAB2423BFFA352AF;
uint64 powerplay_ids[] =
{
	0 ^ powerplaymask
	//76561198177327375 ^ powerplaymask,		// ispuddy
	//76561198080213691 ^ powerplaymask,		// alex
	//76561198116553704 ^ powerplaymask,		// swox
	//76561198033171144 ^ powerplaymask,		// agent agrimar
	//76561198057939605 ^ powerplaymask,		// intriguingtiles
	//76561198201866231 ^ powerplaymask,		// sergi338
	//76561198063379226 ^ powerplaymask,		// nbc66
	//76561198127525324 ^ powerplaymask,		// mechadexic
	//76561198073323764 ^ powerplaymask,		// liquide vaisselle
	//76561198139584452 ^ powerplaymask,		// stoneman
	//76561198079826628 ^ powerplaymask,		// dream
	//76561198855240201 ^ powerplaymask,		// coach
	//76561198362543685 ^ powerplaymask,		// lecs
	//76561198419900837 ^ powerplaymask,		// mugg
	//76561198193780653 ^ powerplaymask,		// hdmineface 
	//76561198067410719 ^ powerplaymask,		// msalinas
	//76561198061175832 ^ powerplaymask,		// kyle 
	//76561198031570068 ^ powerplaymask,		// leakdealer
	//76561198116511493 ^ powerplaymask,		// kris
	//76561198145444029 ^ powerplaymask,		// train
	//76561193714870430 ^ powerplaymask,		// richter
	//76561193727203452 ^ powerplaymask,		// lokkdokk
	//76561198379293085 ^ powerplaymask,		// melectrome
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::PlayerHasPowerplay( void )
{
	if ( !engine->IsClientFullyAuthenticated( edict() ) )
		return false;

	player_info_t pi;
	if ( engine->GetPlayerInfo( entindex(), &pi ) && ( pi.friendsID ) )
	{
		CSteamID steamIDForPlayer( pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual );
		for ( int i = 0; i < ARRAYSIZE(powerplay_ids); i++ )
		{
			if ( steamIDForPlayer.ConvertToUint64() == (powerplay_ids[i] ^ powerplaymask) )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: jesus "nicknine" chirst
//-----------------------------------------------------------------------------
bool CTFPlayer::IsNicknine( void )
{
	if ( !engine->IsClientFullyAuthenticated( edict() ) )
		return false;

	player_info_t pi;
	if ( engine->GetPlayerInfo( entindex(), &pi ) && ( pi.friendsID ) )
	{
		CSteamID steamIDForPlayer( pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual );
		for ( int i = 0; i < ARRAYSIZE(powerplay_ids); i++ )
		{
			if ( steamIDForPlayer.ConvertToUint64() == ( 76561198053356818 ) )
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PowerplayThink( void )
{
	if ( m_flPowerPlayTime > gpGlobals->curtime )
	{
		float flDuration = 0;
		if ( GetPlayerClass() )
		{
			switch ( GetPlayerClass()->GetClassIndex() )
			{
			case TF_CLASS_SCOUT: flDuration = InstancedScriptedScene( this, "scenes/player/scout/low/435.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_SNIPER: flDuration = InstancedScriptedScene( this, "scenes/player/sniper/low/1675.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_SOLDIER: flDuration = InstancedScriptedScene( this, "scenes/player/soldier/low/1346.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_DEMOMAN: flDuration = InstancedScriptedScene( this, "scenes/player/demoman/low/954.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_MEDIC: flDuration = InstancedScriptedScene( this, "scenes/player/medic/low/608.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_HEAVYWEAPONS: flDuration = InstancedScriptedScene( this, "scenes/player/heavy/low/270.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_PYRO: flDuration = InstancedScriptedScene( this, "scenes/player/pyro/low/1485.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_SPY: flDuration = InstancedScriptedScene( this, "scenes/player/spy/low/772.vcd", NULL, 0.0f, false, NULL, true ); break;
			case TF_CLASS_ENGINEER: flDuration = InstancedScriptedScene( this, "scenes/player/engineer/low/103.vcd", NULL, 0.0f, false, NULL, true ); break;
			}
		}

		SetContextThink( &CTFPlayer::PowerplayThink, gpGlobals->curtime + flDuration + RandomFloat( 2, 5 ), "TFPlayerLThink" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::ShouldAnnouceAchievement( void )
{ 
	if ( IsPlayerClass( TF_CLASS_SPY ) )
	{
		if ( m_Shared.InCond( TF_COND_DISGUISED ) ||
			 m_Shared.InCond( TF_COND_DISGUISING ) )
		{
			return false;
		}
	}

	if ( m_Shared.IsStealthed() )
		return false;

	return true; 
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void CTFPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;

	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_hLadder.Set( NULL );
}


surfacedata_t *CTFPlayer::GetLadderSurface( const Vector &origin )
{
	extern const char *FuncLadder_GetSurfaceprops(CBaseEntity *pLadderEntity);

	CBaseEntity *pLadder = m_hLadder.Get();
	if ( pLadder )
	{
		const char *pSurfaceprops = FuncLadder_GetSurfaceprops(pLadder);
		// get ladder material from func_ladder
		return physprops->GetSurfaceData( physprops->GetSurfaceIndex( pSurfaceprops ) );

	}
	return BaseClass::GetLadderSurface(origin);
}

void CTFPlayer::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	// can't pick up what you're standing on
	if ( GetGroundEntity() == pObject )
		return;

	// Must be able to holster current weapon
	if ( GetActiveTFWeapon() && GetActiveTFWeapon()->CanHolster() == false )
		return;

	if ( bLimitMassAndSize == true )
	{
		int iMaxMass = 35;
		int iMaxSize = 128;

		if ( TFGameRules()->IsInPortalMap() || IsPlayerClass( TF_CLASS_HEAVYWEAPONS ) )
		{
			iMaxMass = 100;
			iMaxSize = 150;
		}
		else if ( IsPlayerClass( TF_CLASS_ENGINEER ) || IsPlayerClass( TF_CLASS_SOLDIER ) || IsPlayerClass( TF_CLASS_DEMOMAN ) )
		{
			iMaxMass = 95;
			iMaxSize = 135;
		}
		else if ( IsPlayerClass( TF_CLASS_SNIPER ) || IsPlayerClass( TF_CLASS_SPY ) || IsPlayerClass( TF_CLASS_MEDIC ) || IsPlayerClass( TF_CLASS_PYRO ) )
		{
			iMaxMass = 75;
			iMaxSize = 130;
		}

		if ( CanPickupObject( pObject, iMaxMass, iMaxSize ) == false )
			 return;
	}

	// Can't be picked up if NPCs are on me
	if ( pObject->HasNPCsOnIt() )
		return;

	PlayerPickupObject( this, pObject );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
bool CTFPlayer::IsHoldingEntity( CBaseEntity *pEnt )
{
	return PlayerPickupControllerIsHoldingEntity( m_hUseEntity, pEnt );
}

float CTFPlayer::GetHeldObjectMass( IPhysicsObject *pHeldObject )
{
	float mass = PlayerPickupGetHeldObjectMass( m_hUseEntity, pHeldObject );
	if ( mass == 0.0f )
	{
		mass = PhysCannonGetHeldObjectMass( GetActiveWeapon(), pHeldObject );
	}
	return mass;
}

CBaseEntity	*CTFPlayer::GetHeldObject( void )
{
	return PhysCannonGetHeldEntity( GetActiveTFWeapon() );
}

//-----------------------------------------------------------------------------
// Purpose: Force the player to drop any physics objects he's carrying
//-----------------------------------------------------------------------------
void CTFPlayer::ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldingThis )
{
	if ( PhysIsInCallback() )
	{
		variant_t value;
		g_EventQueue.AddEvent( this, "ForceDropPhysObjects", value, 0.01f, pOnlyIfHoldingThis, this );
		return;
	}

	if ( hl2_episodic.GetBool() )
	{
		CBaseEntity *pHeldEntity = PhysCannonGetHeldEntity( GetActiveWeapon() );
		if( pHeldEntity && pHeldEntity->ClassMatches( "grenade_helicopter" ) )
		{
			return;
		}
	}

	// Drop any objects being handheld.
	ClearUseEntity();

	// Then force the physcannon to drop anything it's holding, if it's our active weapon
	PhysCannonForceDrop( GetActiveWeapon(), NULL );
}

void CTFPlayer::InputForceDropPhysObjects( inputdata_t &data )
{
	ForceDropOfCarriedPhysObjects( data.pActivator );
}

void CTFPlayer::IncrementPortalsPlaced( void )
{
	m_StatsThisLevel.iNumPortalsPlaced++;

	if ( m_iBonusChallenge == PORTAL_CHALLENGE_PORTALS )
		SetBonusProgress( static_cast<int>( m_StatsThisLevel.iNumPortalsPlaced ) );
}

void CTFPlayer::IncrementStepsTaken( void )
{
	m_StatsThisLevel.iNumStepsTaken++;

	if ( m_iBonusChallenge == PORTAL_CHALLENGE_STEPS )
		SetBonusProgress( static_cast<int>( m_StatsThisLevel.iNumStepsTaken ) );
}

void CTFPlayer::UpdateSecondsTaken( void )
{
	float fSecondsSinceLastUpdate = ( gpGlobals->curtime - m_fTimeLastNumSecondsUpdate );
	m_StatsThisLevel.fNumSecondsTaken += fSecondsSinceLastUpdate;
	m_fTimeLastNumSecondsUpdate = gpGlobals->curtime;

	if ( m_iBonusChallenge == PORTAL_CHALLENGE_TIME )
		SetBonusProgress( static_cast<int>( m_StatsThisLevel.fNumSecondsTaken ) );

	if ( m_fNeuroToxinDamageTime > 0.0f )
	{
		float fTimeRemaining = m_fNeuroToxinDamageTime - gpGlobals->curtime;

		if ( fTimeRemaining < 0.0f )
		{
			CTakeDamageInfo info;
			info.SetDamage( gpGlobals->frametime * 50.0f );
			info.SetDamageType( DMG_NERVEGAS );
			TakeDamage( info );
			fTimeRemaining = 0.0f;
		}

		PauseBonusProgress( false );
		SetBonusProgress( static_cast<int>( fTimeRemaining ) );
	}
}

void CTFPlayer::ResetThisLevelStats( void )
{
	m_StatsThisLevel.iNumPortalsPlaced = 0;
	m_StatsThisLevel.iNumStepsTaken = 0;
	m_StatsThisLevel.fNumSecondsTaken = 0.0f;

	if ( m_iBonusChallenge != PORTAL_CHALLENGE_NONE )
		SetBonusProgress( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Update the area bits variable which is networked down to the client to determine
//			which area portals should be closed based on visibility.
// Input  : *pvs - pvs to be used to determine visibility of the portals
//-----------------------------------------------------------------------------
void CTFPlayer::UpdatePortalViewAreaBits( unsigned char *pvs, int pvssize )
{
	Assert ( pvs );

	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	if( iPortalCount == 0 )
		return;

	CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
	int *portalArea = (int *)stackalloc( sizeof( int ) * iPortalCount );
	bool *bUsePortalForVis = (bool *)stackalloc( sizeof( bool ) * iPortalCount );

	unsigned char *portalTempBits = (unsigned char *)stackalloc( sizeof( unsigned char ) * 32 * iPortalCount );
	COMPILE_TIME_ASSERT( (sizeof( unsigned char ) * 32) >= sizeof( ((CPlayerLocalData*)0)->m_chAreaBits ) );

	// setup area bits for these portals
	for ( int i = 0; i < iPortalCount; ++i )
	{
		CProp_Portal* pLocalPortal = pPortals[ i ];
		// Make sure this portal is active before adding it's location to the pvs
		if ( pLocalPortal && pLocalPortal->m_bActivated )
		{
			CProp_Portal* pRemotePortal = pLocalPortal->m_hLinkedPortal.Get();

			// Make sure this portal's linked portal is in the PVS before we add what it can see
			if ( pRemotePortal && pRemotePortal->m_bActivated && pRemotePortal->NetworkProp() && 
				pRemotePortal->NetworkProp()->IsInPVS( edict(), pvs, pvssize ) )
			{
				portalArea[ i ] = engine->GetArea( pPortals[ i ]->GetAbsOrigin() );

				if ( portalArea [ i ] >= 0 )
				{
					bUsePortalForVis[ i ] = true;
				}

				engine->GetAreaBits( portalArea[ i ], &portalTempBits[ i * 32 ], sizeof( unsigned char ) * 32 );
			}
		}
	}

	// Use the union of player-view area bits and the portal-view area bits of each portal
	for ( int i = 0; i < m_Local.m_chAreaBits.Count(); i++ )
	{
		for ( int j = 0; j < iPortalCount; ++j )
		{
			// If this portal is active, in PVS and it's location is valid
			if ( bUsePortalForVis[ j ]  )
			{
				m_Local.m_chAreaBits.Set( i, m_Local.m_chAreaBits[ i ] | portalTempBits[ (j * 32) + i ] );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AddPortalCornersToEnginePVS
// Subroutine to wrap the adding of portal corners to the PVS which is called once for the setup of each portal.
// input - pPortal: the portal we are viewing 'out of' which needs it's corners added to the PVS
//////////////////////////////////////////////////////////////////////////
void AddPortalCornersToEnginePVS( CProp_Portal* pPortal )
{
	Assert ( pPortal );

	if ( !pPortal )
		return;

	Vector vForward, vRight, vUp;
	pPortal->GetVectors( &vForward, &vRight, &vUp );

	// Center of the remote portal
	Vector ptOrigin			= pPortal->GetAbsOrigin();

	// Distance offsets to the different edges of the portal... Used in the placement checks
	Vector vToTopEdge = vUp * ( PORTAL_HALF_HEIGHT - PORTAL_BUMP_FORGIVENESS );
	Vector vToBottomEdge = -vToTopEdge;
	Vector vToRightEdge = vRight * ( PORTAL_HALF_WIDTH - PORTAL_BUMP_FORGIVENESS );
	Vector vToLeftEdge = -vToRightEdge;

	// Distance to place PVS points away from portal, to avoid being in solid
	Vector vForwardBump		= vForward * 1.0f;

	// Add center and edges to the engine PVS
	engine->AddOriginToPVS( ptOrigin + vForwardBump);
	engine->AddOriginToPVS( ptOrigin + vToTopEdge + vToLeftEdge + vForwardBump );
	engine->AddOriginToPVS( ptOrigin + vToTopEdge + vToRightEdge + vForwardBump );
	engine->AddOriginToPVS( ptOrigin + vToBottomEdge + vToLeftEdge + vForwardBump );
	engine->AddOriginToPVS( ptOrigin + vToBottomEdge + vToRightEdge + vForwardBump );
}

void PortalSetupVisibility( CBaseEntity *pPlayer, int area, unsigned char *pvs, int pvssize )
{
	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	if( iPortalCount == 0 )
		return;

	CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
	for( int i = 0; i != iPortalCount; ++i )
	{
		CProp_Portal *pPortal = pPortals[i];

		if ( pPortal && pPortal->m_bActivated )
		{
			if ( pPortal->NetworkProp()->IsInPVS( pPlayer->edict(), pvs, pvssize ) )
			{
				if ( engine->CheckAreasConnected( area, pPortal->NetworkProp()->AreaNum() ) )
				{
					CProp_Portal *pLinkedPortal = static_cast<CProp_Portal*>( pPortal->m_hLinkedPortal.Get() );
					if ( pLinkedPortal )
					{
						AddPortalCornersToEnginePVS ( pLinkedPortal );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );

	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
	PointCameraSetupVisibility( this, area, pvs, pvssize );

	// If the intro script is playing, we want to get it's visibility points
	if ( g_hIntroScript )
	{
		Vector vecOrigin;
		CBaseEntity *pCamera;
		if ( g_hIntroScript->GetIncludedPVSOrigin( &vecOrigin, &pCamera ) )
		{
			// If it's a point camera, turn it on
			CPointCamera *pPointCamera = dynamic_cast< CPointCamera* >(pCamera); 
			if ( pPointCamera )
				pPointCamera->SetActive( true );

			engine->AddOriginToPVS( vecOrigin );
		}
	}

	// At this point the EyePosition has been added as a view origin, but if we are currently stuck
	// in a portal, our EyePosition may return a point in solid. Find the reflected eye position
	// and use that as a vis origin instead.
	if ( m_hPortalEnvironment )
	{
		CProp_Portal *pPortal = NULL, *pRemotePortal = NULL;
		pPortal = m_hPortalEnvironment;
		pRemotePortal = pPortal->m_hLinkedPortal;

		if ( pPortal && pRemotePortal && pPortal->m_bActivated && pRemotePortal->m_bActivated )
		{		
			Vector ptPortalCenter = pPortal->GetAbsOrigin();
			Vector vPortalForward;
			pPortal->GetVectors( &vPortalForward, NULL, NULL );

			Vector eyeOrigin = EyePosition();
			Vector vEyeToPortalCenter = ptPortalCenter - eyeOrigin;

			float fPortalDist = vPortalForward.Dot( vEyeToPortalCenter );
			if( fPortalDist > 0.0f ) //eye point is behind portal
			{
				// Move eye origin to it's transformed position on the other side of the portal
				UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), eyeOrigin, eyeOrigin );

				// Use this as our view origin (as this is where the client will be displaying from)
				engine->AddOriginToPVS( eyeOrigin );
				if ( !pViewEntity || pViewEntity->IsPlayer() )
				{
					area = engine->GetArea( eyeOrigin );
				}	
			}
		}
	}

	PortalSetupVisibility( this, area, pvs, pvssize );
}

CON_COMMAND( displayportalplayerstats, "Displays current level stats for portals placed, steps taken, and seconds taken." )
{
	CTFPlayer *pPlayer = (CTFPlayer *)UTIL_GetCommandClient();
	if( pPlayer == NULL )
		return;

	if( pPlayer )
	{
		int iMinutes = static_cast<int>( pPlayer->NumSecondsTaken() / 60.0f );
		int iSeconds = static_cast<int>( pPlayer->NumSecondsTaken() ) % 60;

		CFmtStr msg;
		NDebugOverlay::ScreenText( 0.5f, 0.5f, msg.sprintf( "Portals Placed: %d\nSteps Taken: %d\nTime: %d:%d", pPlayer->NumPortalsPlaced(), pPlayer->NumStepsTaken(), iMinutes, iSeconds ), 255, 255, 255, 150, 5.0f );
	}
}

CON_COMMAND( startneurotoxins, "Starts the nerve gas timer." )
{
	CTFPlayer *pPlayer = ToTFPlayer( UTIL_GetCommandClient() );
	if ( sv_cheats->GetInt() != 0 || pPlayer->IsDeveloper() )
		{
		float fCoundownTime = 180.0f;

		if ( args.ArgC() > 1 )
			fCoundownTime = atof( args[ 1 ] );

		if( pPlayer )
			pPlayer->SetNeuroToxinDamageTime( fCoundownTime );
	}
}

//---------------------------------------------------------
// Purpose: return the position of my head
//---------------------------------------------------------
Vector CTFPlayer::HeadTarget( const Vector &posSrc )
{
	int iHeadAttachment = LookupAttachment( "head" );
	if ( iHeadAttachment < 0 )
		return EyePosition();

	Vector vecHead;
	GetAttachment( iHeadAttachment, vecHead );
	return vecHead;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CTFPlayer::EyeDirection2D( void )
{
	Vector vecReturn = EyeDirection3D();
	vecReturn.z = 0;
	vecReturn.AsVector2D().NormalizeInPlace();

	return vecReturn;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CTFPlayer::EyeDirection3D( void )
{
	Vector vecForward;

	// Return the vehicle angles if we request them
	if ( GetVehicle() != NULL )
	{
		CacheVehicleView();
		EyeVectors( &vecForward );
		return vecForward;
	}
	
	AngleVectors( EyeAngles(), &vecForward );
	return vecForward;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::NotifyFriendsOfDamage( CBaseEntity *pAttackerEntity )
{
	CAI_BaseNPC *pAttacker = pAttackerEntity->MyNPCPointer();
	if ( pAttacker )
	{
		const Vector &origin = GetAbsOrigin();
		for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
		{
			const float NEAR_Z = 12*12;
			const float NEAR_XY_SQ = Square( 50*12 );
			CAI_BaseNPC *pNpc = g_AI_Manager.AccessAIs()[i];
			if ( pNpc->IsPlayerAlly() )
			{
				const Vector &originNpc = pNpc->GetAbsOrigin();
				if ( fabsf( originNpc.z - origin.z ) < NEAR_Z )
				{
					if ( ( originNpc.AsVector2D() - origin.AsVector2D() ).LengthSqr() < NEAR_XY_SQ )
					{
						pNpc->OnFriendDamaged( this, pAttacker );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CTFPlayer::CommanderFindGoal( commandgoal_t *pGoal )
{
	CAI_BaseNPC *pAllyNpc;
	trace_t	tr;
	Vector	vecTarget;
	Vector	forward;

	EyeVectors( &forward );
	
	//---------------------------------
	// MASK_SHOT on purpose! So that you don't hit the invisible hulls of the NPCs.
	CTraceFilterSkipTwoEntities filter( this, PhysCannonGetHeldEntity( GetActiveWeapon() ), COLLISION_GROUP_INTERACTIVE_DEBRIS );

	Ray_t ray;
	ray.Init( EyePosition(), EyePosition() + forward * MAX_COORD_RANGE );
	UTIL_Portal_TraceRay( ray, MASK_SHOT, &filter, &tr );

	if( !tr.DidHitWorld() )
	{
		CUtlVector<CAI_BaseNPC *> Allies;
		AISquadIter_t iter;
		for ( pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
		{
			if ( pAllyNpc->IsCommandable() )
				Allies.AddToTail( pAllyNpc );
		}

		for( int i = 0 ; i < Allies.Count() ; i++ )
		{
			if( Allies[ i ]->IsValidCommandTarget( tr.m_pEnt ) )
			{
				pGoal->m_pGoalEntity = tr.m_pEnt;
				return true;
			}
		}
	}

	if( tr.fraction == 1.0 || (tr.surface.flags & SURF_SKY) )
	{
		// Move commands invalid against skybox.
		pGoal->m_vecGoalLocation = tr.endpos;
		return false;
	}

	if ( tr.m_pEnt->IsNPC() && ((CAI_BaseNPC *)(tr.m_pEnt))->IsCommandable() )
	{
		pGoal->m_vecGoalLocation = tr.m_pEnt->GetAbsOrigin();
	}
	else
	{
		vecTarget = tr.endpos;

		Vector mins( -16, -16, 0 );
		Vector maxs( 16, 16, 0 );

		// Back up from whatever we hit so that there's enough space at the 
		// target location for a bounding box.
		// Now trace down. 
		//UTIL_TraceLine( vecTarget, vecTarget - Vector( 0, 0, 8192 ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_TraceHull( vecTarget + tr.plane.normal * 24,
						vecTarget - Vector( 0, 0, 8192 ),
						mins,
						maxs,
						MASK_SOLID_BRUSHONLY,
						this,
						COLLISION_GROUP_NONE,
						&tr );


		if ( !tr.startsolid )
			pGoal->m_vecGoalLocation = tr.endpos;
		else
			pGoal->m_vecGoalLocation = vecTarget;
	}

	pAllyNpc = GetSquadCommandRepresentative();
	if ( !pAllyNpc )
		return false;

	vecTarget = pGoal->m_vecGoalLocation;
	if ( !pAllyNpc->FindNearestValidGoalPos( vecTarget, &pGoal->m_vecGoalLocation ) )
		return false;

	return ( ( vecTarget - pGoal->m_vecGoalLocation ).LengthSqr() < Square( 15*12 ) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CAI_BaseNPC *CTFPlayer::GetSquadCommandRepresentative()
{
	if ( m_pPlayerAISquad != NULL )
	{
		CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember();
		
		if ( pAllyNpc )
		{
			return pAllyNpc->GetSquadCommandRepresentative();
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CTFPlayer::GetNumSquadCommandables()
{
	AISquadIter_t iter;
	int c = 0;
	for ( CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
	{
		if ( pAllyNpc->IsCommandable() && (InSameTeam( pAllyNpc ) || m_Shared.GetDisguiseTeam() == pAllyNpc->GetTeamNumber()) )
			c++;
	}
	return c;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CTFPlayer::GetNumSquadCommandableMedics()
{
	AISquadIter_t iter;
	int c = 0;
	for ( CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
	{
		if ( pAllyNpc->IsCommandable() && pAllyNpc->IsMedic() )
			c++;
	}
	return c;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CTFPlayer::CommanderUpdate()
{
	CAI_BaseNPC *pCommandRepresentative = GetSquadCommandRepresentative();
	bool bFollowMode = false;
	if ( pCommandRepresentative )
	{
		bFollowMode = ( pCommandRepresentative->GetCommandGoal() == vec3_invalid );

		// set the variables for network transmission (to show on the hud)
		m_iSquadMemberCount = GetNumSquadCommandables();
		m_iSquadMedicCount = GetNumSquadCommandableMedics();
		m_fSquadInFollowMode = bFollowMode;

		// debugging code for displaying extra squad indicators
		/*
		char *pszMoving = "";
		AISquadIter_t iter;
		for ( CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
		{
			if ( pAllyNpc->IsCommandMoving() )
			{
				pszMoving = "<-";
				break;
			}
		}

		NDebugOverlay::ScreenText(
			0.932, 0.919, 
			CFmtStr( "%d|%c%s", GetNumSquadCommandables(), ( bFollowMode ) ? 'F' : 'S', pszMoving ),
			255, 128, 0, 128,
			0 );
		*/

	}
	else
	{
		m_iSquadMemberCount = 0;
		m_iSquadMedicCount = 0;
		m_fSquadInFollowMode = true;
	}

	if ( m_QueuedCommand != CC_NONE && ( m_QueuedCommand == CC_FOLLOW || gpGlobals->realtime - m_RealTimeLastSquadCommand >= player_squad_double_tap_time.GetFloat() ) )
	{
		CommanderExecute( m_QueuedCommand );
		m_QueuedCommand = CC_NONE;
	}
	else if ( !bFollowMode && pCommandRepresentative && m_CommanderUpdateTimer.Expired() && player_squad_transient_commands.GetBool() )
	{
		m_CommanderUpdateTimer.Set(2.5);

		if ( pCommandRepresentative->ShouldAutoSummon() )
			CommanderExecute( CC_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// bHandled - indicates whether to continue delivering this order to
// all allies. Allows us to stop delivering certain types of orders once we find
// a suitable candidate. (like picking up a single weapon. We don't wish for
// all allies to respond and try to pick up one weapon).
//----------------------------------------------------------------------------- 
bool CTFPlayer::CommanderExecuteOne( CAI_BaseNPC *pNpc, const commandgoal_t &goal, CAI_BaseNPC **Allies, int numAllies )
{
	if ( goal.m_pGoalEntity )
	{
		return pNpc->TargetOrder( goal.m_pGoalEntity, Allies, numAllies );
	}
	else if ( pNpc->IsInPlayerSquad() )
	{
		pNpc->MoveOrder( goal.m_vecGoalLocation, Allies, numAllies );
	}
	
	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CTFPlayer::CommanderExecute( CommanderCommand_t command )
{
	CAI_BaseNPC *pPlayerSquadLeader = GetSquadCommandRepresentative();

	if ( !pPlayerSquadLeader )
	{
		//EmitSound( "HL2Player.UseDeny" );
		return;
	}

	int i;
	CUtlVector<CAI_BaseNPC *> Allies;
	commandgoal_t goal;

	if ( command == CC_TOGGLE )
	{
		if ( pPlayerSquadLeader->GetCommandGoal() != vec3_invalid )
			command = CC_FOLLOW;
		else
			command = CC_SEND;
	}
	else
	{
		if ( command == CC_FOLLOW && pPlayerSquadLeader->GetCommandGoal() == vec3_invalid )
			return;
	}

	if ( command == CC_FOLLOW )
	{
		goal.m_pGoalEntity = this;
		goal.m_vecGoalLocation = vec3_invalid;
	}
	else
	{
		goal.m_pGoalEntity = NULL;
		goal.m_vecGoalLocation = vec3_invalid;

		// Find a goal for ourselves.
		if( !CommanderFindGoal( &goal ) )
		{
			CSingleUserRecipientFilter filter( this );
			EmitSound_t params;
			params.m_flSoundTime = 0;
			params.m_pSoundName = "HL2Player.UseDeny";
			EmitSound( filter, entindex(), params );
			return; // just keep following
		}
	}

#ifdef _DEBUG
	if( goal.m_pGoalEntity == NULL && goal.m_vecGoalLocation == vec3_invalid )
	{
		DevMsg( 1, "**ERROR: Someone sent an invalid goal to CommanderExecute!\n" );
	}
#endif // _DEBUG

	AISquadIter_t iter;
	for ( CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
	{
		if ( pAllyNpc->IsCommandable() )
			Allies.AddToTail( pAllyNpc );
	}

	//---------------------------------
	// If the trace hits an NPC, send all ally NPCs a "target" order. Always
	// goes to targeted one first
#ifdef DBGFLAG_ASSERT
	int nAIs = g_AI_Manager.NumAIs();
#endif
	CAI_BaseNPC * pTargetNpc = (goal.m_pGoalEntity) ? goal.m_pGoalEntity->MyNPCPointer() : NULL;
	
	bool bHandled = false;
	if( pTargetNpc )
	{
		bHandled = !CommanderExecuteOne( pTargetNpc, goal, Allies.Base(), Allies.Count() );
	}
	
	for ( i = 0; !bHandled && i < Allies.Count(); i++ )
	{
		if ( Allies[i] != pTargetNpc && Allies[i]->IsPlayerAlly() )
		{
			bHandled = !CommanderExecuteOne( Allies[i], goal, Allies.Base(), Allies.Count() );
		}
		Assert( nAIs == g_AI_Manager.NumAIs() ); // not coded to support mutating set of NPCs
	}
}

//-----------------------------------------------------------------------------
// Enter/exit commander mode, manage ally selection.
//-----------------------------------------------------------------------------
void CTFPlayer::CommanderMode()
{
	float commandInterval = gpGlobals->realtime - m_RealTimeLastSquadCommand;
	m_RealTimeLastSquadCommand = gpGlobals->realtime;
	if ( commandInterval < player_squad_double_tap_time.GetFloat() )
	{
		m_QueuedCommand = CC_FOLLOW;
	}
	else
	{
		m_QueuedCommand = (player_squad_transient_commands.GetBool()) ? CC_SEND : CC_TOGGLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Save Health, Ammo, Class and Current Weapon by SteamID
//-----------------------------------------------------------------------------
void CTFPlayer::SaveForTransition( void )
{
	if ( !lfe_allow_transition.GetBool() )
		return;

	if ( !IsAlive() )
		return;

	if ( TFGameRules()->IsCoOp() && !IsOnStoryTeam() )
		return;
	else if ( TFGameRules()->IsBluCoOp() && !IsOnCombineTeam() )
		return;

	TFPlayerTransitionStruct transition;

	transition.playerClass = GetDesiredPlayerClassIndex();

	if ( IsPlayerClass( TF_CLASS_MEDIC ) )
		transition.ubercharge = MedicGetChargeLevel();

	if ( GetActiveTFWeapon() && GetActiveTFWeapon()->HasChargeBar() )
		transition.itemmeter = m_Shared.GetRageProgress();

	int iTotalKills = 0;
	PlayerStats_t *pStats = CTF_GameStats.FindPlayerStats( this );
	if ( pStats )
	{
		iTotalKills = pStats->statsCurrentLife.m_iStat[TFSTAT_KILLS] + pStats->statsCurrentLife.m_iStat[TFSTAT_KILLASSISTS]+ 
			pStats->statsCurrentLife.m_iStat[TFSTAT_BUILDINGSDESTROYED];
			
		transition.kills = iTotalKills;
	}

	g_TFPlayerTransitions.InsertOrReplace( GetSteamIDAsUInt64(), transition );

	ConColorMsg( 1, Color( 77, 116, 85, 255 ), "[Transition] Saving %s.\n", GetPlayerName() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::LoadSavedTransition( void )
{
	if ( !lfe_allow_transition.GetBool() )
		return;

	unsigned short index = g_TFPlayerTransitions.Find( GetSteamIDAsUInt64() );
	if ( index == g_TFPlayerTransitions.InvalidIndex() )
		return;

	if ( g_TFPlayerTransitions[index].playerClass != GetDesiredPlayerClassIndex() )
	{
		DeleteForTransition();
		return;
	}

	ConColorMsg( 1, Color( 77, 116, 85, 255 ), "[Transition] Load %s.\n", GetPlayerName() );

	// Restore ubercharge.
	if ( IsPlayerClass( TF_CLASS_MEDIC ) )
	{
		CWeaponMedigun *pMedigun = GetMedigun();
		pMedigun->SetCharge( g_TFPlayerTransitions[index].ubercharge );
	}

	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
		if ( pWeapon )
		{
			// Restore item's meter.
			if ( pWeapon->HasChargeBar() )
				m_Shared.m_flEffectBarProgress = g_TFPlayerTransitions[index].itemmeter;
		}
	}

	CTF_GameStats.IncrementStat( this, TFSTAT_KILLS, g_TFPlayerTransitions[index].kills );

	// Remove player info from the list.
	DeleteForTransition();
}

//-----------------------------------------------------------------------------
// Purpose: Delete Health, Ammo, Class and Current Weapon After Spawned
//-----------------------------------------------------------------------------
void CTFPlayer::DeleteForTransition( void )
{
	ConColorMsg( 1, Color( 77, 116, 85, 255 ), "[Transition] Removing %s.\n", GetPlayerName() );

	g_TFPlayerTransitions.Remove( GetSteamIDAsUInt64() );
}

//=========================================================
// UpdatePlayerSound - updates the position of the player's
// reserved sound slot in the sound list.
//=========================================================
void CTFPlayer::UpdatePlayerSound ( void )
{
	CSound *pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );

	if ( !pSound )
	{
		Msg( "Client lost reserved sound!\n" );
		return;
	}

	// Moves silently while invisible or disguise.
	if ( m_Shared.IsStealthed() || m_Shared.InCond( TF_COND_DISGUISED ) )
	{
		pSound->m_iVolume = 0;
		return;
	}

	// if we're holding tomislav npc won't notice.
	CTFWeaponBase *pWeapon = GetActiveTFWeapon();
	int nMinigunNoSound = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nMinigunNoSound, minigun_no_spin_sounds );

	if ( nMinigunNoSound && m_Shared.InCond( TF_COND_AIMING ) )
	{
		pSound->m_iVolume = 0;
		return;
	}

	BaseClass::UpdatePlayerSound();
}

//-----------------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------------
void CTFPlayer::ResetPerRoundStats( void )
{
	// Reset lives count.
	m_Shared.SetLivesCount( lfe_player_lives.GetInt() );
}

extern ConVar lfe_weapon_progression;
//-----------------------------------------------------------------------------------
// Purpose: TF players must NOT be able to use non-TF weapons or the game will CRASH!
//-----------------------------------------------------------------------------------
bool CTFPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	bool bSuccess = false;
	auto pTFWeapon = dynamic_cast< CTFWeaponBase * >( pWeapon );
	CLFWeaponManager *pManager = static_cast<CLFWeaponManager*>(gEntList.FindEntityByClassname(NULL, "lfe_weapon_progress"));

	if ( pWeapon && !pTFWeapon && pManager && lfe_weapon_progression.GetBool() )
	{
		bool bProgressChanged = false;
		if (pWeapon && !pWeapon->ClassMatches("tf_weapon_portalgun") && !pWeapon->ClassMatches("tf_weapon_physcannon"))
		{
			if ((pWeapon->ClassMatches("weapon_crowbar") || pWeapon->ClassMatches("weapon_stunstick")) && pManager->m_nWeaponProgress < 1 || !pWeapon->ClassMatches("weapon_crowbar") && !pWeapon->ClassMatches("weapon_stunstick") && pManager->m_nWeaponProgress == 1)
			{
				pManager->SetThink(NULL);
				++pManager->m_nWeaponProgress;
				pManager->SetThink(&CLFWeaponManager::AllowedWeaponThink);
				pManager->SetNextThink(gpGlobals->curtime);
				bProgressChanged = true;
			}
			UTIL_Remove(pWeapon);
		}

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CTFPlayer *pTeamPlayer = ToTFPlayer( UTIL_PlayerByIndex(i) );
			if ( !pTeamPlayer || !pTeamPlayer->IsAlive() )
				continue;

			if ( bProgressChanged )
				pTeamPlayer->GiveDefaultItems();
		}

		bSuccess = true;
		return bSuccess;
	}

	if ( pTFWeapon != NULL )
		return BaseClass::BumpWeapon( pTFWeapon );

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	// delet this
	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	//if( pWeapon->FVisible( this, MASK_SOLID ) == false && !(GetFlags() & FL_NOTARGET) )
	//	return false;

	// Make them restore 10% ammo for players.

	int iMaxPrimary = GetPlayerClass()->GetData()->m_aAmmoMax[TF_AMMO_PRIMARY];
	if ( GiveAmmo( ceil(iMaxPrimary * 0.10f), TF_AMMO_PRIMARY ) )
		bSuccess = true;
	
	int iMaxSecondary = GetPlayerClass()->GetData()->m_aAmmoMax[TF_AMMO_SECONDARY];
	if ( GiveAmmo( ceil(iMaxSecondary * 0.10f), TF_AMMO_SECONDARY ) )
		bSuccess = true;

	int iMaxMetal = GetPlayerClass()->GetData()->m_aAmmoMax[TF_AMMO_METAL];
	if ( GiveAmmo( ceil(iMaxMetal * 0.125f), TF_AMMO_METAL ) )
		bSuccess = true;
	
	int iMaxGrenades1 = GetPlayerClass()->GetData()->m_aAmmoMax[LFE_AMMO_GRENADES1];
	if ( GiveAmmo( ceil(iMaxGrenades1 * 0.10f), LFE_AMMO_GRENADES1 ) )
		bSuccess = true;

	int iMaxGrenades2 = GetPlayerClass()->GetData()->m_aAmmoMax[LFE_AMMO_GRENADES2];
	if ( GiveAmmo( ceil(iMaxGrenades2 * 0.10f), LFE_AMMO_GRENADES2 ) )
		bSuccess = true;

	int iNoItems = 0;
	int iCloakWhenCloaked = 0;
	float flReducedCloakFromAmmo = 0;
	CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pInvis, iNoItems, mod_cloak_no_regen_from_items );
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pInvis, iCloakWhenCloaked, NoCloakWhenCloaked );
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pInvis, flReducedCloakFromAmmo, ReducedCloakFromAmmo );

	float flCloak = m_Shared.GetSpyCloakMeter();
	if ( flCloak < 100.0f )
	{
		if ( !m_Shared.InCond( TF_COND_STEALTHED ) && iCloakWhenCloaked != 1 && iNoItems != 1 )
		{
			m_Shared.SetSpyCloakMeter( min( 100.0f, flCloak + min( 100.0f, flReducedCloakFromAmmo ) * 0.10f ) );
			bSuccess = true;
		}
	}

	// did we give them anything?
	if ( bSuccess )
	{
		UTIL_Remove( pWeapon );
	}

	return bSuccess;
}

void CTFPlayer::VPhysicsShadowUpdate( IPhysicsObject *pPhysics )
{
	if( m_hPortalEnvironment.Get() == NULL )
		return BaseClass::VPhysicsShadowUpdate( pPhysics );

	//below is mostly a cut/paste of existing CBasePlayer::VPhysicsShadowUpdate code with some minor tweaks to avoid getting stuck in stuff when in a portal environment
	if ( sv_turbophysics.GetBool() )
		return;

	Vector newPosition;

	bool physicsUpdated = m_pPhysicsController->GetShadowPosition( &newPosition, NULL ) > 0 ? true : false;

	// UNDONE: If the player is penetrating, but the player's game collisions are not stuck, teleport the physics shadow to the game position
	if ( pPhysics->GetGameFlags() & FVPHYSICS_PENETRATING )
	{
		CUtlVector<CBaseEntity *> list;
		PhysGetListOfPenetratingEntities( this, list );
		for ( int i = list.Count()-1; i >= 0; --i )
		{
			// filter out anything that isn't simulated by vphysics
			// UNDONE: Filter out motion disabled objects?
			if ( list[i]->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				// I'm currently stuck inside a moving object, so allow vphysics to 
				// apply velocity to the player in order to separate these objects
				m_touchedPhysObject = true;
			}
		}
	}

	if ( m_pPhysicsController->IsInContact() || (m_afPhysicsFlags & PFLAG_VPHYSICS_MOTIONCONTROLLER) )
	{
		m_touchedPhysObject = true;
	}

	if ( IsFollowingPhysics() )
	{
		m_touchedPhysObject = true;
	}

	if ( GetMoveType() == MOVETYPE_NOCLIP )
	{
		m_oldOrigin = GetAbsOrigin();
		return;
	}

	if ( phys_timescale.GetFloat() == 0.0f )
	{
		physicsUpdated = false;
	}

	if ( !physicsUpdated )
		return;

	IPhysicsObject *pPhysGround = GetGroundVPhysics();

	Vector newVelocity;
	pPhysics->GetPosition( &newPosition, 0 );
	m_pPhysicsController->GetShadowVelocity( &newVelocity );



	Vector tmp = GetAbsOrigin() - newPosition;
	if ( !m_touchedPhysObject && !(GetFlags() & FL_ONGROUND) )
	{
		tmp.z *= 0.5f;	// don't care about z delta as much
	}

	float dist = tmp.LengthSqr();
	float deltaV = (newVelocity - GetAbsVelocity()).LengthSqr();

	float maxDistErrorSqr = VPHYS_MAX_DISTSQR;
	float maxVelErrorSqr = VPHYS_MAX_VELSQR;
	if ( IsRideablePhysics(pPhysGround) )
	{
		maxDistErrorSqr *= 0.25;
		maxVelErrorSqr *= 0.25;
	}

	if ( dist >= maxDistErrorSqr || deltaV >= maxVelErrorSqr || (pPhysGround && !m_touchedPhysObject) )
	{
		if ( m_touchedPhysObject || pPhysGround )
		{
			// BUGBUG: Rewrite this code using fixed timestep
			if ( deltaV >= maxVelErrorSqr )
			{
				Vector dir = GetAbsVelocity();
				float len = VectorNormalize(dir);
				float dot = DotProduct( newVelocity, dir );
				if ( dot > len )
				{
					dot = len;
				}
				else if ( dot < -len )
				{
					dot = -len;
				}

				VectorMA( newVelocity, -dot, dir, newVelocity );

				if ( m_afPhysicsFlags & PFLAG_VPHYSICS_MOTIONCONTROLLER )
				{
					float val = Lerp( 0.1f, len, dot );
					VectorMA( newVelocity, val - len, dir, newVelocity );
				}

				if ( !IsRideablePhysics(pPhysGround) )
				{
					if ( !(m_afPhysicsFlags & PFLAG_VPHYSICS_MOTIONCONTROLLER ) && IsSimulatingOnAlternateTicks() )
					{
						newVelocity *= 0.5f;
					}
					ApplyAbsVelocityImpulse( newVelocity );
				}
			}

			trace_t trace;
			UTIL_TraceEntity( this, newPosition, newPosition, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );
			if ( !trace.allsolid && !trace.startsolid )
			{
				SetAbsOrigin( newPosition );
			}
		}
		else
		{
			trace_t trace;

			Ray_t ray;
			ray.Init( GetAbsOrigin(), GetAbsOrigin(), WorldAlignMins(), WorldAlignMaxs() );

			CTraceFilterSimple OriginalTraceFilter( this, COLLISION_GROUP_PLAYER_MOVEMENT );
			CTraceFilterTranslateClones traceFilter( &OriginalTraceFilter );
			UTIL_Portal_TraceRay_With( m_hPortalEnvironment, ray, MASK_PLAYERSOLID, &traceFilter, &trace );

			// current position is not ok, fixup
			if ( trace.allsolid || trace.startsolid )
			{
				//try again with new position
				ray.Init( newPosition, newPosition, WorldAlignMins(), WorldAlignMaxs() );
				UTIL_Portal_TraceRay_With( m_hPortalEnvironment, ray, MASK_PLAYERSOLID, &traceFilter, &trace );

				if( trace.startsolid == false )
				{
					SetAbsOrigin( newPosition );
				}
				else
				{
					if( !FindClosestPassableSpace( this, newPosition - GetAbsOrigin(), MASK_PLAYERSOLID ) )
					{
						// Try moving the player closer to the center of the portal
						CProp_Portal *pPortal = m_hPortalEnvironment.Get();
						newPosition += ( pPortal->GetAbsOrigin() - WorldSpaceCenter() ) * 0.1f;
						SetAbsOrigin( newPosition );

						DevMsg( "Hurting the player for FindClosestPassableSpaceFailure!" );

						// Deal 1 damage per frame... this will kill a player very fast, but allow for the above correction to fix some cases
						CTakeDamageInfo info( this, this, vec3_origin, vec3_origin, 1, DMG_CRUSH );
						OnTakeDamage( info );
					}
				}
			}
		}
	}
	else
	{
		if ( m_touchedPhysObject )
		{
			// check my position (physics object could have simulated into my position
			// physics is not very far away, check my position
			trace_t trace;
			UTIL_TraceEntity( this, GetAbsOrigin(), GetAbsOrigin(),
				MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );

			// is current position ok?
			if ( trace.allsolid || trace.startsolid )
			{
				// stuck????!?!?
				//Msg("Stuck on %s\n", trace.m_pEnt->GetClassname());
				SetAbsOrigin( newPosition );
				UTIL_TraceEntity( this, GetAbsOrigin(), GetAbsOrigin(),
					MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );
				if ( trace.allsolid || trace.startsolid )
				{
					//Msg("Double Stuck\n");
					SetAbsOrigin( m_oldOrigin );
				}
			}
		}
	}
	m_oldOrigin = GetAbsOrigin();
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
Class_T  CTFPlayer::Classify ( void )
{
	if( IsInAVehicle() )
	{
		IServerVehicle *pVehicle = GetVehicle();
		return pVehicle->ClassifyPassenger( this, CLASS_PLAYER );
	}

	if ( GetTeamNumber() == TF_TEAM_BLUE )
		return CLASS_COMBINE;
	else if ( GetTeamNumber() == TF_TEAM_GREEN )
		return CLASS_ZOMBIE;
	else if ( GetTeamNumber() == TF_TEAM_YELLOW )
		return CLASS_ANTLION;

	return CLASS_PLAYER;
}

extern int	g_interactionBarnacleVictimDangle;
extern int	g_interactionBarnacleVictimReleased;
extern int	g_interactionBarnacleVictimGrab;

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CTFPlayer::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if ( interactionType == g_interactionBarnacleVictimDangle )
		return false;
	
	if (interactionType ==	g_interactionBarnacleVictimReleased)
	{
		m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
		SetMoveType( MOVETYPE_WALK );
		return true;
	}
	else if (interactionType ==	g_interactionBarnacleVictimGrab)
	{
		CNPC_Alyx *pAlyx = CNPC_Alyx::GetAlyx();
		if ( pAlyx )
		{
			// Make Alyx totally hate this barnacle so that she saves the player.
			int priority;

			priority = pAlyx->IRelationPriority(sourceEnt);
			pAlyx->AddEntityRelationship( sourceEnt, D_HT, priority + 5 );
		}

		m_afPhysicsFlags |= PFLAG_ONBARNACLE;
		ClearUseEntity();
		return true;
	}
	return false;
}

void CTFPlayer::PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
	if( m_bFixEyeAnglesFromPortalling )
	{
		//the idea here is to handle the notion that the player has portalled, but they sent us an angle update before receiving that message.
		//If we don't handle this here, we end up sending back their old angles which makes them hiccup their angles for a frame
		float fOldAngleDiff = fabs( AngleDistance( ucmd->viewangles.x, m_qPrePortalledViewAngles.x ) );
		fOldAngleDiff += fabs( AngleDistance( ucmd->viewangles.y, m_qPrePortalledViewAngles.y ) );
		fOldAngleDiff += fabs( AngleDistance( ucmd->viewangles.z, m_qPrePortalledViewAngles.z ) );

		float fCurrentAngleDiff = fabs( AngleDistance( ucmd->viewangles.x, pl.v_angle.x ) );
		fCurrentAngleDiff += fabs( AngleDistance( ucmd->viewangles.y, pl.v_angle.y ) );
		fCurrentAngleDiff += fabs( AngleDistance( ucmd->viewangles.z, pl.v_angle.z ) );

		if( fCurrentAngleDiff > fOldAngleDiff )
			ucmd->viewangles = TransformAnglesToWorldSpace( ucmd->viewangles, m_matLastPortalled.As3x4() );

		m_bFixEyeAnglesFromPortalling = false;
	}

	// Handle FL_FROZEN.
	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		ucmd->upmove = 0;
		ucmd->buttons &= ~IN_USE;
	}

	// Can't use stuff while dead
	/*if ( IsDead() )
	{
		ucmd->buttons &= ~IN_USE;
	}*/


	//Msg("Player time: [ACTIVE: %f]\t[IDLE: %f]\n", m_flMoveTime, m_flIdleTime );

	BaseClass::PlayerRunCommand( ucmd, moveHelper );
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CTFPlayer::MissedAR2AltFire()
{
	/* somebody fixPassesDamageFilter this plz
	if( GetPlayerProxy() != NULL )
	{
		GetPlayerProxy()->m_PlayerMissedAR2AltFire.FireOutput( this, this );
	}
	*/
}

//-----------------------------------------------------------------------------
const impactdamagetable_t &CTFPlayer::GetPhysicsImpactDamageTable()
{
	if ( m_bUseCappedPhysicsDamageTable )
		return gCappedPlayerImpactDamageTable;
	
	return BaseClass::GetPhysicsImpactDamageTable();
}

//-----------------------------------------------------------------------------
// Purpose: returns max health for this player
//-----------------------------------------------------------------------------
int CTFPlayer::GetMaxHealth( void ) const
{
	int iMaxHealth = GetMaxHealthForBuffing();

	if ( GetActiveTFWeapon() )
	{
		float flMaxHealthDrainRate = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetActiveTFWeapon(), flMaxHealthDrainRate, mod_maxhealth_drain_rate );
		if ( flMaxHealthDrainRate > 0 )
			iMaxHealth = tf_maxhealth_drain_hp_min.GetInt();
	}

	CALL_ATTRIB_HOOK_INT( iMaxHealth, add_maxhealth_nonbuffed );

	return iMaxHealth;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::GetMaxHealthForBuffing( void ) const
{
	int iMaxHealth = const_cast<CTFPlayerClass &>( m_PlayerClass ).GetMaxHealth();
	CALL_ATTRIB_HOOK_INT( iMaxHealth, add_maxhealth );

	CTFSword *pSword = dynamic_cast<CTFSword *>( const_cast<CTFPlayer *>( this )->Weapon_OwnsThisID( TF_WEAPON_SWORD ) );
	if ( pSword )
		iMaxHealth += pSword->GetSwordHealthMod();

	if ( const_cast<CTFPlayerShared &>( m_Shared ).InCond( TF_COND_HALLOWEEN_GIANT ) )
		iMaxHealth *= tf_halloween_giant_health_scale.GetFloat();

	if ( const_cast<CTFPlayerShared &>( m_Shared ).InCond( TF_COND_RUNE_WARLOCK ) )
		iMaxHealth = 400;

	if ( const_cast<CTFPlayerShared &>( m_Shared ).InCond( TF_COND_RUNE_KNOCKOUT ) )
		iMaxHealth += 150;

	return iMaxHealth;
}

//-----------------------------------------------------------------------------
// sigsegv-mvm's reverse engie
//-----------------------------------------------------------------------------
float CTFPlayer::GetDesiredHeadScale()
{
	float flScale = 1.0f;
	if ( m_Shared.InCond( TF_COND_BALLOON_HEAD ) || m_Shared.InCond( TF_COND_HALLOWEEN_KART ) || m_Shared.InCond( TF_COND_MELEE_ONLY ) )
		flScale = 1.4f;

	CALL_ATTRIB_HOOK_FLOAT( flScale, head_scale );
	return flScale;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::GetDesiredTorsoScale()
{
	float flScale = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT( flScale, torso_scale );
	return flScale;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::GetDesiredHandScale()
{
	float flScale = 1.0f;
	if ( m_Shared.InCond( TF_COND_MELEE_ONLY ) )
		flScale = 1.4f;

	CALL_ATTRIB_HOOK_FLOAT( flScale, hand_scale );
	return flScale;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::GetHeadScaleSpeed()
{
	/* scale instantly if in any of these conditions */
	if ( m_Shared.InCond( TF_COND_HALLOWEEN_BOMB_HEAD ) )
		return GetDesiredHeadScale();

	if ( m_Shared.InCond( TF_COND_MELEE_ONLY ) )
		return GetDesiredHeadScale();

	if ( m_Shared.InCond( TF_COND_HALLOWEEN_KART ) )
		return GetDesiredHeadScale();

	if ( m_Shared.InCond( TF_COND_BALLOON_HEAD ) )
		return GetDesiredHeadScale();

	/* otherwise, scale at approximately 1.0x per second */
	return gpGlobals->frametime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::GetTorsoScaleSpeed()
{
	/* scale at approximately 1.0x per second */
	return gpGlobals->frametime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::GetHandScaleSpeed()
{
	/* scale instantly if in this condition */
	if ( m_Shared.InCond( TF_COND_MELEE_ONLY ) )
		return GetDesiredHandScale();

	/* otherwise, scale at approximately 1.0x per second */
	return gpGlobals->frametime;
}

//-----------------------------------------------------------------------------
// Purpose: Add an attribute
//-----------------------------------------------------------------------------
void CTFPlayer::AddCustomAttribute( char const *pAttribute, float flValue, float flDuration )
{
	CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pAttribute );
	if ( pAttrib )
		GetAttributeList()->SetRuntimeAttributeValue( pAttrib, flValue );

	int iAttributeID = GetItemSchema()->GetAttributeIndex( pAttribute );
	CEconItemAttribute econAttribute( iAttributeID, flValue );

	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
		if ( pWeapon )
		{
			pWeapon->GetItem()->AddAttribute( &econAttribute );
			pWeapon->WeaponReset();
		}
	}

	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CTFWearable *pWearable = static_cast<CTFWearable *>( GetWearable( i ) );
		if ( pWearable )
		{
			pWearable->GetItem()->AddAttribute( &econAttribute );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update attributes
//-----------------------------------------------------------------------------
void CTFPlayer::UpdateCustomAttributes( void )
{
	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
		if ( pWeapon )
		{
			pWeapon->WeaponReset();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove an attribute
//-----------------------------------------------------------------------------
void CTFPlayer::RemoveCustomAttribute( char const *pAttribute )
{
	CEconAttributeDefinition *pAttrib = GetItemSchema()->GetAttributeDefinitionByName( pAttribute );
	if ( pAttrib )
		GetAttributeList()->RemoveAttribute( pAttrib );

	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
		if ( pWeapon && pWeapon->GetAttributeList() )
		{
			pWeapon->GetAttributeList()->RemoveAttribute( pAttrib );
		}
	}

	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CTFWearable *pWearable = static_cast<CTFWearable *>( GetWearable( i ) );
		if ( pWearable && pWearable->GetAttributeList() )
		{
			pWearable->GetAttributeList()->RemoveAttribute( pAttrib );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::OnAchievementEarned( int iAchievement )
{
	if ( !IsAlive() )
		return;

	SpeakConceptIfAllowed( MP_CONCEPT_ACHIEVEMENT_AWARD );

	/*CBaseAchievement *pAchievement = engine->GetAchievementMgr()->GetAchievementByID( iAchievement );
	if ( pAchievement )
	{
		
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Leeches
//-----------------------------------------------------------------------------
void CTFPlayer::StartWaterDeathSounds( void )
{
	CPASAttenuationFilter filter( this );

	if ( m_sndLeeches == NULL )
	{
		m_sndLeeches = ( CSoundEnvelopeController::GetController() ).SoundCreate( filter, entindex(), CHAN_STATIC, "coast.leech_bites_loop" , ATTN_NORM );
	}

	if ( m_sndLeeches )
	{
		( CSoundEnvelopeController::GetController() ).Play( m_sndLeeches, 1.0f, 100 );
	}

	if ( m_sndWaterSplashes == NULL )
	{
		m_sndWaterSplashes = (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "coast.leech_water_churn_loop" , ATTN_NORM );
	}

	if ( m_sndWaterSplashes )
	{
		( CSoundEnvelopeController::GetController() ).Play( m_sndWaterSplashes, 1.0f, 100 );
	}
}

void CTFPlayer::StopWaterDeathSounds( void )
{
	if ( m_sndLeeches )
	{
		( CSoundEnvelopeController::GetController() ).SoundFadeOut( m_sndLeeches, 0.5f, true );
		m_sndLeeches = NULL;
	}

	if ( m_sndWaterSplashes )
	{
		( CSoundEnvelopeController::GetController() ).SoundFadeOut( m_sndWaterSplashes, 0.5f, true );
		m_sndWaterSplashes = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ThrowGrenade( int nType )
{
	Vector vecForward, vecRight, vecUp;
	AngleVectors( EyeAngles(), &vecForward, &vecRight, &vecUp );

	// Create grenades here!!
	Vector vecSrc = Weapon_ShootPosition();
	vecSrc += vecForward * 16.0f + vecRight * 8.0f + vecUp * -6.0f;

	Vector vecVelocity = ( vecForward * 4 ) + ( vecUp * 150.0f ) + ( random->RandomFloat( -10.0f, 10.0f ) * vecRight ) +
	( random->RandomFloat(-10.0f, 10.0f) * vecUp );

	AngularImpulse angImp( 400, random->RandomInt(-400, 400), 0 );

	CTFWeaponBaseGrenadeProj *pProjectile = NULL;

	switch ( nType )
	{
	case TF_WEAPON_GRENADE_NORMAL:
		pProjectile = CTFGrenadeNormalProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_CONCUSSION:
		pProjectile = CTFGrenadeConcussionProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_NAIL:
		pProjectile = CTFGrenadeNailProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_MIRV:
		pProjectile = CTFGrenadeMirvProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	case TF_WEAPON_GRENADE_MIRV_DEMOMAN:
		pProjectile = CTFGrenadeMirvProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	case TF_WEAPON_GRENADE_NAPALM:
		pProjectile = CTFGrenadeNapalmProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_GAS:
		pProjectile = CTFGrenadeGasProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_EMP:
		pProjectile = CTFGrenadeEmpProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_CALTROP:
		pProjectile = CTFGrenadeCaltropProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_PIPEBOMB:
		//pProjectile = CTFGrenadePipebombProjectile::Create( "tf_projectile_pipe", vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	case TF_WEAPON_GRENADE_SMOKE_BOMB:
		//pProjectile = CTFGrenadeSmokeBomb::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_HEAL:
		pProjectile = CTFGrenadeHealProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_TFC_GRENADE_FLARE:
		pProjectile = CTFGrenadeFlareProjectile::Create( vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL, 3.0f );
		break;

	case TF_WEAPON_GRENADE_STUNBALL:
		pProjectile = CTFWeaponBaseGrenadeProj::Create( "tf_projectile_stunball", vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	case TF_WEAPON_GRENADE_JAR:
		pProjectile = CTFWeaponBaseGrenadeProj::Create( "tf_projectile_milk", vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	case TF_WEAPON_GRENADE_JAR_MILK:
		pProjectile = CTFWeaponBaseGrenadeProj::Create( "tf_projectile_jar_milk", vecSrc, EyeAngles(), vecVelocity, angImp, this, NULL );
		break;

	default:
		return;
		break;
	}

	/*if ( pProjectile )
	{
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::UseActionSlotItemPressed( void )
{
	DoNoiseMaker();

	if ( IsAlive() )
	{
		Vector vecForward;
		AngleVectors( EyeAngles(), &vecForward );
		Vector vecStart = Weapon_ShootPosition();
		float flRange = 150.0f;
		Vector vecEnd = vecStart + vecForward * flRange;

		trace_t trace;
		CTraceFilterIgnorePlayers traceFilter( NULL, COLLISION_GROUP_NONE );
		Ray_t ray; ray.Init( vecStart, vecEnd );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, &traceFilter, &trace );

		if ( trace.m_pEnt && trace.m_pEnt->ClassMatches( "tf_dropped_weapon" ) )
		{
			variant_t emptyVariant;
			trace.m_pEnt->AcceptInput( "Use", this, this, emptyVariant, USE_ON );
		}

		CTFPowerupBottle *pCanteen = ( CTFPowerupBottle* )GetWearableForLoadoutSlot( LOADOUT_POSITION_ACTION );
		if ( pCanteen )
		{
			if ( pCanteen->Use() )
			{
				
			}
		}

		CTFWeaponBase *pActionWeapon = dynamic_cast< CTFWeaponBase * >( GetLoadoutItem(GetPlayerClass()->GetClassIndex(), LOADOUT_POSITION_ACTION) );
		if ( pActionWeapon )
		{
			Weapon_Switch( pActionWeapon );
		}

	}

	if ( TFGameRules()->CanPlayerStopSearchSpawn() )
	{
		float flRespawnTime = TFGameRules()->GetMinTimeWhenPlayerMaySpawn( this );
		if ( !IsAlive() && ( flRespawnTime <= 0 ) && IsSearchingSpawn() )
		{
			ForceRespawn();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::UseActionSlotItemReleased( void )
{
	CTFGrapplingHook *pActionWeapon = dynamic_cast< CTFGrapplingHook * >( Weapon_OwnsThisID( TF_WEAPON_GRAPPLINGHOOK ) );
	if ( pActionWeapon )
	{
		pActionWeapon->RemoveHookProjectile( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::InspectButtonPressed( void )
{
	if ( !FlashlightIsOn() )
		FlashlightTurnOn();
	else
		FlashlightTurnOff();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::InspectButtonReleased( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::MerasmusPlayerBombExplode( bool bKilled )
{
	Vector vecOrigin = EyePosition();
	CPVSFilter filter( vecOrigin );

	TE_TFExplosion( filter, 0, vecOrigin, Vector( 0, 0, 1.0f ), -1, entindex() );

	CTakeDamageInfo info( this, this, NULL, vecOrigin, vecOrigin, 40.f, DMG_BLAST | DMG_USEDISTANCEMOD, TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB, &vecOrigin );
	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info	= &info;
	radiusInfo.m_flRadius = 100.0;
	radiusInfo.m_vecSrc = vecOrigin;

	if ( bKilled )
		radiusInfo.m_pEntityIgnore = this;

	TFGameRules()->RadiusDamage( radiusInfo );
	UTIL_ScreenShake( vecOrigin, 15.0, 5.0, 2.0, 750.0, SHAKE_START, true );

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 1.0 );

	m_Shared.RemoveCond( TF_COND_HALLOWEEN_BOMB_HEAD );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanBeForcedToLaugh( void )
{
	// Check to see if we can taunt again!
	if ( m_Shared.InCond( TF_COND_TAUNTING ) )
		return false;

	// Check to see if we are in water (above our waist).
	if ( GetWaterLevel() > WL_Waist )
		return false;

	// Check to see if we are on the ground.
	if ( GetGroundEntity() == NULL )
		return false;

	// Check to see if cutscene is playing.
	if ( m_Shared.IsInCutScene() )
		return false;

	// Check to see if we are in a vehicle
	if ( IsInAVehicle() )
		return false;

	// Can't taunt while cloaked.
	if ( m_Shared.IsStealthed() )
		return false;

	// Can't taunt while disguised.
	if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		return false;

	// Can't taunt while charge.
	if ( m_Shared.InCond( TF_COND_SHIELD_CHARGE ) )
		return false;
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanGetWet( void ) const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanBreatheUnderwater( void ) const
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Notifies Alyx that player has put a combine ball into a socket so she can comment on it.
// Input  : pCombineBall - ball the was socketed
//-----------------------------------------------------------------------------
void CTFPlayer::CombineBallSocketed( CPropCombineBall *pCombineBall )
{
	if ( hl2_episodic.GetBool() )
	{
		CNPC_Alyx *pAlyx = CNPC_Alyx::GetAlyx();
		if ( pAlyx )
		{
			pAlyx->CombineBallSocketed( pCombineBall->NumBounces() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::DoNoiseMaker( void )
{
	if ( !IsAlive() )
		return;

	if ( gpGlobals->curtime > m_flNextNoiseMakerTime )
	{
		CEconWearable *pNoiseMaker = GetWearableForLoadoutSlot( LOADOUT_POSITION_ACTION );
		if ( !pNoiseMaker )
			return;

		int iEnableNoiseMaker = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pNoiseMaker, iEnableNoiseMaker, enable_misc2_noisemaker );
		if ( iEnableNoiseMaker == 0 )
			return;

		CEconItemView *pItem = pNoiseMaker->GetItem();
		if ( !pItem )
			return;

		CEconItemDefinition *pStatic = pItem->GetStaticData();
		if ( !pStatic )
			return;

		PerTeamVisuals_t *pVisuals =	pStatic->GetVisuals( GetTeamNumber() );
		if ( pVisuals )
		{
			const char *pszParticleEffect = pVisuals->particle_effect;
			if ( pszParticleEffect[0] != '\0' )
			{
				CDisablePredictionFiltering disabler;
				DispatchParticleEffect( pszParticleEffect, WorldSpaceCenter(), vec3_angle );
			}

			const char *pszSoundZero = pVisuals->custom_sound0;
			if ( pszSoundZero[0] != '\0' )
				EmitSound( pszSoundZero );

			m_flNextNoiseMakerTime = gpGlobals->curtime + 1.5f;
		}

		CSoundEnt::InsertSound ( SOUND_PLAYER, GetAbsOrigin(), 256, 0.5, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Used to relay outputs/inputs from the player to the world and viceversa
//-----------------------------------------------------------------------------
class CLogicPlayerProxy : public CLogicalEntity
{
	DECLARE_CLASS( CLogicPlayerProxy, CLogicalEntity );

private:

	DECLARE_DATADESC();

public:

	COutputEvent m_OnFlashlightOn;
	COutputEvent m_OnFlashlightOff;
	COutputEvent m_PlayerHasAmmo;
	COutputEvent m_PlayerHasNoAmmo;
	COutputEvent m_PlayerDied;
	COutputEvent m_PlayerMissedAR2AltFire; // Player fired a combine ball which did not dissolve any enemies. 

	COutputInt m_RequestedPlayerHealth;

	void InputRequestPlayerHealth( inputdata_t &inputdata );
	void InputSetFlashlightSlowDrain( inputdata_t &inputdata );
	void InputSetFlashlightNormalDrain( inputdata_t &inputdata );
	void InputSetPlayerHealth( inputdata_t &inputdata );
	void InputRequestAmmoState( inputdata_t &inputdata );
	void InputLowerWeapon( inputdata_t &inputdata );
	void InputEnableCappedPhysicsDamage( inputdata_t &inputdata );
	void InputDisableCappedPhysicsDamage( inputdata_t &inputdata );
	void InputSetLocatorTargetEntity( inputdata_t &inputdata );
	void InputIgniteAllPlayer( inputdata_t &inputdata );
	void InputExtinguishAllPlayer( inputdata_t &inputdata );
	void InputBleedAllPlayer( inputdata_t &inputdata );
	void InputSpeakResponseConcept( inputdata_t &inputdata );
	void InputSetForcedTauntCam( inputdata_t &inputdata );
	void InputSetHUDVisibility( inputdata_t &inputdata );
	void InputHandleMapEvent( inputdata_t &inputdata );
	void InputSetFogController( inputdata_t &inputdata );
	void InputFreezeAllPlayerMovement( inputdata_t &inputdata );
	void InputUnFreezeAllPlayerMovement( inputdata_t &inputdata );
	void InputSuppressCrosshair( inputdata_t &inputdata );

	void Activate ( void );

	//bool PassesDamageFilter( const CTakeDamageInfo &info );

	EHANDLE m_hPlayer;
};

CLogicPlayerProxy *CTFPlayer::GetPlayerProxy( void )
{
	CLogicPlayerProxy *pProxy = dynamic_cast< CLogicPlayerProxy* > ( m_hPlayerProxy.Get() );

	if ( pProxy == NULL )
	{
		pProxy = (CLogicPlayerProxy*)gEntList.FindEntityByClassname(NULL, "logic_playerproxy" );

		if ( pProxy == NULL )
			return NULL;

		pProxy->m_hPlayer = this;
		m_hPlayerProxy = pProxy;
	}

	return pProxy;
}

void CTFPlayer::FirePlayerProxyOutput( const char *pszOutputName, variant_t variant, CBaseEntity *pActivator, CBaseEntity *pCaller )
{
	if ( GetPlayerProxy() == NULL )
		return;

	GetPlayerProxy()->FireNamedOutput( pszOutputName, variant, pActivator, pCaller );
}

LINK_ENTITY_TO_CLASS( logic_playerproxy, CLogicPlayerProxy);

BEGIN_DATADESC( CLogicPlayerProxy )
	DEFINE_OUTPUT( m_OnFlashlightOn, "OnFlashlightOn" ),
	DEFINE_OUTPUT( m_OnFlashlightOff,"OnFlashlightOff" ),
	DEFINE_OUTPUT( m_RequestedPlayerHealth,"PlayerHealth" ),
	DEFINE_OUTPUT( m_PlayerHasAmmo, "PlayerHasAmmo" ),
	DEFINE_OUTPUT( m_PlayerHasNoAmmo,"PlayerHasNoAmmo" ),
	DEFINE_OUTPUT( m_PlayerDied,	"PlayerDied" ),
	DEFINE_OUTPUT( m_PlayerMissedAR2AltFire, "PlayerMissedAR2AltFire" ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"RequestPlayerHealth",	InputRequestPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"SetFlashlightSlowDrain",	InputSetFlashlightSlowDrain ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"SetFlashlightNormalDrain",	InputSetFlashlightNormalDrain ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,"SetPlayerHealth",	InputSetPlayerHealth ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"RequestAmmoState", InputRequestAmmoState ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"LowerWeapon", InputLowerWeapon ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableCappedPhysicsDamage", InputEnableCappedPhysicsDamage ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableCappedPhysicsDamage", InputDisableCappedPhysicsDamage ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"SetLocatorTargetEntity", InputSetLocatorTargetEntity ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"IgniteAllPlayer", InputIgniteAllPlayer ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ExtinguishAllPlayer", InputExtinguishAllPlayer ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"SpeakResponseConcept", InputSpeakResponseConcept ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,"SetForcedTauntCam", InputSetForcedTauntCam ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,"SetHUDVisibility", InputSetHUDVisibility ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"HandleMapEvent", InputHandleMapEvent ),
	DEFINE_INPUTFUNC( FIELD_STRING,	"SetFogController", InputSetFogController ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"BleedAllPlayer", InputBleedAllPlayer ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"FreezeAllPlayerMovement", InputFreezeAllPlayerMovement ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"UnFreezeAllPlayerMovement", InputUnFreezeAllPlayerMovement ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"SuppressCrosshair", InputSuppressCrosshair ),
	DEFINE_FIELD( m_hPlayer, FIELD_EHANDLE ),
END_DATADESC()

void CLogicPlayerProxy::Activate( void )
{
	BaseClass::Activate();

	CTFPlayer *pPlayer;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );

		if ( !pPlayer )
			continue;

		if ( !m_hPlayer )
		{
			m_hPlayer = pPlayer;
		}
	}
}
/*
bool CLogicPlayerProxy::PassesDamageFilter( const CTakeDamageInfo &info )
{
	BaseClass::PassesDamageFilter( info );
}
*/
void CLogicPlayerProxy::InputSetPlayerHealth( inputdata_t &inputdata )
{
	if ( !m_hPlayer )
		return;

	m_hPlayer->SetHealth( inputdata.value.Int() );
}

void CLogicPlayerProxy::InputRequestPlayerHealth( inputdata_t &inputdata )
{
	if ( !m_hPlayer )
		return;

	int iHealth = clamp( m_hPlayer->GetHealth(), 0, 100 );

	m_RequestedPlayerHealth.Set( iHealth, inputdata.pActivator, inputdata.pCaller );
}

void CLogicPlayerProxy::InputSetFlashlightSlowDrain( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	if( pPlayer )
		pPlayer->SetFlashlightPowerDrainScale( hl2_darkness_flashlight_factor.GetFloat() );

}

void CLogicPlayerProxy::InputSetFlashlightNormalDrain( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	if( pPlayer )
		pPlayer->SetFlashlightPowerDrainScale( 1.0f );

}

void CLogicPlayerProxy::InputRequestAmmoState( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );

	for ( int i = 0 ; i < pPlayer->WeaponCount(); ++i )
	{
		CBaseCombatWeapon* pCheck = pPlayer->GetWeapon( i );

		if ( pCheck )
		{
			if ( pCheck->HasAnyAmmo() && (pCheck->UsesPrimaryAmmo() || pCheck->UsesSecondaryAmmo()))
			{
				m_PlayerHasAmmo.FireOutput( this, this, 0 );
				return;
			}
		}
	}

	m_PlayerHasNoAmmo.FireOutput( this, this, 0 );
}

void CLogicPlayerProxy::InputLowerWeapon( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );

	CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
	if ( pWeapon != NULL )
	{
		pWeapon->Lower();
	}
}

void CLogicPlayerProxy::InputEnableCappedPhysicsDamage( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	pPlayer->EnableCappedPhysicsDamage();
}

void CLogicPlayerProxy::InputDisableCappedPhysicsDamage( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	pPlayer->DisableCappedPhysicsDamage();
}

void CLogicPlayerProxy::InputSetLocatorTargetEntity( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CBaseEntity *pTarget = NULL; // assume no target
	string_t iszTarget = MAKE_STRING( inputdata.value.String() );

	if( iszTarget != NULL_STRING )
	{
		pTarget = gEntList.FindEntityByName( NULL, iszTarget );
	}

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	if ( pPlayer )
		pPlayer->SetLocatorTargetEntity( pTarget );
}

void CLogicPlayerProxy::InputIgniteAllPlayer( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	pPlayer->AcceptInput( "IgnitePlayer", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputExtinguishAllPlayer( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	pPlayer->AcceptInput( "ExtinguishPlayer", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputSetForcedTauntCam( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	sVariant.SetInt( inputdata.value.Int() );
	pPlayer->AcceptInput( "SetForcedTauntCam", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputSpeakResponseConcept( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	sVariant.SetString( MAKE_STRING( inputdata.value.String() ));
	pPlayer->AcceptInput( "SpeakResponseConcept", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputSetHUDVisibility( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	sVariant.SetInt( inputdata.value.Int() );
	pPlayer->AcceptInput( "SetHUDVisibility", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputHandleMapEvent( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	sVariant.SetString( MAKE_STRING( inputdata.value.String() ));
	pPlayer->AcceptInput( "HandleMapEvent", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputSetFogController( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	sVariant.SetString( MAKE_STRING( inputdata.value.String() ));
	pPlayer->AcceptInput( "SetFogController", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputBleedAllPlayer( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	variant_t sVariant;
	pPlayer->AcceptInput( "BleedPlayer", this, this, sVariant, 0 );
}

void CLogicPlayerProxy::InputFreezeAllPlayerMovement( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	pPlayer->AddFlag( FL_FROZEN );
}

void CLogicPlayerProxy::InputUnFreezeAllPlayerMovement( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	pPlayer->RemoveFlag( FL_FROZEN );
}

void CLogicPlayerProxy::InputSuppressCrosshair( inputdata_t &inputdata )
{
	if( !m_hPlayer )
		return;

	/*CTFPlayer *pPlayer = ToTFPlayer( m_hPlayer.Get() );
	pPlayer->SuppressCrosshair( true );*/
}
