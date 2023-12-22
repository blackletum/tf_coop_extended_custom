//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#include "cbase.h"
#include "tf_gamerules.h"
#include "tf_player_shared.h"
#include "takedamageinfo.h"
#include "tf_weaponbase.h"
#include "effect_dispatch_data.h"
#include "tf_item.h"
#include "entity_capture_flag.h"
#include "baseobject_shared.h"
#include "tf_weapon_medigun.h"
#include "tf_weapon_pipebomblauncher.h"
#include "in_buttons.h"
#include "tf_viewmodel.h"
#include "econ_wearable.h"
#include "tf_fx_shared.h"
#include "tf_weapon_buff_item.h"
#include "tf_weapon_invis.h"
#include "tf_wearable_demoshield.h"
#include "tf_weapon_sword.h"
#include "tf_weapon_shotgun.h"
#include "tf_dropped_weapon.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_te_effect_dispatch.h"
#include "c_tf_fx.h"
#include "soundenvelope.h"
#include "c_tf_playerclass.h"
#include "iviewrender.h"
#include "engine/ivdebugoverlay.h"
#include "c_tf_playerresource.h"
#include "c_tf_team.h"
#include "prediction.h"
#include "glow_outline_effect.h"
#include "lfe_hud_pingsystem.h"
#include "prediction.h"

#define CRecipientFilter C_RecipientFilter
#define CTFPlayerClass C_TFPlayerClass

// Server specific.
#else
#include "tf_player.h"
#include "te_effect_dispatch.h"
#include "tf_fx.h"
#include "util.h"
#include "tf_team.h"
#include "tf_gamestats.h"
#include "tf_playerclass.h"
#include "tf_weapon_builder.h"
#include "tf_weapon_physcannon.h"
#include "ai_basenpc.h"
#include "portal_gamestats.h"
#include "func_respawnroom.h"
#include "soundent.h"
#endif

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

extern ConVar lfe_use_hl2_player_hull;
extern ConVar lfe_episodic_flashlight;

ConVar tf_spy_invis_time( "tf_spy_invis_time", "1.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Transition time in and out of spy invisibility", true, 0.1, true, 5.0 );
ConVar tf_spy_invis_unstealth_time( "tf_spy_invis_unstealth_time", "2.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Transition time in and out of spy invisibility", true, 0.1, true, 5.0 );

ConVar tf_spy_max_cloaked_speed( "tf_spy_max_cloaked_speed", "999", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );	// no cap

ConVar tf_feign_death_activate_damage_scale( "tf_feign_death_activate_damage_scale", "0.35", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar tf_feign_death_duration( "tf_feign_death_duration", "3.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Time that feign death buffs last." );
ConVar tf_feign_death_speed_duration( "tf_feign_death_speed_duration", "3", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Time that feign death speed boost last." );
ConVar tf_feign_death_damage_scale( "tf_feign_death_damage_scale", "0.35", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar tf_stealth_damage_reduction( "tf_stealth_damage_reduction", "0.8", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );

ConVar tf_max_health_boost( "tf_max_health_boost", "1.5", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Max health factor that players can be boosted to by healers.", true, 1.0, false, 0 );
ConVar tf_invuln_time( "tf_invuln_time", "1.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Time it takes for invulnerability to wear off." );
ConVar tf_soldier_buff_pulses( "tf_soldier_buff_pulses", "10", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Time it takes for buff to wear off." );

#ifdef GAME_DLL
ConVar tf_boost_drain_time( "tf_boost_drain_time", "15.0", FCVAR_DEVELOPMENTONLY, "Time it takes for a full health boost to drain away from a player.", true, 0.1, false, 0 );
ConVar tf_debug_bullets( "tf_debug_bullets", "0", FCVAR_DEVELOPMENTONLY, "Visualize bullet traces." );
ConVar tf_damage_events_track_for( "tf_damage_events_track_for", "30", FCVAR_DEVELOPMENTONLY );
#endif

ConVar tf_useparticletracers( "tf_useparticletracers", "1", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Use particle tracers instead of old style ones." );
ConVar tf_spy_cloak_consume_rate( "tf_spy_cloak_consume_rate", "10.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "cloak to use per second while cloaked, from 100 max )" );	// 10 seconds of invis
ConVar tf_spy_cloak_regen_rate( "tf_spy_cloak_regen_rate", "3.3", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "cloak to regen per second, up to 100 max" );		// 30 seconds to full charge
ConVar tf_spy_cloak_no_attack_time( "tf_spy_cloak_no_attack_time", "2.0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "time after uncloaking that the spy is prohibited from attacking" );

//ConVar tf_spy_stealth_blink_time( "tf_spy_stealth_blink_time", "0.3", FCVAR_DEVELOPMENTONLY, "time after being hit the spy blinks into view" );
//ConVar tf_spy_stealth_blink_scale( "tf_spy_stealth_blink_scale", "0.85", FCVAR_DEVELOPMENTONLY, "percentage visible scalar after being hit the spy blinks into view" );

ConVar tf_demoman_charge_drain_time( "tf_demoman_charge_drain_time", "1.5", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar tf_demoman_charge_regen_rate( "tf_demoman_charge_regen_rate", "8.3", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );

ConVar tf_scout_energydrink_consume_rate( "tf_scout_energydrink_consume_rate", "12.5", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar tf_scout_energydrink_regen_rate( "tf_scout_energydrink_regen_rate", "3.3", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );

ConVar tf_tournament_hide_domination_icons( "tf_tournament_hide_domination_icons", "0", FCVAR_REPLICATED, "Tournament mode server convar that forces clients to not display the domination icons above players dominating them." );

ConVar tf_damage_disablespread( "tf_damage_disablespread", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Toggles the random damage spread applied to all player damage." );
ConVar tf_always_loser( "tf_always_loser", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Force loserstate to true." );

ConVar sv_showimpacts( "sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client (red) and server (blue) bullet impact point (1=both, 2=client-only, 3=server-only)" );
ConVar sv_showplayerhitboxes( "sv_showplayerhitboxes", "0", FCVAR_REPLICATED, "Show lag compensated hitboxes for the specified player index." );

ConVar lfe_obj_unlimited( "lfe_obj_unlimited", "0", FCVAR_REPLICATED, "Toggle building limitation." );

#ifdef GAME_DLL
	ConVar tf_powerup_max_charge_time( "tf_powerup_max_charge_time", "30", FCVAR_CHEAT | FCVAR_REPLICATED, "" );
	ConVar sv_infinite_ammo( "sv_infinite_ammo", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Player's active weapon will never run out of ammo" );
	extern ConVar	lfe_force_legacy;
	extern ConVar	tf_fireball_burn_duration;
	extern ConVar	lfe_allow_special_classes;
#else
	extern ConVar	tf_respawn_on_loadoutchanges;
#endif

extern ConVar sv_footsteps;
extern ConVar sv_debug_player_use;
extern ConVar tf_grapplinghook_prevent_fall_damage;

extern float IntervalDistance( float x, float x0, float x1 );

#define TF_SPY_STEALTH_BLINKTIME   0.3f
#define TF_SPY_STEALTH_BLINKSCALE  0.85f

#define TF_PLAYER_CONDITION_CONTEXT	"TFPlayerConditionContext"

#define MAX_DAMAGE_EVENTS		128

#define TF_BUFF_RADIUS			450.0f

#define	FLASH_DRAIN_TIME	 5.0f	// 100 units / 90 secs
#define	FLASH_CHARGE_TIME	 50.0f	// 100 units / 2 secs

//=============================================================================
//
// Tables.
//

// Client specific.
#ifdef CLIENT_DLL

BEGIN_RECV_TABLE_NOBASE( CTFPlayerShared, DT_TFPlayerSharedLocal )
	RecvPropInt( RECVINFO( m_nDesiredDisguiseTeam ) ),
	RecvPropInt( RECVINFO( m_nDesiredDisguiseClass ) ),
	RecvPropTime( RECVINFO( m_flStealthNoAttackExpire ) ),
	RecvPropTime( RECVINFO( m_flStealthNextChangeTime ) ),
	RecvPropFloat( RECVINFO( m_flCloakMeter) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bPlayerDominated ), RecvPropBool( RECVINFO( m_bPlayerDominated[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bPlayerDominatingMe ), RecvPropBool( RECVINFO( m_bPlayerDominatingMe[0] ) ) ),
	RecvPropInt( RECVINFO( m_iDesiredWeaponID ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_nStreaks ), RecvPropInt( RECVINFO( m_nStreaks[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_flCondExpireTimeLeft ), RecvPropFloat( RECVINFO( m_flCondExpireTimeLeft[0] ) ) ),
	RecvPropInt( RECVINFO( m_iLives ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx2 ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx3 ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx4 ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( CTFPlayerShared, DT_TFPlayerShared )
	RecvPropInt( RECVINFO( m_nPlayerCond ) ),
	RecvPropInt( RECVINFO( m_bJumping) ),
	RecvPropInt( RECVINFO( m_nNumHealers ) ),
	RecvPropInt( RECVINFO( m_iCritMult ) ),
	RecvPropInt( RECVINFO( m_nAirDucked ) ),
	RecvPropInt( RECVINFO( m_nPlayerState ) ),
	RecvPropInt( RECVINFO( m_iDesiredPlayerClass ) ),
	RecvPropTime( RECVINFO( m_flStunExpireTime ) ),
	RecvPropInt( RECVINFO( m_nStunFlags ) ),
	RecvPropFloat( RECVINFO( m_flStunMovementSpeed ) ),
	RecvPropFloat( RECVINFO( m_flStunResistance ) ),
	RecvPropEHandle( RECVINFO( m_hStunner ) ),
	RecvPropInt( RECVINFO( m_iDecapitations ) ),
	RecvPropInt( RECVINFO( m_bShieldEquipped ) ),
	RecvPropBool( RECVINFO( m_bParachuteEquipped ) ),
	RecvPropInt( RECVINFO( m_iNextMeleeCrit ) ),
	RecvPropEHandle( RECVINFO( m_hCarriedObject ) ),
	RecvPropBool( RECVINFO( m_bCarryingObject ) ),
	RecvPropInt( RECVINFO( m_nTeamTeleporterUsed ) ),
	RecvPropInt( RECVINFO( m_iRespawnParticleID ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropBool( RECVINFO( m_bGunslinger ) ),
	RecvPropBool( RECVINFO( m_bKingRuneBuffActive ) ),
	RecvPropFloat( RECVINFO( m_flEffectBarProgress ) ),
	RecvPropFloat( RECVINFO( m_flEnergyDrinkMeter ) ),
	RecvPropFloat( RECVINFO( m_flChargeMeter ) ),
	RecvPropFloat( RECVINFO( m_flRuneCharge ) ),
	RecvPropFloat( RECVINFO( m_flFlashBattery ) ),
	RecvPropFloat( RECVINFO( m_flHypeMeter ) ),
	RecvPropInt( RECVINFO( m_iAirDash ) ),
	RecvPropInt( RECVINFO( m_nCurrency ) ),
	RecvPropBool( RECVINFO( m_bInUpgradeZone ) ),
	// Spy.
	RecvPropTime( RECVINFO( m_flInvisChangeCompleteTime ) ),
	RecvPropInt( RECVINFO( m_nDisguiseTeam ) ),
	RecvPropInt( RECVINFO( m_nDisguiseClass ) ),
	RecvPropInt( RECVINFO( m_nMaskClass ) ),
	RecvPropInt( RECVINFO( m_iDisguiseTargetIndex ) ),
	RecvPropInt( RECVINFO( m_iDisguiseHealth ) ),
	RecvPropInt( RECVINFO( m_iDisguiseMaxHealth ) ),
	RecvPropFloat( RECVINFO( m_flDisguiseChargeLevel ) ),
	RecvPropDataTable( RECVINFO_DT( m_DisguiseItem ), 0, &REFERENCE_RECV_TABLE( DT_ScriptCreatedItem ) ),
	RecvPropBool( RECVINFO( m_bFeignDeathReady ) ),
	RecvPropBool( RECVINFO( m_bIsPlayerADev ) ),
	RecvPropBool( RECVINFO( m_bIsPlayerNicknine ) ),
	RecvPropVector( RECVINFO( m_vecPingOrigin ) ),
	RecvPropEHandle( RECVINFO( m_hPingTarget ) ),
	RecvPropEHandle( RECVINFO( m_hPasstimePassTarget ) ),
	// Local Data.
	RecvPropDataTable( "tfsharedlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_TFPlayerSharedLocal) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA_NO_BASE( CTFPlayerShared )
	DEFINE_PRED_FIELD( m_nPlayerState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCond, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx2, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx3, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx4, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flCloakMeter, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bJumping, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nAirDucked, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iAirDash, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flInvisChangeCompleteTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iDesiredWeaponID, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iRespawnParticleID, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flEffectBarProgress, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flEnergyDrinkMeter, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flChargeMeter, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flRuneCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flFlashBattery, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flHypeMeter, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nCurrency, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bInUpgradeZone, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

// Server specific.
#else

BEGIN_SEND_TABLE_NOBASE( CTFPlayerShared, DT_TFPlayerSharedLocal )
	SendPropInt( SENDINFO( m_nDesiredDisguiseTeam ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nDesiredDisguiseClass ), 4, SPROP_UNSIGNED ),
	SendPropTime( SENDINFO( m_flStealthNoAttackExpire ) ),
	SendPropTime( SENDINFO( m_flStealthNextChangeTime ) ),
	SendPropFloat( SENDINFO( m_flCloakMeter ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN, 0.0, 100.0 ),
	SendPropArray3( SENDINFO_ARRAY3( m_bPlayerDominated ), SendPropBool( SENDINFO_ARRAY( m_bPlayerDominated ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_bPlayerDominatingMe ), SendPropBool( SENDINFO_ARRAY( m_bPlayerDominatingMe ) ) ),
	SendPropInt( SENDINFO( m_iDesiredWeaponID ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_nStreaks ), SendPropInt( SENDINFO_ARRAY( m_nStreaks ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_flCondExpireTimeLeft ), SendPropFloat( SENDINFO_ARRAY( m_flCondExpireTimeLeft ) ) ),
	SendPropInt( SENDINFO( m_nPlayerCondEx ), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nPlayerCondEx2 ), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nPlayerCondEx3 ), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nPlayerCondEx4 ), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_iLives ), 8 ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CTFPlayerShared, DT_TFPlayerShared )
	SendPropInt( SENDINFO( m_nPlayerCond ), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_bJumping ), 1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nNumHealers ), 5, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_iCritMult ), 8, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nAirDucked ), 2, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nPlayerState ), Q_log2( TF_STATE_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iDesiredPlayerClass ), Q_log2( TF_CLASS_COUNT_ALL ) + 1, SPROP_UNSIGNED ),
	SendPropTime( SENDINFO( m_flStunExpireTime ) ),
	SendPropInt( SENDINFO( m_nStunFlags ), -1, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flStunMovementSpeed ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flStunResistance ), 0, SPROP_NOSCALE ),
	SendPropEHandle( SENDINFO( m_hStunner ) ),
	SendPropInt( SENDINFO( m_iDecapitations ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_bShieldEquipped ), 1, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bParachuteEquipped ) ),
	SendPropInt( SENDINFO( m_iNextMeleeCrit ) ),
	SendPropEHandle( SENDINFO( m_hCarriedObject ) ),
	SendPropBool( SENDINFO( m_bCarryingObject ) ),
	SendPropInt( SENDINFO( m_nTeamTeleporterUsed ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iRespawnParticleID ), -1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iMaxHealth ), 10 ),
	SendPropBool( SENDINFO( m_bGunslinger ) ),
	SendPropBool( SENDINFO( m_bKingRuneBuffActive ) ),
	SendPropFloat( SENDINFO( m_flEffectBarProgress ), 11, 0, 0.0f, 100.0f ),
	SendPropFloat( SENDINFO( m_flEnergyDrinkMeter ), 11, 0, 0.0f, 100.0f ),
	SendPropFloat( SENDINFO( m_flChargeMeter ), 11, 0, 0.0f, 100.0f ),
	SendPropFloat( SENDINFO( m_flRuneCharge ), 11, 0, 0.0f, 100.0f ),
	SendPropFloat( SENDINFO( m_flFlashBattery ), 11, 0, 0.0f, 100.0f ),
	SendPropFloat( SENDINFO( m_flHypeMeter ), 11, 0, 0.0f, 100.0f ),
	SendPropInt( SENDINFO( m_iAirDash ), 10, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_nCurrency ), 10, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropBool( SENDINFO( m_bInUpgradeZone ) ),
	// Spy
	SendPropTime( SENDINFO( m_flInvisChangeCompleteTime ) ),
	SendPropInt( SENDINFO( m_nDisguiseTeam ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nDisguiseClass ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nMaskClass ), 4, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iDisguiseTargetIndex ), 7, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iDisguiseHealth ), 10 ),
	SendPropInt( SENDINFO( m_iDisguiseMaxHealth ), 10 ),
	SendPropFloat( SENDINFO( m_flDisguiseChargeLevel ), 0, SPROP_NOSCALE ),
	SendPropDataTable( SENDINFO_DT( m_DisguiseItem ), &REFERENCE_SEND_TABLE( DT_ScriptCreatedItem ) ),
	SendPropBool( SENDINFO( m_bFeignDeathReady ) ),
	SendPropBool( SENDINFO( m_bIsPlayerADev ) ),
	SendPropBool( SENDINFO( m_bIsPlayerNicknine ) ),
	SendPropVector( SENDINFO( m_vecPingOrigin ) ),
	SendPropEHandle( SENDINFO( m_hPingTarget ) ),
	SendPropEHandle( SENDINFO( m_hPasstimePassTarget ) ),
	// Local Data.
	SendPropDataTable( "tfsharedlocaldata", 0, &REFERENCE_SEND_TABLE( DT_TFPlayerSharedLocal ), SendProxy_SendLocalDataTable ),
END_SEND_TABLE()

#endif


// --------------------------------------------------------------------------------------------------- //
// Shared CTFPlayer implementation.
// --------------------------------------------------------------------------------------------------- //

// --------------------------------------------------------------------------------------------------- //
// CTFPlayerShared implementation.
// --------------------------------------------------------------------------------------------------- //

CTFPlayerShared::CTFPlayerShared()
{
	m_nPlayerState.Set( TF_STATE_WELCOME );
	m_bJumping = false;
	m_iAirDash = 0;
	m_nAirDucked = 0;
	m_flStealthNoAttackExpire = 0.0f;
	m_flStealthNextChangeTime = 0.0f;
	m_iCritMult = 0;
	m_flInvisibility = 0.0f;
	m_iLives = -1;

	m_iDesiredWeaponID = -1;
	m_iRespawnParticleID = 0;

	m_iStunPhase = 0;

	m_nTeamTeleporterUsed = TEAM_UNASSIGNED;
	m_bGunslinger = false;

	m_iDecapitations = 0;

	m_bFeignDeathReady = false;
	m_bShieldEquipped = false;
	m_bParachuteEquipped = false;

	m_nCurrency = 0;
	m_bInUpgradeZone = false;

	m_bIsPlayerADev = false;
	m_bIsPlayerNicknine = false;

	m_flEnergyDrinkDrainRate = tf_scout_energydrink_consume_rate.GetFloat();
	m_flEnergyDrinkRegenRate = tf_scout_energydrink_regen_rate.GetFloat();

#ifdef CLIENT_DLL
	m_iDisguiseWeaponModelIndex = -1;
	m_pDisguiseWeaponInfo = NULL;
	m_pCritSound = NULL;
	m_pCritEffect = NULL;
	m_pInvulnerableSound = NULL;
#else
	memset( m_flChargeOffTime, 0, sizeof( m_flChargeOffTime ) );
	memset( m_bChargeSounds, 0, sizeof( m_bChargeSounds ) );
#endif
}

void CTFPlayerShared::Init( CTFPlayer *pPlayer )
{
	m_pOuter = pPlayer;

	m_flNextBurningSound = 0;

	SetJumping( false );
}

//-----------------------------------------------------------------------------
// Purpose: Add a condition and duration
// duration of PERMANENT_CONDITION means infinite duration
//-----------------------------------------------------------------------------
void CTFPlayerShared::AddCond( int nCond, float flDuration /* = PERMANENT_CONDITION */ )
{
	Assert( nCond >= 0 && nCond < TF_COND_LAST );
	int nCondFlag = nCond;
	int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	*pVar |= ( 1 << nCondFlag );
	m_flCondExpireTimeLeft.Set( nCond, flDuration );
	OnConditionAdded( nCond );
}

//-----------------------------------------------------------------------------
// Purpose: Forcibly remove a condition
//-----------------------------------------------------------------------------
void CTFPlayerShared::RemoveCond( int nCond )
{
	Assert(nCond >= 0 && nCond < TF_COND_LAST);

	int nCondFlag = nCond;
	int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	*pVar &= ~(1 << nCondFlag);
	m_flCondExpireTimeLeft.Set(nCond, 0);

	OnConditionRemoved(nCond);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::InCond( int nCond )
{
	Assert(nCond >= 0 && nCond < TF_COND_LAST);

	int nCondFlag = nCond;
	const int *pVar = NULL;
	if ( nCond < 128 )
	{
		if ( nCond < 96 )
		{
			if ( nCond < 64 )
			{
				if ( nCond < 32 )
				{
					pVar = &m_nPlayerCond.GetForModify();
				}
				else
				{
					pVar = &m_nPlayerCondEx.GetForModify();
					nCondFlag -= 32;
				}
			}
			else
			{
				pVar = &m_nPlayerCondEx2.GetForModify();
				nCondFlag -= 64;
			}
		}
		else
		{
			pVar = &m_nPlayerCondEx3.GetForModify();
			nCondFlag -= 96;
		}
	}
	else
	{
		pVar = &m_nPlayerCondEx4.GetForModify();
		nCondFlag -= 128;
	}

	return ((*pVar & (1 << nCondFlag)) != 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFPlayerShared::GetConditionDuration( int nCond )
{
	Assert( nCond >= 0 && nCond < TF_COND_LAST );

	if ( InCond( nCond ) )
	{
		return m_flCondExpireTimeLeft[nCond];
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsCritBoosted( void )
{
	// Oh man...
	if ( InCond( TF_COND_CRITBOOSTED ) ||
		InCond( TF_COND_CRITBOOSTED_PUMPKIN ) ||
		InCond( TF_COND_CRITBOOSTED_USER_BUFF ) ||
		InCond( TF_COND_CRITBOOSTED_FIRST_BLOOD ) ||
		InCond( TF_COND_CRITBOOSTED_BONUS_TIME ) ||
		InCond( TF_COND_CRITBOOSTED_CTF_CAPTURE ) ||
		InCond( TF_COND_CRITBOOSTED_ON_KILL ) ||
		InCond( TF_COND_CRITBOOSTED_RAGE_BUFF ) ||
		InCond( TF_COND_CRITBOOSTED_CARD_EFFECT ) ||
		InCond( TF_COND_CRITBOOSTED_RUNE_TEMP ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsMiniCritBoosted( void )
{
	if ( InCond( TF_COND_OFFENSEBUFF ) ||
		InCond( TF_COND_ENERGY_BUFF ) ||
		InCond( TF_COND_MINICRITBOOSTED_ON_KILL ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsInvulnerable( void )
{
	// Oh man again...
	if ( InCond( TF_COND_INVULNERABLE ) ||
		InCond( TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE ) ||
		InCond( TF_COND_INVULNERABLE_USER_BUFF ) ||
		InCond( TF_COND_INVULNERABLE_CARD_EFFECT ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsStealthed( void )
{
	if ( InCond( TF_COND_STEALTHED ) ||
		InCond( TF_COND_STEALTHED_USER_BUFF ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsJared( void )
{
	if ( InCond( TF_COND_URINE ) ||
		InCond( TF_COND_MAD_MILK ) ||
		InCond( TF_COND_GAS ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsSpeedBoosted( void )
{
	if ( InCond( TF_COND_SPEED_BOOST ) ||
		InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsBuffed( void )
{
	if ( InCond( TF_COND_OFFENSEBUFF ) ||
		InCond( TF_COND_DEFENSEBUFF ) || 
		InCond( TF_COND_REGENONDAMAGEBUFF ) )
		return true;

	return false;
}

void CTFPlayerShared::DebugPrintConditions( void )
{
#ifndef CLIENT_DLL
	const char *szDll = "Server";
#else
	const char *szDll = "Client";
#endif

	Msg( "( %s ) Conditions for player ( %d )\n", szDll, m_pOuter->entindex() );

	int i;
	int iNumFound = 0;
	for ( i = 0; i < TF_COND_LAST; i++ )
	{
		if ( InCond( i ) )
		{
			if ( m_flCondExpireTimeLeft[i] == PERMANENT_CONDITION )
			{
				Msg( "( %s ) Condition %d - ( permanent cond )\n", szDll, i );
			}
			else
			{
				Msg( "( %s ) Condition %d - ( %.1f left )\n", szDll, i, m_flCondExpireTimeLeft[i] );
			}

			iNumFound++;
		}
	}

	if ( iNumFound == 0 )
	{
		Msg( "( %s ) No active conditions\n", szDll );
	}
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnPreDataChanged( void )
{
	m_nOldConditions = m_nPlayerCond;
	m_nOldConditionsEx = m_nPlayerCondEx;
	m_nOldConditionsEx2 = m_nPlayerCondEx2;
	m_nOldConditionsEx3 = m_nPlayerCondEx3;
	m_nOldConditionsEx4 = m_nPlayerCondEx4;
	m_nOldDisguiseClass = GetDisguiseClass();
	m_nOldDisguiseTeam = GetDisguiseTeam();
	m_iOldDisguiseWeaponModelIndex = m_iDisguiseWeaponModelIndex;
	m_iOldDisguiseWeaponID = m_DisguiseItem.GetItemDefIndex();
	m_bWasCritBoosted = IsCritBoosted();
	m_bWasMiniCritBoosted = IsMiniCritBoosted();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnDataChanged( void )
{
	// Update conditions from last network change
	SyncConditions( m_nPlayerCond, m_nOldConditions, 0, 0 );
	SyncConditions( m_nPlayerCondEx, m_nOldConditionsEx, 0, 32 );
	SyncConditions( m_nPlayerCondEx2, m_nOldConditionsEx2, 0, 64 );
	SyncConditions( m_nPlayerCondEx3, m_nOldConditionsEx3, 0, 96 );
	SyncConditions( m_nPlayerCondEx4, m_nOldConditionsEx4, 0, 128 );

	m_nOldConditions = m_nPlayerCond;
	m_nOldConditionsEx = m_nPlayerCondEx;
	m_nOldConditionsEx2 = m_nPlayerCondEx2;
	m_nOldConditionsEx3 = m_nPlayerCondEx3;
	m_nOldConditionsEx4 = m_nPlayerCondEx4;

	if ( m_bWasCritBoosted != IsCritBoosted() )
	{
		UpdateCritBoostEffect();
	}

	if ( m_nOldDisguiseClass != GetDisguiseClass() || m_nOldDisguiseTeam != GetDisguiseTeam() )
	{
		OnDisguiseChanged();
	}

	if ( m_iOldDisguiseWeaponID != m_DisguiseItem.GetItemDefIndex() )
	{
		RecalcDisguiseWeapon();
	}

	if ( m_iDisguiseWeaponModelIndex != m_iOldDisguiseWeaponModelIndex )
	{
		C_BaseCombatWeapon *pWeapon = m_pOuter->GetActiveWeapon();

		if( pWeapon )
		{
			pWeapon->SetModelIndex( pWeapon->GetWorldModelIndex() );
		}
	}

	if ( IsLoser() )
	{
		C_BaseCombatWeapon *pWeapon = m_pOuter->GetActiveWeapon();
		if ( pWeapon && !pWeapon->IsEffectActive( EF_NODRAW ) )
		{
			pWeapon->SetWeaponVisible( false );
		}
	}

	if ( sv_showplayerhitboxes.GetInt() > 0 )
	{
		CBasePlayer *lagPlayer = UTIL_PlayerByIndex( sv_showplayerhitboxes.GetInt() );
		if ( lagPlayer )
		{
#ifdef CLIENT_DLL
			lagPlayer->DrawClientHitboxes( 0.001, true );
#else
			lagPlayer->DrawServerHitboxes( 0.001, true );
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: check the newly networked conditions for changes
//-----------------------------------------------------------------------------
void CTFPlayerShared::SyncConditions( int nCond, int nOldCond, int nUnused, int iOffset )
{
	if ( nCond == nOldCond )
		return;

	int nCondChanged = nCond ^ nOldCond;
	int nCondAdded = nCondChanged & nCond;
	int nCondRemoved = nCondChanged & nOldCond;

	int i;
	for ( i = 0; i < 32; i++ )
	{
		if ( nCondAdded & (1<<i) )
		{
			OnConditionAdded( i + iOffset );
		}
		else if ( nCondRemoved & (1<<i) )
		{
			OnConditionRemoved( i + iOffset );
		}
	}
}

#endif // CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: Remove any conditions affecting players
//-----------------------------------------------------------------------------
void CTFPlayerShared::RemoveAllCond( void )
{
	int i;
	for ( i = 0; i < TF_COND_LAST; i++ )
	{
		if ( InCond( i ) )
		{
			RemoveCond( i );
		}
	}

	// Now remove all the rest
	m_nPlayerCond = 0;
	m_nPlayerCondEx = 0;
	m_nPlayerCondEx2 = 0;
	m_nPlayerCondEx3 = 0;
	m_nPlayerCondEx4 = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Called on both client and server. Server when we add the bit,
// and client when it recieves the new cond bits and finds one added
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnConditionAdded( int nCond )
{
	switch ( nCond )
	{
	case TF_COND_HEALTH_BUFF:
#ifdef GAME_DLL
		m_flHealFraction = 0;
		m_flDisguiseHealFraction = 0;
#endif
		break;

	case TF_COND_STEALTHED:
	case TF_COND_STEALTHED_USER_BUFF:
		OnAddStealthed();
		break;

	case TF_COND_INVULNERABLE:
	case TF_COND_INVULNERABLE_USER_BUFF:
	case TF_COND_INVULNERABLE_CARD_EFFECT:
		OnAddInvulnerable();
		break;

	case TF_COND_TELEPORTED:
		OnAddTeleported();
		break;

	case TF_COND_DISGUISING:
		OnAddDisguising();
		break;

	case TF_COND_DISGUISED:
		OnAddDisguised();
		break;

	case TF_COND_TAUNTING:
		OnAddTaunting();
		break;

	case TF_COND_SHIELD_CHARGE:
		OnAddShieldCharge();
		break;

	#ifdef CLIENT_DLL
	case TF_COND_CRITBOOSTED_DEMO_CHARGE:
		m_pOuter->StopSound( "DemoCharge.ChargeCritOn" );
		m_pOuter->EmitSound( "DemoCharge.ChargeCritOn" );
		UpdateCritBoostEffect();
		break;
	#endif

	case TF_COND_MEGAHEAL:
		OnAddMegaHeal();
		break;

	case TF_COND_CRITBOOSTED:
	case TF_COND_CRITBOOSTED_PUMPKIN:
	case TF_COND_CRITBOOSTED_USER_BUFF:
	case TF_COND_CRITBOOSTED_FIRST_BLOOD:
	case TF_COND_CRITBOOSTED_BONUS_TIME:
	case TF_COND_CRITBOOSTED_CTF_CAPTURE:
	case TF_COND_CRITBOOSTED_ON_KILL:
	case TF_COND_CRITBOOSTED_CARD_EFFECT:
	case TF_COND_CRITBOOSTED_RUNE_TEMP:
	//case TF_COND_NOHEALINGDAMAGEBUFF: // this one doesn't have spark effect.
	case TF_COND_MINICRITBOOSTED_ON_KILL:
		OnAddCritboosted();
		break;

	case TF_COND_BURNING:
		OnAddBurning();
		break;

	case TF_COND_BLEEDING:
	case TF_COND_GRAPPLINGHOOK_BLEEDING:
		OnAddBleeding();
		break;
		
	case TF_COND_PHASE:
		OnAddPhase();
		break;

		
	case TF_COND_HEALTH_OVERHEALED:
#ifdef CLIENT_DLL
		m_pOuter->UpdateOverhealEffect();
#endif
		break;

	case TF_COND_DISGUISED_AS_DISPENSER:
	case TF_COND_ENERGY_BUFF:
		m_pOuter->TeamFortress_SetSpeed();
		break;

	case TF_COND_HALLOWEEN_GIANT:
		OnAddHalloweenGiant();
		break;

	case TF_COND_HALLOWEEN_TINY:
		OnAddHalloweenTiny();
		break;

	case TF_COND_STUNNED:
		OnAddStunned();
		break;

	case TF_COND_URINE:
	case TF_COND_SWIMMING_CURSE:
		OnAddUrine();
		break;

	case TF_COND_MAD_MILK:
		OnAddMadMilk();
		break;

	case TF_COND_GAS:
		OnAddCondGas();
		break;

	case TF_COND_PARACHUTE_ACTIVE:
		OnAddCondParachute();
		break;

	case TF_COND_SPEED_BOOST:
	case TF_COND_HALLOWEEN_SPEED_BOOST:
		OnAddSpeedBoost( true );
		break;

	case TF_COND_TEAM_GLOWS:
		OnAddTeamGlows();
		break;

	case TF_COND_RUNE_STRENGTH:
	case TF_COND_RUNE_HASTE:
	case TF_COND_RUNE_REGEN:
	case TF_COND_RUNE_RESIST:
	case TF_COND_RUNE_VAMPIRE:
	case TF_COND_RUNE_WARLOCK:
	case TF_COND_RUNE_PRECISION:
	case TF_COND_RUNE_AGILITY:
	case TF_COND_RUNE_KNOCKOUT:
	case TF_COND_RUNE_KING:
	case TF_COND_RUNE_PLAGUE:
	case TF_COND_RUNE_SUPERNOVA:
		OnAddRune();
		break;

	case TF_COND_OFFENSEBUFF:
	case TF_COND_DEFENSEBUFF:
	case TF_COND_REGENONDAMAGEBUFF:
		OnAddBuff();
		if ( nCond == TF_COND_REGENONDAMAGEBUFF && !InCond( TF_COND_SPEED_BOOST ) )
			AddCond( TF_COND_SPEED_BOOST );
		break;

	case TF_COND_PURGATORY:
		OnAddInPurgatory();
		break;

	case TF_COND_RADIUSHEAL:
		OnAddRadiusHeal();
		break;

	case TF_COND_FEIGN_DEATH:
		OnAddFeignDeath();
		break;

	case TF_COND_SAPPED:
		OnAddSapped();
		break;

	case TF_COND_PLAGUE:
		OnAddPlague();
		break;

	case TF_COND_MARKEDFORDEATH:
#ifdef CLIENT_DLL
		if ( !m_pMarkForDeath && !m_pOuter->IsLocalPlayer() )
			m_pMarkForDeath = m_pOuter->ParticleProp()->Create( "mark_for_death", PATTACH_POINT_FOLLOW, "partyhat" );
#endif
		break;

	case TF_COND_HALLOWEEN_THRILLER:
#ifdef CLIENT_DLL
		if ( m_pOuter == C_TFPlayer::GetLocalTFPlayer() )
		{
			m_pOuter->EmitSound( "Halloween.dance_howl" );
			m_pOuter->EmitSound( "Halloween.dance_loop" );
		}
#endif
		break;

	case TF_COND_HALLOWEEN_GHOST_MODE:
#ifdef GAME_DLL
		if ( m_pOuter->GetTeamNumber() == TF_TEAM_RED )
			Q_snprintf( m_pOuter->m_iszCustomModel.GetForModify(), MAX_PATH, "models/props_halloween/ghost_no_hat_red.mdl" );
		else
			Q_snprintf( m_pOuter->m_iszCustomModel.GetForModify(), MAX_PATH, "models/props_halloween/ghost_no_hat.mdl" );

		m_pOuter->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
#endif
		break;

	case LFE_COND_FLASHLIGHT:
		OnAddFlashlight();
		break;

	case LFE_COND_NOCLIP:
		OnAddNoclip();
		break;

	case LFE_COND_CUTSCENE:
		OnAddCutscene();
		break;

	case LFE_COND_POWERPLAY:
		OnAddPowerPlay();
		break;

	case LFE_COND_ZOMBIE_SPAWN:
		OnAddZombieSpawn();
		break;

	case LFE_COND_ZOMBIE_LEAP:
		OnAddZombieLeap();
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called on both client and server. Server when we remove the bit,
// and client when it recieves the new cond bits and finds one removed
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnConditionRemoved( int nCond )
{
	switch ( nCond )
	{
	case TF_COND_ZOOMED:
		OnRemoveZoomed();
		break;

	case TF_COND_HEALTH_BUFF:
#ifdef GAME_DLL
		m_flHealFraction = 0;
		m_flDisguiseHealFraction = 0;
#endif
		break;

	case TF_COND_STEALTHED:
		OnRemoveStealthed();
		break;

	case TF_COND_STEALTHED_USER_BUFF:
		OnRemoveStealthed();
		FadeInvis( tf_spy_invis_unstealth_time.GetFloat() );
		break;

	case TF_COND_DISGUISED:
		OnRemoveDisguised();
		break;

	case TF_COND_DISGUISING:
		OnRemoveDisguising();
		break;

	case TF_COND_INVULNERABLE:
	case TF_COND_INVULNERABLE_USER_BUFF:
	case TF_COND_INVULNERABLE_CARD_EFFECT:
		OnRemoveInvulnerable();
		break;

	case TF_COND_TELEPORTED:
		OnRemoveTeleported();
		break;

	case TF_COND_TAUNTING:
		OnRemoveTaunting();
		break;

	case TF_COND_SHIELD_CHARGE:
		OnRemoveShieldCharge();
		break;

	#ifdef CLIENT_DLL
	case TF_COND_CRITBOOSTED_DEMO_CHARGE:
		m_pOuter->StopSound( "DemoCharge.ChargeCritOn" );
		m_pOuter->EmitSound( "DemoCharge.ChargeCritOff" );
		UpdateCritBoostEffect();
		break;
	#endif

	case TF_COND_MEGAHEAL:
	case TF_COND_HALLOWEEN_HELL_HEAL:
		OnRemoveMegaHeal();
		break;

	case TF_COND_CRITBOOSTED:
	case TF_COND_CRITBOOSTED_PUMPKIN:
	case TF_COND_CRITBOOSTED_USER_BUFF:
	case TF_COND_CRITBOOSTED_FIRST_BLOOD:
	case TF_COND_CRITBOOSTED_BONUS_TIME:
	case TF_COND_CRITBOOSTED_CTF_CAPTURE:
	case TF_COND_CRITBOOSTED_ON_KILL:
	case TF_COND_CRITBOOSTED_CARD_EFFECT:
	case TF_COND_CRITBOOSTED_RUNE_TEMP:
	//case TF_COND_NOHEALINGDAMAGEBUFF: // this one doesn't have spark effect.
	case TF_COND_MINICRITBOOSTED_ON_KILL:
		OnRemoveCritboosted();
		break;

	case TF_COND_BURNING:
		OnRemoveBurning();
		break;

	case TF_COND_BLEEDING:
	case TF_COND_GRAPPLINGHOOK_BLEEDING:
		OnRemoveBleeding();
		break;

	case TF_COND_PHASE:
		OnRemovePhase();
		break;

	case TF_COND_HEALTH_OVERHEALED:
#ifdef CLIENT_DLL
		m_pOuter->UpdateOverhealEffect();
#endif
		break;

	case TF_COND_DISGUISED_AS_DISPENSER:
	case TF_COND_ENERGY_BUFF:
		m_pOuter->TeamFortress_SetSpeed();
		break;

	case TF_COND_HALLOWEEN_GIANT:
		OnRemoveHalloweenGiant();
		break;

	case TF_COND_HALLOWEEN_TINY:
		OnRemoveHalloweenTiny();
		break;

	case TF_COND_STUNNED:
		OnRemoveStunned();
		break;

	case TF_COND_URINE:
	case TF_COND_SWIMMING_CURSE:
		OnRemoveUrine();
		break;

	case TF_COND_MAD_MILK:
		OnRemoveMadMilk();
		break;

	case TF_COND_GAS:
		OnRemoveCondGas();
		break;

	case TF_COND_PARACHUTE_ACTIVE:
		OnRemoveCondParachute();
		break;

	case TF_COND_SPEED_BOOST:
	case TF_COND_HALLOWEEN_SPEED_BOOST:
		OnRemoveSpeedBoost();
		break;

	case TF_COND_TEAM_GLOWS:
		OnRemoveTeamGlows();
		break;

	case TF_COND_RUNE_STRENGTH:
	case TF_COND_RUNE_HASTE:
	case TF_COND_RUNE_REGEN:
	case TF_COND_RUNE_RESIST:
	case TF_COND_RUNE_VAMPIRE:
	case TF_COND_RUNE_WARLOCK:
	case TF_COND_RUNE_PRECISION:
	case TF_COND_RUNE_AGILITY:
	case TF_COND_RUNE_KNOCKOUT:
	case TF_COND_RUNE_KING:
	case TF_COND_RUNE_PLAGUE:
	case TF_COND_RUNE_SUPERNOVA:
		OnRemoveRune();
		break;

	case TF_COND_OFFENSEBUFF:
	case TF_COND_DEFENSEBUFF:
	case TF_COND_REGENONDAMAGEBUFF:
		OnRemoveBuff();
		if ( nCond == TF_COND_REGENONDAMAGEBUFF && InCond( TF_COND_SPEED_BOOST ) )
			RemoveCond( TF_COND_SPEED_BOOST );
		break;

	case TF_COND_PURGATORY:
		OnRemoveInPurgatory();
		break;

	case TF_COND_RADIUSHEAL:
		OnRemoveRadiusHeal();
		break;

	case TF_COND_FEIGN_DEATH:
		OnRemoveFeignDeath();
		break;

	case TF_COND_MARKEDFORDEATH:
#ifdef CLIENT_DLL
		if ( m_pMarkForDeath )
		{
			m_pOuter->ParticleProp()->StopEmission( m_pMarkForDeath );
			m_pMarkForDeath = NULL;
		}
#endif
		break;

	case TF_COND_HALLOWEEN_THRILLER:
#ifdef GAME_DLL
	StopHealing( m_pOuter );
#else
	if ( m_pOuter == C_TFPlayer::GetLocalTFPlayer() )
		m_pOuter->StopSound( "Halloween.dance_loop" );
#endif
		break;

	case TF_COND_SAPPED:
		OnRemoveSapped();
		break;

	case TF_COND_HALLOWEEN_GHOST_MODE:
#ifdef GAME_DLL
		Q_snprintf( m_pOuter->m_iszCustomModel.GetForModify(), MAX_PATH, "" );
		m_pOuter->SetMoveType( MOVETYPE_WALK );
#endif
		break;

	case LFE_COND_FLASHLIGHT:
		OnRemoveFlashlight();
		break;

	case LFE_COND_NOCLIP:
		OnRemoveNoclip();
		break;

	case LFE_COND_CUTSCENE:
		OnRemoveCutscene();
		break;

	case LFE_COND_POWERPLAY:
		OnRemovePowerPlay();
		break;

	case LFE_COND_ZOMBIE_SPAWN:
		OnRemoveZombieSpawn();
		break;

	case LFE_COND_ZOMBIE_LEAP:
		OnRemoveZombieLeap();
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns max bonus health for this player
//-----------------------------------------------------------------------------
int CTFPlayerShared::GetMaxBuffedHealth( void )
{
	float flBoostMax = m_pOuter->GetMaxHealthForBuffing() * tf_max_health_boost.GetFloat();

	int iRoundDown = floor( flBoostMax / 5 );
	iRoundDown = iRoundDown * 5;

	return iRoundDown;
}

//-----------------------------------------------------------------------------
// Purpose: returns disguise class max health for this player
//-----------------------------------------------------------------------------
int CTFPlayerShared::GetDisguiseMaxBuffedHealth( void )
{
	float flBoostMax = GetDisguiseMaxHealth() * tf_max_health_boost.GetFloat();

	int iRoundDown = floor( flBoostMax / 5 );
	iRoundDown = iRoundDown * 5;

	return iRoundDown;
}

//-----------------------------------------------------------------------------
// Purpose: Runs SERVER SIDE only Condition Think
// If a player needs something to be updated no matter what do it here (invul, etc).
//-----------------------------------------------------------------------------
void CTFPlayerShared::ConditionGameRulesThink( void )
{
#ifdef GAME_DLL
	if ( m_flNextCritUpdate < gpGlobals->curtime )
	{
		UpdateCritMult();
		m_flNextCritUpdate = gpGlobals->curtime + 0.5;
	}

	int i;
	for ( i = 0; i < TF_COND_LAST; i++ )
	{
		if ( InCond( i ) )
		{
			// Ignore permanent conditions
			if ( m_flCondExpireTimeLeft[i] != PERMANENT_CONDITION )
			{
				float flReduction = gpGlobals->frametime;

				if ( ConditionExpiresFast( i ) )
				{
					// If we're being healed, we reduce bad conditions faster
					if ( m_aHealers.Count() > 0 )
					{
						if ( i == TF_COND_URINE || i == TF_COND_MAD_MILK || i == TF_COND_GAS )
							flReduction *= m_aHealers.Count() + 1;
						else
							flReduction += ( m_aHealers.Count() * flReduction * 4 );
					}
				}

				m_flCondExpireTimeLeft.Set( i, max( m_flCondExpireTimeLeft[i] - flReduction, 0 ) );

				if ( m_flCondExpireTimeLeft[i] == 0 )
				{
					RemoveCond( i );
				}
			}
		}
	}

	// Our health will only decay ( from being medic buffed ) if we are not being healed by a medic
	// Dispensers can give us the TF_COND_HEALTH_BUFF, but will not maintain or give us health above 100%s
	bool bDecayHealth = true;

	// Get our overheal differences, and our base overheal.
	int iOverhealDifference = ( GetMaxBuffedHealth() - m_pOuter->GetMaxHealth() );

	// If we're being healed, heal ourselves
	if ( InCond( TF_COND_HEALTH_BUFF ) )
	{
		float flMaxOverhealRatio = 0.0;
		float flOverhealAmount;

		// Heal faster if we haven't been in combat for a while
		float flTimeSinceDamage = gpGlobals->curtime - m_pOuter->GetLastDamageTime();
		float flScale = RemapValClamped( flTimeSinceDamage, 10, 15, 1.0, 3.0 );

		bool bHasFullHealth = m_pOuter->GetHealth() >= m_pOuter->GetMaxHealth();

		float fTotalHealAmount = 0.0f;
		for ( int i = 0; i < m_aHealers.Count(); i++ )
		{
			// Dispensers refill cloak.
			if ( m_aHealers[i].bDispenserHeal )
			{
				AddToSpyCloakMeter( m_aHealers[i].flAmount * gpGlobals->frametime );
			}

			// Dispensers don't heal above 100%
			if ( bHasFullHealth && m_aHealers[i].bDispenserHeal )
			{
				continue;
			}

			// Being healed by a medigun, don't decay our health
			bDecayHealth = false;

			// Dispensers heal at a constant rate
			if ( m_aHealers[i].bDispenserHeal )
			{
				// Dispensers heal at a slower rate, but ignore flScale
				m_flHealFraction += gpGlobals->frametime * m_aHealers[i].flAmount;
			}
			else
			{
				// We're being healed by a medic
				flOverhealAmount = 1.0f;
				// Check our overheal level, and cap if necessary.
				if ( m_aHealers[i].pPlayer.IsValid() )
				{
					CTFPlayer *pHealer = static_cast< CTFPlayer  *>( static_cast< CBaseEntity  *>( m_aHealers[i].pPlayer ) );
					CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pHealer, flOverhealAmount, mult_medigun_overheal_amount );
				}
						
				// Iterate our overheal amount, if we're a higher value.
				if ( flOverhealAmount > flMaxOverhealRatio )
					flMaxOverhealRatio = flOverhealAmount;

				// Check our healer's overheal attribute.
				if ( bHasFullHealth )
				{			
					// Calculate out the max health we can heal up to for the person.
					int iMaxOverheal = floor( ( iOverhealDifference * flOverhealAmount ) + m_pOuter->GetMaxHealth() );
					// Don't heal if our health is above the overheal ratio.
					if ( m_pOuter->GetHealth() > iMaxOverheal )
						continue;
				}
					// Player heals are affected by the last damage time
					m_flHealFraction += gpGlobals->frametime * m_aHealers[i].flAmount * flScale;
			}

			fTotalHealAmount += m_aHealers[i].flAmount;
		}

		int nHealthToAdd = ( int )m_flHealFraction;
		if ( nHealthToAdd > 0 )
		{
			m_flHealFraction -= nHealthToAdd;

			// Overheal modifier attributes
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flMaxOverhealRatio, mult_patient_overheal_penalty );
			CTFWeaponBase *pWeapon = m_pOuter->GetActiveTFWeapon();
			if ( pWeapon )
			{
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flMaxOverhealRatio, mult_patient_overheal_penalty_active );
			}

			// Modify our max overheal.
			int iBoostMax;
			if ( flMaxOverhealRatio != 1.0f )
				iBoostMax = ( ( iOverhealDifference ) * flMaxOverhealRatio ) + m_pOuter->GetMaxHealth();
			else
				iBoostMax = GetMaxBuffedHealth();

			if ( InCond( TF_COND_DISGUISED ) )
			{
				// Separate cap for disguised health
				int iFakeBoostMax = GetDisguiseMaxBuffedHealth();
				int nFakeHealthToAdd = clamp( nHealthToAdd, 0, iFakeBoostMax - m_iDisguiseHealth );
				m_iDisguiseHealth += nFakeHealthToAdd;
			}

			// Cap it to the max we'll boost a player's health
			CALL_ATTRIB_HOOK_INT_ON_OTHER( m_pOuter, nHealthToAdd, mult_health_fromhealers );
			nHealthToAdd = clamp( nHealthToAdd, 0, ( iBoostMax - m_pOuter->GetHealth() ) );
			m_pOuter->TakeHealth( nHealthToAdd, DMG_IGNORE_MAXHEALTH );

			// split up total healing based on the amount each healer contributes
			for ( int i = 0; i < m_aHealers.Count(); i++ )
			{
				if ( m_aHealers[i].pPlayer.IsValid() )
				{
					CTFPlayer *pPlayer = static_cast<CTFPlayer *>( static_cast<CBaseEntity *>( m_aHealers[i].pPlayer ) );
					float flAmount = 0.0f;

					if ( IsAlly( pPlayer ) )
					{
						flAmount = (float)nHealthToAdd * ( m_aHealers[i].flAmount / fTotalHealAmount );
						CTF_GameStats.Event_PlayerHealedOther( pPlayer, flAmount );
					}
					else
					{
						flAmount = (float)nHealthToAdd * ( m_aHealers[i].flAmount / fTotalHealAmount );
						CTF_GameStats.Event_PlayerLeachedHealth( m_pOuter, m_aHealers[i].bDispenserHeal, flAmount );
					}
					// Store off how much this guy healed.
					m_aHealers[i].iRecentAmount += nHealthToAdd;

					// Show how much this player healed every second.
					if ( gpGlobals->curtime >= m_aHealers[i].flNextNofityTime )
					{
						if ( m_aHealers[i].iRecentAmount > 0 )
						{
							IGameEvent *event = gameeventmanager->CreateEvent( "player_healed" );
							if ( event )
							{
								event->SetInt( "priority", 1 );
								event->SetInt( "patient", m_pOuter->GetUserID() );
								event->SetInt( "healer", pPlayer->GetUserID() );
								event->SetInt( "amount", m_aHealers[i].iRecentAmount );

								gameeventmanager->FireEvent( event );
							}
						}

						m_aHealers[i].iRecentAmount = 0;
						m_aHealers[i].flNextNofityTime = gpGlobals->curtime + 1.0f;
					}

					CWeaponMedigun *pMedigun = pPlayer->GetMedigun();
					if ( pMedigun && ( m_pOuter->GetHealth() != GetMaxBuffedHealth() ) )
					{
						int nRageOnHeal = 0;
						CALL_ATTRIB_HOOK_INT_ON_OTHER( pMedigun, nRageOnHeal, generate_rage_on_heal );
						if ( nRageOnHeal != 0 )
							pMedigun->SetEnergyMeter( flAmount / 2.0f );
					}
				}
			}

		}

		if ( InCond( TF_COND_BURNING ) )
		{
			// Reduce the duration of this burn 
			float flReduction = 2;	 // ( flReduction + 1 ) x faster reduction
			m_flFlameRemoveTime -= flReduction * gpGlobals->frametime;
		}

		if ( InCond( TF_COND_BLEEDING ) )
		{
			for ( int i=0; i<m_aBleeds.Count(); ++i )
			{
				bleed_struct_t *bleed = &m_aBleeds[i];
				bleed->m_flEndTime -= gpGlobals->frametime + gpGlobals->frametime;
			}
		}
	}

	if ( bDecayHealth )
	{
		// If we're not being buffed, our health drains back to our max
		if ( m_pOuter->GetHealth() > m_pOuter->GetMaxHealth() )
		{
			float flBoostMaxAmount = GetMaxBuffedHealth() - m_pOuter->GetMaxHealth();
			m_flHealFraction += ( gpGlobals->frametime * ( flBoostMaxAmount / tf_boost_drain_time.GetFloat() ) );

			if ( m_pOuter->GetActiveTFWeapon() )
			{
				float flMaxHealthDrainRate = 0;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter->GetActiveTFWeapon(), flMaxHealthDrainRate, mod_maxhealth_drain_rate );
				if ( flMaxHealthDrainRate > 0)
					m_flHealFraction += ( gpGlobals->frametime * ( flMaxHealthDrainRate ) );
			}

			int nHealthToDrain = (int)m_flHealFraction;
			if ( nHealthToDrain > 0 )
			{
				m_flHealFraction -= nHealthToDrain;

				// Manually subtract the health so we don't generate pain sounds / etc
				m_pOuter->m_iHealth -= nHealthToDrain;
			}
		}

		if ( InCond( TF_COND_DISGUISED ) && m_iDisguiseHealth > m_iDisguiseMaxHealth )
		{
			float flBoostMaxAmount = GetDisguiseMaxBuffedHealth() - m_iDisguiseMaxHealth;
			m_flDisguiseHealFraction += ( gpGlobals->frametime * ( flBoostMaxAmount / tf_boost_drain_time.GetFloat() ) );

			int nHealthToDrain = (int)m_flDisguiseHealFraction;
			if ( nHealthToDrain > 0 )
			{
				m_flDisguiseHealFraction -= nHealthToDrain;

				// Reduce our fake disguised health by roughly the same amount
				m_iDisguiseHealth -= nHealthToDrain;
			}
		}
	}

	if ( m_pOuter->GetHealth() > m_pOuter->GetMaxHealth() )
	{
		if ( !InCond( TF_COND_HEALTH_OVERHEALED ) )
		{
			AddCond( TF_COND_HEALTH_OVERHEALED );
		}
	}
	else
	{
		if ( InCond( TF_COND_HEALTH_OVERHEALED ) )
		{
			RemoveCond( TF_COND_HEALTH_OVERHEALED );
		}
	}

	// Taunt
	if ( InCond( TF_COND_TAUNTING ) )
	{
		if ( gpGlobals->curtime > m_flTauntRemoveTime )
		{
			m_pOuter->ResetTauntHandle();

			//m_pOuter->SnapEyeAngles( m_pOuter->m_angTauntCamera );
			//m_pOuter->SetAbsAngles( m_pOuter->m_angTauntCamera );
			//m_pOuter->SetLocalAngles( m_pOuter->m_angTauntCamera );

			RemoveCond( TF_COND_TAUNTING );
		}
	}

	if ( InCond( TF_COND_BURNING ) && ( m_pOuter->m_flPowerPlayTime < gpGlobals->curtime ) )
	{
		// If we're underwater, put the fire out
		if ( gpGlobals->curtime > m_flFlameRemoveTime || m_pOuter->GetWaterLevel() >= WL_Waist )
		{
			RemoveCond( TF_COND_BURNING );
		}
		else if ( ( gpGlobals->curtime >= m_flFlameBurnTime ) && ( ( TF_CLASS_PYRO != m_pOuter->GetPlayerClass()->GetClassIndex() ) || !InCond( TF_COND_AFTERBURN_IMMUNE ) ) )
		{
			float flBurnDamage = TF_BURNING_DMG;
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_hBurnWeapon, flBurnDamage, mult_wpn_burndmg );

			// Burn the player (if not pyro, who does not take persistent burning damage)
			CTakeDamageInfo info( m_hBurnAttacker, m_hBurnAttacker, m_hBurnWeapon, flBurnDamage, DMG_BURN | DMG_PREVENT_PHYSICS_FORCE, TF_DMG_CUSTOM_BURNING );
			m_pOuter->TakeDamage( info );
			m_flFlameBurnTime = gpGlobals->curtime + TF_BURNING_FREQUENCY;
		}

		if ( m_flNextBurningSound < gpGlobals->curtime )
		{
			m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_ONFIRE );
			m_flNextBurningSound = gpGlobals->curtime + 2.5f;
		}
	}

	if ( InCond( TF_COND_BLEEDING ) )
	{
		for (int i = m_aBleeds.Count() - 1; i >= 0; --i)
		{
			bleed_struct_t *bleed = &m_aBleeds[i];
			if ( gpGlobals->curtime >= bleed->m_flEndTime )
			{
				m_aBleeds.FastRemove( i );
				continue;
			}
			else if ( gpGlobals->curtime >= bleed->m_flBleedTime )
			{
				bleed->m_flBleedTime = gpGlobals->curtime + TF_BLEEDING_FREQUENCY;

				CTakeDamageInfo info( bleed->m_hAttacker, bleed->m_hAttacker, bleed->m_hWeapon, (float)bleed->m_iDamage, DMG_SLASH, TF_DMG_CUSTOM_BLEEDING );
				m_pOuter->TakeDamage( info );
			}
		}

		if ( m_aBleeds.IsEmpty() )
			RemoveCond( TF_COND_BLEEDING );
	}

	if ( IsJared() && m_pOuter->GetWaterLevel() >= WL_Waist )
 	{
		RemoveCond( TF_COND_URINE );
		RemoveCond( TF_COND_MAD_MILK );
		RemoveCond( TF_COND_GAS );
	}

	if ( InCond( TF_COND_DISGUISING ) )
	{
		if ( gpGlobals->curtime > m_flDisguiseCompleteTime )
		{
			CompleteDisguise();
		}
	}

	// Stops the drain hack.
	if ( m_pOuter->IsPlayerClass( TF_CLASS_MEDIC ) || m_pOuter->Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) )
	{
		CWeaponMedigun *pWeapon = m_pOuter->GetMedigun();
		if ( pWeapon && pWeapon->IsReleasingCharge() )
		{
			pWeapon->DrainCharge();
		}
	}

	TestAndExpireChargeEffect( TF_CHARGE_INVULNERABLE );
	TestAndExpireChargeEffect( TF_CHARGE_CRITBOOSTED );
	TestAndExpireChargeEffect( TF_CHARGE_MEGAHEAL );

	if ( InCond( TF_COND_STEALTHED_BLINK ) )
	{
		if ( TF_SPY_STEALTH_BLINKTIME/*tf_spy_stealth_blink_time.GetFloat()*/ < ( gpGlobals->curtime - m_flLastStealthExposeTime ) )
		{
			RemoveCond( TF_COND_STEALTHED_BLINK );
		}
	}

	if ( InCond( TF_COND_STUNNED ) )
	{
		if ( gpGlobals->curtime > m_flStunExpireTime )
		{
			// Only check stun phase if we're unable to move
			if ( !( m_nStunFlags & TF_STUNFLAG_BONKSTUCK ) || m_iStunPhase == STUN_PHASE_END )
			{
				RemoveCond( TF_COND_STUNNED );
			}
		}
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do CLIENT/SERVER SHARED condition thinks.
//-----------------------------------------------------------------------------
void CTFPlayerShared::ConditionThink( void )
{
	bool bIsLocalPlayer = false;
#ifdef CLIENT_DLL
	bIsLocalPlayer = m_pOuter->IsLocalPlayer();
#else
	bIsLocalPlayer = true;
#endif

	if ( InCond( TF_COND_PHASE ) )
	{
		if ( gpGlobals->curtime > m_flPhaseTime )
		{
			UpdatePhaseEffects();

			// limit how often we can update in case of spam
			m_flPhaseTime = gpGlobals->curtime + 0.25f;
		}
	}

	if ( InCond( TF_COND_SPEED_BOOST ) )
		UpdateSpeedBoostEffects();

	UpdateRageBuffsAndRage();

#ifdef GAME_DLL
	UpdateCloakMeter();
	UpdateChargeMeter();
	UpdateEnergyDrinkMeter();
#endif

	if ( InCond( TF_COND_RUNE_KING ) )
	{
		m_bKingRuneBuffActive = true;
		PulseKingRuneBuff();
	}
	else
	{
		m_bKingRuneBuffActive = false;
	}

	// Supernova
	if ( CanRuneCharge() )
	{
		#ifdef GAME_DLL
		float flChargeTime = tf_powerup_max_charge_time.GetFloat();
		m_flRuneCharge += ( 100.0f / flChargeTime ) * gpGlobals->frametime;
		#endif

		if ( m_flRuneCharge >= 100.0f )
		{
			m_flRuneCharge = 100.0f;
			ClientPrint( m_pOuter, HUD_PRINTCENTER, "TF_Powerup_Supernova_Deploy" );
		}
	}

	if ( lfe_episodic_flashlight.GetBool() )
		UpdateFlashlightBattery();
	else
		SetFlashlightBattery( 100.0f );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::ClientShieldChargeThink( void )
{
	if (m_bShieldChargeStopped && ( gpGlobals->curtime - m_flShieldChargeEndTime ) >= 0.3f)
	{
		RemoveCond( TF_COND_CRITBOOSTED_DEMO_CHARGE );

		m_pOuter->StopSound( "DemoCharge.ChargeCritOn" );
		m_pOuter->EmitSound( "DemoCharge.ChargeCritOff" );

		UpdateCritBoostEffect();

		m_bShieldChargeStopped = false;
	}
	else if ( m_flChargeMeter < 75.0f && InCond( TF_COND_SHIELD_CHARGE ) )
	{
		AddCond( TF_COND_CRITBOOSTED_DEMO_CHARGE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::ClientDemoBuffThink( void )
{
	if ( m_iDecapitations > 0 && m_iDecapitations != m_iDecapitationsParity )
	{
		m_iDecapitationsParity = m_iDecapitations;
		if ( m_pOuter->IsPlayerClass( TF_CLASS_DEMOMAN ) )
			m_pOuter->UpdateDemomanEyeEffect( m_iDecapitations );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveZoomed( void )
{
#ifdef GAME_DLL
	m_pOuter->SetFOV( m_pOuter, 0, 0.1f );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddDisguising( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->m_pDisguisingEffect )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pOuter->m_pDisguisingEffect );
	}

	if ( ( !m_pOuter->IsLocalPlayer() || !m_pOuter->InFirstPersonView() ) && ( !IsStealthed() || !m_pOuter->IsEnemyPlayer() ) )
	{
		const char *pszEffectName = ConstructTeamParticle( "spy_start_disguise_%s", m_pOuter->GetTeamNumber() );

		m_pOuter->m_pDisguisingEffect = m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
		m_pOuter->m_flDisguiseEffectStartTime = gpGlobals->curtime;
	}

	m_pOuter->EmitSound( "Player.Spy_Disguise" );

#endif
}

//-----------------------------------------------------------------------------
// Purpose: set up effects for when player finished disguising
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddDisguised( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->m_pDisguisingEffect )
	{
		// turn off disguising particles
		m_pOuter->ParticleProp()->StopEmission( m_pOuter->m_pDisguisingEffect );
		m_pOuter->m_pDisguisingEffect = NULL;
	}
	m_pOuter->m_flDisguiseEndEffectStartTime = gpGlobals->curtime;
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: start, end, and changing disguise classes
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnDisguiseChanged( void )
{
	// recalc disguise model index
	//RecalcDisguiseWeapon();
	UpdateCritBoostEffect();
	m_pOuter->UpdateOverhealEffect();
	m_pOuter->UpdateRecentlyTeleportedEffect();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddInvulnerable( void )
{
#ifndef CLIENT_DLL
	// Stock uber removes negative conditions.
	if ( InCond( TF_COND_BURNING ) )
	{
		RemoveCond( TF_COND_BURNING );
	}

	if ( InCond( TF_COND_BLEEDING ) )
	{
		RemoveCond( TF_COND_BLEEDING );
	}

	if ( IsJared() )
	{
		RemoveCond( TF_COND_URINE );
		RemoveCond( TF_COND_MAD_MILK );
		RemoveCond( TF_COND_GAS );
	}
#else
	if ( m_pOuter->IsLocalPlayer() )
	{
		char *pEffectName = NULL;

		switch( m_pOuter->GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		case TF_TEAM_RED:
			pEffectName =  "effects/invuln_overlay_red";
			break;
		default:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		}

		IMaterial *pMaterial = materials->FindMaterial( pEffectName, TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}

	if ( !m_pInvulnerableSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		CLocalPlayerFilter filter;
		m_pInvulnerableSound = controller.SoundCreate( filter, m_pOuter->entindex(), "TFPlayer.InvulnerableIdle" );
		controller.Play( m_pInvulnerableSound, 1.0, 100 );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveInvulnerable( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}

	if ( m_pInvulnerableSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pInvulnerableSound );
		m_pInvulnerableSound = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddMegaHeal( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		char *pEffectName = NULL;

		switch( m_pOuter->GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		case TF_TEAM_RED:
			pEffectName =  "effects/invuln_overlay_red";
			break;
		default:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		}

		IMaterial *pMaterial = materials->FindMaterial( pEffectName, TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}

	// Start the heal effect
	if ( !m_pMegaHeal )
	{
		const char *pszEffectName = ConstructTeamParticle( "medic_megaheal_%s", m_pOuter->GetTeamNumber() );

		m_pMegaHeal = m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
#else
	Heal( m_pOuter, 24 * 3 );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveMegaHeal( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}

	if ( m_pMegaHeal )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pMegaHeal );
		m_pMegaHeal = NULL;
	}
#else
	StopHealing( m_pOuter );
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::ShouldShowRecentlyTeleported( void )
{
	if ( m_pOuter->IsPlayerClass( TF_CLASS_SPY ) )
	{
		if ( IsStealthed() )
			return false;

		if ( InCond( TF_COND_DISGUISED ) && ( m_pOuter->IsLocalPlayer() || m_pOuter->IsEnemyPlayer() ) )
		{
			if ( GetDisguiseTeam() != m_nTeamTeleporterUsed )
				return false;
		}
		else if ( m_pOuter->GetTeamNumber() != m_nTeamTeleporterUsed )
			return false;
	}

	return ( InCond( TF_COND_TELEPORTED ) );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddTeleported( void )
{
#ifdef CLIENT_DLL
	m_pOuter->UpdateRecentlyTeleportedEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveTeleported( void )
{
#ifdef CLIENT_DLL
	m_pOuter->UpdateRecentlyTeleportedEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddTaunting( void )
{
	CTFWeaponBase *pWpn = m_pOuter->GetActiveTFWeapon();
	if ( pWpn )
	{
		// cancel any reload in progress.
		pWpn->AbortReload();
	}

#ifdef CLIENT_DLL
	C_EconWearable *pTaunt = m_pOuter->GetWearableForLoadoutSlot( LOADOUT_POSITION_ACTION );
	if ( pTaunt )
	{
		if ( pTaunt->GetItem() )
		{
			if ( pTaunt->GetItem()->GetStaticData() )
			{
				if ( pTaunt->GetItem()->GetStaticData()->is_partner_taunt )
					m_pOuter->CreateTauntWithMeEffect();
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveTaunting( void )
{
#ifdef CLIENT_DLL
	m_pOuter->StopTauntWithMeEffect();
	m_pOuter->ParticleProp()->StopParticlesNamed( "taunt_heavy_table_steam" );
#else
	m_pOuter->ClearTauntAttack();
	m_pOuter->StopTaunt();
	m_pOuter->SetCurrentTauntMoveSpeed( 0.0f );

	if ( GetActiveTFWeapon() )
		GetActiveTFWeapon()->SetWeaponVisible( true );
#endif
	m_pOuter->m_bAllowMoveDuringTaunt = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddShieldCharge( void )
{
	m_pOuter->TeamFortress_SetSpeed();

	m_pOuter->EmitSound( "DemoCharge.Charging" );

	UpdatePhaseEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveShieldCharge( void )
{
	m_pOuter->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	for (int i = 0; i < m_pPhaseTrails.Count(); i++)
	{
		UTIL_Remove( m_pPhaseTrails[i] );
	}
	m_pPhaseTrails.RemoveAll();
#else
	m_pOuter->ParticleProp()->StopEmission( m_pWarp );
	m_pWarp = NULL;

	m_bShieldChargeStopped = true;
	m_flShieldChargeEndTime = gpGlobals->curtime;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddCritboosted( void )
{
#ifdef CLIENT_DLL
	UpdateCritBoostEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveCritboosted( void )
{
#ifdef CLIENT_DLL
	UpdateCritBoostEffect();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddStunned( void )
{
	if ( IsControlStunned() || IsLoser() )
	{
		RemoveCond( TF_COND_SHIELD_CHARGE );

		CTFWeaponBase *pWeapon = m_pOuter->GetActiveTFWeapon();
		if ( pWeapon )
		{
			pWeapon->OnControlStunned();
		}

		m_pOuter->TeamFortress_SetSpeed();
	}

	// Check if effects are disabled
	if ( !( m_nStunFlags & TF_STUNFLAG_NOSOUNDOREFFECT ) )
	{
#ifdef CLIENT_DLL
		if ( !m_pStun )
		{
			if ( m_nStunFlags & TF_STUNFLAG_BONKEFFECT )
			{
				// Half stun
				m_pStun = m_pOuter->ParticleProp()->Create( "conc_stars", PATTACH_POINT_FOLLOW, "head" );
			}
			else if ( m_nStunFlags & TF_STUNFLAG_GHOSTEFFECT )
			{
				// Ghost stun
				m_pStun = m_pOuter->ParticleProp()->Create( "yikes_fx", PATTACH_POINT_FOLLOW, "head" );
			}
		}
#else
		if ( ( m_nStunFlags & TF_STUNFLAG_GHOSTEFFECT ) )
		{
			// Play the scream sound
			m_pOuter->EmitSound( "Halloween.PlayerScream" );
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveStunned( void )
{
	m_flStunExpireTime = 0.0f;
	m_flStunMovementSpeed = 0.0f;
	m_flStunResistance = 0.0f;
	m_hStunner = NULL;
	m_iStunPhase = STUN_PHASE_NONE;

	m_pOuter->TeamFortress_SetSpeed();

	CTFWeaponBase *pWeapon = m_pOuter->GetActiveTFWeapon();
	if ( pWeapon )
		pWeapon->SetWeaponVisible( true );

#ifdef CLIENT_DLL
	m_pOuter->ParticleProp()->StopEmission( m_pStun );
	m_pStun = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddHalloweenGiant( void )
{
#ifdef GAME_DLL
	m_pOuter->SetModelScale( 2.0, 0.0 );

	m_pOuter->SetHealth( m_pOuter->GetMaxHealth() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveHalloweenGiant( void )
{
#ifdef GAME_DLL
	if ( lfe_use_hl2_player_hull.GetBool() )
		m_pOuter->SetModelScale( 0.9, 0.0 );
	else
		m_pOuter->SetModelScale( 1.0, 0.0 );

	m_pOuter->SetHealth( m_pOuter->GetMaxHealth() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddPhase( void )
{
#ifdef GAME_DLL
	m_pOuter->DropFlag();
#endif
	UpdatePhaseEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemovePhase( void )
{
#ifdef GAME_DLL
	m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_TIRED );

	for ( int i = 0; i < m_pPhaseTrails.Count(); i++ )
	{
		m_pPhaseTrails[i]->SUB_Remove();
	}
	m_pPhaseTrails.RemoveAll();
#else
	m_pOuter->ParticleProp()->StopEmission( m_pWarp );
	m_pWarp = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddHalloweenTiny( void )
{
#ifdef GAME_DLL
	m_pOuter->SetModelScale( 0.5, 0.0 );
#endif
	AddCond( TF_COND_SPEED_BOOST );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveHalloweenTiny( void )
{
#ifdef GAME_DLL
	m_pOuter->SetModelScale( 1.0, 0.0 );
#endif
	RemoveCond( TF_COND_SPEED_BOOST );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddTeamGlows( void )
{
// need to be client.
/*#ifdef GAME_DLL
	for ( int i = 1;  i <= MAX_PLAYERS; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			if ( pPlayer->InSameTeam( m_pOuter ) )
			{
				if ( pPlayer->IsAlive() )
				{
					pPlayer->AddGlowEffect();
				}

			}
		}
	}
#else
#endif*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveTeamGlows( void )
{
// need to be client.
/*#ifdef GAME_DLL
	for ( int i = 1;  i <= MAX_PLAYERS; i++ )
	{
		CTFPlayer *pPlayer = ToTFPlayer( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			if ( pPlayer->InSameTeam( m_pOuter ) )
			{
				if ( pPlayer->IsAlive() )
				{
					pPlayer->RemoveGlowEffect();
				}

			}
		}
	}
#else
#endif*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddSpeedBoost( bool bParticle )
{
#ifdef GAME_DLL
	if ( !m_pOuter->IsInAVehicle() )
	{
		CSingleUserRecipientFilter filter( m_pOuter );
		m_pOuter->EmitSound( filter, m_pOuter->entindex(), "DisciplineDevice.PowerUp" );
	}
#endif

	m_pOuter->TeamFortress_SetSpeed();

	if ( bParticle )
		UpdateSpeedBoostEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveSpeedBoost( void )
{
#ifdef GAME_DLL
	if ( !m_pOuter->IsInAVehicle() )
	{
		CSingleUserRecipientFilter filter( m_pOuter );
		m_pOuter->EmitSound( filter, m_pOuter->entindex(), "DisciplineDevice.PowerDown" );
	}
#endif

	m_pOuter->TeamFortress_SetSpeed();

	UpdateSpeedBoostEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddUrine( void )
{
#ifdef GAME_DLL
	m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_JARATE_HIT );
#else
	m_pOuter->ParticleProp()->Create( "peejar_drips", PATTACH_ABSORIGIN_FOLLOW ); 

	// set the piss screen overlay
	if ( m_pOuter->IsLocalPlayer() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/jarate_overlay", TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddMadMilk( void )
{
#ifdef GAME_DLL
	m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_JARATE_HIT );
#else
	m_pOuter->ParticleProp()->Create( "peejar_drips_milk", PATTACH_ABSORIGIN_FOLLOW );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddCondGas( void )
{
#ifdef GAME_DLL
	m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_JARATE_HIT );
#else
	const char *pszEffectName = ConstructTeamParticle( "gas_can_drips_%s", m_pOuter->GetTeamNumber() );
	m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW ); 

	// set the gas screen overlay
	if ( m_pOuter->IsLocalPlayer() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/gas_overlay", TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveUrine( void )
{
#ifdef GAME_DLL
	if( m_nPlayerState != TF_STATE_DYING )
	{
		m_hUrineAttacker = NULL;
	}
#else
	m_pOuter->ParticleProp()->StopParticlesNamed( "peejar_drips" );

	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveMadMilk( void )
{
#ifdef GAME_DLL
	if( m_nPlayerState != TF_STATE_DYING )
	{
		m_hUrineAttacker = NULL;
	}
#else
	m_pOuter->ParticleProp()->StopParticlesNamed( "peejar_drips_milk" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveCondGas( void )
{
#ifdef GAME_DLL
	if( m_nPlayerState != TF_STATE_DYING )
	{
		m_hUrineAttacker = NULL;
	}
#else
	m_pOuter->ParticleProp()->StopParticlesNamed( "gas_can_drips_red" );
	m_pOuter->ParticleProp()->StopParticlesNamed( "gas_can_drips_blue" );

	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddCondParachute( void )
{
#ifdef CLIENT_DLL
	m_pOuter->EmitSound( "Parachute_open" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveCondParachute( void )
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddRune( void )
{
#ifdef CLIENT_DLL
	m_pOuter->UpdateRuneIcon( true );
#endif
	m_pOuter->TeamFortress_SetSpeed();

	if ( InCond( TF_COND_RUNE_KNOCKOUT ) )
		m_pOuter->Weapon_Switch( m_pOuter->Weapon_GetSlot( LOADOUT_POSITION_MELEE ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveRune( void )
{
#ifdef CLIENT_DLL
	m_pOuter->UpdateRuneIcon( false );
#endif
	m_pOuter->TeamFortress_SetSpeed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddBuff( void )
{
#ifdef CLIENT_DLL
	// Start the buff effect
	if ( !m_pBuffAura )
	{
		const char *pszEffectName = ConstructTeamParticle( "soldierbuff_%s_buffed", m_pOuter->GetTeamNumber() );
		m_pBuffAura = m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveBuff( void )
{
#ifdef CLIENT_DLL
	if ( m_pBuffAura )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pBuffAura );
		m_pBuffAura = NULL;
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddInPurgatory( void )
{
#ifdef GAME_DLL
	m_pOuter->SetHealth( m_pOuter->GetMaxHealth() );
	m_pOuter->RemoveOwnedProjectiles();
	AddCond( TF_COND_INVULNERABLE, 1.5f );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Apply effects when player escapes the underworld
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveInPurgatory( void )
{
#ifdef GAME_DLL
	if ( m_nPlayerState != TF_STATE_DYING )
	{
		AddCond( TF_COND_INVULNERABLE, 10.0f );
		AddCond( TF_COND_SPEED_BOOST, 10.0f );
		AddCond( TF_COND_CRITBOOSTED_PUMPKIN, 10.0f );
		m_pOuter->SetHealth( GetMaxBuffedHealth() );

		m_pOuter->m_purgatoryDuration.Start( 10.0f );

		TFGameRules()->BroadcastSound( 255, "Halloween.PlayerEscapedUnderworld" );
		m_pOuter->RemoveOwnedProjectiles();

		// Write to chat that player has escaped the underworld
		CReliableBroadcastRecipientFilter filter;
		UTIL_SayText2Filter( filter, m_pOuter, false, TFGameRules()->IsHalloweenScenario( CTFGameRules::HALLOWEEN_SCENARIO_LAKESIDE ) ? "#TF_Halloween_Skull_Island_Escape" : "#TF_Halloween_Underworld", m_pOuter->GetPlayerName() );

		// Let the map know we escaped the underworld
		IGameEvent *event = gameeventmanager->CreateEvent( "escaped_loot_island" );
		if ( event )
		{
			event->SetInt( "player", m_pOuter->GetUserID() );
			gameeventmanager->FireEvent( event, true );
		}

		CTeam *pTeam = m_pOuter->GetTeam();
		if ( pTeam )
		{
			UTIL_LogPrintf( "HALLOWEEN: \"%s<%i><%s><%s>\" %s\n", m_pOuter->GetPlayerName(), m_pOuter->GetUserID(), m_pOuter->GetNetworkIDString(), pTeam->GetName(), "purgatory_escaped" );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddFeignDeath( void )
{
	AddCond( TF_COND_STEALTHED );
	AddCond( TF_COND_AFTERBURN_IMMUNE, tf_feign_death_duration.GetFloat() );
	AddCond( TF_COND_SPEED_BOOST, tf_feign_death_speed_duration.GetFloat() );
	AddCond( TF_COND_PREVENT_DEATH, 1 );

	float flConsumeOnActivate = 100.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flConsumeOnActivate, cloak_consume_on_feign_death_activate );
	SetSpyCloakMeter( min( 100.0f, flConsumeOnActivate ) );

#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		char *pEffectName = NULL;

		switch( m_pOuter->GetTeamNumber() )
		{
		case TF_TEAM_BLUE:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		case TF_TEAM_RED:
			pEffectName =  "effects/invuln_overlay_red";
			break;
		default:
			pEffectName = "effects/invuln_overlay_blue";
			break;
		}

		IMaterial *pMaterial = materials->FindMaterial( pEffectName, TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
			view->SetScreenOverlayMaterial( pMaterial );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveFeignDeath( void )
{
	if ( InCond( TF_COND_AFTERBURN_IMMUNE ) )
		RemoveCond( TF_COND_AFTERBURN_IMMUNE );

	if ( InCond( TF_COND_PREVENT_DEATH ) )
		RemoveCond( TF_COND_PREVENT_DEATH );

#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
		view->SetScreenOverlayMaterial( NULL );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddSapped( void )
{
#ifdef CLIENT_DLL
	if ( !m_pSapped )
	{
		m_pSapped = m_pOuter->ParticleProp()->Create( "sapper_sentry1_fx", PATTACH_POINT_FOLLOW, "head" );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveSapped( void )
{
#ifdef CLIENT_DLL
	if ( m_pSapped )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pSapped );
		m_pSapped = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddPlague( void )
{
#ifdef GAME_DLL
	ClientPrint( m_pOuter, HUD_PRINTCENTER, "TF_Powerup_Contract_Plague" );
	m_pOuter->EmitSound( "Powerup.PickUpPlagueInfected" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddFlashlight( void )
{
#ifdef GAME_DLL
	if ( m_pOuter->FlashlightIsOn() )
		return;

	m_pOuter->EmitSound( "HL2Player.FlashlightOn" );
	m_pOuter->AddEffects( EF_DIMLIGHT );
#else

	/*C_TFWeaponBase *pWeapon = GetActiveTFWeapon();
	if( pWeapon )
	{
		int iAttachment = pWeapon->LookupAttachment( "muzzle" );

		if ( iAttachment < 0 )
		{
			iAttachment = pWeapon->GetViewmodelAddon() ? pWeapon->GetViewmodelAddon()->LookupAttachment( "root" ) : pWeapon->LookupAttachment( "root" );
		}

		if ( !m_pOuter->ShouldDrawThisPlayer() ) // in firstperson
		{
			C_TFViewModel *pViewModel = dynamic_cast<C_TFViewModel *>( m_pOuter->GetViewModel() );
			if ( pViewModel )
			{
				if ( !m_pFlashlightBeam )
					m_pFlashlightBeam = pViewModel->ParticleProp()->Create( "flashlight_firstperson_", PATTACH_POINT_FOLLOW, iAttachment );
			}
		}
		else if ( !m_pOuter->IsDormant() ) // in thirdperson
		{
			if ( !m_pFlashlightBeam )
				m_pFlashlightBeam = pWeapon->ParticleProp()->Create( "flashlight_thirdperson", PATTACH_POINT_FOLLOW, iAttachment );
		}
	}*/
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveFlashlight( void )
{
#ifdef GAME_DLL
	m_pOuter->EmitSound( "HL2Player.FlashlightOff" );
	m_pOuter->RemoveEffects( EF_DIMLIGHT );
#else

#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddNoclip( void )
{
#ifdef GAME_DLL
	m_pOuter->SetMoveType( MOVETYPE_NOCLIP );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveNoclip( void )
{
#ifdef GAME_DLL
	m_pOuter->SetMoveType( MOVETYPE_WALK );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: some fortnite cinematic settings
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddCutscene( void )
{
	m_pOuter->m_Local.m_iHideHUD |= HIDEHUD_HEALTH | HIDEHUD_MISCSTATUS | HIDEHUD_CROSSHAIR;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveCutscene( void )
{
	m_pOuter->m_Local.m_iHideHUD &= ( ~HIDEHUD_HEALTH & ~HIDEHUD_MISCSTATUS & ~HIDEHUD_CROSSHAIR );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddRadiusHeal( void )
{
#ifdef CLIENT_DLL
	// Start the heal effect
	if ( !m_pRadiusHeal )
	{
		//const char *pszEffectName = ConstructTeamParticle( "medic_healradius_%s_buffed", m_pOuter->GetTeamNumber() );
		const char *pszEffectName = ConstructTeamParticle( "medic_megaheal_%s", m_pOuter->GetTeamNumber() );
		m_pRadiusHeal = m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveRadiusHeal( void )
{
#ifdef CLIENT_DLL
	if ( m_pRadiusHeal )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pRadiusHeal );
		m_pRadiusHeal = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddRunePlague( void )
{
#ifdef CLIENT_DLL
	// Start the heal effect
	if ( !m_pRunePlague )
		m_pRunePlague = m_pOuter->ParticleProp()->Create( "powerup_plague_carrier", PATTACH_ABSORIGIN_FOLLOW );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveRunePlague( void )
{
#ifdef CLIENT_DLL
	if ( m_pRunePlague )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pRunePlague );
		m_pRunePlague = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddPowerPlay( void )
{
#ifdef GAME_DLL
	m_pOuter->SetPowerplayEnabled( true );
	RecalculateChargeEffects();
	Burn( m_pOuter );
	UTIL_ScreenShake( m_pOuter->GetAbsOrigin(), 100.0f, 150.0, 1.0, 256.0f, SHAKE_START );
#else
	if ( !m_pOuter->ShouldDrawThisPlayer() )
	{
		m_pOuter->ParticleProp()->Create( "australium_bar_glow", PATTACH_POINT_FOLLOW, "head" );
		m_pOuter->ParticleProp()->Create( "mvm_soldier_shockwave", PATTACH_ABSORIGIN_FOLLOW );

		if ( m_bIsPlayerADev )
			m_pOuter->ParticleProp()->Create( ConstructTeamParticle( "kart_dust_trail_%s", m_pOuter->GetTeamNumber() ), PATTACH_POINT_FOLLOW, "eyeglow_l" );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemovePowerPlay( void )
{
	RemoveCond( TF_COND_BURNING );
#ifdef GAME_DLL
	m_pOuter->SetPowerplayEnabled( false );
	RecalculateChargeEffects();
#else

	m_pOuter->ParticleProp()->StopParticlesNamed( "australium_bar_glow" );
	m_pOuter->ParticleProp()->StopParticlesNamed( "mvm_soldier_shockwave" );

	if ( m_bIsPlayerADev )
		m_pOuter->ParticleProp()->StopParticlesNamed( ConstructTeamParticle( "kart_dust_trail_%s", m_pOuter->GetTeamNumber() ) );
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddZombieSpawn( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "debug/yuv", TEXTURE_GROUP_OTHER, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveZombieSpawn( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddZombieLeap( void )
{
	UpdatePhaseEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveZombieLeap( void )
{
#ifdef GAME_DLL
	for (int i = 0; i < m_pPhaseTrails.Count(); i++)
	{
		UTIL_Remove( m_pPhaseTrails[i] );
	}
	m_pPhaseTrails.RemoveAll();
#else
	m_pOuter->ParticleProp()->StopEmission( m_pWarp );
	m_pWarp = NULL;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::RecalculatePlayerBodygroups( void )
{
	/*
	int iSlot = LOADOUT_POSITION_COUNT;
	//CTFWeaponBase::UpdateWeaponBodyGroups((CTFWeaponBase *)v4, 0);

	
	CEconWearable *pWearable = GetWearableForLoadoutSlot( iSlot );
	if ( pWearable )
	{
		pWearable->UpdateWearableBodyGroups( m_pOuter->IsLocalPlayer() );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::Burn( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon /*= NULL*/, float flFlameDuration /*= -1.0f*/ )
{
#ifdef GAME_DLL
	// Don't bother igniting players who have just been killed by the fire damage.
	if ( !m_pOuter->IsAlive() )
		return;

	// pyros don't burn persistently or take persistent burning damage, but we show brief burn effect so attacker can tell they hit
	bool bVictimIsPyro = ( TF_CLASS_PYRO == m_pOuter->GetPlayerClass()->GetClassIndex() );

	if ( !InCond( TF_COND_BURNING ) )
	{
		// Start burning
		AddCond( TF_COND_BURNING );
		m_flFlameBurnTime = gpGlobals->curtime;	//asap
		// let the attacker know he burned me
		if ( pAttacker && !bVictimIsPyro )
		{
			pAttacker->OnBurnOther( m_pOuter );
		}
	}

	float flFlameLife = TF_BURNING_FLAME_LIFE;
	if ( pWeapon )
	{
		flFlameLife = pWeapon->GetAfterburnRateOnHit();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, flFlameLife, mult_wpn_burntime );
	}

	if ( bVictimIsPyro )
		flFlameLife = TF_BURNING_FLAME_LIFE_PYRO;

	if ( flFlameDuration != -1.0f )
		flFlameLife = flFlameDuration;

	if ( pWeapon && !pWeapon->IsWeapon( TF_WEAPON_ROCKETLAUNCHER_FIREBALL ) )
	{
		m_flFlameRemoveTime = gpGlobals->curtime + flFlameLife;
	}
	else
	{
		// dragon's fury afterburn is 2 second
		m_flFlameRemoveTime = bVictimIsPyro ? gpGlobals->curtime + flFlameLife : gpGlobals->curtime + tf_fireball_burn_duration.GetFloat();
	}

	m_hBurnAttacker = pAttacker;
	m_hBurnWeapon = pWeapon;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: BLOOD LEAKING
//-----------------------------------------------------------------------------
void CTFPlayerShared::MakeBleed( CTFPlayer *pAttacker, CTFWeaponBase *pWeapon, float flBleedDuration, int iDamage )
{
#ifdef GAME_DLL
	if (m_pOuter->IsAlive() && ( pAttacker || pWeapon ))
	{
		float flEndAt = gpGlobals->curtime + flBleedDuration;
		for (int i=0; i<m_aBleeds.Count(); ++i)
		{
			bleed_struct_t *bleed = &m_aBleeds[i];
			if (bleed->m_hAttacker == pAttacker && bleed->m_hWeapon == pWeapon)
			{
				bleed->m_flEndTime = flEndAt;

				if (!InCond( TF_COND_BLEEDING ))
					AddCond( TF_COND_BLEEDING );

				return;
			}
		}

		bleed_struct_t bleed = {
			pAttacker,
			pWeapon,
			flBleedDuration,
			flEndAt,
			iDamage
		};
		m_aBleeds.AddToTail( bleed );

		if (!InCond( TF_COND_BLEEDING ))
			AddCond( TF_COND_BLEEDING );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::StunPlayer( float flDuration, float flSpeed, float flResistance, int nStunFlags, CTFPlayer *pStunner )
{
	float flNextStunExpireTime = max( m_flStunExpireTime, gpGlobals->curtime + flDuration );
	m_hStunner = pStunner;
	m_nStunFlags = nStunFlags;
	m_flStunMovementSpeed = flSpeed;
	m_flStunResistance = flResistance;

	if ( m_flStunExpireTime < flNextStunExpireTime )
	{
		AddCond( TF_COND_STUNNED );
		m_flStunExpireTime = flNextStunExpireTime;

#ifdef GAME_DLL
		if( !( m_nStunFlags & TF_STUNFLAG_THIRDPERSON ) )
			m_pOuter->StunSound( m_hStunner, m_nStunFlags /*, current stun flags*/ );
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::Concussion( CTFPlayer *pAttacker, float flDistance )
{
#ifdef GAME_DLL
	m_pOuter->ViewPunch( QAngle( -5, 0, 0 ) * 0.0004f * flDistance );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsControlStunned( void )
{
	if ( InCond( TF_COND_STUNNED ) && ( m_nStunFlags & TF_STUNFLAG_BONKSTUCK ) != 0 )
		return true;
	return false;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Bonk phase effects
//-----------------------------------------------------------------------------
void CTFPlayerShared::AddPhaseEffects( void )
{
	CTFPlayer *pPlayer = m_pOuter;
	if ( !pPlayer )
		return;

	// TF2Vintage's bonk effect
	// TODO: Clean this up a bit more
	const char* pszEffect = m_pOuter->GetTeamNumber() == TF_TEAM_BLUE ? "effects/beam001_blu.vmt" : "effects/beam001_red.vmt";
	Vector vecOrigin = pPlayer->GetAbsOrigin();
	
	CSpriteTrail *pPhaseTrail = CSpriteTrail::SpriteTrailCreate( pszEffect, vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 12.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 1.0 );
	pPhaseTrail->SetAttachment( pPlayer, pPlayer->LookupAttachment( "back_upper" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( pszEffect, vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 16.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 1.0 );
	pPhaseTrail->SetAttachment( pPlayer, pPlayer->LookupAttachment( "back_lower" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	// White trail for socks
	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( "effects/beam001_white.vmt", vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 8.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 0.5 );
	pPhaseTrail->SetAttachment( pPlayer, pPlayer->LookupAttachment( "foot_R" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );

	pPhaseTrail = CSpriteTrail::SpriteTrailCreate( "effects/beam001_white.vmt", vecOrigin, true );
	pPhaseTrail->SetTransparency( kRenderTransAlpha, 255, 255, 255, 255, 0 );
	pPhaseTrail->SetStartWidth( 8.0f );
	pPhaseTrail->SetTextureResolution( 0.01416667 );
	pPhaseTrail->SetLifeTime( 0.5 );
	pPhaseTrail->SetAttachment( pPlayer, pPlayer->LookupAttachment( "foot_L" ) );
	m_pPhaseTrails.AddToTail( pPhaseTrail );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Update phase effects
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdatePhaseEffects(void)
{
	if ( !InCond( TF_COND_PHASE ) && !InCond( TF_COND_SHIELD_CHARGE ) && !InCond( LFE_COND_ZOMBIE_LEAP ) )
		return;

#ifdef CLIENT_DLL
	if ( !m_pWarp )
	{
		m_pWarp = m_pOuter->ParticleProp()->Create( "warp_version", PATTACH_ABSORIGIN_FOLLOW );
	}
#else
	if ( m_pPhaseTrails.IsEmpty() )
	{
		AddPhaseEffects();
	}
		
	// Turn on the trails if they're not active already
	if ( m_pPhaseTrails[0] && !m_pPhaseTrails[0]->IsOn() )
	{
		for( int i = 0; i < m_pPhaseTrails.Count(); i++ )
		{
			m_pPhaseTrails[i]->TurnOn();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Update speedboost effects
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateSpeedBoostEffects( void )
{
#ifdef CLIENT_DLL
	if ( IsSpeedBoosted() )
	{
		if ( m_pSpeedTrails && IsStealthed() )
		{
			m_pOuter->ParticleProp()->StopEmission( m_pSpeedTrails );
			m_pSpeedTrails = NULL;
		}
		if(  m_pOuter->GetAbsVelocity() != vec3_origin )
		{
			// We're on the move
			if ( !m_pSpeedTrails && !IsStealthed() )
			{
				m_pSpeedTrails = m_pOuter->ParticleProp()->Create( "speed_boost_trail", PATTACH_ABSORIGIN_FOLLOW );
			}
		}
		else
		{
			// We're not moving
			if( m_pSpeedTrails )
			{
				m_pOuter->ParticleProp()->StopEmission( m_pSpeedTrails );
				m_pSpeedTrails = NULL;
			}
		}
	}
	else
	{
		m_pOuter->ParticleProp()->StopEmission( m_pSpeedTrails );
		m_pSpeedTrails = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveBurning( void )
{
#ifdef CLIENT_DLL
	m_pOuter->StopBurningSound();

	if ( m_pOuter->m_pBurningEffect )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pOuter->m_pBurningEffect );
		m_pOuter->m_pBurningEffect = NULL;
	}

	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}

	m_pOuter->m_flBurnEffectStartTime = 0;
	m_pOuter->m_flBurnEffectEndTime = 0;
#else
	m_hBurnAttacker = NULL;
	m_hBurnWeapon = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveBleeding( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->IsLocalPlayer() )
	{
		view->SetScreenOverlayMaterial( NULL );
	}

#else

#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddStealthed( void )
{
	// set our offhand weapon to be the invis weapon
	SetOffHandWatch();
	CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
#ifdef CLIENT_DLL
	if ( !pInvis->HasFeignDeath() )
		m_pOuter->EmitSound( "Player.Spy_Cloak" );

	UpdateCritBoostEffect();
	m_pOuter->UpdateOverhealEffect();
	m_pOuter->UpdateRecentlyTeleportedEffect();
	m_pOuter->RemoveAllDecals();
#endif

	if ( pInvis->HasFeignDeath() )
	{
		m_flInvisChangeCompleteTime = gpGlobals->curtime;
	}
	else
	{
		float flCloakRate = tf_spy_invis_time.GetFloat();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flCloakRate, mult_cloak_rate );
		m_flInvisChangeCompleteTime = gpGlobals->curtime + flCloakRate;
	}

	m_pOuter->TeamFortress_SetSpeed();
}

void CTFPlayerShared::SetOffHandWatch( void )
{
	// set our offhand weapon to be the invis weapon
	CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );

	// try to switch to this weapon
	m_pOuter->SetOffHandWeapon( pInvis );
	
	if ( pInvis->HasFeignDeath() )
		m_bFeignDeathReady = true;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveStealthed(void)
{
#ifdef CLIENT_DLL
	const char *pUncloak = "Player.Spy_UnCloak";

	int nQuiet = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_pOuter, nQuiet, set_quiet_unstealth );

	CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
	if ( pInvis->HasFeignDeath() )
		pUncloak = "Player.Spy_UnCloakFeignDeath";
	else if ( nQuiet != 0 )
		pUncloak = "Player.Spy_UnCloakReduced";

	m_pOuter->EmitSound( pUncloak );

	UpdateCritBoostEffect();
	m_pOuter->UpdateOverhealEffect();
	m_pOuter->UpdateRecentlyTeleportedEffect();
#endif

	m_pOuter->HolsterOffHandWeapon();

	m_pOuter->TeamFortress_SetSpeed();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveDisguising( void )
{
#ifdef CLIENT_DLL
	if ( m_pOuter->m_pDisguisingEffect )
	{
		m_pOuter->ParticleProp()->StopEmission( m_pOuter->m_pDisguisingEffect );
		m_pOuter->m_pDisguisingEffect = NULL;
	}
#else
	// PistonMiner: Removed the reset as we need this for later.

	//m_nDesiredDisguiseTeam = TF_SPY_UNDEFINED;

	// Do not reset this value, we use the last desired disguise class for the
	// 'lastdisguise' command

	//m_nDesiredDisguiseClass = TF_CLASS_UNDEFINED;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnRemoveDisguised( void )
{
#ifdef CLIENT_DLL

	// if local player is on the other team, reset the model of this player
	if ( m_pOuter->IsEnemyPlayer() )
	{
		TFPlayerClassData_t *pData = GetPlayerClassData( TF_CLASS_SPY );
		int iIndex = modelinfo->GetModelIndex( pData->GetModelName() );

		m_pOuter->SetModelIndex( iIndex );
	}

	m_pOuter->EmitSound( "Player.Spy_Disguise" );

	// They may have called for medic and created a visible medic bubble
	m_pOuter->ParticleProp()->StopParticlesNamed( "speech_mediccall", true );

#else
	m_nDisguiseTeam = TF_SPY_UNDEFINED;
	m_nDisguiseClass.Set( TF_CLASS_UNDEFINED );
	m_nMaskClass = TF_CLASS_UNDEFINED;
	m_hDisguiseTarget.Set( NULL );
	m_iDisguiseTargetIndex = TF_DISGUISE_TARGET_INDEX_NONE;
	m_iDisguiseHealth = 0;
	m_iDisguiseMaxHealth = 0;
	m_flDisguiseChargeLevel = 0.0f;
	m_DisguiseItem.SetItemDefIndex( -1 );

	// Update the player model and skin.
	m_pOuter->UpdateModel();

	m_pOuter->TeamFortress_SetSpeed();

	m_pOuter->ClearExpression();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddBurning( void )
{
#ifdef CLIENT_DLL
	// Start the burning effect
	if ( !m_pOuter->m_pBurningEffect )
	{
		const char *pszEffectName = ConstructTeamParticle( "burningplayer_%s", m_pOuter->GetTeamNumber() );

		m_pOuter->m_pBurningEffect = m_pOuter->ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );

		m_pOuter->m_flBurnEffectStartTime = gpGlobals->curtime;
		m_pOuter->m_flBurnEffectEndTime = gpGlobals->curtime + TF_BURNING_FLAME_LIFE;
	}
	// set the burning screen overlay
	if ( m_pOuter->IsLocalPlayer() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/imcookin", TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}
#endif

	/*
#ifdef GAME_DLL

	if ( player == robin || player == cook )
	{
	CSingleUserRecipientFilter filter( m_pOuter );
	TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_SPECIAL );
	}

	#endif
	*/

	// play a fire-starting sound
	m_pOuter->EmitSound( "Fire.Engulf" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnAddBleeding( void )
{
#ifdef CLIENT_DLL
	// set the bleeding screen overlay
	if ( m_pOuter->IsLocalPlayer() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/bleed_overlay", TEXTURE_GROUP_CLIENT_EFFECTS, false );
		if ( !IsErrorMaterial( pMaterial ) )
		{
			view->SetScreenOverlayMaterial( pMaterial );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayerShared::GetStealthNoAttackExpireTime( void )
{
	return m_flStealthNoAttackExpire;
}

//-----------------------------------------------------------------------------
// Purpose: Sets whether this player is dominating the specified other player
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetPlayerDominated( CTFPlayer *pPlayer, bool bDominated )
{
	int iPlayerIndex = pPlayer->entindex();
	m_bPlayerDominated.Set( iPlayerIndex, bDominated );
	pPlayer->m_Shared.SetPlayerDominatingMe( m_pOuter, bDominated );
}

//-----------------------------------------------------------------------------
// Purpose: Sets whether this player is being dominated by the other player
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetPlayerDominatingMe( CTFPlayer *pPlayer, bool bDominated )
{
	int iPlayerIndex = pPlayer->entindex();
	m_bPlayerDominatingMe.Set( iPlayerIndex, bDominated );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether this player is dominating the specified other player
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsPlayerDominated( int iPlayerIndex )
{
#ifdef CLIENT_DLL
	// On the client, we only have data for the local player.
	// As a result, it's only valid to ask for dominations related to the local player
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalPlayer )
		return false;

	Assert( m_pOuter->IsLocalPlayer() || pLocalPlayer->entindex() == iPlayerIndex );

	if ( m_pOuter->IsLocalPlayer() )
		return m_bPlayerDominated.Get( iPlayerIndex );

	return pLocalPlayer->m_Shared.IsPlayerDominatingMe( m_pOuter->entindex() );
#else
	// Server has all the data.
	return m_bPlayerDominated.Get( iPlayerIndex );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsPlayerDominatingMe( int iPlayerIndex )
{
	return m_bPlayerDominatingMe.Get( iPlayerIndex );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::InCutScene( bool bCutscene )	
{
	if ( bCutscene )
		AddCond( LFE_COND_CUTSCENE );
	else
		RemoveCond( LFE_COND_CUTSCENE );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsInCutScene( void )	
{
	if ( InCond( LFE_COND_CUTSCENE ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::NoteLastDamageTime( int nDamage )
{
	if ( !IsStealthed() )
		return;

	// we took damage
	if ( nDamage > 5 )
	{
		m_flLastStealthExposeTime = gpGlobals->curtime;
		AddCond( TF_COND_STEALTHED_BLINK );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::OnSpyTouchedByEnemy( void )
{
	m_flLastStealthExposeTime = gpGlobals->curtime;
	AddCond( TF_COND_STEALTHED_BLINK );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::FadeInvis( float flInvisFadeTime )
{
	RemoveCond( TF_COND_STEALTHED );

	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flInvisFadeTime, mult_decloak_rate );

	if ( ( flInvisFadeTime < 0.15 ) || InCond( TF_COND_STEALTHED_USER_BUFF ) )
	{
		// this was a force respawn, they can attack whenever
	}
	else
	{
		// next attack in some time
		m_flStealthNoAttackExpire = gpGlobals->curtime + tf_spy_cloak_no_attack_time.GetFloat();
	}

	m_flInvisChangeCompleteTime = gpGlobals->curtime + flInvisFadeTime;
}

//-----------------------------------------------------------------------------
// Purpose: Approach our desired level of invisibility
//-----------------------------------------------------------------------------
void CTFPlayerShared::InvisibilityThink( void )
{
	float flTargetInvis = 0.0f;
	float flTargetInvisScale = 1.0f;
	if ( InCond( TF_COND_STEALTHED_BLINK ) || IsJared() )
	{
		// We were bumped into or hit for some damage.
		flTargetInvisScale = TF_SPY_STEALTH_BLINKSCALE;/*tf_spy_stealth_blink_scale.GetFloat();*/
	}

	// Go invisible or appear.
	if ( m_flInvisChangeCompleteTime > gpGlobals->curtime )
	{
		if ( IsStealthed() )
		{
			flTargetInvis = 1.0f - ( ( m_flInvisChangeCompleteTime - gpGlobals->curtime ) );
		}
		else
		{
			flTargetInvis = ( ( m_flInvisChangeCompleteTime - gpGlobals->curtime ) * 0.5f );
		}
	}
	else
	{
		if ( IsStealthed() )
		{
			flTargetInvis = 1.0f;

			CTFWeaponInvis *pInvis = dynamic_cast<CTFWeaponInvis *>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
			if ( ( pInvis && pInvis->HasMotionCloak() ) && m_flCloakMeter == 0.0f )
			{
				float flSpeed = m_pOuter->GetAbsVelocity().LengthSqr();
				float flMaxSpeed = Square( m_pOuter->MaxSpeed() );
				if ( flMaxSpeed == 0.0f )
				{
					if (flSpeed >= 0.0f)
						flTargetInvis *= 0.5f;
				}
				else
				{
					flTargetInvis *= 1.0f + ( ( flSpeed * -0.5f ) / flMaxSpeed );
				}
			}
		}
		else
		{
			flTargetInvis = 0.0f;
		}
	}

	flTargetInvis *= flTargetInvisScale;
	m_flInvisibility = clamp( flTargetInvis, 0.0f, 1.0f );
}


//-----------------------------------------------------------------------------
// Purpose: How invisible is the player [0..1]
//-----------------------------------------------------------------------------
float CTFPlayerShared::GetPercentInvisible( void )
{
	return m_flInvisibility;
}

//-----------------------------------------------------------------------------
// Purpose: Start the process of disguising
//-----------------------------------------------------------------------------
void CTFPlayerShared::Disguise( int nTeam, int nClass, CBaseEntity *pTarget, bool b1 )
{
#ifndef CLIENT_DLL
	int nRealTeam = m_pOuter->GetTeamNumber();
	int nRealClass = m_pOuter->GetPlayerClass()->GetClassIndex();

	Assert( ( nClass >= TF_CLASS_SCOUT ) && ( nClass <= TF_CLASS_COUNT ) );

	// we don't have cigarette pda
	if ( !m_pOuter->Weapon_OwnsThisID( TF_WEAPON_PDA_SPY ) )
		return;

	// Can't disguise while taunting.
	if ( InCond( TF_COND_TAUNTING ) )
		return;

	if ( ( nClass == TF_CLASS_CIVILIAN ) && ( !TFGameRules()->IsTFCAllowed() || !lfe_allow_special_classes.GetBool() ) )
	{
		RemoveDisguise();
		return;
	}

	// we're not disguising as anything but ourselves (so reset everything)
	if ( nRealTeam == nTeam && nRealClass == nClass )
	{
		RemoveDisguise();
		return;
	}

	// Ignore disguise of the same type, switch disguise weapon instead.
	if ( nTeam == m_nDisguiseTeam && nClass == m_nDisguiseClass && !b1 )
	{
		CTFWeaponBase *pWeapon = m_pOuter->GetActiveTFWeapon();
		RecalcDisguiseWeapon( pWeapon ? pWeapon->GetSlot() : 0 );
		return;
	}

	// invalid team
	if ( nTeam <= TEAM_SPECTATOR || nTeam >= TF_TEAM_COUNT )
		return;

	// invalid class
	if ( nClass <= TF_CLASS_UNDEFINED || nClass >= TF_CLASS_COUNT )
		return;

	// Can't disguise while holding something with gravity gun.
	CTFWeaponBase *pWeapon = m_pOuter->GetActiveTFWeapon();
	if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_PHYSCANNON ) )
	{
		CWeaponPhysCannon *pGravGun = static_cast<CWeaponPhysCannon *>( pWeapon );

		if ( pGravGun->m_hAttachedObject.Get() )
			return;
	}

	m_hForcedDisguise = pTarget;

	m_nDesiredDisguiseClass = nClass;
	m_nDesiredDisguiseTeam = nTeam;

	AddCond( TF_COND_DISGUISING );

	// Start the think to complete our disguise
	float flDisguiseTime = gpGlobals->curtime + TF_TIME_TO_DISGUISE;

	// Switching disguises is faster if we're already disguised
	if ( InCond( TF_COND_DISGUISED ) )
		flDisguiseTime = gpGlobals->curtime + TF_TIME_TO_CHANGE_DISGUISE;

	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_pOuter, flDisguiseTime, disguise_speed_penalty );

	m_flDisguiseCompleteTime = pTarget ? 0.0f : flDisguiseTime;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Set our target with a player we've found to emulate
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CTFPlayerShared::FindDisguiseTarget( void )
{
	m_hDisguiseTarget = m_pOuter->TeamFortress_GetDisguiseTarget( m_nDisguiseTeam, m_nDisguiseClass );

	if ( m_hForcedDisguise )
	{
		if ( m_hForcedDisguise->IsPlayer() )
			m_hDisguiseTarget = m_hForcedDisguise.Get();

		m_hForcedDisguise = nullptr;
	}

	if ( m_hDisguiseTarget )
	{
		m_iDisguiseTargetIndex.Set( m_hDisguiseTarget->entindex() );
		Assert( m_iDisguiseTargetIndex >= 1 && m_iDisguiseTargetIndex <= MAX_PLAYERS );
	}
	else
	{
		m_iDisguiseTargetIndex.Set( TF_DISGUISE_TARGET_INDEX_NONE );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Complete our disguise
//-----------------------------------------------------------------------------
void CTFPlayerShared::CompleteDisguise( bool bDisguiseOnKill )
{
#ifndef CLIENT_DLL
	AddCond( TF_COND_DISGUISED );

	m_nDisguiseClass = m_nDesiredDisguiseClass;
	m_nDisguiseTeam = m_nDesiredDisguiseTeam;

	RemoveCond( TF_COND_DISGUISING );

	FindDisguiseTarget();

	CTFPlayer *pDisguiseTarget = ToTFPlayer( GetDisguiseTarget() );

	// If we have a disguise target with matching class then take their values.
	// Otherwise, generate random health and uber.
	if ( pDisguiseTarget && pDisguiseTarget->IsPlayerClass( m_nDisguiseClass ) )
	{
		m_iDisguiseHealth = pDisguiseTarget->GetHealth();
		m_iDisguiseMaxHealth = pDisguiseTarget->GetMaxHealth();
		if ( m_nDisguiseClass == TF_CLASS_MEDIC )
		{
			m_flDisguiseChargeLevel = pDisguiseTarget->MedicGetChargeLevel();
		}
	}
	else
	{
		int iMaxHealth = GetPlayerClassData( m_nDisguiseClass )->m_nMaxHealth;
		m_iDisguiseHealth = random->RandomInt( iMaxHealth / 2, iMaxHealth );
		m_iDisguiseMaxHealth = iMaxHealth;
		if ( m_nDisguiseClass == TF_CLASS_MEDIC )
		{
			m_flDisguiseChargeLevel = random->RandomFloat( 0.0f, 0.99f );
		}
	}

	if ( m_nDisguiseClass == TF_CLASS_SPY )
	{
		m_nMaskClass = random->RandomInt( TF_FIRST_NORMAL_CLASS, TF_LAST_NORMAL_CLASS );
	}

	// Update the player model and skin.
	m_pOuter->UpdateModel();

	m_pOuter->TeamFortress_SetSpeed();

	m_pOuter->ClearExpression();

	m_DisguiseItem.SetItemDefIndex( -1 );

	RecalcDisguiseWeapon();


	int nConsumeCloak = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER(m_pOuter, nConsumeCloak, mod_disguise_consumes_cloak);
	if (nConsumeCloak != 0 && !bDisguiseOnKill)
	{
		SetSpyCloakMeter(0.0f);
	}

#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetDisguiseHealth( int iDisguiseHealth )
{
	m_iDisguiseHealth = iDisguiseHealth;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayerShared::AddDisguiseHealth( int iHealthToAdd, bool bOverheal /*= false*/ )
{
	Assert( InCond( TF_COND_DISGUISED ) );

	int iMaxHealth = bOverheal ? GetDisguiseMaxBuffedHealth() : GetDisguiseMaxHealth();
	iHealthToAdd = clamp( iHealthToAdd, 0, iMaxHealth - m_iDisguiseHealth );
	if ( iHealthToAdd <= 0 )
		return 0;

	m_iDisguiseHealth += iHealthToAdd;

	return iHealthToAdd;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::RemoveDisguise( void )
{
#ifdef CLIENT_DLL


#else
	RemoveCond( TF_COND_DISGUISED );
	RemoveCond( TF_COND_DISGUISING );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::RecalcDisguiseWeapon( int iSlot /*= 0*/ )
{
#ifndef CLIENT_DLL
	if ( !InCond( TF_COND_DISGUISED ) )
	{
		m_DisguiseItem.SetItemDefIndex( -1 );
		return;
	}

	Assert( m_pOuter->GetPlayerClass()->GetClassIndex() == TF_CLASS_SPY );

	CEconItemView *pDisguiseItem = NULL;
	CTFPlayer *pDisguiseTarget = ToTFPlayer( GetDisguiseTarget() );

	// Find the weapon in the same slot
	for ( int i = 0; i < LOADOUT_POSITION_BUFFER; i++ )
	{
		// Use disguise target's weapons if possible.
		CEconItemView *pItem = NULL;

		if ( pDisguiseTarget && pDisguiseTarget->IsPlayerClass( m_nDisguiseClass ) )
			pItem = pDisguiseTarget->GetLoadoutItem( m_nDisguiseClass, i );
		else
			pItem = GetTFInventory()->GetItem( m_nDisguiseClass, i, 0 );

		if ( !pItem )
			continue;

		CTFWeaponInfo *pWeaponInfo = GetTFWeaponInfoForItem( pItem, m_nDisguiseClass );
		if ( pWeaponInfo && pWeaponInfo->iSlot == iSlot )
		{
			pDisguiseItem = pItem;
			break;
		}
	}

	if ( iSlot == 0 )
	{
		AssertMsg( pDisguiseItem, "Cannot find primary disguise weapon for desired disguise class %d\n", m_nDisguiseClass );
	}

	// Don't switch to builder as it's too complicated.
	if ( pDisguiseItem )
	{
		m_DisguiseItem = *pDisguiseItem;
	}
#else
	if ( !InCond( TF_COND_DISGUISED ) )
	{
		m_iDisguiseWeaponModelIndex = -1;
		m_pDisguiseWeaponInfo = NULL;
		return;
	}

	m_pDisguiseWeaponInfo = GetTFWeaponInfoForItem( &m_DisguiseItem, m_nDisguiseClass );

	if ( m_pDisguiseWeaponInfo )
	{
		m_iDisguiseWeaponModelIndex = modelinfo->GetModelIndex( m_DisguiseItem.GetWorldDisplayModel() );
	}
	else
	{
		m_iDisguiseWeaponModelIndex = -1;
	}
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Crit effects handling.
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateCritBoostEffect( bool bForceHide /*= false*/ )
{
	bool bShouldShow = !bForceHide;

	if ( bShouldShow )
	{
		if ( m_pOuter->IsDormant() )
		{
			bShouldShow = false;
		}
		else if ( !IsCritBoosted() && !InCond( TF_COND_CRITBOOSTED_DEMO_CHARGE ) )
		{
			bShouldShow = false;
		}
		else if ( IsStealthed() )
		{
			bShouldShow = false;
		}
		else if ( InCond( TF_COND_DISGUISED ) &&
			m_pOuter->IsEnemyPlayer() &&
			m_pOuter->GetTeamNumber() != GetDisguiseTeam() )
		{
			// Don't show crit effect for disguised enemy spies unless they're disguised
			// as their own team.
			bShouldShow = false;
		}
	}

	if ( bShouldShow )
	{
		// Update crit effect model.
		if ( m_pCritEffect )
		{
			if ( m_hCritEffectHost.Get() )
				m_hCritEffectHost->ParticleProp()->StopEmission( m_pCritEffect );

			m_pCritEffect = NULL;
		}

		if ( !m_pOuter->ShouldDrawThisPlayer() )
		{
			m_hCritEffectHost = m_pOuter->GetViewModel();
		}
		else
		{
			C_BaseCombatWeapon *pWeapon = m_pOuter->GetActiveWeapon();

			// Don't add crit effect to weapons without a model.
			if ( pWeapon && pWeapon->GetWorldModelIndex() != 0 )
			{
				m_hCritEffectHost = pWeapon;
			}
			else
			{
				m_hCritEffectHost = m_pOuter;
			}
		}

		if ( m_hCritEffectHost.Get() )
		{
			const char *pszEffect = ConstructTeamParticle( "critgun_weaponmodel_%s", m_pOuter->GetTeamNumber(), true, g_aTeamNamesShort );
			m_pCritEffect = m_hCritEffectHost->ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );

			m_hCritEffectHost->AddEffects( EF_ITEM_BLINK );

			/*if ( m_hCritEffectHost->IsBaseCombatWeapon() )
			{
				C_TFWeaponBase *pTFWeapon = dynamic_cast<C_TFWeaponBase *>( m_hCritEffectHost.Get() );
				if ( pTFWeapon && pTFWeapon->GetViewmodelAddon() )
				{
					pTFWeapon->GetViewmodelAddon()->AddEffects( EF_ITEM_BLINK );
				}
			}*/
		}

		if ( !m_pCritSound )
		{
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CLocalPlayerFilter filter;
			m_pCritSound = controller.SoundCreate( filter, m_pOuter->entindex(), "Weapon_General.CritPower" );
			controller.Play( m_pCritSound, 1.0, 100 );
		}
	}
	else
	{
		if ( m_pCritEffect )
		{
			if ( m_hCritEffectHost.Get() )
			{
				m_hCritEffectHost->ParticleProp()->StopEmission( m_pCritEffect );

				m_hCritEffectHost->RemoveEffects( EF_ITEM_BLINK );

				/*if ( m_hCritEffectHost->IsBaseCombatWeapon() )
				{
					C_TFWeaponBase *pTFWeapon = dynamic_cast<C_TFWeaponBase *>( m_hCritEffectHost.Get() );
					if ( pTFWeapon && pTFWeapon->GetViewmodelAddon() )
					{
						pTFWeapon->GetViewmodelAddon()->RemoveEffects( EF_ITEM_BLINK );
					}
				}*/
			}

			m_pCritEffect = NULL;
		}

		m_hCritEffectHost = NULL;

		if ( m_pCritSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pCritSound );
			m_pCritSound = NULL;
		}
	}
}

CTFWeaponInfo *CTFPlayerShared::GetDisguiseWeaponInfo( void )
{
	if ( InCond( TF_COND_DISGUISED ) && m_pDisguiseWeaponInfo == NULL )
	{
		RecalcDisguiseWeapon();
	}

	return m_pDisguiseWeaponInfo;
}
#endif

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Heal players.
// pPlayer is person who healed us
//-----------------------------------------------------------------------------
void CTFPlayerShared::Heal( CTFPlayer *pPlayer, float flAmount, bool bDispenserHeal /* = false */ )
{
	Assert( FindHealerIndex( pPlayer ) == m_aHealers.InvalidIndex() );

	healers_t newHealer;
	newHealer.pPlayer = pPlayer;
	newHealer.flAmount = flAmount;
	newHealer.bDispenserHeal = bDispenserHeal;
	newHealer.iRecentAmount = 0;
	newHealer.flNextNofityTime = gpGlobals->curtime + 1.0f;
	m_aHealers.AddToTail( newHealer );

	AddCond( TF_COND_HEALTH_BUFF, PERMANENT_CONDITION );

	RecalculateChargeEffects();

	m_nNumHealers = m_aHealers.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Heal players.
// pPlayer is person who healed us
//-----------------------------------------------------------------------------
void CTFPlayerShared::StopHealing( CTFPlayer *pPlayer )
{
	int iIndex = FindHealerIndex( pPlayer );
	if ( iIndex == m_aHealers.InvalidIndex() )
		return;

	m_aHealers.Remove( iIndex );

	if ( !m_aHealers.Count() )
	{
		RemoveCond( TF_COND_HEALTH_BUFF );
	}

	RecalculateChargeEffects();

	m_nNumHealers = m_aHealers.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
medigun_charge_types CTFPlayerShared::GetChargeEffectBeingProvided( CTFPlayer *pPlayer )
{
	if ( !pPlayer->IsPlayerClass( TF_CLASS_MEDIC ) && !pPlayer->Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) )
		return TF_CHARGE_NONE;

	CTFWeaponBase *pWpn = pPlayer->GetActiveTFWeapon();
	if ( !pWpn )
		return TF_CHARGE_NONE;

	CWeaponMedigun *pMedigun = dynamic_cast <CWeaponMedigun*>( pWpn );
	if ( pMedigun && pMedigun->IsReleasingCharge() )
		return pMedigun->GetChargeType();

	return TF_CHARGE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::RecalculateChargeEffects( bool bInstantRemove )
{
	bool bShouldCharge[TF_CHARGE_COUNT] = { };
	CTFPlayer *pProviders[TF_CHARGE_COUNT] = { };

	if ( m_pOuter->m_flPowerPlayTime > gpGlobals->curtime )
	{
		bShouldCharge[TF_CHARGE_INVULNERABLE] = true;
	}

	medigun_charge_types selfCharge = GetChargeEffectBeingProvided( m_pOuter );

	// Charging self?
	if ( selfCharge != TF_CHARGE_NONE )
	{
		bShouldCharge[selfCharge] = true;
		pProviders[selfCharge] = m_pOuter;
	}
	else
	{
		// Check players healing us.
		for ( int i = 0; i < m_aHealers.Count(); i++ )
		{
			CTFPlayer *pPlayer = ToTFPlayer( m_aHealers[i].pPlayer );
			if ( !pPlayer )
				continue;

			medigun_charge_types chargeType = GetChargeEffectBeingProvided( pPlayer );

			if ( chargeType != TF_CHARGE_NONE )
			{
				bShouldCharge[chargeType] = true;
				pProviders[chargeType] = pPlayer;
			}
		}
	}

	// Deny stock uber while carrying flag.
	if ( m_pOuter->HasTheFlag() )
	{
		bShouldCharge[TF_CHARGE_INVULNERABLE] = false;
	}

	for ( int i = 0; i < TF_CHARGE_COUNT; i++ )
	{
		float flRemoveTime = ( i == TF_CHARGE_INVULNERABLE ) ? tf_invuln_time.GetFloat() : 0.0f;
		SetChargeEffect( (medigun_charge_types)i, bShouldCharge[i], bInstantRemove, g_MedigunEffects[i], flRemoveTime, pProviders[i] );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetChargeEffect( medigun_charge_types chargeType, bool bShouldCharge, bool bInstantRemove, const MedigunEffects_t &chargeEffect, float flRemoveTime, CTFPlayer *pProvider )
{
	CTFShotgun_Revenge *pShotgun = dynamic_cast<CTFShotgun_Revenge *>( GetActiveTFWeapon() );
	if ( pShotgun && pShotgun->CanGetRevengeCrits() )
	{
		if ( chargeEffect.condition_enable == TF_COND_CRITBOOSTED )
			return;
	}

	if ( InCond( chargeEffect.condition_enable ) == bShouldCharge )
	{
		if ( bShouldCharge && m_flChargeOffTime[chargeType] != 0.0f )
		{
			m_flChargeOffTime[chargeType] = 0.0f;

			if ( chargeEffect.condition_disable != TF_COND_LAST )
				RemoveCond( chargeEffect.condition_disable );
		}
		return;
	}

	if ( bShouldCharge )
	{
		Assert( chargeType != TF_CHARGE_INVULNERABLE || !m_pOuter->HasTheFlag() );

		if ( m_flChargeOffTime[chargeType] != 0.0f )
		{
			m_pOuter->StopSound( chargeEffect.sound_disable );

			m_flChargeOffTime[chargeType] = 0.0f;

			if ( chargeEffect.condition_disable != TF_COND_LAST )
			{
				RemoveCond( chargeEffect.condition_disable );
			}
		}

		// Charge on.
		AddCond( chargeEffect.condition_enable );

		CSingleUserRecipientFilter filter( m_pOuter );
		m_pOuter->EmitSound( filter, m_pOuter->entindex(), chargeEffect.sound_enable );
		m_bChargeSounds[chargeType] = true;
	}
	else
	{
		if ( m_bChargeSounds[chargeType] )
		{
			m_pOuter->StopSound( chargeEffect.sound_enable );
			m_bChargeSounds[chargeType] = false;
		}

		if ( m_flChargeOffTime[chargeType] == 0.0f )
		{
			CSingleUserRecipientFilter filter( m_pOuter );
			m_pOuter->EmitSound( filter, m_pOuter->entindex(), chargeEffect.sound_disable );
		}

		if ( bInstantRemove )
		{
			m_flChargeOffTime[chargeType] = 0.0f;
			RemoveCond( chargeEffect.condition_enable );

			if ( chargeEffect.condition_disable != TF_COND_LAST )
			{
				RemoveCond( chargeEffect.condition_disable );
			}
		}
		else
		{
			// Already turning it off?
			if ( m_flChargeOffTime[chargeType] != 0.0f )
				return;

			if ( chargeEffect.condition_disable != TF_COND_LAST )
				AddCond( chargeEffect.condition_disable );

			m_flChargeOffTime[chargeType] = gpGlobals->curtime + flRemoveTime;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::TestAndExpireChargeEffect( medigun_charge_types chargeType )
{
	if ( InCond( g_MedigunEffects[chargeType].condition_enable ) )
	{
		bool bRemoveCharge = false;

		if ( ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN ) && ( TFGameRules()->GetWinningTeam() != m_pOuter->GetTeamNumber() ) )
		{
			bRemoveCharge = true;
		}

		if ( m_flChargeOffTime[chargeType] != 0.0f )
		{
			if ( gpGlobals->curtime > m_flChargeOffTime[chargeType] )
			{
				bRemoveCharge = true;
			}
		}

		if ( bRemoveCharge == true )
		{
			m_flChargeOffTime[chargeType] = 0.0f;

			if ( g_MedigunEffects[chargeType].condition_disable != TF_COND_LAST )
			{
				RemoveCond( g_MedigunEffects[chargeType].condition_disable );
			}

			RemoveCond( g_MedigunEffects[chargeType].condition_enable );
		}
	}
	else if ( m_bChargeSounds[chargeType] )
	{
		// If we're still playing charge sound but not actually charged, stop the sound.
		// This can happen if player respawns while crit boosted.
		m_pOuter->StopSound( g_MedigunEffects[chargeType].sound_enable );
		m_bChargeSounds[chargeType] = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
EHANDLE	CTFPlayerShared::GetHealerByIndex( int index )
{
	if (m_aHealers.IsValidIndex( index ))
		return m_aHealers[index].pPlayer;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFPlayerShared::FindHealerIndex( CTFPlayer *pPlayer )
{
	for ( int i = 0; i < m_aHealers.Count(); i++ )
	{
		if ( m_aHealers[i].pPlayer == pPlayer )
			return i;
	}

	return m_aHealers.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Returns the first healer in the healer array.  Note that this
//		is an arbitrary healer.
//-----------------------------------------------------------------------------
EHANDLE CTFPlayerShared::GetFirstHealer()
{
	if ( m_aHealers.Count() > 0 )
		return m_aHealers.Head().pPlayer;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::HealthKitPickupEffects( int iAmount )
{
	if ( InCond( TF_COND_BURNING ) )
		RemoveCond( TF_COND_BURNING );
	if ( InCond( TF_COND_BLEEDING ) )
		RemoveCond( TF_COND_BLEEDING );

	if ( IsStealthed() || !m_pOuter )
		return;

	IGameEvent *event = gameeventmanager->CreateEvent( "player_healonhit" );
	if ( event )
	{
		event->SetInt( "amount", iAmount );
		event->SetInt( "entindex", m_pOuter->entindex() );
		gameeventmanager->FireEvent( event );
	}

}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWeaponBase *CTFPlayerShared::GetActiveTFWeapon() const
{
	return m_pOuter->GetActiveTFWeapon();
}

//-----------------------------------------------------------------------------
// Purpose: Team check.
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsAlly( CBaseEntity *pEntity )
{
	return ( pEntity->GetTeamNumber() == m_pOuter->GetTeamNumber() );
}

//-----------------------------------------------------------------------------
// Purpose: Used to determine if player should do loser animations.
//-----------------------------------------------------------------------------
bool CTFPlayerShared::IsLoser( void )
{
	if ( !m_pOuter->IsAlive() )
		return false;

	if ( tf_always_loser.GetBool() )
		return true;

	if ( TFGameRules() && TFGameRules()->State_Get() == GR_STATE_TEAM_WIN )
	{
		int iWinner = TFGameRules()->GetWinningTeam();
		if ( iWinner != m_pOuter->GetTeamNumber() )
		{
			if ( m_pOuter->IsPlayerClass( TF_CLASS_SPY ) )
			{
				if ( InCond( TF_COND_DISGUISED ) )
				{
					return ( iWinner != GetDisguiseTeam() );
				}
			}
			return true;
		}
	}

	// Check if we should be in the loser state while stunned
	if ( InCond( TF_COND_STUNNED ) && ( m_nStunFlags & TF_STUNFLAG_THIRDPERSON ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayerShared::GetDesiredPlayerClassIndex( void )
{
	return m_iDesiredPlayerClass;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
// Output : Successful
//-----------------------------------------------------------------------------
bool CTFPlayerShared::AddToSpyCloakMeter( float amt, bool bForce, bool bIgnoreAttribs )
{
	CTFWeaponInvis *pInvis = dynamic_cast<CTFWeaponInvis *>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
	if ( !pInvis )
		return false;

	if ( !bForce && pInvis->HasMotionCloak() )
		return false;

	if ( pInvis->HasFeignDeath() )
		amt = Min( amt, 35.0f );

	if ( bIgnoreAttribs )
	{
		if (amt <= 0.0f || m_flCloakMeter >= 100.0f)
			return false;

		m_flCloakMeter = Clamp( m_flCloakMeter + amt, 0.0f, 100.0f );
		return true;
	}

	int iNoRegenFromItems = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pInvis, iNoRegenFromItems, mod_cloak_no_regen_from_items );
	if ( iNoRegenFromItems )
		return false;

	int iNoCloakWhenCloaked = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pInvis, iNoCloakWhenCloaked, NoCloakWhenCloaked );
	if ( iNoCloakWhenCloaked )
	{
		if (InCond( TF_COND_STEALTHED ))
			return false;
	}

	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pInvis, amt, ReducedCloakFromAmmo );
	if ( amt <= 0.0f || m_flCloakMeter >= 100.0f )
		return false;

	m_flCloakMeter = Clamp( m_flCloakMeter + amt, 0.0f, 100.0f );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetJumping( bool bJumping )
{
	m_bJumping = bJumping;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetAirDash( int iAirDash )
{
	m_iAirDash = iAirDash;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::IncrementAirDucks( void )
{
	m_nAirDucked++;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::ResetAirDucks( void )
{
	m_nAirDucked = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayerShared::GetSequenceForDeath( CBaseAnimating *pAnim, int iDamageCustom )
{
	const char *pszSequence = NULL;

	switch( iDamageCustom )
	{
	case TF_DMG_CUSTOM_BACKSTAB:
		pszSequence = "primary_death_backstab";
		break;
	case TF_DMG_CUSTOM_HEADSHOT:
	case TF_DMG_CUSTOM_DECAPITATION:
	case TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING:
	case TF_DMG_CUSTOM_DECAPITATION_BOSS:
		pszSequence = "primary_death_headshot";
		break;
	case TF_DMG_CUSTOM_BURNING:
	case TF_DMG_CUSTOM_BURNING_FLARE:
	case TF_DMG_CUSTOM_BURNING_ARROW:
	case TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE:
	case TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING:
		pszSequence = "primary_death_burning"; // we need shorter animation
		break;
	}

	if ( pszSequence != NULL )
	{
		return pAnim->LookupSequence( pszSequence );
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayerShared::GetCritMult( void )
{
	float flRemapCritMul = RemapValClamped( m_iCritMult, 0, 255, 1.0, TF_DAMAGE_CRITMOD_MAXMULT );
	/*#ifdef CLIENT_DLL
		Msg("CLIENT: Crit mult %.2f - %d\n",flRemapCritMul, m_iCritMult);
		#else
		Msg("SERVER: Crit mult %.2f - %d\n", flRemapCritMul, m_iCritMult );
		#endif*/

	return flRemapCritMul;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateFlashlightBattery( void )
{
#ifdef GAME_DLL
	if ( lfe_force_legacy.GetBool() )
	{
		m_flFlashBattery = 100.0f;
		return;
	}
#endif

	if ( InCond( LFE_COND_FLASHLIGHT ) )
	{
		if ( m_flFlashBattery > 0.0f )
		{
			m_flFlashBattery -= FLASH_DRAIN_TIME * gpGlobals->frametime;
			if ( m_flFlashBattery < 0.0f )
			{
				#ifdef GAME_DLL
				m_pOuter->FlashlightTurnOff();
				#endif
				m_flFlashBattery = 0.0f;
			}
		}
	}
	else
	{
		m_flFlashBattery += FLASH_CHARGE_TIME * gpGlobals->frametime;
		if ( m_flFlashBattery > 100.0f )
		{
			m_flFlashBattery = 100.0f;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update rage buffs
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateRageBuffsAndRage( void )
{
	if ( IsRageActive() )
	{
		if ( m_flEffectBarProgress > 0.0f )
		{
			if ( gpGlobals->curtime > m_flNextRageCheckTime && m_flRageTimeRemaining > 0.0f )
			{
				m_flNextRageCheckTime = gpGlobals->curtime + 1.0f;
				m_flRageTimeRemaining--;
				PulseRageBuff();
			}
 			m_flEffectBarProgress -= ( 100.0f / tf_soldier_buff_pulses.GetFloat() ) * gpGlobals->frametime;
		}
		else
		{
			ResetRageSystem();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set rage meter progress
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetRageMeter( float flRagePercent, int iBuffType )
{
	if ( !IsRageActive() )
	{
		if ( m_pOuter )
		{
			if ( iBuffType == TF_BUFF_CRITBOOSTED )
			{
				m_flEffectBarProgress = min( m_flEffectBarProgress + flRagePercent, 100.0f );
				return;
			}

			CTFBuffItem *pBuffItem = ( CTFBuffItem * )m_pOuter->Weapon_GetSlot( LOADOUT_POSITION_SECONDARY );
			if ( pBuffItem && pBuffItem->GetBuffType() == iBuffType )
			{
				// Only build rage if we're using this type of banner
				m_flEffectBarProgress = min( m_flEffectBarProgress + flRagePercent, 100.0f );
			}
		}
	}
}

 //-----------------------------------------------------------------------------
// Purpose: Activate rage
//-----------------------------------------------------------------------------
void CTFPlayerShared::ActivateRageBuff( CBaseEntity *pEntity, int iBuffType )
{
	if ( m_flEffectBarProgress < 100.0f )
		return;

 	m_flNextRageCheckTime = gpGlobals->curtime + 1.0f;

	float flBuffDuration = tf_soldier_buff_pulses.GetFloat();
	float flModBuffDuration = 0.0f;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flModBuffDuration, mod_buff_duration );
	m_flRageTimeRemaining = flBuffDuration+flModBuffDuration;
	m_iActiveBuffType = iBuffType;
 #ifdef GAME_DLL
	//*(this + 112) = pEntity;
 	if ( m_pOuter )
	{
		if ( iBuffType == TF_BUFF_OFFENSE )
		{
			m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_BATTLECRY );
		}
		else if ( iBuffType == TF_BUFF_DEFENSE )
		{
			m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_INCOMING );
		}
	}
 #endif
 	if ( !IsRageActive() )
	{
		m_bRageActive = true;
	}
 	PulseRageBuff();
}

//-----------------------------------------------------------------------------
// Purpose: Give rage buffs to nearby players
//-----------------------------------------------------------------------------
void CTFPlayerShared::PulseRageBuff( /*CTFPlayerShared::ERageBuffSlot*/ )
{
	// g_SoldierBuffAttributeIDToConditionMap is called here in Live TF2
#ifdef GAME_DLL
	CTFPlayer *pOuter = m_pOuter;
 	if ( !m_pOuter )
		return;

	if ( m_iActiveBuffType == TF_BUFF_CRITBOOSTED )
	{
		if ( pOuter->GetActiveTFWeapon() )
		{
			int nSurpriseMotherFucker = 0;
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pOuter->GetActiveTFWeapon(), nSurpriseMotherFucker, set_buff_type );
			if ( nSurpriseMotherFucker == TF_BUFF_CRITBOOSTED )
				pOuter->m_Shared.AddCond( TF_COND_CRITBOOSTED_RAGE_BUFF, 1.2f );
			else
				pOuter->m_Shared.RemoveCond( TF_COND_CRITBOOSTED_RAGE_BUFF );
		}
		return;
	}

 	CBaseEntity *pEntity = NULL;
	Vector vecOrigin = pOuter->GetAbsOrigin();
 	for ( CEntitySphereQuery sphere( vecOrigin, TF_BUFF_RADIUS ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pEntity )
			continue;

 		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( vecOrigin, &vecHitPoint );
		Vector vecDir = vecHitPoint - vecOrigin;
 		CTFPlayer *pPlayer = ToTFPlayer( pEntity );
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
 		if ( vecDir.LengthSqr() < ( TF_BUFF_RADIUS * TF_BUFF_RADIUS ) )
		{
			if ( pPlayer && pPlayer->InSameTeam( pOuter ) )
			{
				switch ( m_iActiveBuffType )
				{
				case TF_BUFF_OFFENSE:
					pPlayer->m_Shared.AddCond( TF_COND_OFFENSEBUFF, 1.2f );
					break;
				case TF_BUFF_DEFENSE:
					pPlayer->m_Shared.AddCond( TF_COND_DEFENSEBUFF, 1.2f );
					break;
				case TF_BUFF_REGENONDAMAGE:
					pPlayer->m_Shared.AddCond( TF_COND_REGENONDAMAGEBUFF, 1.2f );
					break;
				}
 				// Achievements
				IGameEvent *event = gameeventmanager->CreateEvent( "player_buff" );
				if ( event )
				{
					event->SetInt( "userid", pPlayer->GetUserID() );
					event->SetInt( "buff_type", m_iActiveBuffType );
					event->SetInt( "buff_owner", pOuter->entindex() );
 					gameeventmanager->FireEvent( event );
				}
			}
			else if ( pNPC && pNPC->InSameTeam( pOuter ) && pNPC->IsAlive() && !pNPC->NoReward() )
			{
				switch ( m_iActiveBuffType )
				{
				case TF_BUFF_OFFENSE:
					pNPC->AddCond(TF_COND_OFFENSEBUFF, 1.2f);
					break;
				case TF_BUFF_DEFENSE:
					pNPC->AddCond(TF_COND_DEFENSEBUFF, 1.2f);
					break;
				case TF_BUFF_REGENONDAMAGE:
					pNPC->AddCond(TF_COND_REGENONDAMAGEBUFF, 1.2f);
					break;
				}
				IGameEvent *event = gameeventmanager->CreateEvent( "player_buff" );
				if ( event )
				{
					event->SetInt( "userid", pNPC->entindex() );
					event->SetInt( "buff_type", m_iActiveBuffType );
					event->SetInt( "buff_owner", pOuter->entindex() );
 					gameeventmanager->FireEvent( event );
				}
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Reset rage variables
//-----------------------------------------------------------------------------
void CTFPlayerShared::ResetRageSystem( void )
{
	m_flEffectBarProgress = 0.0f;
	m_flNextRageCheckTime = 0.0f;
	m_flRageTimeRemaining = 0.0f;
	m_iActiveBuffType = 0;
	m_bRageActive = false;
#ifdef CLIENT_DLL
	if ( m_pOuter )
	{
		// Remove the banner
		C_EconWearable *pWearable = m_pOuter->GetWearableForLoadoutSlot( LOADOUT_POSITION_SECONDARY );
		if ( pWearable )
		{
			pWearable->DestroyBoneAttachments();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Atomic force-a-nature hype
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetScoutHypeMeter( float flPercent )
{
	CTFPlayer *pOuter = m_pOuter;
 	if ( !m_pOuter )
		return;

	CTFWeaponBase *pGun = dynamic_cast<CTFWeaponBase*>( pOuter->Weapon_GetSlot( LOADOUT_POSITION_PRIMARY ) );
	if ( pGun )
	{
		if ( flPercent >= 0.0f )
			m_flHypeMeter = min( m_flHypeMeter + flPercent, 100.0f );
		else
			m_flHypeMeter = flPercent;

		pOuter->TeamFortress_SetSpeed();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set weapon with IHasGenericMeter back to 0
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetDefaultItemChargeMeters( void )
{
	//m_flEffectBarProgress = GetDefaultItemChargeMeterValue();
}

//-----------------------------------------------------------------------------
// Purpose: Set weapon with IHasGenericMeter progress
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetItemChargeMeter( loadout_positions_t eSlot, float flPercent )
{
	if ( !m_pOuter )
		return;

	CTFWeaponBase *pWPN = ( CTFWeaponBase * )m_pOuter->Weapon_GetSlot( (int)eSlot );
	if ( pWPN )
		m_flEffectBarProgress = min( m_flEffectBarProgress + flPercent, 100.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Ugly slayer
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetRevengeCrits( int nKills )
{
	CTFPlayer *pOuter = m_pOuter;
 	if ( !m_pOuter )
		return;

	CTFShotgun_Revenge *pShotgun = dynamic_cast <CTFShotgun_Revenge *>( pOuter->Weapon_GetSlot( LOADOUT_POSITION_PRIMARY ) );
	if ( pShotgun )
	{
		pShotgun->SentryKilled( nKills );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayerShared::HasDemoShieldEquipped( void ) const
{
	return m_bShieldEquipped;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::PulseKingRuneBuff( void )
{
#ifdef GAME_DLL
	CTFPlayer *pOuter = m_pOuter;
 	if ( !m_pOuter )
		return;

 	CBaseEntity *pEntity = NULL;
	Vector vecOrigin = pOuter->GetAbsOrigin();
 	for ( CEntitySphereQuery sphere( vecOrigin, TF_BUFF_RADIUS ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pEntity )
			continue;

 		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( vecOrigin, &vecHitPoint );
		Vector vecDir = vecHitPoint - vecOrigin;
 		CTFPlayer *pPlayer = ToTFPlayer( pEntity );
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
 		if ( vecDir.LengthSqr() < ( TF_BUFF_RADIUS * TF_BUFF_RADIUS ) )
		{
			if ( pPlayer && pPlayer->InSameTeam( pOuter ) )
			{
				pPlayer->m_Shared.AddCond( TF_COND_RADIUSHEAL, 1.2f );
				pPlayer->TakeHealth( 1, DMG_GENERIC );
			}
			else if ( pNPC && pNPC->InSameTeam( pOuter ) )
			{
				pNPC->AddCond( TF_COND_RADIUSHEAL, 1.2f );
				pNPC->TakeHealth( 1, DMG_GENERIC );
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::PulseMedicRadiusHeal( void )
{
#ifdef GAME_DLL
	CTFPlayer *pOuter = m_pOuter;
 	if ( !m_pOuter )
		return;

	m_pOuter->m_Shared.AddCond( TF_COND_RADIUSHEAL, TF_MEDIC_REGEN_TIME );

 	CBaseEntity *pEntity = NULL;
	Vector vecOrigin = pOuter->GetAbsOrigin();
 	for ( CEntitySphereQuery sphere( vecOrigin, TF_BUFF_RADIUS ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pEntity )
			continue;

 		Vector vecHitPoint;
		pEntity->CollisionProp()->CalcNearestPoint( vecOrigin, &vecHitPoint );
		Vector vecDir = vecHitPoint - vecOrigin;
 		CTFPlayer *pPlayer = ToTFPlayer( pEntity );
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
 		if ( vecDir.LengthSqr() < ( TF_BUFF_RADIUS * TF_BUFF_RADIUS ) )
		{
			int iHealthRegenAOE = 0;
			int iHealthRestored = 0;

			// More time since combat equals faster healing. Healing increases up to 300%.
			int iAoEHealthBase = 26; 

			if ( pPlayer && pPlayer->InSameTeam( pOuter ) )
			{
				float flTimeSinceDamageAOE = gpGlobals->curtime - pPlayer->GetLastDamageTime();
				float flScaleAoE = RemapValClamped( flTimeSinceDamageAOE, 5, 10, iAoEHealthBase, (iAoEHealthBase * 3 ) );
				iHealthRegenAOE = ceil( TF_MEDIC_REGEN_AMOUNT * flScaleAoE );

				// Check how much health we give.
				pPlayer->m_Shared.AddCond( TF_COND_RADIUSHEAL, TF_MEDIC_REGEN_TIME );
				iHealthRestored = pPlayer->TakeHealth( iHealthRegenAOE, DMG_GENERIC );
				if ( iHealthRestored > 0 )
				{
					CTF_GameStats.Event_PlayerHealedOther( pOuter, iHealthRestored );
					IGameEvent *event = gameeventmanager->CreateEvent( "player_healed" );
					if ( event )
					{
						event->SetInt( "patient", pPlayer->GetUserID() );
						event->SetInt( "healer", pOuter->GetUserID() );
						event->SetInt( "amount", iHealthRestored );
					}
				}
			}
			else if ( pNPC && pNPC->InSameTeam( pOuter ) )
			{
				float flTimeSinceDamageAOE = gpGlobals->curtime - pNPC->GetLastDamageTime();
				float flScaleAoE = RemapValClamped( flTimeSinceDamageAOE, 5, 10, iAoEHealthBase, (iAoEHealthBase * 3 ) );
				iHealthRegenAOE = ceil( TF_MEDIC_REGEN_AMOUNT * flScaleAoE );

				pNPC->AddCond( TF_COND_RADIUSHEAL, TF_MEDIC_REGEN_TIME );
				iHealthRestored = pNPC->TakeHealth( iHealthRegenAOE, DMG_GENERIC );
				if ( iHealthRestored > 0 )
				{
					CTF_GameStats.Event_PlayerHealedOther( pOuter, iHealthRestored );
					IGameEvent *event = gameeventmanager->CreateEvent( "npc_healed" );
					if ( event )
					{
						event->SetInt( "patient", pNPC->entindex() );
						event->SetInt( "healer", pOuter->GetUserID() );
						event->SetInt( "amount", iHealthRestored );
					}
				}
			}
		}
	}
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::CalcChargeCrit( bool bForceCrit )
{
	if (m_flChargeMeter <= 33.0f || bForceCrit)
	{
		m_iNextMeleeCrit = kCritType_Crit;
	}
	else if (m_flChargeMeter <= 75.0f)
	{
		m_iNextMeleeCrit = kCritType_MiniCrit;
	}

	m_pOuter->SetContextThink( &CTFPlayer::RemoveMeleeCrit, gpGlobals->curtime + 0.3f, "RemoveMeleeCrit" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateCloakMeter( void )
{
	CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_INVIS ) );

	if ( m_pOuter->IsPlayerClass( TF_CLASS_SPY ) )
	{
		float flConsumeRate = tf_spy_cloak_consume_rate.GetFloat();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flConsumeRate, mult_cloak_meter_consume_rate );

		float flRegenRate = tf_spy_cloak_regen_rate.GetFloat();
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( m_pOuter, flRegenRate, mult_cloak_meter_regen_rate );

		if ( InCond( TF_COND_STEALTHED ) )
		{
			if ( pInvis->HasMotionCloak() )
			{
				float flSpeed = m_pOuter->GetAbsVelocity().LengthSqr();
				if ( flSpeed == 0.0f )
				{
					m_flCloakMeter += gpGlobals->frametime * flRegenRate;

					if ( m_flCloakMeter >= 100.0f )
						m_flCloakMeter = 100.0f;
				}
				else
				{
					float flMaxSpeed = Square( m_pOuter->MaxSpeed() );
					if (flMaxSpeed == 0.0f)
					{
						m_flCloakMeter -= flConsumeRate * gpGlobals->frametime * 1.5f;
					}
					else
					{
						m_flCloakMeter -= ( flConsumeRate * gpGlobals->frametime * 1.5f ) * Min( flSpeed / flMaxSpeed, 1.0f );
					}
				}
			}
			else
			{
				m_flCloakMeter -= gpGlobals->frametime * flConsumeRate;
			}

			if ( m_flCloakMeter <= 0.0f )
			{
				m_flCloakMeter = 0.0f;

				if ( !pInvis->HasMotionCloak() )
				{
					RemoveCond( TF_COND_STEALTHED_BLINK );
					FadeInvis( tf_spy_invis_unstealth_time.GetFloat() );
				}
				else
				{
					AddCond( TF_COND_STEALTHED_BLINK );
				}
			}
		}
		else
		{
			m_flCloakMeter += gpGlobals->frametime * flRegenRate;

			if ( m_flCloakMeter >= 100.0f )
				m_flCloakMeter = 100.0f;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateChargeMeter( void )
{
	if ( InCond( TF_COND_SHIELD_CHARGE ) )
	{
		m_flChargeMeter -= ( 100 / m_flChargeDrainRate ) * gpGlobals->frametime;

		if (m_flChargeMeter <= 0.0f)
		{
			m_flChargeMeter = 0.0f;
			RemoveCond( TF_COND_SHIELD_CHARGE );
		}
	}
	else if (m_flChargeMeter < 100.0f)
	{
		m_flChargeMeter += m_flChargeRegenRate * gpGlobals->frametime;
		m_flChargeMeter = Min( m_flChargeMeter.Get(), 100.0f );
	}
	else if (m_flChargeMeter > 100.0f)
	{
		m_flChargeMeter = 100.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateEnergyDrinkMeter( void )
{
	if ( InCond( TF_COND_SODAPOPPER_HYPE ) )
	{
		m_flHypeMeter -= ( gpGlobals->frametime * m_flEnergyDrinkDrainRate ) * 0.75;

		if ( m_flHypeMeter <= 0.0f )
		{
			RemoveCond( TF_COND_SODAPOPPER_HYPE );
			m_flHypeMeter = 0.0f;
			m_pOuter->EmitSound( "DisciplineDevice.PowerDown" );
		}
	}

	if ( InCond( TF_COND_PHASE ) || InCond( TF_COND_ENERGY_BUFF ) )
	{
		m_flEnergyDrinkMeter -= m_flEnergyDrinkDrainRate * gpGlobals->frametime;

		if ( m_flEnergyDrinkMeter <= 0.0f )
		{
			m_flEnergyDrinkMeter = 0.0f;
			RemoveCond( TF_COND_PHASE );
			RemoveCond( TF_COND_ENERGY_BUFF );

			m_pOuter->SpeakConceptIfAllowed( MP_CONCEPT_TIRED );
		}
		else if ( InCond( TF_COND_PHASE ) )
		{
			UpdatePhaseEffects();
		}
	}
	else
	{
		if ( m_flEnergyDrinkMeter >= 100.0f )
		{
			m_flEnergyDrinkMeter = 100.0f;
			return;
		}

		m_flEnergyDrinkMeter += m_flEnergyDrinkRegenRate * gpGlobals->frametime;

		if ( m_pOuter->Weapon_OwnsThisID( TF_WEAPON_LUNCHBOX ) && m_pOuter->Weapon_OwnsThisID( TF_WEAPON_LUNCHBOX )->ClassMatches( "tf_weapon_lunchbox_drink" ) )
		{
			if ( m_flEnergyDrinkMeter >= 100.0f )
			{
				m_flEnergyDrinkMeter = 100.0f;
				return;
			}

			if ( m_pOuter->GetAmmoCount( TF_AMMO_GRENADES2 ) != m_pOuter->GetMaxAmmo( TF_AMMO_GRENADES2 ) )
				return;
		}

		m_flEnergyDrinkMeter = Min( m_flEnergyDrinkMeter.Get(), 100.0f );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::EndCharge( void )
{
	if (InCond( TF_COND_SHIELD_CHARGE ) && m_flChargeMeter < 90.0f)
	{
#ifdef GAME_DLL
		CalcChargeCrit( false );

		CTFWearableDemoShield *pShield = GetEquippedDemoShield( m_pOuter );
		if (pShield) pShield->ShieldBash( m_pOuter );
#endif

		m_flChargeMeter = 0;
	}
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::UpdateCritMult( void )
{
	const float flMinMult = 1.0;
	const float flMaxMult = TF_DAMAGE_CRITMOD_MAXMULT;

	if ( m_DamageEvents.Count() == 0 )
	{
		m_iCritMult = RemapValClamped( flMinMult, flMinMult, flMaxMult, 0, 255 );
		return;
	}

	//Msg( "Crit mult update for %s\n", m_pOuter->GetPlayerName() );
	//Msg( "   Entries: %d\n", m_DamageEvents.Count() );

	// Go through the damage multipliers and remove expired ones, while summing damage of the others
	float flTotalDamage = 0;
	for ( int i = m_DamageEvents.Count() - 1; i >= 0; i-- )
	{
		float flDelta = gpGlobals->curtime - m_DamageEvents[i].flTime;
		if ( flDelta > tf_damage_events_track_for.GetFloat() )
		{
			//Msg( "      Discarded (%d: time %.2f, now %.2f)\n", i, m_DamageEvents[i].flTime, gpGlobals->curtime );
			m_DamageEvents.Remove( i );
			continue;
		}

		// Ignore damage we've just done. We do this so that we have time to get those damage events
		// to the client in time for using them in prediction in this code.
		if ( flDelta < TF_DAMAGE_CRITMOD_MINTIME )
		{
			//Msg( "      Ignored (%d: time %.2f, now %.2f)\n", i, m_DamageEvents[i].flTime, gpGlobals->curtime );
			continue;
		}

		if ( flDelta > TF_DAMAGE_CRITMOD_MAXTIME )
			continue;

		//Msg( "      Added %.2f (%d: time %.2f, now %.2f)\n", m_DamageEvents[i].flDamage, i, m_DamageEvents[i].flTime, gpGlobals->curtime );

		flTotalDamage += m_DamageEvents[i].flDamage;
	}

	float flMult = RemapValClamped( flTotalDamage, 0, TF_DAMAGE_CRITMOD_DAMAGE, flMinMult, flMaxMult );

	//Msg( "   TotalDamage: %.2f   -> Mult %.2f\n", flTotalDamage, flMult );

	m_iCritMult = (int)RemapValClamped( flMult, flMinMult, flMaxMult, 0, 255 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::RecordDamageEvent( const CTakeDamageInfo &info, bool bKill )
{
	if ( m_DamageEvents.Count() >= MAX_DAMAGE_EVENTS )
	{
		// Remove the oldest event
		m_DamageEvents.Remove( m_DamageEvents.Count() - 1 );
	}

	int iIndex = m_DamageEvents.AddToTail();
	m_DamageEvents[iIndex].flDamage = info.GetDamage();
	m_DamageEvents[iIndex].flTime = gpGlobals->curtime;
	m_DamageEvents[iIndex].bKill = bKill;

	// Don't count critical damage
	if ( info.GetDamageType() & DMG_CRITICAL )
	{
		m_DamageEvents[iIndex].flDamage /= TF_DAMAGE_CRIT_MULTIPLIER;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFPlayerShared::GetNumKillsInTime( float flTime )
{
	if ( tf_damage_events_track_for.GetFloat() < flTime )
	{
		Warning( "Player asking for damage events for time %.0f, but tf_damage_events_track_for is only tracking events for %.0f\n", flTime, tf_damage_events_track_for.GetFloat() );
	}

	int iKills = 0;
	for ( int i = m_DamageEvents.Count() - 1; i >= 0; i-- )
	{
		float flDelta = gpGlobals->curtime - m_DamageEvents[i].flTime;
		if ( flDelta < flTime )
		{
			if ( m_DamageEvents[i].bKill )
			{
				iKills++;
			}
		}
	}

	return iKills;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Add Mannpower Revenge Crit
//-----------------------------------------------------------------------------
void CTFPlayerShared::AddTempCritBonus( float flDuration )
{
	AddCond( TF_COND_CRITBOOSTED_RUNE_TEMP, flDuration );
	AddCond( TF_COND_RUNE_IMBALANCE, flDuration );
}

//=============================================================================
//
// Shared player code that isn't CTFPlayerShared
//

//-----------------------------------------------------------------------------
// Purpose:
//   Input: info
//          bDoEffects - effects (blood, etc.) should only happen client-side.
//-----------------------------------------------------------------------------
void CTFPlayer::FireBullet( const FireBulletsInfo_t &info, bool bDoEffects, int nDamageType, int nCustomDamageType /*= TF_DMG_CUSTOM_NONE*/ )
{
	// Fire a bullet (ignoring the shooter).
	Vector vecStart = info.m_vecSrc;
	Vector vecEnd = vecStart + info.m_vecDirShooting * info.m_flDistance;
	trace_t trace;

	// Skip multiple entities when tracing
	CTraceFilterSimpleList traceFilter( COLLISION_GROUP_NONE );
	traceFilter.SetPassEntity( this ); // Standard pass entity for THIS so that it can be easily removed from the list after passing through a portal
	// Also ignore a vehicle we're a passenger in
	//if ( MyCombatCharacterPointer() && MyCombatCharacterPointer()->IsInAVehicle() )
	//	traceFilter.AddEntityToIgnore( MyCombatCharacterPointer()->GetVehicleEntity() );

	CProp_Portal *pShootThroughPortal = NULL;
	float flPortalFraction = 2.0f;
	Ray_t rayBullet;
	rayBullet.Init( info.m_vecSrc, vecEnd );
	pShootThroughPortal = UTIL_Portal_FirstAlongRay( rayBullet, flPortalFraction );

	CTraceFilterIgnoreFriendlyCombatItems traceFilterCombatItem( this, COLLISION_GROUP_NONE, GetTeamNumber() );
	CTraceFilterChain traceFilterChain( &traceFilter, &traceFilterCombatItem );
	if ( !UTIL_Portal_TraceRay_Bullets( pShootThroughPortal, rayBullet, MASK_SHOT, &traceFilterChain, &trace ) )
		pShootThroughPortal = NULL;

	//UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT, this, &traceFilter, &trace );

#ifdef CLIENT_DLL
	if ( sv_showimpacts.GetInt() == 1 || sv_showimpacts.GetInt() == 2 )
	{
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( trace.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( trace.m_pEnt && trace.m_pEnt->IsCombatCharacter() )
		{
			C_BaseCombatCharacter *pCombat = ToBaseCombatCharacter( trace.m_pEnt );
			pCombat->DrawClientHitboxes( 4, true );
		}
	}
#else
	if ( sv_showimpacts.GetInt() == 1 || sv_showimpacts.GetInt() == 3 )
	{
		// draw blue server impact markers
		NDebugOverlay::Box( trace.endpos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 0, 255, 127, 4 );

		if ( trace.m_pEnt && trace.m_pEnt->IsCombatCharacter() )
		{
			CBaseCombatCharacter *pCombat = ToBaseCombatCharacter( trace.m_pEnt );
			pCombat->DrawServerHitboxes( 4, true );
		}
	}

	if ( tf_debug_bullets.GetBool() )
		NDebugOverlay::Line( vecStart, trace.endpos, 0, 255, 0, true, 30 );
#endif

	if ( !trace.startsolid )
	{
		vecStart = trace.endpos - trace.startpos;
		VectorNormalize( vecStart );
	}

	if ( trace.fraction < 1.0 )
	{
		// Verify we have an entity at the point of impact.
		Assert( trace.m_pEnt );

		if ( bDoEffects )
		{
			// If shot starts out of water and ends in water
			if ( !( enginetrace->GetPointContents( trace.startpos ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) &&
				( enginetrace->GetPointContents( trace.endpos ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) )
			{
				// Water impact effects.
				ImpactWaterTrace( trace, vecStart );
			}
			else
			{
				// Regular impact effects.

				// don't decal your teammates or objects on your team
				if ( trace.m_pEnt->GetTeamNumber() != GetTeamNumber() )
				{
					UTIL_ImpactTrace( &trace, nDamageType );
				}
			}

#ifdef CLIENT_DLL
			static int	tracerCount;
			if ( ( info.m_iTracerFreq != 0 ) && ( tracerCount++ % info.m_iTracerFreq ) == 0 )
			{
				// if this is a local player, start at attachment on view model
				// else start on attachment on weapon model

				int iEntIndex = entindex();
				int iUseAttachment = TRACER_DONT_USE_ATTACHMENT;
				int iAttachment = 1;

				C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
				C_TFWeaponBase *pTFWeapon = dynamic_cast< C_TFWeaponBase* >( pWeapon );

				if( pWeapon )
				{
					if ( pTFWeapon )
					{
						iAttachment = pTFWeapon->GetViewmodelAddon() ? pTFWeapon->GetViewmodelAddon()->LookupAttachment( "muzzle" ) : pWeapon->LookupAttachment( "muzzle" );
					}
					else
					{
						iAttachment = pWeapon->LookupAttachment( "muzzle" );
					}
				}

				bool bInToolRecordingMode = clienttools->IsInRecordingMode();

				// try to align tracers to actual weapon barrel if possible
				if ( !ShouldDrawThisPlayer() && !bInToolRecordingMode )
				{
					C_TFViewModel *pViewModel = dynamic_cast<C_TFViewModel *>( GetViewModel() );

					if ( pViewModel )
					{
						iEntIndex = pViewModel->entindex();
						pViewModel->GetAttachment( iAttachment, vecStart );
					}
				}
				else if ( !IsDormant() )
				{
					// fill in with third person weapon model index
					if( pWeapon )
					{
						iEntIndex = pWeapon->entindex();

						int nModelIndex = pWeapon->GetModelIndex();
						int nWorldModelIndex = pWeapon->GetWorldModelIndex();
						if ( bInToolRecordingMode && nModelIndex != nWorldModelIndex )
						{
							pWeapon->SetModelIndex( nWorldModelIndex );
						}

						pWeapon->GetAttachment( iAttachment, vecStart );

						if ( bInToolRecordingMode && nModelIndex != nWorldModelIndex )
						{
							pWeapon->SetModelIndex( nModelIndex );
						}
					}
				}

				if ( tf_useparticletracers.GetBool() )
				{
					const char *pszTracerEffect = GetTracerType();
					if ( pszTracerEffect && pszTracerEffect[0] )
					{
						char szTracerEffect[128];
						if ( nDamageType & DMG_CRITICAL )
						{
							if ( Q_strcmp( pszTracerEffect, "dxhr_sniper_rail_red" ) || Q_strcmp( pszTracerEffect, "dxhr_sniper_rail_blue" ) || Q_strcmp( pszTracerEffect, "tfc_sniper_distortion_trail" ) )
							{
								Q_snprintf( szTracerEffect, sizeof(szTracerEffect), "%s_crit", pszTracerEffect );
								pszTracerEffect = szTracerEffect;
							}
						}

						if ( pShootThroughPortal )
						{
							trace.endpos = info.m_vecSrc + ( vecEnd - info.m_vecSrc ) * flPortalFraction;
						}

						FX_TFTracer( pszTracerEffect, vecStart, trace.endpos, entindex(), true );

						if ( pShootThroughPortal )
						{
							Vector vTransformedIntersection;
							UTIL_Portal_PointTransform( pShootThroughPortal->MatrixThisToLinked(), trace.endpos, vTransformedIntersection );
							ComputeTracerStartPosition( vTransformedIntersection, &vecStart );

							FX_TFTracer( pszTracerEffect, vecStart, trace.endpos, entindex(), true );

							// Shooting through a portal, the damage direction is translated through the passed-through portal
							// so the damage indicator hud animation is correct
							Vector vDmgOriginThroughPortal;
							UTIL_Portal_PointTransform( pShootThroughPortal->MatrixThisToLinked(), vecStart, vDmgOriginThroughPortal );
							g_MultiDamage.SetDamagePosition ( vDmgOriginThroughPortal );
						}
						else
						{
							g_MultiDamage.SetDamagePosition ( info.m_vecSrc );
						}
					}
				}
				else
				{
					UTIL_Tracer( vecStart, trace.endpos, entindex(), iUseAttachment, 5000, true, GetTracerType() );
				}
			}
#endif
		}

		// Server specific.
#ifdef GAME_DLL
		// See what material we hit.
		CTakeDamageInfo dmgInfo( this, info.m_pAttacker, GetActiveWeapon(), info.m_flDamage, nDamageType, nCustomDamageType );
		CalculateBulletDamageForce( &dmgInfo, info.m_iAmmoType, info.m_vecDirShooting, trace.endpos, 1.0 );	//MATTTODO bullet forces
		trace.m_pEnt->DispatchTraceAttack( dmgInfo, info.m_vecDirShooting, &trace );
		//CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, trace.endpos, 400, 0.4f, this, SOUNDENT_CHANNEL_BULLET_IMPACT );
#endif
	}
}

#ifdef CLIENT_DLL
ConVar tf_impactwatertimeenable( "tf_impactwatertimeenable", "0", FCVAR_CHEAT, "Draw impact debris effects." );
ConVar tf_impactwatertime( "tf_impactwatertime", "1.0f", FCVAR_CHEAT, "Draw impact debris effects." );
#endif

//-----------------------------------------------------------------------------
// Purpose: Trace from the shooter to the point of impact (another player,
//          world, etc.), but this time take into account water/slime surfaces.
//   Input: trace - initial trace from player to point of impact
//          vecStart - starting point of the trace 
//-----------------------------------------------------------------------------
void CTFPlayer::ImpactWaterTrace( trace_t &trace, const Vector &vecStart )
{
#ifdef CLIENT_DLL
	if ( tf_impactwatertimeenable.GetBool() )
	{
		if ( m_flWaterImpactTime > gpGlobals->curtime )
			return;
	}
#endif 

	trace_t traceWater;
	Ray_t ray; ray.Init( vecStart, trace.endpos );
	UTIL_Portal_TraceRay( ray, ( MASK_SHOT | CONTENTS_WATER | CONTENTS_SLIME ), this, COLLISION_GROUP_NONE, &traceWater );
	if ( traceWater.fraction < 1.0f )
	{
		CEffectData	data;
		data.m_vOrigin = traceWater.endpos;
		data.m_vNormal = traceWater.plane.normal;
		data.m_flScale = random->RandomFloat( 8, 12 );
		if ( traceWater.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		const char *pszEffectName = "tf_gunshotsplash";
		CTFWeaponBase *pWeapon = GetActiveTFWeapon();
		if ( pWeapon && ( TF_WEAPON_MINIGUN == pWeapon->GetWeaponID() ) )
		{
			// for the minigun, use a different, cheaper splash effect because it can create so many of them
			pszEffectName = "tf_gunshotsplash_minigun";
		}
		DispatchEffect( pszEffectName, data );

#ifdef CLIENT_DLL
		if ( tf_impactwatertimeenable.GetBool() )
		{
			m_flWaterImpactTime = gpGlobals->curtime + tf_impactwatertime.GetFloat();
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFWeaponBase *CTFPlayer::GetActiveTFWeapon( void ) const
{
	CBaseCombatWeapon *pRet = GetActiveWeapon();
	if ( pRet )
	{
		Assert( dynamic_cast<CTFWeaponBase*>( pRet ) != NULL );
		return static_cast<CTFWeaponBase *>( pRet );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::IsActiveTFWeapon( int iWeaponID )
{
	CTFWeaponBase *pWeapon = GetActiveTFWeapon();
	if ( pWeapon )
	{
		return pWeapon->GetWeaponID() == iWeaponID;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: How much build resource ( metal ) does this player have
//-----------------------------------------------------------------------------
int CTFPlayer::GetBuildResources( void )
{
	return GetAmmoCount( TF_AMMO_METAL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::TeamFortress_SetSpeed()
{
	int playerclass = GetPlayerClass()->GetClassIndex();
	float maxfbspeed = 0.0f;

	// Spectators can move while in Classic Observer mode
	if ( IsObserver() )
	{
		if ( GetObserverMode() == OBS_MODE_ROAMING )
			SetMaxSpeed( GetPlayerClassData( TF_CLASS_SCOUT )->m_flMaxSpeed );
		else
			SetMaxSpeed( 0 );
		return;
	}

	// Check for any reason why they can't move at all
	if ( playerclass == TF_CLASS_UNDEFINED || GameRules()->InRoundRestart() )
	{
		SetAbsVelocity( vec3_origin );
		SetMaxSpeed( 1 );
		return;
	}

	// First, get their max class speed
	maxfbspeed = GetPlayerClassData( playerclass )->m_flMaxSpeed;

	CALL_ATTRIB_HOOK_FLOAT( maxfbspeed, mult_player_movespeed );

	CTFWeaponBase *pWeapon = GetActiveTFWeapon();

	if ( IsAlive() && pWeapon )
		CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pWeapon, maxfbspeed, mult_player_movespeed_active );

	if ( IsAlive() && m_Shared.HasDemoShieldEquipped() )
		CALL_ATTRIB_HOOK_FLOAT( maxfbspeed, mult_player_movespeed_shieldrequired );

	// speed boost
	if ( m_Shared.IsSpeedBoosted() )
		maxfbspeed *= 1.2f;

	// haste
	if ( m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		maxfbspeed *= 1.2f;

	// agility
	if ( m_Shared.InCond( TF_COND_RUNE_AGILITY ) )
		maxfbspeed *= 1.3f;

	// Slow us down if we're disguised as a slower class
	// unless we're cloaked..
	if ( m_Shared.InCond( TF_COND_DISGUISED ) && !m_Shared.IsStealthed() )
	{
		if (m_Shared.GetDisguiseClass() != TF_CLASS_WILDCARD && m_Shared.GetDisguiseClass() != TF_CLASS_ZOMBIEFAST)
		{
			float flMaxDisguiseSpeed = GetPlayerClassData(m_Shared.GetDisguiseClass())->m_flMaxSpeed;
			maxfbspeed = min(flMaxDisguiseSpeed, maxfbspeed);
		}
	}

	// Second, see if any flags are slowing them down
	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		CCaptureFlag *pFlag = dynamic_cast<CCaptureFlag*>( GetItem() );

		if ( pFlag )
		{
			if ( pFlag->GetGameType() == TF_FLAGTYPE_ATTACK_DEFEND || pFlag->GetGameType() == TF_FLAGTYPE_TERRITORY_CONTROL )
			{
				maxfbspeed *= 0.5;
			}
		}
	}

	// if they're aiming or spun up, reduce their speed
	if ( m_Shared.InCond( TF_COND_AIMING ) )
	{
		// Heavy moves slightly faster spun-up
		if ( pWeapon && pWeapon->IsWeapon( TF_WEAPON_MINIGUN ) )
		{
			if ( maxfbspeed > 110 )
				maxfbspeed = 110;
		}
		else
		{
			if ( maxfbspeed > 80 )
				maxfbspeed = 80;
		}
	}

	// Engineer moves slower while a hauling a building.
	if ( playerclass == TF_CLASS_ENGINEER && m_Shared.IsCarryingObject() )
	{
		maxfbspeed *= 0.9f;
	}

	if ( Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) )
	{
		float flResourceMult = 1.f;
		CALL_ATTRIB_HOOK_FLOAT( flResourceMult, mult_player_movespeed_resource_level );
		if ( flResourceMult != 1.0f )
		{
			CWeaponMedigun *pMedigun = dynamic_cast<CWeaponMedigun *>( Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) );
			maxfbspeed *= RemapValClamped( pMedigun->GetChargeLevel(), 0.0, 1.0, 1.0, flResourceMult );
		}
	}

	if ( m_Shared.IsStealthed() )
	{
		if ( maxfbspeed > tf_spy_max_cloaked_speed.GetFloat() )
			maxfbspeed = tf_spy_max_cloaked_speed.GetFloat();
	}

	if ( pWeapon )
		maxfbspeed *= pWeapon->GetSpeedMod();

	int nBoostOnDamage = 0;

	for ( int i = 0; i < LOADOUT_POSITION_BUFFER; i++ )
	{
		CTFWeaponBase *pAllWeapon = ( CTFWeaponBase * )Weapon_GetSlot( i );
		if ( pAllWeapon )
		{
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pAllWeapon, nBoostOnDamage, boost_on_damage );
			if ( nBoostOnDamage > 0 )
				maxfbspeed *= ( ( m_Shared.GetScoutHypeMeter() / 100 ) + 1 );
		}
	}

	CTFSword *pSword = dynamic_cast<CTFSword *>( Weapon_OwnsThisID( TF_WEAPON_SWORD ) );
	if ( pSword )
		maxfbspeed *= pSword->GetSwordSpeedMod();

	if ( m_Shared.InCond( TF_COND_SHIELD_CHARGE ) )
		maxfbspeed = 725.0f;

	CWeaponMedigun *pMedigun = GetMedigun();
	if ( pMedigun )
	{
		CTFPlayer *pHealTarget = ToTFPlayer( MedicGetHealTarget() );
		if ( /*pMedigun->m_bHealing &&*/ pHealTarget && GetPlayerClassData( pHealTarget->GetPlayerClass()->GetClassIndex() )->m_flMaxSpeed > maxfbspeed )
			maxfbspeed = GetPlayerClassData( pHealTarget->GetPlayerClass()->GetClassIndex() )->m_flMaxSpeed;
	}

	if ( m_Shared.InCond( TF_COND_DISGUISED_AS_DISPENSER ) )
		maxfbspeed *= 2.0f;

	// (Potentially) Reduce our speed if we were stunned
	if ( m_Shared.InCond( TF_COND_STUNNED ) && ( m_Shared.m_nStunFlags & TF_STUNFLAG_SLOWDOWN ) )
	{
		maxfbspeed *= m_Shared.m_flStunMovementSpeed;
	}

	// if we're in bonus time because a team has won, give the winners 110% speed and the losers 90% speed
	if ( TFGameRules()->State_Get() == GR_STATE_TEAM_WIN )
	{
		int iWinner = TFGameRules()->GetWinningTeam();

		if ( iWinner != TEAM_UNASSIGNED )
		{
			if ( iWinner == GetTeamNumber() )
			{
				maxfbspeed *= 1.1f;
			}
			else
			{
				maxfbspeed *= 0.9f;
			}
		}
	}

	// Set the speed
	SetMaxSpeed( maxfbspeed );

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::HasItem( void )
{
	return ( m_hItem != NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::SetItem( CTFItem *pItem )
{
	m_hItem = pItem;

#ifndef CLIENT_DLL
	if ( pItem && pItem->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		RemoveInvisibility();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFItem	*CTFPlayer::GetItem( void )
{
	return m_hItem;
}

//-----------------------------------------------------------------------------
// Purpose: Is the player allowed to use a teleporter ?
//-----------------------------------------------------------------------------
bool CTFPlayer::HasTheFlag( void )
{
	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Are we allowed to pick the flag up?
//-----------------------------------------------------------------------------
bool CTFPlayer::IsAllowedToPickUpFlag( void )
{
	int bNotAllowedToPickUpFlag = 0;
	CALL_ATTRIB_HOOK_INT( bNotAllowedToPickUpFlag, cannot_pick_up_intelligence );

	if ( bNotAllowedToPickUpFlag > 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Are we allowed to taunt?
//-----------------------------------------------------------------------------
bool CTFPlayer::IsAllowedToTaunt( void )
{
	// Check to see if we can taunt again!
	if ( m_Shared.InCond( TF_COND_TAUNTING ) )
		return false;

	// Check to see if we are in water (above our waist).
	if ( GetWaterLevel() > WL_Waist )
		return false;

	// Check to see if we are on the ground.
	if ( GetGroundEntity() == NULL && !m_Shared.InCond( TF_COND_HALLOWEEN_KART ) && ( GetMoveType() != MOVETYPE_NOCLIP ) )
		return false;

	// Will our weapon let us taunt?
	CTFWeaponBase *pWeapon = GetActiveTFWeapon();
	if ( pWeapon )
	{
		if( !pWeapon->OwnerCanTaunt() )
			return false;

		if ( pWeapon->GetWeaponID() == TF_WEAPON_PDA_ENGINEER_BUILD || pWeapon->GetWeaponID() == TF_WEAPON_PDA_ENGINEER_DESTROY )
			return false;
	}

	// Check to see if cutscene is playing.
	if ( m_Shared.IsInCutScene() )
		return false;

	// Can't taunt while cloaked.
	if ( m_Shared.IsStealthed() || m_Shared.InCond( TF_COND_STEALTHED_BLINK ) )
		return false;

	// Can't taunt while disguised.
	if ( m_Shared.InCond( TF_COND_DISGUISED ) || m_Shared.InCond( TF_COND_DISGUISING ) )
		return false;

	// Can't taunt while charge.
	if ( m_Shared.InCond( TF_COND_SHIELD_CHARGE ) )
		return false;

/*#ifdef GAME_DLL
	if( pWeapon && ( pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX ) && pWeapon->ClassMatches( "tf_weapon_lunchbox_drink" ) )
	{
		// If we're already in a buff, prevent another from getting applied
		if ( m_Shared.InCond( TF_COND_ENERGY_BUFF ) || m_Shared.InCond( TF_COND_PHASE ) )
			return false;

		// Can't use unless fully charged
		if ( m_Shared.m_flEnergyDrinkMeter < 100.0f )
			return false;

		int nLunchBoxAddsMinicrits = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nLunchBoxAddsMinicrits, set_weapon_mode );
		if ( nLunchBoxAddsMinicrits == 3 )
			return false;
	}
#endif*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this player's allowed to build another one of the specified object
//-----------------------------------------------------------------------------
int CTFPlayer::CanBuild( int iObjectType, int iObjectMode )
{
	if ( iObjectType < 0 || iObjectType >= OBJ_LAST )
		return CB_UNKNOWN_OBJECT;

#ifndef CLIENT_DLL
	CTFPlayerClass *pCls = GetPlayerClass();

	if ( m_Shared.IsCarryingObject() )
	{
		CBaseObject *pObject = m_Shared.GetCarriedObject();
		if ( pObject && pObject->GetType() == iObjectType && pObject->GetObjectMode() == iObjectMode )
		{
			return CB_CAN_BUILD;
		}
		else
		{
			Assert( 0 );
		}
	}

	if ( pCls && pCls->CanBuildObject( iObjectType ) == false )
	{
		return CB_CANNOT_BUILD;
	}

	if ( GetObjectInfo( iObjectType )->m_AltModes.Count() != 0
		&& GetObjectInfo( iObjectType )->m_AltModes.Count() <= iObjectMode * 3 )
	{
		return CB_CANNOT_BUILD;
	}
	else if ( GetObjectInfo( iObjectType )->m_AltModes.Count() == 0 && iObjectMode != 0 )
	{
		return CB_CANNOT_BUILD;
	}

#endif

	int iObjectCount = GetNumObjects( iObjectType, iObjectMode );

	// Make sure we haven't hit maximum number
	if ( !lfe_obj_unlimited.GetBool() )
	{
		if ( iObjectCount >= GetObjectInfo( iObjectType )->m_nMaxObjects && GetObjectInfo( iObjectType )->m_nMaxObjects != -1 )
		{
			return CB_LIMIT_REACHED;
		}
	}

	// Find out how much the object should cost
	int iCost = CalculateObjectCost( iObjectType, HasGunslinger() );

	// Make sure we have enough resources
	if ( GetBuildResources() < iCost )
	{
		return CB_NEED_RESOURCES;
	}

	return CB_CAN_BUILD;
}

//-----------------------------------------------------------------------------
// Purpose: Get the number of objects of the specified type that this player has
//-----------------------------------------------------------------------------
int CTFPlayer::GetNumObjects( int iObjectType, int iObjectMode )
{
	int iCount = 0;
	for ( int i = 0; i < GetObjectCount(); i++ )
	{
		if ( !GetObject( i ) )
			continue;

		if ( GetObject( i )->GetType() == iObjectType && GetObject( i )->GetObjectMode() == iObjectMode && !GetObject( i )->IsBeingCarried() )
		{
			iCount++;
		}
	}

	return iCount;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::ItemPostFrame()
{
	if ( m_hOffHandWeapon.Get() && m_hOffHandWeapon->IsWeaponVisible() )
	{
		if ( gpGlobals->curtime < m_flNextAttack )
		{
			m_hOffHandWeapon->ItemBusyFrame();
		}
		else
		{
#if defined( CLIENT_DLL )
			// Not predicting this weapon
			if ( m_hOffHandWeapon->IsPredicted() )
#endif
			{
				m_hOffHandWeapon->ItemPostFrame();
			}
		}
	}
#ifdef GAME_DLL
	if( sv_infinite_ammo.GetBool() && ( GetActiveTFWeapon() ) )
	{
		// Refill clip in all weapons.
		for ( int i = 0; i < WeaponCount(); i++ )
		{
			CTFWeaponBase *pWeapon = ( CTFWeaponBase * )GetWeapon( i );
			if ( !pWeapon )
				continue;

			for ( int i = TF_AMMO_PRIMARY; i < TF_AMMO_COUNT; i++ )
			{
				GiveAmmo( GetMaxAmmo( i ), i, false, TF_AMMO_SOURCE_RESUPPLY );
			}

			pWeapon->GiveDefaultAmmo();
			pWeapon->WeaponRegenerate();

			// Refill charge meters
			m_Shared.m_flEffectBarProgress = 100.0f;
			//pWeapon->EffectBarRegenFinished();

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
		}
	}
#endif
	BaseClass::ItemPostFrame();
}

void CTFPlayer::SetOffHandWeapon( CTFWeaponBase *pWeapon )
{
	m_hOffHandWeapon = pWeapon;
	if ( m_hOffHandWeapon.Get() )
	{
		m_hOffHandWeapon->Deploy();
	}
}

// Set to NULL at the end of the holster?
void CTFPlayer::HolsterOffHandWeapon( void )
{
	if ( m_hOffHandWeapon.Get() )
	{
		m_hOffHandWeapon->Holster();

		CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
		if ( pInvis )
		{
			m_hOffHandWeapon = pInvis;
			if ( pInvis->HasFeignDeath() )
				m_Shared.m_bFeignDeathReady = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return true if we should record our last weapon when switching between the two specified weapons
//-----------------------------------------------------------------------------
bool CTFPlayer::Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon )
{
	// if the weapon doesn't want to be auto-switched to, don't!	
	CTFWeaponBase *pTFWeapon = dynamic_cast<CTFWeaponBase *>( pOldWeapon );

	if ( pTFWeapon->AllowsAutoSwitchTo() == false )
	{
		return false;
	}

	return BaseClass::Weapon_ShouldSetLast( pOldWeapon, pNewWeapon );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	if ( m_PlayerAnimState )
		m_PlayerAnimState->ResetGestureSlot( GESTURE_SLOT_ATTACK_AND_RELOAD );

	return BaseClass::Weapon_Switch( pWeapon, viewmodelindex );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::GetStepSoundVelocities( float *velwalk, float *velrun )
{
	float flMaxSpeed = MaxSpeed();

	if ( ( GetFlags() & FL_DUCKING ) || ( GetMoveType() == MOVETYPE_LADDER ) )
	{
		*velwalk = flMaxSpeed * 0.25;
		*velrun = flMaxSpeed * 0.3;
	}
	else
	{
		*velwalk = flMaxSpeed * 0.3;
		*velrun = flMaxSpeed * 0.8;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking )
{
	float flMaxSpeed = MaxSpeed();

	switch ( iStepSoundTime )
	{
	case STEPSOUNDTIME_NORMAL:
	case STEPSOUNDTIME_WATER_FOOT:
		m_flStepSoundTime = RemapValClamped( flMaxSpeed, 200, 450, 400, 200 );
		if ( bWalking )
		{
			m_flStepSoundTime += 100;
		}
		break;

	case STEPSOUNDTIME_ON_LADDER:
		m_flStepSoundTime = 350;
		break;

	case STEPSOUNDTIME_WATER_KNEE:
		m_flStepSoundTime = RemapValClamped( flMaxSpeed, 200, 450, 600, 400 );
		break;

	default:
		Assert( 0 );
		break;
	}

	if ( ( GetFlags() & FL_DUCKING ) || ( GetMoveType() == MOVETYPE_LADDER ) )
	{
		m_flStepSoundTime += 100;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::CanAttack( void )
{
	CTFGameRules *pRules = TFGameRules();

	Assert( pRules );

	// Only regular cloak prevents us from firing.
	if ( m_Shared.GetStealthNoAttackExpireTime() > gpGlobals->curtime || m_Shared.InCond( TF_COND_STEALTHED ) || m_Shared.m_bFeignDeathReady && !m_Shared.InCond( TF_COND_STEALTHED_USER_BUFF ) )
	{
#ifdef CLIENT_DLL
		HintMessage( HINT_CANNOT_ATTACK_WHILE_CLOAKED, true, true );
#endif
		return false;
	}

	if ( ( pRules->State_Get() == GR_STATE_TEAM_WIN ) && ( pRules->GetWinningTeam() != GetTeamNumber() ) )
		return false;

	if ( m_Shared.InCond( TF_COND_TAUNTING ) )
		return false;

	// Stunned players cannot attack
	if ( m_Shared.InCond( TF_COND_STUNNED ) )
		return false;

	if ( m_Shared.IsInCutScene() )
		return false;

	int iNoAttack = 0;
	CALL_ATTRIB_HOOK_INT( iNoAttack, no_attack );
	if ( iNoAttack != 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: is engihere?
//-----------------------------------------------------------------------------
bool CTFPlayer::CanPickupBuilding( CBaseObject *pObject )
{
	if ( !pObject )
		return false;
#ifdef GAME_DLL
	if ( lfe_force_legacy.GetBool() )
		return false;
#endif
	if ( m_Shared.IsLoser() )
		return false;

	if ( IsActiveTFWeapon( TF_WEAPON_BUILDER ) )
		return false;

	int bCannotPickUpBuildings = 0;
	CALL_ATTRIB_HOOK_INT( bCannotPickUpBuildings, cannot_pick_up_buildings );
	if ( bCannotPickUpBuildings != 0 )
		return false;

	if ( pObject->GetBuilder() != this )
		return false;

	if ( pObject->IsBuilding() || pObject->IsUpgrading() || pObject->IsRedeploying() || pObject->IsDisabled() )
		return false;

	if ( pObject->HasSapper() )
		return false;

	if ( m_Shared.InCond( TF_COND_STUNNED ) )
		return false;

	return true;
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::TryToPickupBuilding( void )
{
	Vector vecForward;
	AngleVectors( EyeAngles(), &vecForward );
	Vector vecSwingStart = Weapon_ShootPosition();

	// Check if we can pick up buildings remotely, and how much it costs.
	bool bHasRemotePickup = false;
	int nRemotePickup = 0;
	if ( GetActiveTFWeapon() )
	{
		CALL_ATTRIB_HOOK_INT_ON_OTHER( GetActiveTFWeapon() , nRemotePickup, building_teleporting_pickup );
		if ( nRemotePickup != 0 )
		{
			// Check if we also have enough metal to perform a remote pickup.
			if ( GetBuildResources() > nRemotePickup )
				bHasRemotePickup = true;
		}
	}

	float flRange = 150.0f;
	int iMaxHaulingChecks = ( bHasRemotePickup ? 2 : 1 );
	for ( int iTraces = 1; iTraces <= iMaxHaulingChecks; iTraces++ )
	{
		// Increase our range on the teleport pickup iteration.
		if ( iTraces == 2 )
			flRange = 5500.0f;

		Vector vecSwingEnd = vecSwingStart + vecForward * flRange;

		// only trace against objects
		// See if we hit anything.
		trace_t trace;
		CTraceFilterIgnorePlayers traceFilter( NULL, COLLISION_GROUP_NONE );
		Ray_t ray; ray.Init( vecSwingStart, vecSwingEnd );
		UTIL_Portal_TraceRay( ray, MASK_SOLID, &traceFilter, &trace );

		if ( ( trace.fraction < 1.0f ) && trace.m_pEnt && trace.m_pEnt->IsBaseObject() && InSameTeam( trace.m_pEnt ) )
		{
			CBaseObject *pObject = dynamic_cast<CBaseObject*>( trace.m_pEnt );
			if ( CanPickupBuilding( pObject ) )
			{
				CTFWeaponBase *pWpn = Weapon_OwnsThisID( TF_WEAPON_BUILDER );
				if ( pWpn )
				{
					CTFWeaponBuilder *pBuilder = dynamic_cast<CTFWeaponBuilder *>( pWpn );
					// Is this the builder that builds the object we're looking for?
					if ( pBuilder )
					{
						pObject->MakeCarriedObject( this );

						pBuilder->SetSubType( pObject->ObjectType() );
						pBuilder->SetObjectMode( pObject->GetObjectMode() );

						SpeakConceptIfAllowed( MP_CONCEPT_PICKUP_BUILDING );

						// try to switch to this weapon
						Weapon_Switch( pBuilder );

						m_flNextCarryTalkTime = gpGlobals->curtime + RandomFloat( 6.0f, 12.0f );

						// If remote pickup, subtract the metal from our current metal reserve.
						if ( iTraces == 2 )
							RemoveBuildResources( nRemotePickup );

						return true;
					}
				}
			}
		}
	}
	return false;
}

void CTFPlayerShared::SetCarriedObject( CBaseObject *pObj )
{
	if ( pObj )
	{
		m_bCarryingObject = true;
		m_hCarriedObject = pObj;
	}
	else
	{
		m_bCarryingObject = false;
		m_hCarriedObject = NULL;
	}

	m_pOuter->TeamFortress_SetSpeed();
}

CBaseObject* CTFPlayerShared::GetCarriedObject( void )
{
	CBaseObject *pObj = m_hCarriedObject.Get();
	return pObj;
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Weapons can call this on secondary attack and it will link to the class
// ability
//-----------------------------------------------------------------------------
bool CTFPlayer::DoClassSpecialSkill( void )
{
	bool bDoSkill = false;

	switch ( GetPlayerClass()->GetClassIndex() )
	{
	case TF_CLASS_SPY:
	{
		CTFWeaponInvis *pInvis = static_cast<CTFWeaponInvis*>( Weapon_OwnsThisID( TF_WEAPON_INVIS ) );
		if ( m_Shared.m_flStealthNextChangeTime <= gpGlobals->curtime )
		{	// Dead Ringer
			if ( pInvis->HasFeignDeath() )
			{
				if ( m_Shared.m_bFeignDeathReady )
				{
					HolsterOffHandWeapon();

					if ( m_Shared.InCond( TF_COND_STEALTHED ) )
					{
						m_Shared.FadeInvis( tf_spy_invis_unstealth_time.GetFloat() );
						m_Shared.RemoveCond( TF_COND_FEIGN_DEATH );
					}

					bDoSkill = true;
				}
				else if ( !m_Shared.m_bFeignDeathReady && m_Shared.GetSpyCloakMeter() == 100.0f && !m_Shared.InCond( TF_COND_STEALTHED ) )
				{
					m_Shared.SetOffHandWatch();
					bDoSkill = true;
				}
			}
			else
			{	// Toggle invisibility
				if ( m_Shared.InCond( TF_COND_STEALTHED ) )
				{
					m_Shared.FadeInvis( tf_spy_invis_unstealth_time.GetFloat() );
					bDoSkill = true;
				}
				else if ( CanGoInvisible() && ((m_Shared.GetSpyCloakMeter() > 8.0f) || pInvis->HasMotionCloak() ) )	// must have over 10% cloak to start
				{
					m_Shared.AddCond( TF_COND_STEALTHED );

					bDoSkill = true;
				}
			}
			if ( bDoSkill )
				m_Shared.m_flStealthNextChangeTime = gpGlobals->curtime + 0.5;
		}
	break;
	}

	case TF_CLASS_DEMOMAN:
	{
		CTFPipebombLauncher *pPipebombLauncher = static_cast<CTFPipebombLauncher*>(Weapon_OwnsThisID(TF_WEAPON_PIPEBOMBLAUNCHER));

		if ( pPipebombLauncher )
		{
			pPipebombLauncher->SecondaryAttack();
			bDoSkill = true;
		}
		break;
	}

	case TF_CLASS_ENGINEER:
	{
		bDoSkill = false;
#ifdef GAME_DLL
		bDoSkill = TryToPickupBuilding();
#endif
		break;
	}

	default:
		break;
	}

	CTFWearableDemoShield *pShield = GetEquippedDemoShield( this );
	if ( pShield )
	{
		bDoSkill = false;
#ifdef GAME_DLL
		if ( pShield->DoSpecialAction( this ) )
		{
			bDoSkill = true;
			
		}
#endif
	}

	return bDoSkill;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFPlayer::CanGoInvisible( void )
{
	if ( HasItem() && GetItem()->GetItemID() == TF_ITEM_CAPTURE_FLAG )
	{
		HintMessage( HINT_CANNOT_CLOAK_WITH_FLAG );
		return false;
	}

	CTFGameRules *pRules = TFGameRules();

	Assert( pRules );

	if ( ( pRules->State_Get() == GR_STATE_TEAM_WIN ) && ( pRules->GetWinningTeam() != GetTeamNumber() ) )
		return false;

	if ( m_Shared.InCond( TF_COND_STUNNED ) )
		return false;

	if ( m_Shared.IsInCutScene() )
		return false;

	if ( IsInAVehicle() )
		return false;

	return true;
}

//ConVar testclassviewheight( "testclassviewheight", "0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
//Vector vecTestViewHeight(0,0,0);

//-----------------------------------------------------------------------------
// Purpose: Return class-specific standing eye height
//-----------------------------------------------------------------------------
Vector CTFPlayer::GetClassEyeHeight( void )
{
	CTFPlayerClass *pClass = GetPlayerClass();

	if ( !pClass )
		return VEC_VIEW_SCALED( this );

	/*if ( testclassviewheight.GetFloat() > 0 )
	{
	vecTestViewHeight.z = testclassviewheight.GetFloat();
	return vecTestViewHeight;
	}*/

	int iClassIndex = pClass->GetClassIndex();

	if ( iClassIndex < TF_FIRST_NORMAL_CLASS || iClassIndex > TF_CLASS_COUNT )
		return VEC_VIEW_SCALED( this );

	//float flViewHeightMultiplier = 1;
	//CALL_ATTRIB_HOOK_FLOAT(flViewHeightMultiplier, view_height_multiplier);

	return ( g_TFClassViewVectors[pClass->GetClassIndex()] * GetModelScale());
}

CTFWeaponBase *CTFPlayer::Weapon_OwnsThisID( int iWeaponID )
{
	for (int i = 0;i < WeaponCount(); i++) 
	{
		CTFWeaponBase *pWpn = ( CTFWeaponBase *)GetWeapon( i );

		if ( pWpn == NULL )
			continue;

		if ( pWpn->GetWeaponID() == iWeaponID )
		{
			return pWpn;
		}
	}

	return NULL;
}

CTFWeaponBase *CTFPlayer::Weapon_GetWeaponByType( int iType )
{
	for (int i = 0;i < WeaponCount(); i++) 
	{
		CTFWeaponBase *pWpn = ( CTFWeaponBase *)GetWeapon( i );

		if ( pWpn == NULL )
			continue;

		int iWeaponRole = pWpn->GetTFWpnData().m_iWeaponType;

		if ( iWeaponRole == iType )
		{
			return pWpn;
		}
	}

	return NULL;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFPlayer::MedicGetChargeLevel( void )
{
	if ( IsPlayerClass( TF_CLASS_MEDIC ) )
	{
		CWeaponMedigun *pMedigun = GetMedigun();

		if ( pMedigun )
			return pMedigun->GetChargeLevel();

		return 0.0f;
	}

	// Spy has a fake uber level.
	if ( IsPlayerClass( TF_CLASS_SPY ) )
	{
		return m_Shared.m_flDisguiseChargeLevel;
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFPlayer::MedicGetHealTarget( void )
{
	CWeaponMedigun *pMedigun = dynamic_cast<CWeaponMedigun *>( GetActiveTFWeapon() );
	if ( pMedigun )
		return pMedigun->GetHealTarget();

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponMedigun *CTFPlayer::GetMedigun( void )
{
	CTFWeaponBase *pWeapon = Weapon_OwnsThisID( TF_WEAPON_MEDIGUN );
	if ( pWeapon )
		return static_cast<CWeaponMedigun *>( pWeapon );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconEntity *CTFPlayer::GetEntityForLoadoutSlot( int iSlot )
{
	if ( iSlot >= LOADOUT_POSITION_HAT )
	{
		// Weapons don't get equipped in cosmetic slots.
		return GetWearableForLoadoutSlot( iSlot );
	}

	int iClass = m_PlayerClass.GetClassIndex();

	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CBaseCombatWeapon *pWeapon = GetWeapon( i );
		if ( !pWeapon )
			continue;

		CEconItemDefinition *pItemDef = pWeapon->GetItem()->GetStaticData();

		if ( pItemDef && pItemDef->GetLoadoutSlot( iClass ) == iSlot )
		{
			return pWeapon;
		}
	}

	// Wearable?
	CEconWearable *pWearable = GetWearableForLoadoutSlot( iSlot );
	if ( pWearable )
		return pWearable;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEconWearable *CTFPlayer::GetWearableForLoadoutSlot( int iSlot )
{
	int iClass = m_PlayerClass.GetClassIndex();

	for ( int i = 0; i < GetNumWearables(); i++ )
	{
		CEconWearable *pWearable = GetWearable( i );

		if ( !pWearable )
			continue;

		CEconItemDefinition *pItemDef = pWearable->GetItem()->GetStaticData();

		if ( pItemDef && pItemDef->GetLoadoutSlot( iClass ) == iSlot )
		{
			return pWearable;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFPlayer::GetMaxAmmo( int iAmmoIndex, int iClassNumber /*= -1*/ )
{
	if ( !GetPlayerClass()->GetData() )
		return 0;

	int iMaxAmmo = 0;

	if ( iClassNumber != -1 )
	{
		iMaxAmmo = GetPlayerClassData( iClassNumber )->m_aAmmoMax[iAmmoIndex];
	}
	else
	{
		iMaxAmmo = GetPlayerClass()->GetData()->m_aAmmoMax[iAmmoIndex];
	}

	// If we have a weapon that overrides max ammo, use its value.
	// BUG: If player has multiple weapons using same ammo type then only the first one's value is used.
	for ( int i = 0; i < WeaponCount(); i++ )
	{
		CTFWeaponBase *pWpn = (CTFWeaponBase *)GetWeapon( i );

		if ( !pWpn )
			continue;

		if ( pWpn->GetPrimaryAmmoType() != iAmmoIndex )
			continue;

		int iCustomMaxAmmo = pWpn->GetMaxAmmo();
		if ( iCustomMaxAmmo )
		{
			iMaxAmmo = iCustomMaxAmmo;
			break;
		}
	}

	if ( m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		iMaxAmmo *= 2;

	switch ( iAmmoIndex )
	{
	case TF_AMMO_PRIMARY:
		CALL_ATTRIB_HOOK_INT( iMaxAmmo, mult_maxammo_primary );
		break;

	case TF_AMMO_SECONDARY:
		CALL_ATTRIB_HOOK_INT( iMaxAmmo, mult_maxammo_secondary );
		break;

	case TF_AMMO_METAL:
		CALL_ATTRIB_HOOK_INT( iMaxAmmo, mult_maxammo_metal );
		break;

	case TF_AMMO_GRENADES1:
	case LFE_AMMO_GRENADES1:
		CALL_ATTRIB_HOOK_INT( iMaxAmmo, mult_maxammo_grenades1 );
		break;

	case 6:
	default:
		iMaxAmmo = 1;
		break;
	}

	return iMaxAmmo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::PlayStepSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force)
{
#ifdef CLIENT_DLL
	// Don't make predicted footstep sounds in third person, animevents will take care of that.
	if ( prediction->InPrediction() && C_BasePlayer::ShouldDrawLocalPlayer() )
		return;
#else
	IncrementStepsTaken();
#endif

	BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
}
void CTFPlayer::OnEmitFootstepSound(CSoundParameters const &params, Vector const &vecOrigin, float flVolume)
{
	int nJingleOnStep = 0;
	CALL_ATTRIB_HOOK_INT(nJingleOnStep, add_jingle_to_footsteps);
	if (nJingleOnStep > 0)
	{
		const char *szSound = NULL;
		switch (nJingleOnStep)
		{
		case 1:
			szSound = "xmas.jingle";
			break;
		case 2:
		default:
			szSound = "xmas.jingle_higher";
			break;
		}

		CPASFilter filter(vecOrigin);


		EmitSound_t parm;
		parm.m_nChannel = CHAN_BODY;
		parm.m_pSoundName = szSound;
		parm.m_flVolume = flVolume;
		parm.m_SoundLevel = params.soundlevel;
		parm.m_nFlags = SND_CHANGE_VOL;
		parm.m_nPitch = params.pitch;
		parm.m_pOrigin = &vecOrigin;

		CBaseEntity::EmitSound(filter, entindex(), parm);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CTFPlayer::GetOverrideStepSound(const char *pszBaseStepSoundName)
{

	if (TFGameRules() && TFGameRules()->IsMannVsMachineMode() && GetTeamNumber() == TF_TEAM_PVE_INVADERS && !IsMiniBoss() && !m_Shared.InCond(TF_COND_DISGUISED))	{
		return "MVM.BotStep";
	}

	Assert(pszBaseStepSoundName);

	struct override_sound_entry_t { int iOverrideIndex; const char *pszBaseSoundName; const char *pszNewSoundName; };

	enum
	{
		kFootstepSoundSet_Default = 0,
		kFootstepSoundSet_SoccerCleats = 1,
		kFootstepSoundSet_HeavyGiant = 2,
		kFootstepSoundSet_SoldierGiant = 3,
		kFootstepSoundSet_DemoGiant = 4,
		kFootstepSoundSet_ScoutGiant = 5,
		kFootstepSoundSet_PyroGiant = 6,
		kFootstepSoundSet_SentryBuster = 7,
		kFootstepSoundSet_TreasureChest = 8,
		kFootstepSoundSet_Octopus = 9,
		kFootstepSoundSet_Robot = 10,
	};

	int iOverrideFootstepSoundSet = kFootstepSoundSet_Default;
	CALL_ATTRIB_HOOK_INT(iOverrideFootstepSoundSet, override_footstep_sound_set);

	if (iOverrideFootstepSoundSet != kFootstepSoundSet_Default)
	{
		static const override_sound_entry_t s_ReplacementSounds[] =
		{
			{ kFootstepSoundSet_SoccerCleats, "Default.StepLeft", "cleats_conc.StepLeft" },
			{ kFootstepSoundSet_SoccerCleats, "Default.StepRight", "cleats_conc.StepRight" },
			{ kFootstepSoundSet_SoccerCleats, "Dirt.StepLeft", "cleats_dirt.StepLeft" },
			{ kFootstepSoundSet_SoccerCleats, "Dirt.StepRight", "cleats_dirt.StepRight" },
			{ kFootstepSoundSet_SoccerCleats, "Concrete.StepLeft", "cleats_conc.StepLeft" },
			{ kFootstepSoundSet_SoccerCleats, "Concrete.StepRight", "cleats_conc.StepRight" },

			//
			{ kFootstepSoundSet_Octopus, "Default.StepLeft", "Octopus.StepCommon" },
			{ kFootstepSoundSet_Octopus, "Default.StepRight", "Octopus.StepCommon" },
			{ kFootstepSoundSet_Octopus, "Dirt.StepLeft", "Octopus.StepCommon" },
			{ kFootstepSoundSet_Octopus, "Dirt.StepRight", "Octopus.StepCommon" },
			{ kFootstepSoundSet_Octopus, "Concrete.StepLeft", "Octopus.StepCommon" },
			{ kFootstepSoundSet_Octopus, "Concrete.StepRight", "Octopus.StepCommon" },

			//
			{ kFootstepSoundSet_HeavyGiant, "", "MVM.GiantHeavyStep" },

			//
			{ kFootstepSoundSet_SoldierGiant, "", "MVM.GiantSoldierStep" },

			//
			{ kFootstepSoundSet_DemoGiant, "", "MVM.GiantDemomanStep" },

			//
			{ kFootstepSoundSet_ScoutGiant, "", "MVM.GiantScoutStep" },

			//
			{ kFootstepSoundSet_PyroGiant, "", "MVM.GiantPyroStep" },

			//
			{ kFootstepSoundSet_SentryBuster, "", "MVM.SentryBusterStep" },

			//
			{ kFootstepSoundSet_TreasureChest, "", "Chest.Step" },

			{ kFootstepSoundSet_Robot, "", "MVM.BotStep" },
		};

		for (int i = 0; i < ARRAYSIZE(s_ReplacementSounds); i++)
		{
			if (iOverrideFootstepSoundSet == s_ReplacementSounds[i].iOverrideIndex)
			{
				if (!s_ReplacementSounds[i].pszBaseSoundName[0] ||
					!Q_stricmp(pszBaseStepSoundName, s_ReplacementSounds[i].pszBaseSoundName))
					return s_ReplacementSounds[i].pszNewSoundName;
			}

		}
	}
	// Fallback.
	return pszBaseStepSoundName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFPlayer::FindUseEntity()
{
	Vector forward, up;
	EyeVectors( &forward, NULL, &up );

	trace_t tr;
	// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
	Vector searchCenter = EyePosition();

	// NOTE: Some debris objects are useable too, so hit those as well
	// A button, etc. can be made out of clip brushes, make sure it's +use via a traceline, too.
	int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP | CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;

	UTIL_TraceLine( searchCenter, searchCenter + forward * 1024, useableContents, this, COLLISION_GROUP_NONE, &tr );
	// try the hit entity if there is one, or the ground entity if there isn't.
	CBaseEntity *pNearest = NULL;
	CBaseEntity *pObject = tr.m_pEnt;

	// TODO: Removed because we no longer have ghost animatings. We may need similar code that clips rays against transformed objects.
//#ifndef CLIENT_DLL
//	// Check for ghost animatings (these aren't hit in the normal trace because they aren't solid)
//	if ( !IsUseableEntity(pObject, 0) )
//	{
//		Ray_t rayGhostAnimating;
//		rayGhostAnimating.Init( searchCenter, searchCenter + forward * 1024 );
//
//		CBaseEntity *list[1024];
//		int nCount = UTIL_EntitiesAlongRay( list, 1024, rayGhostAnimating, 0 );
//
//		// Loop through all entities along the pick up ray
//		for ( int i = 0; i < nCount; i++ )
//		{
//			CGhostAnimating *pGhostAnimating = dynamic_cast<CGhostAnimating*>( list[i] );
//
//			// If the entity is a ghost animating
//			if( pGhostAnimating )
//			{
//				trace_t trGhostAnimating;
//				enginetrace->ClipRayToEntity( rayGhostAnimating, MASK_ALL, pGhostAnimating, &trGhostAnimating );
//
//				if ( trGhostAnimating.fraction < tr.fraction )
//				{
//					// If we're not grabbing the clipped ghost
//					VPlane plane = pGhostAnimating->GetLocalClipPlane();
//					UTIL_Portal_PlaneTransform( pGhostAnimating->GetCloneTransform(), plane, plane );
//					if ( plane.GetPointSide( trGhostAnimating.endpos ) != SIDE_FRONT )
//					{
//						tr = trGhostAnimating;
//						pObject = tr.m_pEnt;
//					}
//				}
//			}
//		}
//	}
//#endif

	int count = 0;
	// UNDONE: Might be faster to just fold this range into the sphere query
	const int NUM_TANGENTS = 7;
	while ( !IsUseableEntity(pObject, 0) && count < NUM_TANGENTS)
	{
		// trace a box at successive angles down
		//							45 deg, 30 deg, 20 deg, 15 deg, 10 deg, -10, -15
		const float tangents[NUM_TANGENTS] = { 1, 0.57735026919f, 0.3639702342f, 0.267949192431f, 0.1763269807f, -0.1763269807f, -0.267949192431f };
		Vector down = forward - tangents[count]*up;
		VectorNormalize(down);
		UTIL_TraceHull( searchCenter, searchCenter + down * 72, -Vector(16,16,16), Vector(16,16,16), useableContents, this, COLLISION_GROUP_NONE, &tr );
		pObject = tr.m_pEnt;
		count++;
	}
	float nearestDot = CONE_90_DEGREES;
	if ( IsUseableEntity(pObject, 0) )
	{
		Vector delta = tr.endpos - tr.startpos;
		float centerZ = CollisionProp()->WorldSpaceCenter().z;
		delta.z = IntervalDistance( tr.endpos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
		float dist = delta.Length();
		if ( dist < PLAYER_USE_RADIUS )
		{
#ifndef CLIENT_DLL

			if ( sv_debug_player_use.GetBool() )
			{
				NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
				NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
			}

			if ( pObject->MyNPCPointer() && pObject->MyNPCPointer()->IsPlayerAlly( this ) )
			{
				// If about to select an NPC, do a more thorough check to ensure
				// that we're selecting the right one from a group.
				pObject = DoubleCheckUseNPC( pObject, searchCenter, forward );
			}

			g_PortalGameStats.Event_PlayerUsed( searchCenter, forward, pObject );
#endif

			return pObject;
		}
	}

#ifndef CLIENT_DLL
	CBaseEntity *pFoundByTrace = pObject;
#endif

	// check ground entity first
	// if you've got a useable ground entity, then shrink the cone of this search to 45 degrees
	// otherwise, search out in a 90 degree cone (hemisphere)
	if ( GetGroundEntity() && IsUseableEntity(GetGroundEntity(), FCAP_USE_ONGROUND) )
	{
		pNearest = GetGroundEntity();
		nearestDot = CONE_45_DEGREES;
	}

	for ( CEntitySphereQuery sphere( searchCenter, PLAYER_USE_RADIUS ); ( pObject = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pObject )
			continue;

		if ( !IsUseableEntity( pObject, FCAP_USE_IN_RADIUS ) )
			continue;

		// see if it's more roughly in front of the player than previous guess
		Vector point;
		pObject->CollisionProp()->CalcNearestPoint( searchCenter, &point );

		Vector dir = point - searchCenter;
		VectorNormalize(dir);
		float dot = DotProduct( dir, forward );

		// Need to be looking at the object more or less
		if ( dot < 0.8 )
			continue;

		if ( dot > nearestDot )
		{
			// Since this has purely been a radius search to this point, we now
			// make sure the object isn't behind glass or a grate.
			trace_t trCheckOccluded;
			UTIL_TraceLine( searchCenter, point, useableContents, this, COLLISION_GROUP_NONE, &trCheckOccluded );

			if ( trCheckOccluded.fraction == 1.0 || trCheckOccluded.m_pEnt == pObject )
			{
				pNearest = pObject;
				nearestDot = dot;
			}
		}
	}

#ifndef CLIENT_DLL
	if ( !pNearest )
	{
		// Haven't found anything near the player to use, nor any NPC's at distance.
		// Check to see if the player is trying to select an NPC through a rail, fence, or other 'see-though' volume.
		trace_t trAllies;
		UTIL_TraceLine( searchCenter, searchCenter + forward * PLAYER_USE_RADIUS, MASK_OPAQUE_AND_NPCS, this, COLLISION_GROUP_NONE, &trAllies );

		if ( trAllies.m_pEnt && IsUseableEntity( trAllies.m_pEnt, 0 ) && trAllies.m_pEnt->MyNPCPointer() && trAllies.m_pEnt->MyNPCPointer()->IsPlayerAlly( this ) )
		{
			// This is an NPC, take it!
			pNearest = trAllies.m_pEnt;
		}
	}

	if ( pNearest && pNearest->MyNPCPointer() && pNearest->MyNPCPointer()->IsPlayerAlly( this ) )
	{
		pNearest = DoubleCheckUseNPC( pNearest, searchCenter, forward );
	}

	if ( sv_debug_player_use.GetBool() )
	{
		if ( !pNearest )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 255, 0, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 255, 0, 0, true, 30 );
		}
		else if ( pNearest == pFoundByTrace )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
		}
		else
		{
			NDebugOverlay::Box( pNearest->WorldSpaceCenter(), Vector(-8, -8, -8), Vector(8, 8, 8), 0, 255, 0, true, 30 );
		}
	}

	g_PortalGameStats.Event_PlayerUsed( searchCenter, forward, pNearest );
#endif

	return pNearest;
}

CBaseEntity* CTFPlayer::FindUseEntityThroughPortal( void )
{
	Vector forward, up;
	EyeVectors( &forward, NULL, &up );

	CProp_Portal *pPortal = GetHeldObjectPortal();

	trace_t tr;
	// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
	Vector searchCenter = EyePosition();

	Vector vTransformedForward, vTransformedUp, vTransformedSearchCenter;

	VMatrix matThisToLinked = pPortal->MatrixThisToLinked();
	UTIL_Portal_PointTransform( matThisToLinked, searchCenter, vTransformedSearchCenter );
	UTIL_Portal_VectorTransform( matThisToLinked, forward, vTransformedForward );
	UTIL_Portal_VectorTransform( matThisToLinked, up, vTransformedUp );


	// NOTE: Some debris objects are useable too, so hit those as well
	// A button, etc. can be made out of clip brushes, make sure it's +useable via a traceline, too.
	int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP | CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;

	//UTIL_TraceLine( vTransformedSearchCenter, vTransformedSearchCenter + vTransformedForward * 1024, useableContents, this, COLLISION_GROUP_NONE, &tr );
	Ray_t rayLinked;
	rayLinked.Init( searchCenter, searchCenter + forward * 1024 );
	UTIL_PortalLinked_TraceRay( pPortal, rayLinked, useableContents, this, COLLISION_GROUP_NONE, &tr );

	// try the hit entity if there is one, or the ground entity if there isn't.
	CBaseEntity *pNearest = NULL;
	CBaseEntity *pObject = tr.m_pEnt;
	int count = 0;
	// UNDONE: Might be faster to just fold this range into the sphere query
	const int NUM_TANGENTS = 7;
	while ( !IsUseableEntity(pObject, 0) && count < NUM_TANGENTS)
	{
		// trace a box at successive angles down
		//							45 deg, 30 deg, 20 deg, 15 deg, 10 deg, -10, -15
		const float tangents[NUM_TANGENTS] = { 1, 0.57735026919f, 0.3639702342f, 0.267949192431f, 0.1763269807f, -0.1763269807f, -0.267949192431f };
		Vector down = vTransformedForward - tangents[count]*vTransformedUp;
		VectorNormalize(down);
		UTIL_TraceHull( vTransformedSearchCenter, vTransformedSearchCenter + down * 72, -Vector(16,16,16), Vector(16,16,16), useableContents, this, COLLISION_GROUP_NONE, &tr );
		pObject = tr.m_pEnt;
		count++;
	}
	float nearestDot = CONE_90_DEGREES;
	if ( IsUseableEntity(pObject, 0) )
	{
		Vector delta = tr.endpos - tr.startpos;
		float centerZ = CollisionProp()->WorldSpaceCenter().z;
		delta.z = IntervalDistance( tr.endpos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
		float dist = delta.Length();
		if ( dist < PLAYER_USE_RADIUS )
		{
#ifndef CLIENT_DLL

			if ( pObject->MyNPCPointer() && pObject->MyNPCPointer()->IsPlayerAlly( this ) )
			{
				// If about to select an NPC, do a more thorough check to ensure
				// that we're selecting the right one from a group.
				pObject = DoubleCheckUseNPC( pObject, vTransformedSearchCenter, vTransformedForward );
			}
#endif

			return pObject;
		}
	}

	// check ground entity first
	// if you've got a useable ground entity, then shrink the cone of this search to 45 degrees
	// otherwise, search out in a 90 degree cone (hemisphere)
	if ( GetGroundEntity() && IsUseableEntity(GetGroundEntity(), FCAP_USE_ONGROUND) )
	{
		pNearest = GetGroundEntity();
		nearestDot = CONE_45_DEGREES;
	}

	for ( CEntitySphereQuery sphere( vTransformedSearchCenter, PLAYER_USE_RADIUS ); ( pObject = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pObject )
			continue;

		if ( !IsUseableEntity( pObject, FCAP_USE_IN_RADIUS ) )
			continue;

		// see if it's more roughly in front of the player than previous guess
		Vector point;
		pObject->CollisionProp()->CalcNearestPoint( vTransformedSearchCenter, &point );

		Vector dir = point - vTransformedSearchCenter;
		VectorNormalize(dir);
		float dot = DotProduct( dir, vTransformedForward );

		// Need to be looking at the object more or less
		if ( dot < 0.8 )
			continue;

		if ( dot > nearestDot )
		{
			// Since this has purely been a radius search to this point, we now
			// make sure the object isn't behind glass or a grate.
			trace_t trCheckOccluded;
			UTIL_TraceLine( vTransformedSearchCenter, point, useableContents, this, COLLISION_GROUP_NONE, &trCheckOccluded );

			if ( trCheckOccluded.fraction == 1.0 || trCheckOccluded.m_pEnt == pObject )
			{
				pNearest = pObject;
				nearestDot = dot;
			}
		}
	}

#ifndef CLIENT_DLL
	if ( !pNearest )
	{
		// Haven't found anything near the player to use, nor any NPC's at distance.
		// Check to see if the player is trying to select an NPC through a rail, fence, or other 'see-though' volume.
		trace_t trAllies;
		UTIL_TraceLine( vTransformedSearchCenter, vTransformedSearchCenter + vTransformedForward * PLAYER_USE_RADIUS, MASK_OPAQUE_AND_NPCS, this, COLLISION_GROUP_NONE, &trAllies );

		if ( trAllies.m_pEnt && IsUseableEntity( trAllies.m_pEnt, 0 ) && trAllies.m_pEnt->MyNPCPointer() && trAllies.m_pEnt->MyNPCPointer()->IsPlayerAlly( this ) )
		{
			// This is an NPC, take it!
			pNearest = trAllies.m_pEnt;
		}
	}

	if ( pNearest && pNearest->MyNPCPointer() && pNearest->MyNPCPointer()->IsPlayerAlly( this ) )
	{
		pNearest = DoubleCheckUseNPC( pNearest, vTransformedSearchCenter, vTransformedForward );
	}

#endif

	return pNearest;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanAirDash( void ) const
{
	int iNoDoubleJump = 0;
	CALL_ATTRIB_HOOK_INT( iNoDoubleJump, set_scout_doublejump_disabled );
	if ( iNoDoubleJump != 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanDuck( void ) const
{
	int iNoDuck = 0;
	CALL_ATTRIB_HOOK_INT( iNoDuck, no_duck );
	if ( iNoDuck != 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanJump( void ) const
{
	int iNoJump = 0;
	CALL_ATTRIB_HOOK_INT( iNoJump, no_jump );
	if ( iNoJump != 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanMoveDuringTaunt( void )
{
	return m_bAllowMoveDuringTaunt;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::IsInspecting( void ) const
{
	if ( m_nButtons & IN_INSPECT )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::ParseSharedTauntDataFromEconItemView( const CEconItemView* pItem )
{
	/*pAttrDef_TauntForceMoveForward
	pAttrDef_TauntMoveAccelerationTime
	pAttrDef_TauntTurnAccelerationTime
	pAttrDef_TauntTurnSpeed*/

	if ( m_iTauntItemDefIndex == 0 )
		return;

	CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_iTauntItemDefIndex );
	if ( !pItemDef )
		return;

	float flTauntMoveSpeed = 0.0f;
	static CSchemaFieldHandle<CEconAttributeDefinition> pAttrDef_TauntMoveSpeed( "taunt move speed" );
	if ( pAttrDef_TauntMoveSpeed )
	{
		CAttributeIterator_GetSpecificAttribute<float> func( pAttrDef_TauntMoveSpeed, &flTauntMoveSpeed );
		pItemDef->IterateAttributes( &func );
	}

	m_flCurrentTauntMoveSpeed = flTauntMoveSpeed;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanPickupDroppedWeapon( const CTFDroppedWeapon *pWeapon )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayer::CanPlayerMove( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayer::SharedSpawn()
{
	BaseClass::SharedSpawn();

	// bots don't have blood
	/// HACK HACK: why is this being ignore in both sides?
	if ( TFGameRules() && TFGameRules()->IsMvMModelsAllowed() )
		SetBloodColor( BLOOD_COLOR_MECH );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetPasstimePassTarget( CBaseEntity *pTarget )
{
	m_hPasstimePassTarget = pTarget;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayer::SetGrapplingHookTarget( CBaseEntity *pTarget, bool bSomething )
{
	if ( pTarget )
	{
		m_Shared.AddCond( TF_COND_GRAPPLINGHOOK );

		if ( tf_grapplinghook_prevent_fall_damage.GetBool() )
			m_Shared.AddCond( TF_COND_GRAPPLINGHOOK_SAFEFALL );

		if ( pTarget->IsPlayer() || pTarget->IsNPC() )
			m_Shared.AddCond( TF_COND_GRAPPLED_TO_PLAYER );

		DoAnimationEvent( PLAYERANIMEVENT_CUSTOM, ACT_GRAPPLE_PULL_START );
	}
	else
	{
		m_Shared.RemoveCond( TF_COND_GRAPPLINGHOOK );
		m_Shared.RemoveCond( TF_COND_GRAPPLED_TO_PLAYER );
	}

	m_hGrapplingHookTarget = pTarget;
}


#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPlayerShared::SetCarryingRuneType( RuneTypes_t RuneType )
{
	switch ( RuneType )
	{
	case TF_RUNE_NONE:
		break;
	case TF_RUNE_STRENGTH:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_STRENGTH );
		m_pOuter->EmitSound( "Powerup.PickUpStrength" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Strength" );
		break;
	case TF_RUNE_HASTE:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_HASTE );
		m_pOuter->EmitSound( "Powerup.PickUpHaste" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Haste" );
		break;
	case TF_RUNE_REGEN:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_REGEN );
		m_pOuter->EmitSound( "Powerup.PickUpRegeneration" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Regen" );
		break;
	case TF_RUNE_RESIST:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_RESIST );
		m_pOuter->EmitSound( "Powerup.PickUpResistance" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Resist" );
		break;
	case TF_RUNE_VAMPIRE:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_VAMPIRE );
		m_pOuter->EmitSound( "Powerup.PickUpVampire" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Vampire" );
		break;
	case TF_RUNE_WARLOCK:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_WARLOCK );
		m_pOuter->EmitSound( "Powerup.PickUpReflect" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Reflect" );
		break;
	case TF_RUNE_PRECISION:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_PRECISION );
		m_pOuter->EmitSound( "Powerup.PickUpPrecision" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Precision" );
		break;
	case TF_RUNE_AGILITY:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_AGILITY );
		m_pOuter->EmitSound( "Powerup.PickUpAgility" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Agility" );
		break;
	case TF_RUNE_KNOCKOUT:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_KNOCKOUT );
		m_pOuter->EmitSound( "Powerup.PickUpKnockout" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Knockout" );
		break;
	case TF_RUNE_KING:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_KING );
		m_pOuter->EmitSound( "Powerup.PickUpKing" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_King" );
		break;
	case TF_RUNE_PLAGUE:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_PLAGUE );
		m_pOuter->EmitSound( "Powerup.PickUpPlague" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Plague" );
		break;
	case TF_RUNE_SUPERNOVA:
		m_pOuter->m_Shared.AddCond( TF_COND_RUNE_SUPERNOVA );
		m_pOuter->EmitSound( "Powerup.PickUpSupernova" );
		ClientPrint( m_pOuter, HUD_PRINTCENTER, "#TF_Powerup_Pickup_Supernova" );
		break;

	default:
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
RuneTypes_t CTFPlayerShared::GetCarryingRuneType( void ) const
{
	if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) )
		return TF_RUNE_STRENGTH;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		return TF_RUNE_HASTE;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_REGEN ) )
		return TF_RUNE_REGEN;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_RESIST ) )
		return TF_RUNE_RESIST;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_VAMPIRE ) )
		return TF_RUNE_VAMPIRE;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_WARLOCK ) )
		return TF_RUNE_WARLOCK;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_PRECISION ) )
		return TF_RUNE_PRECISION;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_AGILITY ) )
		return TF_RUNE_AGILITY;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_KNOCKOUT ) )
		return TF_RUNE_KNOCKOUT;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_KING ) )
		return TF_RUNE_KING;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_PLAGUE ) )
		return TF_RUNE_PLAGUE;
	else if ( m_pOuter->m_Shared.InCond( TF_COND_RUNE_SUPERNOVA ) )
		return TF_RUNE_SUPERNOVA;

	return TF_RUNE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::CanRuneCharge( void ) const
{
	return ( GetCarryingRuneType() == TF_RUNE_SUPERNOVA );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFPlayerShared::CanFallStomp( void )
{
	int nFallingStomp = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER( m_pOuter, nFallingStomp, boots_falling_stomp );
	if ( nFallingStomp != 0 )
		return true;

	if ( m_pOuter->m_Shared.InCond( TF_COND_ROCKETPACK ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::CreatePingEffect( int nType )
{
#ifdef GAME_DLL
	trace_t tr;
	Vector forward;
	m_pOuter->EyeVectors( &forward );
	Ray_t ray; ray.Init( m_pOuter->EyePosition(), m_pOuter->EyePosition() + forward * MAX_COORD_RANGE );
	UTIL_Portal_TraceRay( ray, MASK_SOLID, m_pOuter, COLLISION_GROUP_NONE, &tr );

	m_vecPingOrigin = tr.endpos;
	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		m_hPingTarget = tr.m_pEnt;
		m_vecPingOrigin = m_hPingTarget->GetAbsOrigin();
	}

	if ( tr.surface.flags & SURF_SKY )
		return;

	if ( ( nType == TF_PING_GO || nType == TF_PING_HELP ) && m_pOuter->m_iSquadMemberCount != 0 )
	{
		CEffectData	data;
		const char *pszEffect = "ping_circle";
		data.m_nHitBox = GetParticleSystemIndex( pszEffect );

		if ( nType == TF_PING_HELP )
			data.m_vOrigin = m_pOuter->GetAbsOrigin();
		else
			data.m_vOrigin = m_vecPingOrigin;

		data.m_vAngles = m_pOuter->EyeAngles();
		data.m_nEntIndex = 0;
		CSingleUserRecipientFilter filter( m_pOuter );

		te->DispatchEffect( filter, 0.0, data.m_vOrigin += Vector(0,0,1), "ParticleEffect", data );

		/*EmitSound_t params;
		params.m_flSoundTime = 0;
		params.m_pSoundName = "";
		EmitSound( filter, entindex(), params );*/
	}
#else
	// Don't create them for the local player
	//if ( !m_pOuter->ShouldDrawThisPlayer() )
	//	return;

	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();

	// Only show the bubble to teammates and players who are on the our disguise team.
	if ( !m_pOuter->InSameTeam( pLocalPlayer ) && !( InCond( TF_COND_DISGUISED ) && GetDisguiseTeam() == pLocalPlayer->GetTeamNumber() ) )
		return;

	if ( pLocalPlayer && pLocalPlayer->IsAlive() )
	{
		int iNumPlayers = GetGlobalTFTeam( m_pOuter->GetTeamNumber() )->GetNumPlayers();
		if ( iNumPlayers < 10 )
		{
			Vector vecEnd = m_vecPingOrigin;
			vecEnd += Vector(0,0,4);
			CTFPingSystemPanel::AddPing( nType, vecEnd, 2.0, m_hPingTarget );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::FireGameEvent( IGameEvent *event )
{
	const char *eventName = event->GetName();

	if ( !Q_strcmp( eventName, "post_inventory_application" ) )
	{
		if ( m_pOuter->IsAlive() )
		{
			bool bInRespawnRoom = false;
#ifdef GAME_DLL
			bInRespawnRoom = PointInRespawnRoom( m_pOuter, m_pOuter->WorldSpaceCenter() );
#endif
			if ( bInRespawnRoom )
			{
#ifdef CLIENT_DLL
				if ( !tf_respawn_on_loadoutchanges.GetBool() )
					return;
#else
				m_pOuter->ForceRespawn();
#endif
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFPlayerShared::ForceRespawn( void )
{
#ifdef GAME_DLL
	m_pOuter->ForceRespawn();
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Add an attribute
//-----------------------------------------------------------------------------
void CTFPlayerShared::ApplyAttributeToPlayer( char const *pAttribute, float flValue )
{
	m_pOuter->AddCustomAttribute( pAttribute, flValue );
}

//-----------------------------------------------------------------------------
// Purpose: Remove an attribute
//-----------------------------------------------------------------------------
void CTFPlayerShared::RemoveAttributeFromPlayer( char const *pAttribute )
{
	m_pOuter->RemoveCustomAttribute( pAttribute );
}
#endif