//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "c_tf_player.h"
#include "c_user_message_register.h"
#include "view.h"
#include "iclientvehicle.h"
#include "ivieweffects.h"
#include "input.h"
#include "IEffects.h"
#include "fx.h"
#include "c_basetempentity.h"
#include "hud_macros.h"
#include "engine/ivdebugoverlay.h"
#include "smoke_fog_overlay.h"
#include "playerandobjectenumerator.h"
#include "bone_setup.h"
#include "in_buttons.h"
#include "r_efx.h"
#include "dlight.h"
#include "shake.h"
#include "cl_animevent.h"
#include "tf_weaponbase.h"
#include "c_tf_playerresource.h"
#include "toolframework/itoolframework.h"
#include "tier1/KeyValues.h"
#include "tier0/vprof.h"
#include "prediction.h"
#include "effect_dispatch_data.h"
#include "c_te_effect_dispatch.h"
#include "tf_fx_muzzleflash.h"
#include "tf_gamerules.h"
#include "view_scene.h"
#include "c_baseobject.h"
#include "toolframework_client.h"
#include "soundenvelope.h"
#include "voice_status.h"
#include "clienteffectprecachesystem.h"
#include "functionproxy.h"
#include "toolframework_client.h"
#include "choreoevent.h"
#include "c_entitydissolve.h"
#include "vguicenterprint.h"
#include "eventlist.h"
#include "tf_hud_statpanel.h"
#include "input.h"
#include "tf_weapon_medigun.h"
#include "tf_weapon_pipebomblauncher.h"
#include "tf_hud_mediccallers.h"
#include "in_main.h"
#include "basemodelpanel.h"
#include "c_team.h"
#include "collisionutils.h"
// for spy material proxy
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "c_tf_team.h"
#include "tf_viewmodel.h"
#include "c_tf_viewmodeladdon.h"
#include "tf_inventory.h"
#include "flashlighteffect.h"
#include "ScreenSpaceEffects.h"
#include "tf_wearable_demoshield.h"
#if defined( CTFPlayer )
#undef CTFPlayer
#endif

#include "materialsystem/imesh.h"		//for materials->FindMaterial
#include "iviewrender.h"				//for view->

#include "cam_thirdperson.h"
#include "tf_hud_chat.h"
#include "iclientmode.h"

#include "vcollide_parse.h"
#include "takedamageinfo.h"
#include "iviewrender_beams.h"
#include "PortalRender.h"
#include "prop_portal_shared.h"

#include "tf_wearable.h"
#include "achievements_tf.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar tf_playergib_forceup( "tf_playersgib_forceup", "1.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Upward added velocity for gibs." );
ConVar tf_playergib_force( "tf_playersgib_force", "500.0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Gibs force." );
ConVar tf_playergib_maxspeed( "tf_playergib_maxspeed", "400", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Max gib speed." );

ConVar cl_autorezoom( "cl_autorezoom", "1", FCVAR_USERINFO | FCVAR_ARCHIVE, "When set to 1, sniper rifle will re-zoom after firing a zoomed shot." );
ConVar cl_autoreload( "cl_autoreload", "0",  FCVAR_USERINFO | FCVAR_ARCHIVE, "When set to 1, clip-using weapons will automatically be reloaded whenever they're not being fired." );

ConVar cl_thirdperson_offset( "cl_thirdperson_offset", "0 0 0",  FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_CHEAT, "Thirdperson Camera offset." );

ConVar lfe_muzzlelight( "lfe_muzzlelight", "1", FCVAR_ARCHIVE, "Enable dynamic lights for muzzleflashes and the flamethrower" );
ConVar lfe_enable_team_glow( "lfe_enable_team_glow", "0",  FCVAR_ARCHIVE, "Enable outline effect on teammate." );

ConVar tf_taunt_first_person ( "tf_taunt_first_person", "0",  FCVAR_ARCHIVE, "1 = taunts remain first-person" );

ConVar lfe_dev_mark( "lfe_dev_mark", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );

ConVar cl_npc_speedmod_intime( "cl_npc_speedmod_intime", "0.25", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar cl_npc_speedmod_outtime( "cl_npc_speedmod_outtime", "1.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

ConVar cl_pyrovision( "cl_pyrovision", "0", FCVAR_USERINFO | FCVAR_ARCHIVE, "Toggle pyrovision for pyro" );
ConVar cl_romevision( "cl_romevision", "0", FCVAR_DEVELOPMENTONLY );

ConVar tf_respawn_on_loadoutchanges( "tf_respawn_on_loadoutchanges", "1", FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY, "When set to 1, you will automatically respawn whenever you change loadouts inside a respawn zone." );

ConVar tf_particles_disable_weather( "tf_particles_disable_weather", "0",  FCVAR_ARCHIVE, "Disable particles related to weather effects." );

/*static void AddResistParticle( C_TFPlayer *pPlayer, medigun_resist_types_t resist_types_t, ETFCond eCond )
{
}

static void AddResistParticle( C_TFPlayer *pPlayer, medigun_resist_types_t resist_types_t )
{
}

static void AddResistShield( C_LocalTempEntity *pTempEntity, C_TFPlayer *pPlayer, ETFCond eCond )
{
	model_t *pModel = (model_t*)engine->LoadModel( TF_RESIST_SHIELD_MODEL );
	pTempEntity = tempents->SpawnTempModel( pModel, pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles(), Vector( 0, 0, 0 ), 1, FTENT_NEVERDIE );

	switch ( pPlayer->GetTeamNumber() )
	{
		case TF_TEAM_RED:
			pTempEntity->m_nSkin = 0;
			break;
		case TF_TEAM_BLUE:
			pTempEntity->m_nSkin = 1;
			break;
	}

	switch ( eCond )
	{
		case TF_COND_MEDIGUN_UBER_BULLET_RESIST:
		case TF_COND_MEDIGUN_SMALL_BULLET_RESIST:
			break;

		case TF_COND_MEDIGUN_UBER_BLAST_RESIST:
		case TF_COND_MEDIGUN_SMALL_BLAST_RESIST:
			break;

		case TF_COND_MEDIGUN_UBER_FIRE_RESIST:
		case TF_COND_MEDIGUN_SMALL_FIRE_RESIST:
			break;
	}

	pTempEntity->SetParent( pPlayer );
}

static void RemoveResistShield( C_LocalTempEntity *pTempEntity, C_TFPlayer *pPlayer )
{
	pTempEntity->SetParent( NULL );
	pTempEntity->flags = FTENT_FADEOUT;
	pTempEntity->die = gpGlobals->curtime;
	pTempEntity->fadeSpeed = 0.1f;
	pTempEntity = NULL;
}*/

static void BuildDecapitatedTransform( C_BaseAnimating *pAnimating )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "bip_head" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "prp_helmet" );
		if ( iBone != -1 )
			 MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "prp_hat" );
		if ( iBone != -1 )
			 MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_neck" );
		if ( iBone != -1 )
			 MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );
	}
}

static void BuildNeckScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "bip_head" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "prp_helmet" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "prp_hat" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_neck" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );
	}
}

static void BuildTorsoScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		/*int iBone = pAnimating->LookupBone( "bip_spine_0" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_spine_1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/

		/*iBone = pAnimating->LookupBone( "bip_spine_2" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_spine_3" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/
	}
}

static void BuildHandScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "bip_hand_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_0_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_0_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_0_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_0_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_0_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_1_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_1_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_1_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_1_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_1_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );



		iBone = pAnimating->LookupBone( "bip_hand_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_0_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_0_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_0_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_0_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_0_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_1_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_1_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_1_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_1_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_1_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_thumb_2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_index_2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_middle_2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_ring_2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "bip_pinky_2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );
	}
}

#define BDAY_HAT_MODEL		"models/effects/bday_hat.mdl"

IMaterial	*g_pHeadLabelMaterial[4] = { NULL, NULL }; 
void	SetupHeadLabelMaterials( void );

extern CBaseEntity *BreakModelCreateSingle( CBaseEntity *pOwner, breakmodel_t *pModel, const Vector &position, 
										   const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, int nSkin, const breakablepropparams_t &params );

const char *pszHeadLabelNames[] =
{
	"effects/speech_voice_red",
	"effects/speech_voice_blue",
	"effects/speech_voice_red",
	"effects/speech_voice_red"
};

const char *g_pszHeadGibs[] = 
{
	"",
	"models/player/gibs/scoutgib007.mdl",
	"models/player/gibs/snipergib005.mdl",
	"models/player/gibs/soldiergib007.mdl",
	"models/player/gibs/demogib006.mdl",
	"models/player/gibs/medicgib007.mdl",
	"models/player/gibs/heavygib007.mdl",
	"models/player/gibs/pyrogib008.mdl",
	"models/player/gibs/spygib007.mdl",
	"models/player/gibs/engineergib006.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
};

const char *g_pszBotHeadGibs[] = 
{
	"",
	"models/bots/gibs/scoutbot_gib_head.mdl",
	"models/bots/gibs/sniperbot_gib_head.mdl",
	"models/bots/gibs/soldierbot_gib_head.mdl",
	"models/bots/gibs/demobot_gib_head.mdl",
	"models/bots/gibs/medicbot_gib_head.mdl",
	"models/bots/gibs/heavybot_gib_head.mdl",
	"models/bots/gibs/pyrobot_gib_head.mdl",
	"models/bots/gibs/spybot_gib_head.mdl",
	"models/bots/gibs/medicbot_gib_head.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs.mdl",
};

#define TF_PLAYER_HEAD_LABEL_RED 0
#define TF_PLAYER_HEAD_LABEL_BLUE 1

#define TF_FLASHLIGHT_DISTANCE		1000

#define REORIENTATION_RATE 120.0f
#define REORIENTATION_ACCELERATION_RATE 400.0f

#define ENABLE_PORTAL_EYE_INTERPOLATION_CODE

#define DEATH_CC_LOOKUP_FILENAME "materials/correction/cc_death.raw"
#define DEATH_CC_FADE_SPEED 0.05f

ConVar cl_reorient_in_air("cl_reorient_in_air", "1", FCVAR_ARCHIVE, "Allows the player to only reorient from being upside down while in the air." ); 

CLIENTEFFECT_REGISTER_BEGIN( PrecacheInvuln )
CLIENTEFFECT_MATERIAL( "models/effects/invulnfx_blue.vmt" )
CLIENTEFFECT_MATERIAL( "models/effects/invulnfx_red.vmt" )
CLIENTEFFECT_REGISTER_END()

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		VPROF( "C_TEPlayerAnimEvent::PostDataUpdate" );

		// Create the effect.
		if ( m_iPlayerIndex == TF_PLAYER_INDEX_NONE )
			return;

		EHANDLE hPlayer = cl_entitylist->GetNetworkableHandle( m_iPlayerIndex );
		if ( !hPlayer )
			return;

		C_TFPlayer *pPlayer = dynamic_cast< C_TFPlayer* >( hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkVar( int, m_iPlayerIndex );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

//-----------------------------------------------------------------------------
// Data tables and prediction tables.
//-----------------------------------------------------------------------------
BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropInt( RECVINFO( m_iPlayerIndex ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()


//=============================================================================
//
// Ragdoll
//
// ----------------------------------------------------------------------------- //
// Client ragdoll entity.
// ----------------------------------------------------------------------------- //
ConVar cl_ragdoll_physics_enable( "cl_ragdoll_physics_enable", "1", 0, "Enable/disable ragdoll physics." );
ConVar cl_ragdoll_fade_time( "cl_ragdoll_fade_time", "15", FCVAR_CLIENTDLL );
ConVar cl_ragdoll_forcefade( "cl_ragdoll_forcefade", "0", FCVAR_CLIENTDLL );
ConVar cl_ragdoll_pronecheck_distance( "cl_ragdoll_pronecheck_distance", "64", FCVAR_GAMEDLL );

ConVar tf_always_deathanim( "tf_always_deathanim", "0", FCVAR_CHEAT, "Force death anims to always play." );

extern C_EntityDissolve *DissolveEffect( C_BaseEntity *pTarget, float flTime );
class C_TFRagdoll : public C_BaseFlex
{
public:

	DECLARE_CLASS( C_TFRagdoll, C_BaseFlex );
	DECLARE_CLIENTCLASS();

	C_TFRagdoll();
	~C_TFRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	virtual int InternalDrawModel( int flags );

	IRagdoll *GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );

	void ClientThink( void );
	void StartFadeOut( float fDelay );
	void EndFadeOut();

	EHANDLE GetPlayerHandle( void )
	{
		if ( m_iPlayerIndex == TF_PLAYER_INDEX_NONE )
			return NULL;
		return cl_entitylist->GetNetworkableHandle( m_iPlayerIndex );
	}

	bool IsRagdollVisible();
	bool IsDecapitation();
	float GetBurnStartTime() { return m_flBurnEffectStartTime; }

	void DissolveEntity( C_BaseEntity *pEntity );

	virtual void SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	virtual float FrameAdvance( float flInterval = 0.0f );

	virtual void BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed );
	virtual bool GetAttachment( int number, matrix3x4_t &matrix );

	float GetInvisibilityLevel( void )
	{
		if ( m_bCloaked )
			return m_flInvisibilityLevel;

		if ( m_flUncloakCompleteTime == 0.0f )
			return 0.0f;

		return RemapValClamped( m_flUncloakCompleteTime - gpGlobals->curtime, 0.0f, 2.0f, 0.0f, 1.0f );
	}

private:

	C_TFRagdoll( const C_TFRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity );

	void CreateTFRagdoll( void );
	void CreateTFGibs( bool bKill, bool bLocalOrigin );
	void CreateTFHeadGib( void );
	//void CreateWearableGibs( void );
private:

	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
	int	  m_iPlayerIndex;
	float m_fDeathTime;
	bool  m_bGib;
	bool  m_bBurning;
	bool  m_bElectrocuted;
	bool  m_bSlamRagdoll;
	bool  m_bDissolve;
	bool  m_bFeignDeath;
	bool  m_bWasDisguised;
	bool  m_bBecomeAsh;
	bool  m_bOnGround;
	bool  m_bCloaked;
	float m_flInvisibilityLevel;
	float m_flUncloakCompleteTime;
	int   m_iDamageCustom;
	int	  m_iTeam;
	int	  m_iClass;
	bool  m_bStartedDying;
	bool  m_bGoldRagdoll;
	bool  m_bIceRagdoll;
	bool  m_bCritOnHardHit;
	float m_flBurnEffectStartTime;	// start time of burning, or 0 if not burning
	float m_flDeathDelay;
	bool  m_bFixedConstraints;
	bool  m_bFadingOut;
	bool  m_bPlayDeathAnim;
	CountdownTimer m_timer1;
	CountdownTimer m_timer2;

	// Decapitation
	matrix3x4_t m_Head;
	bool m_bHeadTransform;

	CMaterialReference m_MatOverride;

	CNetworkArray( CHandle<C_EconEntity>, m_hRagWearables, LOADOUT_POSITION_COUNT );
};

IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_TFRagdoll, DT_TFRagdoll, CTFRagdoll )
	RecvPropVector( RECVINFO( m_vecRagdollOrigin ) ),
	RecvPropInt( RECVINFO( m_iPlayerIndex ) ),
	RecvPropVector( RECVINFO( m_vecForce ) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),
	RecvPropInt( RECVINFO( m_nForceBone ) ),
	RecvPropBool( RECVINFO( m_bGib ) ),
	RecvPropBool( RECVINFO( m_bBurning ) ),
	RecvPropBool( RECVINFO( m_bElectrocuted ) ),
	RecvPropBool( RECVINFO( m_bFeignDeath ) ),
	RecvPropBool( RECVINFO( m_bWasDisguised ) ),
	RecvPropBool( RECVINFO( m_bBecomeAsh ) ),
	RecvPropBool( RECVINFO( m_bOnGround ) ),
	RecvPropBool( RECVINFO( m_bCloaked ) ),
	RecvPropInt( RECVINFO( m_iDamageCustom ) ),
	RecvPropInt( RECVINFO( m_iTeam ) ),
	RecvPropInt( RECVINFO( m_iClass ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_hRagWearables ), RecvPropEHandle( RECVINFO( m_hRagWearables[0] ) ) ),
	RecvPropBool( RECVINFO( m_bGoldRagdoll ) ),
	RecvPropBool( RECVINFO( m_bIceRagdoll ) ),
	RecvPropBool( RECVINFO( m_bCritOnHardHit ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
C_TFRagdoll::C_TFRagdoll()
{
	m_iPlayerIndex = TF_PLAYER_INDEX_NONE;
	m_fDeathTime = -1.0f;
	m_flBurnEffectStartTime = 0.0f;
	m_iDamageCustom = TF_DMG_CUSTOM_NONE;
	m_bGoldRagdoll = false;
	m_bSlamRagdoll = false;
	m_bFadingOut = false;
	m_bCloaked = false;
	m_timer1.Invalidate();
	m_timer2.Invalidate();
	m_iTeam = -1;
	m_iClass = -1;
	m_nForceBone = -1;
	m_bHeadTransform = 0;
	m_bStartedDying = 0;
	m_flDeathDelay = 0.3f;

	UseClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
C_TFRagdoll::~C_TFRagdoll()
{
	m_MatOverride.Shutdown();
	PhysCleanupFrictionSounds( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSourceEntity - 
//-----------------------------------------------------------------------------
void C_TFRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;

	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();

	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		const char *pszName = pDestEntry->watcher->GetDebugName();
		for ( int j = 0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(), pszName ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Dissolve the targetted entity
//-----------------------------------------------------------------------------
void C_TFRagdoll::DissolveEntity( C_BaseEntity *pEntity )
{
	C_EntityDissolve *pDissolver = DissolveEffect( pEntity, gpGlobals->curtime );
	if ( !pDissolver )
		return;

	if ( m_iTeam == TF_TEAM_BLUE )
		pDissolver->SetEffectColor( Vector( BitsToFloat( 0x4337999A ), BitsToFloat( 0x42606666 ), BitsToFloat( 0x426A999A ) ) );
	else
		pDissolver->SetEffectColor( Vector( BitsToFloat( 0x42AFF333 ), BitsToFloat( 0x43049999 ), BitsToFloat( 0x4321ECCD ) ) );

	pDissolver->SetOwnerEntity( NULL );
	pDissolver->SetRenderMode( kRenderTransColor );
	pDissolver->m_vDissolverOrigin = GetLocalOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: Setup vertex weights for drawing
//-----------------------------------------------------------------------------
void C_TFRagdoll::SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights )
{
	// While we're dying, we want to mimic the facial animation of the player. Once they're dead, we just stay as we are.
	EHANDLE hPlayer = GetPlayerHandle();
	if ( ( hPlayer && hPlayer->IsAlive() ) || !hPlayer )
	{
		BaseClass::SetupWeights( pBoneToWorld, nFlexWeightCount, pFlexWeights, pFlexDelayedWeights );
	}
	else if ( hPlayer )
	{
		hPlayer->SetupWeights( pBoneToWorld, nFlexWeightCount, pFlexWeights, pFlexDelayedWeights );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTrace - 
//			iDamageType - 
//			*pCustomImpactName - 
//-----------------------------------------------------------------------------
void C_TFRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName )
{
	VPROF( "C_TFRagdoll::ImpactTrace" );
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( !pPhysicsObject )
		return;

	Vector vecDir;
	VectorSubtract( pTrace->endpos, pTrace->startpos, vecDir );

	if ( iDamageType == DMG_BLAST )
	{
		// Adjust the impact strength and apply the force at the center of mass.
		vecDir *= 4000;
		pPhysicsObject->ApplyForceCenter( vecDir );
	}
	else
	{
		// Find the apporx. impact point.
		Vector vecHitPos;
		VectorMA( pTrace->startpos, pTrace->fraction, vecDir, vecHitPos );
		VectorNormalize( vecDir );

		// Adjust the impact strength and apply the force at the impact point..
		vecDir *= 4000;
		pPhysicsObject->ApplyForceOffset( vecDir, vecHitPos );
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}

// ---------------------------------------------------------------------------- -
// Purpose: 
// Input  : flInterval - 
// Output : float
//-----------------------------------------------------------------------------
float C_TFRagdoll::FrameAdvance( float flInterval )
{
	if ( m_timer1.HasStarted() && !m_timer1.IsElapsed() )
		return BaseClass::FrameAdvance( flInterval );

	float flRet = 0.0f; 
	matrix3x4_t boneDelta0[MAXSTUDIOBONES];
	matrix3x4_t boneDelta1[MAXSTUDIOBONES];
	matrix3x4_t currentBones[MAXSTUDIOBONES];
	const float boneDt = 0.05f;

	if ( m_timer2.HasStarted() )
	{
		if ( !m_timer2.IsElapsed() )
			return flRet;

		m_timer2.Invalidate();
		m_nRenderFX = kRenderFxRagdoll;

		GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt, true );

		SetAbsVelocity( vec3_origin );

		m_bStartedDying = true;
	}

	flRet = BaseClass::FrameAdvance( flInterval );

	if ( !m_bStartedDying && IsSequenceFinished() && m_bPlayDeathAnim )
	{
		m_nRenderFX = kRenderFxRagdoll;

		GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt, false );

		SetAbsVelocity( vec3_origin );

		m_bStartedDying = true;

		StartFadeOut( cl_ragdoll_fade_time.GetFloat() );
	}

	return flRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed );

	if ( IsDecapitation() && !m_bHeadTransform )
	{
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
		BuildDecapitatedTransform( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_TFRagdoll::GetAttachment( int number, matrix3x4_t &matrix )
{
	// Sword decapitation
	if ( IsDecapitation() && LookupAttachment( "head" ) == number )
	{
		MatrixCopy( m_Head, matrix );
		return true;
	}

	return BaseClass::GetAttachment( number, matrix );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFRagdoll::CreateTFRagdoll( void )
{
	// Get the player.
	C_TFPlayer *pPlayer = NULL;
	EHANDLE hPlayer = GetPlayerHandle();
	if ( hPlayer )
	{
		pPlayer = dynamic_cast<C_TFPlayer *>( hPlayer.Get() );
	}

	TFPlayerClassData_t *pData = GetPlayerClassData( m_iClass );
	if ( pData )
	{
		int nModelIndex = modelinfo->GetModelIndex( pData->GetModelName() );
		SetModelIndex( nModelIndex );

		switch ( m_iTeam )
		{
			case TF_TEAM_RED:
				m_nSkin = 0;
				break;

			case TF_TEAM_BLUE:
				m_nSkin = 1;
				break;
		}
	}

	if ( m_bGoldRagdoll || m_iDamageCustom == TF_DMG_CUSTOM_GOLD_WRENCH )
	{
		EmitSound( "Saxxy.TurnGold" );
		m_bFixedConstraints = true;
	}

	if ( m_bIceRagdoll )
	{
		EmitSound( "Icicle.TurnToIce" );
		ParticleProp()->Create( "xms_icicle_impact_dryice", PATTACH_ABSORIGIN_FOLLOW );

		m_timer1.Start( RandomFloat( 0.1f, 0.75f ) );
		m_timer2.Start( RandomFloat( 9.0f, 11.0f ) );
	}

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// Move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.		
		if ( !pPlayer->IsLocalPlayer() && pPlayer->IsInterpolationEnabled() )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( pPlayer->GetRenderOrigin() );
			SetAbsAngles( pPlayer->GetRenderAngles() );
			SetAbsVelocity( m_vecRagdollVelocity );

			// Hack! Find a neutral standing pose or use the idle.
			int iSeq = LookupSequence( "RagdollSpawn" );
			if ( iSeq == -1 )
			{
				Assert( false );
				iSeq = 0;
			}
			SetSequence( iSeq );
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}

		m_nBody = pPlayer->GetBody();
	}
	else
	{
		// Overwrite network origin so later interpolation will use this position.
		SetNetworkOrigin( m_vecRagdollOrigin );
		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );
	}

	if ( m_bCloaked )
		AddEffects( EF_NOSHADOW );

	if ( pPlayer && !m_bGoldRagdoll )
	{
		bool bFloatingAnim = false;
		int iSeq = pPlayer->m_Shared.GetSequenceForDeath( this, m_iDamageCustom );
		int iDesiredSeq = -1;

		if ( m_bDissolve && !m_bGib )
		{
			iSeq = pPlayer->LookupSequence( "dieviolent" );
			bFloatingAnim = true;
		}

		if ( iSeq >= 0 )
		{
			switch ( m_iDamageCustom )
			{
				case TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING:
				case TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH:
				case TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF:
					iDesiredSeq = iSeq;
					break;
				default:
					if ( m_bIceRagdoll || tf_always_deathanim.GetBool() || RandomFloat() <= 0.25f )
						iDesiredSeq = iSeq;
					break;
			}
		}

		if ( iDesiredSeq >= 0 && ( m_bOnGround || bFloatingAnim ) && cl_ragdoll_physics_enable.GetBool() )
		{
			// Slam velocity when doing death animation.
			SetAbsOrigin( pPlayer->GetNetworkOrigin() );
			SetAbsAngles( pPlayer->GetRenderAngles() );
			SetAbsVelocity( vec3_origin );
			m_vecForce = vec3_origin;

			ClientLeafSystem()->SetRenderGroup( GetRenderHandle(), RENDER_GROUP_OPAQUE_ENTITY );
			UpdateVisibility();

			SetSequence( iSeq );
			SetCycle( 0.0f );
			ResetSequenceInfo();

			m_bPlayDeathAnim = true;
		}
		else if ( m_bIceRagdoll )
		{
			m_timer1.Invalidate();
			m_timer2.Invalidate();
			m_bFixedConstraints = true;
		}
	}

	// Turn it into a ragdoll.
	if ( !m_bPlayDeathAnim )
	{
		if ( cl_ragdoll_physics_enable.GetBool() )
		{
			// Make us a ragdoll..
			m_nRenderFX = kRenderFxRagdoll;

			matrix3x4_t boneDelta0[MAXSTUDIOBONES];
			matrix3x4_t boneDelta1[MAXSTUDIOBONES];
			matrix3x4_t currentBones[MAXSTUDIOBONES];
			const float boneDt = 0.05f;

			// We have to make sure that we're initting this client ragdoll off of the same model.
			// GetRagdollInitBoneArrays uses the *player* Hdr, which may be a different model than
			// the ragdoll Hdr, if we try to create a ragdoll in the same frame that the player
			// changes their player model.
			CStudioHdr *pRagdollHdr = GetModelPtr();
			CStudioHdr *pPlayerHdr = NULL;
			if ( pPlayer )
				pPlayerHdr = pPlayer->GetModelPtr();

			bool bChangedModel = false;

			if ( pRagdollHdr && pPlayerHdr )
			{
				bChangedModel = pRagdollHdr->GetVirtualModel() != pPlayerHdr->GetVirtualModel();

				Assert( !bChangedModel && "C_TFRagdoll::CreateTFRagdoll: Trying to create ragdoll with a different model than the player it's based on" );
			}

			if ( pPlayer && !pPlayer->IsDormant() && !bChangedModel )
			{
				pPlayer->GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
			}
			else
			{
				GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
			}

			InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt, m_bFixedConstraints );
		}
		else
		{
			ClientLeafSystem()->SetRenderGroup( GetRenderHandle(), RENDER_GROUP_TRANSLUCENT_ENTITY );
		}
	}

	StartFadeOut( cl_ragdoll_fade_time.GetFloat() );

	if ( m_bBurning )
	{
		m_flBurnEffectStartTime = gpGlobals->curtime;
		ParticleProp()->Create( "burningplayer_corpse", PATTACH_ABSORIGIN_FOLLOW );
	}

	if ( m_bElectrocuted )
	{
		const char *pszEffect = ConstructTeamParticle( "electrocuted_%s", m_iTeam );
		EmitSound( "TFPlayer.MedicChargedDeath" );
		ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
	}

	if ( m_bBecomeAsh && !m_bDissolve && !m_bGib )
	{
		ParticleProp()->Create( "drg_fiery_death", PATTACH_ABSORIGIN_FOLLOW );
		m_flDeathDelay = 0.5f;
	}

	if ( pPlayer )
	{
		//pPlayer->CreateBoneAttachmentsFromWearables( this, m_bWasDisguised );
		pPlayer->MoveBoneAttachments( this );

		int nBombinomiconDeath = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pPlayer, nBombinomiconDeath, bombinomicon_effect_on_death );
		if ( nBombinomiconDeath == 1 && !m_bGib && !m_bGoldRagdoll )
			m_flDeathDelay = 1.2f;
	}

	// Birthday mode.
	if ( pPlayer && TFGameRules() && TFGameRules()->IsBirthday() )
	{
		AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );
		breakablepropparams_t breakParams( m_vecRagdollOrigin, GetRenderAngles(), m_vecRagdollVelocity, angularImpulse );
		breakParams.impactEnergyScale = 1.0f;
		pPlayer->DropPartyHat( breakParams, m_vecRagdollVelocity.GetForModify() );
	}

	const char *pszMaterial = NULL;
	if ( m_bGoldRagdoll )
		pszMaterial = "models/player/shared/gold_player.vmt";
	if ( m_bIceRagdoll )
		pszMaterial = "models/player/shared/ice_player.vmt";

	if ( pszMaterial )
	{
		m_MatOverride.Init( pszMaterial, "ClientEffect textures", true );

		for ( C_BaseEntity *pClientEntity = cl_entitylist->FirstBaseEntity(); pClientEntity; pClientEntity = cl_entitylist->NextBaseEntity( pClientEntity ) )
		{
			if ( pClientEntity->GetFollowedEntity() == this )
			{
				C_EconEntity *pEconEnt = dynamic_cast<C_EconEntity *>( pClientEntity );
				if ( pEconEnt )
					pEconEnt->SetMaterialOverride( GetTeamNumber(), pszMaterial );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::CreateTFGibs( bool bKill, bool bLocalOrigin )
{
	C_TFPlayer *pPlayer = NULL;
	EHANDLE hPlayer = GetPlayerHandle();
	if ( hPlayer )
	{
		pPlayer = dynamic_cast<C_TFPlayer *>( hPlayer.Get() );
	}
	if ( pPlayer )
	{
		int nBombinomiconDeath = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pPlayer, nBombinomiconDeath, bombinomicon_effect_on_death );
		if ( nBombinomiconDeath == 1 )
		{
			m_vecForce *= Vector( 2, 2, 6 );
			const char *pszEffect = "bombinomicon_burningdebris";
			if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) )
				pszEffect = "bombinomicon_burningdebris_halloween";

			Vector vecOrigin;
			if ( bLocalOrigin )
				vecOrigin = GetLocalOrigin();
			else
				vecOrigin = m_vecRagdollOrigin;

			DispatchParticleEffect( pszEffect, vecOrigin, GetLocalAngles() );
			EmitSound( "Bombinomicon.Explode" );
		}

		if ( pPlayer->m_hFirstGib == NULL || m_bFeignDeath )
		{
			Vector vecVelocity = m_vecForce + m_vecRagdollVelocity;
			VectorNormalize( vecVelocity );

			Vector vecOrigin;
			if ( bLocalOrigin )
				vecOrigin = GetLocalOrigin();
			else
				vecOrigin = m_vecRagdollOrigin;

			pPlayer->CreatePlayerGibs( vecOrigin, vecVelocity, m_vecForce.Length(), m_bBurning, false );
		}
	}

	if ( pPlayer )
	{
		if ( TFGameRules()->IsBirthdayOrPyroVision() )
		{
			DispatchParticleEffect( "bday_confetti", pPlayer->GetAbsOrigin() + Vector( 0, 0, 32 ), vec3_angle );

			if( TFGameRules()->IsBirthday() )
				C_BaseEntity::EmitSound( "Game.HappyBirthday" );
		}

		if ( TFGameRules()->IsHolidayActive( kHoliday_NewYears ) )
		{
			DispatchParticleEffect( "mvm_pow_gold_seq_firework_mid", pPlayer->GetAbsOrigin() + Vector( 0, 0, 32 ), vec3_angle );
			C_BaseEntity::EmitSound( "ui.cratesmash_ultrarare_short" );
		}
	}

	if ( bKill )
	{
		EndFadeOut();
	}
	else
	{
		SetRenderMode( kRenderNone );
		UpdateVisibility();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::CreateTFHeadGib( void )
{
	C_TFPlayer *pPlayer = NULL;
	EHANDLE hPlayer = GetPlayerHandle();
	if ( hPlayer )
	{
		pPlayer = dynamic_cast<C_TFPlayer *>( hPlayer.Get() );
	}
	if ( pPlayer && ( pPlayer->m_hFirstGib == NULL ) )
	{
		Vector vecVelocity = m_vecForce + m_vecRagdollVelocity;
		VectorNormalize( vecVelocity );
		pPlayer->CreatePlayerGibs( m_vecRagdollOrigin, vecVelocity, m_vecForce.Length(), m_bBurning, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
//-----------------------------------------------------------------------------
void C_TFRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		bool bCreateRagdoll = true;

		// Get the player.
		EHANDLE hPlayer = GetPlayerHandle();
		if ( hPlayer )
		{
			// If we're getting the initial update for this player (e.g., after resetting entities after
			//  lots of packet loss, then don't create gibs, ragdolls if the player and it's gib/ragdoll
			//  both show up on same frame.
			if ( abs( hPlayer->GetCreationTick() - gpGlobals->tickcount ) < TIME_TO_TICKS( 1.0f ) )
			{
				bCreateRagdoll = false;
			}
		}
		else if ( C_BasePlayer::GetLocalPlayer() )
		{
			// Ditto for recreation of the local player
			if ( abs( C_BasePlayer::GetLocalPlayer()->GetCreationTick() - gpGlobals->tickcount ) < TIME_TO_TICKS( 1.0f ) )
			{
				bCreateRagdoll = false;
			}
		}

		C_TFPlayer *pPlayer = ToTFPlayer( hPlayer );
		if ( !m_bCloaked && pPlayer && pPlayer->GetPercentInvisible() > 0 )
			m_flUncloakCompleteTime = gpGlobals->curtime * 2.0f + pPlayer->GetPercentInvisible();

		if ( m_iDamageCustom == TF_DMG_CUSTOM_PLASMA_CHARGED )
		{
			if ( !m_bBecomeAsh )
				m_bDissolve = true;

			m_bGib = true;
		}
		else if ( m_iDamageCustom == TF_DMG_CUSTOM_PLASMA )
		{
			if ( !m_bBecomeAsh )
				m_bDissolve = true;

			m_bGib = false;
		}

		if ( bCreateRagdoll )
		{
			if ( m_bGib )
			{
				CreateTFGibs( !m_bDissolve, false );
			}
			else
			{
				CreateTFRagdoll();

				if ( IsDecapitation() )
				{
					CreateTFHeadGib();

					if ( !UTIL_IsLowViolence() ) // Pyrovision check
					{
						if ( TFGameRules() && TFGameRules()->IsMvMModelsAllowed() )
						{
							EmitSound( "BaseExplosionEffect.Sound" );
							ParticleProp()->Create( "spark_electric01", PATTACH_POINT_FOLLOW, "head" );
						}
						else
						{
							EmitSound( "TFPlayer.Decapitated" );
							ParticleProp()->Create( "blood_decap_arterial_spray", PATTACH_POINT_FOLLOW, "head" );
							ParticleProp()->Create( "blood_decap_fountain", PATTACH_POINT_FOLLOW, "head" );
							ParticleProp()->Create( "blood_decap_streaks", PATTACH_POINT_FOLLOW, "head" );
						}

					}
				}
			}
		}
	}
	else
	{
		if ( !cl_ragdoll_physics_enable.GetBool() )
		{
			// Don't let it set us back to a ragdoll with data from the server.
			m_nRenderFX = kRenderFxNone;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : 
//-----------------------------------------------------------------------------
int C_TFRagdoll::InternalDrawModel( int flags )
{
	if ( m_MatOverride )
		modelrender->ForcedMaterialOverride( m_MatOverride );

	int result = C_BaseAnimating::InternalDrawModel( flags );

	if ( m_MatOverride )
		modelrender->ForcedMaterialOverride( NULL );

	return result;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : IRagdoll*
//-----------------------------------------------------------------------------
IRagdoll *C_TFRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_TFRagdoll::IsRagdollVisible()
{
	Vector vMins = Vector( -1, -1, -1 );	//WorldAlignMins();
	Vector vMaxs = Vector( 1, 1, 1 );	//WorldAlignMaxs();

	Vector origin = GetAbsOrigin();

	if ( !engine->IsBoxInViewCluster( vMins + origin, vMaxs + origin ) )
	{
		return false;
	}
	else if ( engine->CullBox( vMins + origin, vMaxs + origin ) )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_TFRagdoll::IsDecapitation()
{
	// Only decapitate if the ragdoll is going to stick around for a while (?)
	if ( cl_ragdoll_fade_time.GetFloat() > 5.0f && ( m_iDamageCustom == TF_DMG_CUSTOM_DECAPITATION || m_iDamageCustom == TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING || m_iDamageCustom == TF_DMG_CUSTOM_DECAPITATION_BOSS || m_iDamageCustom == TF_DMG_CUSTOM_HEADSHOT_DECAPITATION ) )
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::ClientThink( void )
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	if ( IsDecapitation() )
	{
		m_bHeadTransform = true;
		BaseClass::GetAttachment( LookupAttachment( "head" ), m_Head );
		m_bHeadTransform = false;

		m_BoneAccessor.SetReadableBones( 0 );
		SetupBones( NULL, -1, BONE_USED_BY_ATTACHMENT, gpGlobals->curtime );
	}

	if ( m_bCloaked )
	{
		if ( m_flInvisibilityLevel < 1.0f )
			m_flInvisibilityLevel = Min( m_flInvisibilityLevel + gpGlobals->frametime, 1.0f );
	}

	C_TFPlayer *pPlayer = ToTFPlayer( GetPlayerHandle() );
	bool bBombinomiconDeath = false;
	if ( pPlayer )
	{
		int nBombinomiconDeath = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pPlayer, nBombinomiconDeath, bombinomicon_effect_on_death );
		bBombinomiconDeath = nBombinomiconDeath != 0;
	}

	if ( !m_bGib )
	{
		if ( !m_bDissolve )
		{
			if ( bBombinomiconDeath && ( GetFlags() & FL_DISSOLVING ) )
			{
				m_flDeathDelay -= gpGlobals->frametime;
				if ( m_flDeathDelay <= 0 )
					CreateTFGibs( !m_bDissolve, true );
			}
			else
			{
				if ( !m_bBecomeAsh )
				{
					if ( bBombinomiconDeath )
					{
						m_flDeathDelay -= gpGlobals->frametime;
						if ( m_flDeathDelay <= 0 )
						{
							CreateTFGibs( !m_bDissolve, true );
							return;
						}
					}
				}
				else
				{
					m_flDeathDelay -= gpGlobals->frametime;
					if ( m_flDeathDelay <= 0 )
					{
						if ( !bBombinomiconDeath )
						{
							EndFadeOut();
							return;
						}

						CreateTFGibs( !m_bDissolve, true );
						return;
					}
				}
			}
		}
		else
		{
			m_bDissolve = false;
			m_flDeathDelay = 1.2f;
			DissolveEntity( this );
			EmitSound( "TFPlayer.Dissolve" );
		}
	}
	else
	{
		if ( m_bDissolve )
		{
			m_flDeathDelay -= gpGlobals->frametime;
			if ( m_flDeathDelay > 0 )
				return;

			m_bDissolve = false;

			if ( pPlayer )
			{
				if ( bBombinomiconDeath )
				{
					CreateTFGibs( true, true );
				}
				else
				{
					for ( int i=0; i<pPlayer->m_hSpawnedGibs.Count(); ++i )
					{
						C_BaseEntity *pGiblet = pPlayer->m_hSpawnedGibs[i];

						pGiblet->SetAbsVelocity( vec3_origin );
						DissolveEntity( pGiblet );
						pGiblet->ParticleProp()->StopParticlesInvolving( pGiblet );
					}
				}
			}

			EndFadeOut();
			return;
		}
	}

	if ( m_bFadingOut == true )
	{
		int iAlpha = GetRenderColor().a;
		int iFadeSpeed = 600.0f;

		iAlpha = Max( iAlpha - (int)( iFadeSpeed * gpGlobals->frametime ), 0 );

		SetRenderMode( kRenderTransAlpha );
		SetRenderColorA( iAlpha );

		if ( iAlpha == 0 )
		{
			EndFadeOut(); // remove clientside ragdoll
		}

		return;
	}

	// if the player is looking at us, delay the fade
	if ( IsRagdollVisible() )
	{
		if ( cl_ragdoll_forcefade.GetBool() )
		{
			m_bFadingOut = true;
			float flDelay = cl_ragdoll_fade_time.GetFloat() * 0.33f;
			m_fDeathTime = gpGlobals->curtime + flDelay;

			// If we were just fully healed, remove all decals
			RemoveAllDecals();
		}

		StartFadeOut( cl_ragdoll_fade_time.GetFloat() * 0.33f );
		return;
	}

	if ( m_fDeathTime > gpGlobals->curtime )
		return;

	EndFadeOut(); // remove clientside ragdoll
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::StartFadeOut( float fDelay )
{
	if ( !cl_ragdoll_forcefade.GetBool() )
	{
		m_fDeathTime = gpGlobals->curtime + fDelay;
	}
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFRagdoll::EndFadeOut()
{
	SetNextClientThink( CLIENT_THINK_NEVER );
	ClearRagdoll();
	SetRenderMode( kRenderNone );
	UpdateVisibility();
}


//-----------------------------------------------------------------------------
// Purpose: Used for spy invisiblity material
//-----------------------------------------------------------------------------
class CSpyInvisProxy : public CEntityMaterialProxy
{
public:
						CSpyInvisProxy( void );
	virtual				~CSpyInvisProxy( void );
	virtual bool		Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void		OnBind( C_BaseEntity *pC_BaseEntity );
	virtual IMaterial *	GetMaterial();

private:

	IMaterialVar		*m_pPercentInvisible;
	IMaterialVar		*m_pCloakColorTint;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSpyInvisProxy::CSpyInvisProxy( void )
{
	m_pPercentInvisible = NULL;
	m_pCloakColorTint = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSpyInvisProxy::~CSpyInvisProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get pointer to the color value
// Input  : *pMaterial - 
//-----------------------------------------------------------------------------
bool CSpyInvisProxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues )
{
	Assert( pMaterial );

	// Need to get the material var
	bool bInvis;
	m_pPercentInvisible = pMaterial->FindVar( "$cloakfactor", &bInvis );

	bool bTint;
	m_pCloakColorTint = pMaterial->FindVar( "$cloakColorTint", &bTint );

	return ( bInvis && bTint );
}

ConVar tf_teammate_max_invis( "tf_teammate_max_invis", "0.95", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
//-----------------------------------------------------------------------------
void CSpyInvisProxy::OnBind( C_BaseEntity *pEnt )
{
	if( !m_pPercentInvisible || !m_pCloakColorTint )
		return;

	if ( !pEnt )
		return;

	C_TFPlayer *pPlayer = ToTFPlayer( pEnt );

	if ( !pPlayer )
	{
		// This might be a cosmetic parented to a player.
		pPlayer = ToTFPlayer( pEnt->GetMoveParent() );
	}

	if ( pPlayer )
	{
		m_pPercentInvisible->SetFloatValue( pPlayer->GetEffectiveInvisibilityLevel() );
	}
	else
	{
		C_TFRagdoll *pRagdoll = dynamic_cast<C_TFRagdoll *>( pEnt );

		if ( pRagdoll )
		{
			pPlayer = ToTFPlayer( pRagdoll->GetPlayerHandle().Get() );
			m_pPercentInvisible->SetFloatValue( pRagdoll->GetInvisibilityLevel() );
		}
		else
		{
			m_pPercentInvisible->SetFloatValue( 0.0 );
			return;
		}
	}

	if ( !pPlayer )
		return;

	float r, g, b;

	switch( pPlayer->GetTeamNumber() )
	{
		case TF_TEAM_RED:
			r = 1.0; g = 0.5; b = 0.4;
			break;

		case TF_TEAM_BLUE:
			r = 0.4; g = 0.5; b = 1.0;
			break;

		case TF_TEAM_GREEN:
			r = 0.4; g = 1.0; b = 0.5;
			break;

		case TF_TEAM_YELLOW:
			r = 1.0; g = 0.5; b = 0.5;
			break;

		default:
			r = 0.4; g = 0.5; b = 1.0;
			break;
	}

	m_pCloakColorTint->SetVecValue( r, g, b );
}

IMaterial *CSpyInvisProxy::GetMaterial()
{
	if ( !m_pPercentInvisible )
		return NULL;

	return m_pPercentInvisible->GetOwningMaterial();
}

EXPOSE_INTERFACE( CSpyInvisProxy, IMaterialProxy, "spy_invis" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Used for invulnerability material
//			Returns 1 if the player is invulnerable, and 0 if the player is losing / doesn't have invuln.
//-----------------------------------------------------------------------------
class CProxyInvulnLevel : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
		Assert( m_pResult );

		C_TFPlayer *pPlayer = NULL;
		C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
		if ( !pEntity )
		{
			m_pResult->SetFloatValue( 0.0 );
			return;
		}

		if ( pEntity->IsPlayer() )
		{
			pPlayer = ToTFPlayer( pEntity );
		}
		else
		{
			// See if it's a weapon
			C_TFWeaponBase *pWeapon = dynamic_cast<C_TFWeaponBase *>( pEntity );
			if ( pWeapon )
			{
				pPlayer = ToTFPlayer( pWeapon->GetOwner() );
			}
			else
			{
				C_TFViewModel *pVM = dynamic_cast<C_TFViewModel *>( pEntity );
				if ( pVM )
				{
					pPlayer = ToTFPlayer( pVM->GetOwner() );
				}
				else
				{
					C_ViewmodelAttachmentModel *pVMAddon = dynamic_cast<C_ViewmodelAttachmentModel *>( pEntity );
					if ( pVMAddon )
					{
						pVM = pVMAddon->m_ViewModel.Get();
						if ( pVM )
							pPlayer = ToTFPlayer( pVM->GetOwner() );
					}
				}
			}
		}

		if ( pPlayer )
		{
			if ( pPlayer->m_Shared.IsInvulnerable() &&
				!pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE_WEARINGOFF ) )
			{
				m_pResult->SetFloatValue( 1.0 );
			}
			else
			{
				m_pResult->SetFloatValue( 0.0 );
			}
		}
		else if ( pEntity->IsNPC() )
		{
			// See if it's NPC.
			C_AI_BaseNPC *pNPC = assert_cast<C_AI_BaseNPC *>( pEntity );
			if ( pNPC->IsInvulnerable() &&
				!pNPC->InCond( TF_COND_INVULNERABLE_WEARINGOFF ) )
			{
				m_pResult->SetFloatValue( 1.0 );
			}
			else
			{
				m_pResult->SetFloatValue( 0.0 );
			}
		}


		if ( ToolsEnabled() )
		{
			ToolFramework_RecordMaterialParams( GetMaterial() );
		}
	}
};

EXPOSE_INTERFACE( CProxyInvulnLevel, IMaterialProxy, "InvulnLevel" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Used for burning material on player models
//			Returns 0.0->1.0 for level of burn to show on player skin
//-----------------------------------------------------------------------------
class CProxyBurnLevel : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
		Assert( m_pResult );

		if ( !pC_BaseEntity )
		{
			m_pResult->SetFloatValue(0.0f);
			return;
		}

		C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
		if ( !pEntity )
			return;

		// default to zero
		float flBurnStartTime = 0;
			
		C_TFPlayer *pPlayer = dynamic_cast< C_TFPlayer* >( pEntity );

		if ( pPlayer )		
		{
			// is the player burning?
			if ( pPlayer->m_Shared.InCond( TF_COND_BURNING ) )
			{
				flBurnStartTime = pPlayer->m_flBurnEffectStartTime;
			}
		}
		else
		{
			// is the ragdoll burning?
			C_TFRagdoll *pRagDoll = dynamic_cast< C_TFRagdoll* >( pEntity );
			if ( pRagDoll )
			{
				flBurnStartTime = pRagDoll->GetBurnStartTime();
			}

			C_AI_BaseNPC *pNPC = dynamic_cast< C_AI_BaseNPC* >( pEntity );
			if ( pNPC )
			{
				if ( pNPC->InCond( TF_COND_BURNING ) )
				{
					flBurnStartTime = pNPC->m_flBurnEffectStartTime;
				}
			}
		}

		float flResult = 0.0;
		
		// if player/ragdoll is burning, set the burn level on the skin
		if ( flBurnStartTime > 0 )
		{
			float flBurnPeakTime = flBurnStartTime + 0.3;
			float flTempResult;
			if ( gpGlobals->curtime < flBurnPeakTime )
			{
				// fade in from 0->1 in 0.3 seconds
				flTempResult = RemapValClamped( gpGlobals->curtime, flBurnStartTime, flBurnPeakTime, 0.0, 1.0 );
			}
			else
			{
				// fade out from 1->0 in the remaining time until flame extinguished
				flTempResult = RemapValClamped( gpGlobals->curtime, flBurnPeakTime, flBurnStartTime + TF_BURNING_FLAME_LIFE, 1.0, 0.0 );
			}	

			// We have to do some more calc here instead of in materialvars.
			flResult = 1.0 - abs( flTempResult - 1.0 );
		}

		m_pResult->SetFloatValue( flResult );

		if ( ToolsEnabled() )
		{
			ToolFramework_RecordMaterialParams( GetMaterial() );
		}
	}
};

EXPOSE_INTERFACE( CProxyBurnLevel, IMaterialProxy, "BurnLevel" IMATERIAL_PROXY_INTERFACE_VERSION );

const Vector g_aUrineLevels[TF_TEAM_COUNT] =
{
	Vector( 1, 1, 1 ),
	Vector( 1, 1, 1 ),
	Vector( 7, 5, 1 ),
	Vector( 9, 6, 2 ),
	Vector( 5, 7, 1 ),
	Vector( 9, 6, 1 ),
};

//-----------------------------------------------------------------------------
// Purpose: Used for jarate
//			Returns the RGB value for the appropriate tint condition.
//-----------------------------------------------------------------------------
class CProxyUrineLevel : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
		Assert( m_pResult );

		if ( !pC_BaseEntity )
		{
			m_pResult->SetVecValue( 1, 1, 1 );
			return;
		}

		C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
		if ( !pEntity )
			return;

		C_TFPlayer *pPlayer = ToTFPlayer( pEntity );
		C_AI_BaseNPC *pNPC = dynamic_cast<C_AI_BaseNPC *>( pEntity );

		if ( pEntity->IsPlayer() )
		{
			if ( !pPlayer )
			{
				C_BaseCombatWeapon *pWeapon = pEntity->MyCombatWeaponPointer();
				if ( pWeapon )
				{
					pPlayer = ToTFPlayer( pWeapon->GetOwner() );
				}
				else
				{
					C_TFViewModel *pVM;
					C_ViewmodelAttachmentModel *pVMAddon = dynamic_cast<C_ViewmodelAttachmentModel *>( pEntity );
					if ( pVMAddon )
					{
						pVM = dynamic_cast<C_TFViewModel *>( pVMAddon->m_ViewModel.Get() );
					}
					else
					{
						pVM = dynamic_cast<C_TFViewModel *>( pEntity );
					}

					if ( pVM )
					{
						pPlayer = ToTFPlayer( pVM->GetOwner() );
					}
				}
			}

			if ( pPlayer && pPlayer->m_Shared.InCond( TF_COND_URINE ) )
			{
				int iTeam = pPlayer->GetTeamNumber();
				if ( pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && pPlayer->IsEnemyPlayer() )
				{
					iTeam = pPlayer->m_Shared.GetDisguiseTeam();
				}

				if ( iTeam >= FIRST_GAME_TEAM && iTeam < TF_TEAM_COUNT )
				{
					float r = g_aUrineLevels[iTeam].x;
					float g = g_aUrineLevels[iTeam].y;
					float b = g_aUrineLevels[iTeam].z;

					m_pResult->SetVecValue( r, g, b );
					return;
				}
			}
		}

		if ( pEntity->IsNPC() )
		{
			if ( pNPC && pNPC->InCond( TF_COND_URINE ) )
			{
				int iTeam = pNPC->GetTeamNumber();

				if ( iTeam >= FIRST_GAME_TEAM && iTeam < TF_TEAM_COUNT )
				{
					float r = g_aUrineLevels[iTeam].x;
					float g = g_aUrineLevels[iTeam].y;
					float b = g_aUrineLevels[iTeam].z;

					m_pResult->SetVecValue( r, g, b );
					return;
				}
			}
		}

		m_pResult->SetVecValue( 1, 1, 1 );
	}
};

EXPOSE_INTERFACE( CProxyUrineLevel, IMaterialProxy, "YellowLevel" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Used for the weapon glow color when critted
//-----------------------------------------------------------------------------
class CProxyModelGlowColor : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
		Assert( m_pResult );

		if ( !pC_BaseEntity )
		{
			m_pResult->SetVecValue( 1, 1, 1 );
			return;
		}

		C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
		if ( !pEntity )
			return;

		Vector vecColor = Vector( 1, 1, 1 );

		C_TFPlayer *pPlayer = ToTFPlayer( pEntity );;
		//C_AI_BaseNPC *pNPC = assert_cast<C_AI_BaseNPC *>( pEntity );

		if ( !pPlayer )
		{
			C_BaseCombatWeapon *pWeapon = pEntity->MyCombatWeaponPointer();
			if ( pWeapon )
			{
				pPlayer = ToTFPlayer( pWeapon->GetOwner() );
			}
			else
			{
				C_TFViewModel *pVM;
				C_ViewmodelAttachmentModel *pVMAddon = dynamic_cast< C_ViewmodelAttachmentModel * >( pEntity );
				if ( pVMAddon )
				{
					pVM = dynamic_cast< C_TFViewModel * >( pVMAddon->m_ViewModel.Get() );
				}
				else
				{
					pVM = dynamic_cast< C_TFViewModel * >( pEntity );
				}

				if ( pVM )
				{
					pPlayer = ToTFPlayer( pVM->GetOwner() );
				}
			}
		}
		/*
			Live TF2 crit glow colors
			RED Crit: 94 8 5
			BLU Crit: 6 21 80
			RED Mini-Crit: 237 140 55
			BLU Mini-Crit: 28 168 112
			Hype Mode: 50 2 50
			*/

		if ( pPlayer && pPlayer->m_Shared.IsCritBoosted() )
		{
			if ( !pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) ||
				!pPlayer->IsEnemyPlayer() ||
				pPlayer->GetTeamNumber() == pPlayer->m_Shared.GetDisguiseTeam() )
			{
				switch ( pPlayer->GetTeamNumber() )
				{
				case TF_TEAM_RED:
					vecColor = Vector( 94, 8, 5 );
					break;
				case TF_TEAM_BLUE:
					vecColor = Vector( 6, 21, 80 );
					break;
				case TF_TEAM_GREEN:
					//vecColor = Vector( 1, 28, 9 );
					vecColor = Vector( 6, 21, 100 );
					break;
				case TF_TEAM_YELLOW:
					vecColor = Vector( 28, 28, 9 );
					break;
				}
			}
		}
		else if ( pPlayer && pPlayer->m_Shared.IsMiniCritBoosted() )
		{
			if ( !pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) ||
				!pPlayer->IsEnemyPlayer() ||
				pPlayer->GetTeamNumber() == pPlayer->m_Shared.GetDisguiseTeam() )
			{
				switch ( pPlayer->GetTeamNumber() )
				{
				case TF_TEAM_RED:
					vecColor = Vector( 237, 140, 55 );
					break;
				case TF_TEAM_BLUE:
					vecColor = Vector( 28, 168, 112 );
					break;
				}
			}
		}
		else if ( pPlayer && pPlayer->m_Shared.InCond( TF_COND_SODAPOPPER_HYPE ) )
		{
			if ( !pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) ||
				!pPlayer->IsEnemyPlayer() ||
				pPlayer->GetTeamNumber() == pPlayer->m_Shared.GetDisguiseTeam() )
			{
					vecColor = Vector( 50, 2, 50 );
			}
		}
		else if ( pPlayer && ( pPlayer->m_Shared.InCond( TF_COND_SHIELD_CHARGE ) || pPlayer->m_Shared.m_bShieldChargeStopped ) )
		{
			float flAmt = 1.0f;
			if (pPlayer->m_Shared.InCond( TF_COND_SHIELD_CHARGE ))
				flAmt = ( 100.0f - pPlayer->m_Shared.GetShieldChargeMeter() ) / 100.0f;
			else
				flAmt = Min( ( ( gpGlobals->curtime - pPlayer->m_Shared.m_flShieldChargeEndTime ) + -1.5f ) * 10.0f / 3.0f, 1.0f );

			switch (pPlayer->GetTeamNumber())
			{
				case TF_TEAM_RED:
					vecColor.x = Max( flAmt * 80.0f, 1.0f );
					vecColor.y = Max( flAmt * 8.0f, 1.0f );
					vecColor.z = Max( flAmt * 5.0f, 1.0f );
					break;
				case TF_TEAM_BLUE:
					vecColor.x = Max( flAmt * 5.0f, 1.0f );
					vecColor.y = Max( flAmt * 20.0f, 1.0f );
					vecColor.z = Max( flAmt * 80.0f, 1.0f );
					break;
			}
		}/*
		else if ( pNPC && pNPC->IsCritBoosted() )
		{
			if ( !pNPC->InCond( TF_COND_DISGUISED ) )
			{
				switch ( pNPC->GetTeamNumber() )
				{
				case TF_TEAM_RED:
					vecColor = Vector( 94, 8, 5 );
					break;
				case TF_TEAM_BLUE:
					vecColor = Vector( 6, 21, 80 );
					break;
				case TF_TEAM_GREEN:
					vecColor = Vector( 1, 28, 9 );
					break;
				case TF_TEAM_YELLOW:
					vecColor = Vector( 28, 28, 9 );
					break;
				}
			}
		}*/

		m_pResult->SetVecValue( vecColor.Base(), 3 );
		/*
		if ( ToolsEnabled() )
		{
			ToolFramework_RecordMaterialParams( GetMaterial() );
		}
		*/
	}
};

EXPOSE_INTERFACE( CProxyModelGlowColor, IMaterialProxy, "ModelGlowColor" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Used for coloring items 
//
//-----------------------------------------------------------------------------
class CProxyItemTintColor : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
		Assert( m_pResult );

		C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );
		if ( !pEntity )
			return;

		if ( TFGameRules() )
		{
			Vector vecColor = pEntity->GetItemTintColor();

			if ( vecColor == vec3_origin )
			{
				// Entity doesn't have its own color, get the controlling entity.
				C_BaseEntity *pOwner = pEntity->GetItemTintColorOwner();
				if ( pOwner )
				{
					vecColor = pOwner->GetItemTintColor();
				}
			}

			m_pResult->SetVecValue( vecColor.x, vecColor.y, vecColor.z );
			return;
		}

		m_pResult->SetVecValue( 1, 1, 1 );
	}

private:

	IMaterialVar		*m_pColorTint;
};

EXPOSE_INTERFACE( CProxyItemTintColor, IMaterialProxy, "ItemTintColor" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: 
//
//-----------------------------------------------------------------------------
class CProxyResistShield : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity )
	{
	}
};

EXPOSE_INTERFACE( CProxyResistShield, IMaterialProxy, "ResistShield" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Stub class for the CommunityWeapon material proxy used by live TF2
//-----------------------------------------------------------------------------
class CProxyCommunityWeapon : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		return true;
	}
	void OnBind( void *pC_BaseEntity )
	{
	}
};

EXPOSE_INTERFACE( CProxyCommunityWeapon, IMaterialProxy, "CommunityWeapon" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Stub class for the AnimatedWeaponSheen material proxy used by live TF2
//-----------------------------------------------------------------------------
class CProxyAnimatedWeaponSheen : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		return true;
	}
	void OnBind( void *pC_BaseEntity )
	{
	}
};

EXPOSE_INTERFACE( CProxyAnimatedWeaponSheen, IMaterialProxy, "AnimatedWeaponSheen" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Universal proxy from live tf2 used for spy invisiblity material
//			Its' purpose is to replace weapon_invis, vm_invis and spy_invis
//-----------------------------------------------------------------------------
class CInvisProxy : public CEntityMaterialProxy
{
public:
	CInvisProxy( void );
	virtual				~CInvisProxy( void );
	virtual bool		Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void		OnBind( C_BaseEntity *pC_BaseEntity );
	virtual IMaterial *	GetMaterial();

	virtual void		HandleSpyInvis( C_TFPlayer *pPlayer );
	virtual void		HandleVMInvis( C_BaseViewModel *pVM );
	virtual void		HandleWeaponInvis( C_BaseEntity *pC_BaseEntity );

private:

	IMaterialVar		*m_pPercentInvisible;
	IMaterialVar		*m_pCloakColorTint;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CInvisProxy::CInvisProxy(void)
{
	m_pPercentInvisible = NULL;
	m_pCloakColorTint = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CInvisProxy::~CInvisProxy(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Get pointer to the color value
// Input  : *pMaterial - 
//-----------------------------------------------------------------------------
bool CInvisProxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues )
{
	Assert( pMaterial );

	// Need to get the material var
	bool bInvis;
	m_pPercentInvisible = pMaterial->FindVar( "$cloakfactor", &bInvis );

	bool bTint;
	m_pCloakColorTint = pMaterial->FindVar( "$cloakColorTint", &bTint );

	// if we have $cloakColorTint, it's spy_invis
	if ( bTint )
	{
		return ( bInvis && bTint );
	}

	return ( bTint );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
//-----------------------------------------------------------------------------
void CInvisProxy::OnBind( C_BaseEntity *pEnt )
{
	if ( !pEnt )
		return;

	m_pPercentInvisible->SetFloatValue( 0.0 );

	C_TFPlayer *pPlayer = ToTFPlayer( pEnt );
	if ( pPlayer )
	{
		HandleSpyInvis( pPlayer );
		return;
	}

	C_TFViewModel *pVM;
	C_ViewmodelAttachmentModel *pVMAddon = dynamic_cast<C_ViewmodelAttachmentModel *>( pEnt );
	if ( pVMAddon )
	{
		pVM = dynamic_cast<C_TFViewModel *>( pVMAddon->m_ViewModel.Get() );
	}
	else
	{
		pVM = dynamic_cast<C_TFViewModel *>( pEnt );
	}

	if ( pVM )
	{
		HandleVMInvis( pVM );
		return;
	}

	HandleWeaponInvis( pEnt );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
//-----------------------------------------------------------------------------
void CInvisProxy::HandleSpyInvis( C_TFPlayer *pPlayer )
{
	if ( !m_pPercentInvisible || !m_pCloakColorTint )
		return;

	m_pPercentInvisible->SetFloatValue( pPlayer->GetEffectiveInvisibilityLevel() );

	float r, g, b;

	switch ( pPlayer->GetTeamNumber() )
	{
	case TF_TEAM_RED:
		r = 1.0; g = 0.5; b = 0.4;
		break;

	case TF_TEAM_BLUE:
		r = 0.4; g = 0.5; b = 1.0;
		break;

	case TF_TEAM_GREEN:
		r = 0.4; g = 1.0; b = 0.5;
		break;

	case TF_TEAM_YELLOW:
		r = 1.0; g = 0.5; b = 0.5;
		break;

	default:
		r = 0.4; g = 0.5; b = 1.0;
		break;
	}

	m_pCloakColorTint->SetVecValue( r, g, b );
}

extern ConVar tf_vm_min_invis;
extern ConVar tf_vm_max_invis;
//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
//-----------------------------------------------------------------------------
void CInvisProxy::HandleVMInvis( C_BaseViewModel *pVM )
{
	if ( !m_pPercentInvisible )
		return;

	C_TFPlayer *pPlayer = ToTFPlayer( pVM->GetOwner() );

	if ( !pPlayer )
	{
		m_pPercentInvisible->SetFloatValue( 0.0f );
		return;
	}

	float flPercentInvisible = pPlayer->GetPercentInvisible();

	// remap from 0.22 to 0.5
	// but drop to 0.0 if we're not invis at all
	float flWeaponInvis = ( flPercentInvisible < 0.01 ) ?
	0.0 :
	RemapVal( flPercentInvisible, 0.0, 1.0, tf_vm_min_invis.GetFloat(), tf_vm_max_invis.GetFloat() );

	if ( pPlayer->m_Shared.InCond( TF_COND_STEALTHED_BLINK ) )
	{
		// Hacky fix to make viewmodel blink more obvious
		m_pPercentInvisible->SetFloatValue( flWeaponInvis - 0.1 );
	}
	else
	{
		m_pPercentInvisible->SetFloatValue( flWeaponInvis );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
//-----------------------------------------------------------------------------
void CInvisProxy::HandleWeaponInvis( C_BaseEntity *pEnt )
{
	if ( !m_pPercentInvisible )
		return;

	C_BaseEntity *pMoveParent = pEnt->GetMoveParent();
	if ( !pMoveParent || !pMoveParent->IsPlayer() )
	{
		m_pPercentInvisible->SetFloatValue( 0.0f );
		return;
	}

	C_TFPlayer *pPlayer = ToTFPlayer( pMoveParent );
	Assert( pPlayer );

	m_pPercentInvisible->SetFloatValue( pPlayer->GetEffectiveInvisibilityLevel() );
}

IMaterial *CInvisProxy::GetMaterial()
{
	if ( !m_pPercentInvisible )
		return NULL;

	return m_pPercentInvisible->GetOwningMaterial();
}

EXPOSE_INTERFACE(CInvisProxy, IMaterialProxy, "invis" IMATERIAL_PROXY_INTERFACE_VERSION);

//-----------------------------------------------------------------------------
// Purpose: Stub class for the WeaponSkin material proxy used by live TF2
//-----------------------------------------------------------------------------
class CWeaponSkinProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		return true;
	}
	void OnBind( void *pC_BaseEntity )
	{
	}
};

EXPOSE_INTERFACE( CWeaponSkinProxy, IMaterialProxy, "WeaponSkin" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: Stub class for the BuildingRescueLevel material proxy used by live TF2
//-----------------------------------------------------------------------------
class CProxyBuildingRescueLevel : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		return true;
	}
	void OnBind( void *pC_BaseEntity )
	{
		C_TFPlayer *pPlayer = ToTFPlayer( C_BasePlayer::GetLocalPlayer() );
		if ( pPlayer == nullptr )
			return;

		int nTeleportBuildings = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pPlayer, nTeleportBuildings, building_teleporting_pickup );
		if ( nTeleportBuildings == 0 )
			return;

		float flScale = 10.f;
		const int nMetal = pPlayer->GetAmmoCount( TF_AMMO_METAL );

		if ( nMetal >= nTeleportBuildings )
			flScale = ( 3.f - ( ( ( nMetal - nTeleportBuildings ) / ( pPlayer->GetMaxAmmo( TF_AMMO_METAL ) - nTeleportBuildings ) ) * 3.f ) ) + 1.f;

		VMatrix matrix, output;
		MatrixBuildTranslation( output, -0.5, -0.5, 0.0 );
		MatrixBuildScale( matrix, 1.0, flScale, 1.0 );
		MatrixMultiply( matrix, output, output );
		MatrixBuildTranslation( matrix, 0.5, 0.5, 0.0 );
		MatrixMultiply( matrix, output, output );

		m_pResult->SetMatrixValue( output );

		if ( ToolsEnabled() )
			ToolFramework_RecordMaterialParams( GetMaterial() );
	}
};

EXPOSE_INTERFACE( CProxyBuildingRescueLevel, IMaterialProxy, "BuildingRescueLevel" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CCustomTextureOnItemProxy : public IMaterialProxy
{
public:
	CCustomTextureOnItemProxy();
	virtual ~CCustomTextureOnItemProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pProxyData );
	virtual void Release( void ) { delete this; }
	virtual IMaterial *GetMaterial();

private:
	IMaterialVar* m_BaseTextureVar;
};

CCustomTextureOnItemProxy::CCustomTextureOnItemProxy()
{
	m_BaseTextureVar = NULL;
}

CCustomTextureOnItemProxy::~CCustomTextureOnItemProxy()
{
}


bool CCustomTextureOnItemProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool foundVar;
	m_BaseTextureVar = pMaterial->FindVar( "$basetexture", &foundVar, false );
	if ( !foundVar )
		return false;

	return foundVar;
}

void CCustomTextureOnItemProxy::OnBind( void *pProxyData )
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pPlayer == nullptr )
		return;

	// Find player logo for shooter
	player_info_t info;
	engine->GetPlayerInfo( pPlayer->entindex(), &info );

	// Make sure we've got the material for this player's logo
	char texname[ 512 ];

	// Doesn't have a logo?
	if ( !info.customFiles[0] )	
		return;

	char logohex[ 16 ];
	Q_binarytohex( (byte *)&info.customFiles[0], sizeof( info.customFiles[0] ), logohex, sizeof( logohex ) );

	// See if logo has been downloaded.
	Q_snprintf( texname, 512, "temp/%s", logohex );
	char fulltexname[ 512 ];
	Q_snprintf( fulltexname, sizeof( fulltexname ), "materials/temp/%s.vtf", logohex );

	if ( !filesystem->FileExists( fulltexname ) )
	{
		char custname[ 512 ];
		Q_snprintf( custname, sizeof( custname ), "download/user_custom/%c%c/%s.dat", logohex[0], logohex[1], logohex );
		// it may have been downloaded but not copied under materials folder
		if ( !filesystem->FileExists( custname ) )
			return; // not downloaded yet

		if ( !engine->CopyLocalFile( custname, fulltexname ) )
			return;
	}

	ITexture *texture = materials->FindTexture( texname, TEXTURE_GROUP_DECAL );
	if ( IsErrorTexture( texture ) ) 
		return;

	m_BaseTextureVar->SetTextureValue( texture );

	if ( ToolsEnabled() )
	{
		ToolFramework_RecordMaterialParams( GetMaterial() );
	}
}

IMaterial *CCustomTextureOnItemProxy::GetMaterial()
{
	return m_BaseTextureVar->GetOwningMaterial();
}

EXPOSE_INTERFACE( CCustomTextureOnItemProxy, IMaterialProxy, "CustomSteamImageOnModel" IMATERIAL_PROXY_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Purpose: RecvProxy that converts the Player's object UtlVector to entindexes
//-----------------------------------------------------------------------------
void RecvProxy_PlayerObjectList( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_TFPlayer *pPlayer = (C_TFPlayer*)pStruct;
	CBaseHandle *pHandle = (CBaseHandle*)(&(pPlayer->m_aObjects[pData->m_iElement])); 
	RecvProxy_IntToEHandle( pData, pStruct, pHandle );
}

void RecvProxyArrayLength_PlayerObjects( void *pStruct, int objectID, int currentArrayLength )
{
	C_TFPlayer *pPlayer = (C_TFPlayer*)pStruct;

	if ( pPlayer->m_aObjects.Count() != currentArrayLength )
	{
		pPlayer->m_aObjects.SetSize( currentArrayLength );
	}

	pPlayer->ForceUpdateObjectHudState();
}

// specific to the local player
BEGIN_RECV_TABLE_NOBASE( C_TFPlayer, DT_TFLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropArray2( 
		RecvProxyArrayLength_PlayerObjects,
		RecvPropInt( "player_object_array_element", 0, SIZEOF_IGNORE, 0, RecvProxy_PlayerObjectList ), 
		MAX_OBJECTS_PER_PLAYER, 
		0, 
		"player_object_array"	),

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
//	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
	RecvPropInt( RECVINFO( m_iSquadMemberCount ) ),
	RecvPropInt( RECVINFO( m_iSquadMedicCount ) ),
	RecvPropBool( RECVINFO( m_fSquadInFollowMode ) ),
	RecvPropEHandle( RECVINFO( m_hLadder ) ),
	RecvPropVector( RECVINFO( m_vecLocatorOrigin ) ),
	RecvPropString( RECVINFO( m_iszCustomModel ) ),

END_RECV_TABLE()

// all players except the local player
BEGIN_RECV_TABLE_NOBASE( C_TFPlayer, DT_TFNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),

END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_TFPlayer, DT_TFPlayer, CTFPlayer )

	RecvPropBool( RECVINFO( m_bSaveMeParity ) ),

	// This will create a race condition will the local player, but the data will be the same so.....
	RecvPropInt( RECVINFO( m_nWaterLevel ) ),
	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),
	RecvPropDataTable( RECVINFO_DT( m_PlayerClass ), 0, &REFERENCE_RECV_TABLE( DT_TFPlayerClassShared ) ),
	RecvPropDataTable( RECVINFO_DT( m_Shared ), 0, &REFERENCE_RECV_TABLE( DT_TFPlayerShared ) ),
	RecvPropDataTable( RECVINFO_DT( m_AttributeManager ), 0, &REFERENCE_RECV_TABLE( DT_AttributeManager ) ),

	RecvPropEHandle( RECVINFO( m_hItem ) ),
	RecvPropEHandle( RECVINFO( m_hGrapplingHookTarget ) ),

	RecvPropDataTable( "tflocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_TFLocalPlayerExclusive) ),
	RecvPropDataTable( "tfnonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_TFNonLocalPlayerExclusive) ),

	RecvPropInt( RECVINFO( m_iSpawnCounter ) ),
	RecvPropBool( RECVINFO( m_bSearchingSpawn ) ),
	RecvPropInt( RECVINFO( m_nForceTauntCam ) ),
	RecvPropTime( RECVINFO( m_flLastDamageTime ) ),
	RecvPropTime( RECVINFO( m_flNextNoiseMakerTime ) ),
	RecvPropBool( RECVINFO( m_bTyping ) ),
	RecvPropBool( RECVINFO( m_bHasLongJump ) ),

	RecvPropBool( RECVINFO( m_bAllowMoveDuringTaunt ) ),
	RecvPropBool( RECVINFO( m_bIsReadyToHighFive ) ),
	RecvPropEHandle( RECVINFO( m_hHighFivePartner ) ),
	RecvPropFloat( RECVINFO( m_flCurrentTauntMoveSpeed ) ),
	RecvPropFloat( RECVINFO( m_flTauntYaw ) ),
	RecvPropInt( RECVINFO( m_iTauntItemDefIndex ) ),
	RecvPropInt( RECVINFO( m_nActiveTauntSlot ) ),

	RecvPropFloat( RECVINFO( m_flHeadScale ) ),
	RecvPropFloat( RECVINFO( m_flTorsoScale ) ),
	RecvPropFloat( RECVINFO( m_flHandScale ) ),

	RecvPropBool( RECVINFO( m_bHeldObjectOnOppositeSideOfPortal ) ),
	RecvPropEHandle( RECVINFO( m_pHeldObjectPortal ) ),
	RecvPropBool( RECVINFO( m_bPitchReorientation ) ),
	RecvPropEHandle( RECVINFO( m_hPortalEnvironment ) ),
	RecvPropEHandle( RECVINFO( m_hSurroundingLiquidPortal ) ),

END_RECV_TABLE()


BEGIN_PREDICTION_DATA( C_TFPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_Shared, CTFPlayerShared ),
	DEFINE_PRED_FIELD( m_nSkin, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_nBody, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_ARRAY_TOL( m_flEncodedController, FIELD_FLOAT, MAXSTUDIOBONECTRLS, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE, 0.02f ),
	DEFINE_PRED_FIELD( m_nNewSequenceParity, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nResetEventsParity, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nMuzzleFlashParity, FIELD_CHARACTER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE  ),
	DEFINE_PRED_FIELD( m_hOffHandWeapon, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hLadder, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hPortalEnvironment, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

extern bool g_bUpsideDown;

// ------------------------------------------------------------------------------------------ //
// C_TFPlayer implementation.
// ------------------------------------------------------------------------------------------ //

C_TFPlayer::C_TFPlayer() : 
	m_iv_angEyeAngles( "C_TFPlayer::m_iv_angEyeAngles" )
{
	m_pAttributes = this;

	m_PlayerAnimState = CreateTFPlayerAnimState( this );
	m_Shared.Init( this );

	m_iIDEntIndex = 0;

	m_angEyeAngles.Init();
	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_pTeleporterEffect = NULL;
	m_pBurningSound = NULL;
	m_pBurningEffect = NULL;
	m_flBurnEffectStartTime = 0;
	m_flBurnEffectEndTime = 0;
	m_pDisguisingEffect = NULL;
	m_pSaveMeEffect = NULL;
	m_pTypingEffect = NULL;
	m_pTauntWithMeEffect = NULL;
	m_pOverhealEffect = NULL;
	m_pDemoEyeEffect = NULL;

	m_aGibs.Purge();

	m_bCigaretteSmokeActive = false;

	m_hRagdoll.Set( NULL );

	m_iPreviousMetal = 0;
	m_bIsDisplayingNemesisIcon = false;

	m_bWasTaunting = false;
	m_flTauntOffTime = 0.0f;
	m_angTauntPredViewAngles.Init();
	m_angTauntEngViewAngles.Init();

	m_flWaterImpactTime = 0.0f;

	m_flWaterEntryTime = 0;
	m_nOldWaterLevel = WL_NotInWater;
	m_bWaterExitEffectActive = false;

	m_bUpdateObjectHudState = false;

	m_bTyping = false;

	ListenForGameEvent( "localplayer_changeteam" );
	ListenForGameEvent( "post_inventory_application" );

	ConVarRef scissor("r_flashlightscissor");
	if ( scissor.GetBool() )
		scissor.SetValue("0");

	m_iSquadMemberCount = 0;
	m_iSquadMedicCount = 0;
	m_fSquadInFollowMode = false;
	m_vecLocatorOrigin = vec3_origin;

	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;

	m_bPitchReorientation = false;
	m_fReorientationRate = 0.0f;

	// Character Lip Sync
	engine->AddPhonemeFile( "scripts/gamesounds/game_sounds_vo_phonemes.txt" );
	engine->AddPhonemeFile( "scripts/gamesounds/game_sounds_vo_phonemes_local.txt" );
	engine->AddPhonemeFile( 0 ); //not sure about this but livetf2 does it
}

C_TFPlayer::~C_TFPlayer()
{
	ShowNemesisIcon( false );
	m_PlayerAnimState->Release();
}


C_TFPlayer* C_TFPlayer::GetLocalTFPlayer()
{
	return ToTFPlayer( C_BasePlayer::GetLocalPlayer() );
}

const QAngle& C_TFPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateOnRemove( void )
{
	// Stop the taunt.
	if ( m_bWasTaunting )
	{
		TurnOffTauntCam();
	}

	// HACK!!! ChrisG needs to fix this in the particle system.
	ParticleProp()->OwnerSetDormantTo( true );
	ParticleProp()->StopParticlesInvolving( this );

	m_Shared.RemoveAllCond();

	m_Shared.UpdateCritBoostEffect( true );

	if ( IsLocalPlayer() )
	{
		CTFStatPanel *pStatPanel = GetStatPanel();
		pStatPanel->OnLocalPlayerRemove( this );
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: returns max health for this player
//-----------------------------------------------------------------------------
int C_TFPlayer::GetMaxHealth( void ) const
{
	C_TF_PlayerResource *tf_PR = GetTFPlayerResource();
	if ( tf_PR )
	{
		int index = ( (C_BasePlayer *) this )->entindex();
		return tf_PR->GetMaxHealth( index );
	}
	return TF_HEALTH_UNDEFINED;
}

int C_TFPlayer::GetMaxHealthForBuffing( void ) const
{
	if (g_PR)
	{
		C_TF_PlayerResource *tf_PR = dynamic_cast<C_TF_PlayerResource *>( g_PR );
		if (tf_PR)
		{
			int index = ( (C_BasePlayer *)this )->entindex();
			return tf_PR->GetMaxHealthForBuffing( index );
		}
	}
	return TF_HEALTH_UNDEFINED;
}

//-----------------------------------------------------------------------------
// Deal with recording
//-----------------------------------------------------------------------------
void C_TFPlayer::GetToolRecordingState( KeyValues *msg )
{
	BaseClass::GetToolRecordingState( msg );
	BaseEntityRecordingState_t *pBaseEntityState = (BaseEntityRecordingState_t*)msg->GetPtr( "baseentity" );

	bool bDormant = IsDormant();
	bool bDead = !IsAlive();
	bool bSpectator = ( GetTeamNumber() == TEAM_SPECTATOR );
	bool bNoRender = ( GetRenderMode() == kRenderNone );
	bool bDeathCam = (GetObserverMode() == OBS_MODE_DEATHCAM);
	bool bNoDraw = IsEffectActive(EF_NODRAW);

	bool bVisible = 
		!bDormant && 
		!bDead && 
		!bSpectator &&
		!bNoRender &&
		!bDeathCam &&
		!bNoDraw;

	bool changed = m_bToolRecordingVisibility != bVisible;
	// Remember state
	m_bToolRecordingVisibility = bVisible;

	pBaseEntityState->m_bVisible = bVisible;
	if ( changed && !bVisible )
	{
		// If the entity becomes invisible this frame, we still want to record a final animation sample so that we have data to interpolate
		//  toward just before the logs return "false" for visiblity.  Otherwise the animation will freeze on the last frame while the model
		//  is still able to render for just a bit.
		pBaseEntityState->m_bRecordFinalVisibleSample = true;
	}
}


void C_TFPlayer::UpdateClientSideAnimation()
{
	// Update the animation data. It does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).

	if ( this == C_TFPlayer::GetLocalTFPlayer() )
		m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
	else
		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	BaseClass::UpdateClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::SetDormant( bool bDormant )
{
	// If I'm burning, stop the burning sounds
	if ( !IsDormant() && bDormant )
	{
		if ( m_pBurningSound ) 
		{
			StopBurningSound();
		}
		if ( m_bIsDisplayingNemesisIcon )
		{
			ShowNemesisIcon( false );
		}
	}

	if ( IsDormant() && !bDormant )
	{
		m_bUpdatePartyHat = true;
	}

	m_Shared.UpdateCritBoostEffect();

	// Deliberately skip base player.
	C_BaseEntity::SetDormant( bDormant );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::OnPreDataChanged( DataUpdateType_t updateType )
{
	Assert( m_pPortalEnvironment_LastCalcView == m_hPortalEnvironment.Get() );
	PreDataChanged_Backup.m_hPortalEnvironment = m_hPortalEnvironment;
	PreDataChanged_Backup.m_hSurroundingLiquidPortal = m_hSurroundingLiquidPortal;

	BaseClass::OnPreDataChanged( updateType );

	m_iOldHealth = m_iHealth;
	m_iOldPlayerClass = m_PlayerClass.GetClassIndex();
	m_iOldState = m_Shared.GetCond();
	m_iOldSpawnCounter = m_iSpawnCounter;
	m_bOldSaveMeParity = m_bSaveMeParity;
	m_nOldWaterLevel = GetWaterLevel();

	m_iOldTeam = GetTeamNumber();
	C_TFPlayerClass *pClass = GetPlayerClass();
	m_iOldClass = pClass ? pClass->GetClassIndex() : TF_CLASS_UNDEFINED;
	m_bDisguised = m_Shared.InCond( TF_COND_DISGUISED );
	m_iOldDisguiseTeam = m_Shared.GetDisguiseTeam();
	m_iOldDisguiseClass = m_Shared.GetDisguiseClass();
	m_hOldActiveWeapon.Set( GetActiveTFWeapon() );

	m_Shared.OnPreDataChanged();
}

void C_TFPlayer::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit( state );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::OnDataChanged( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );

	BaseClass::OnDataChanged( updateType );

	if( m_hSurroundingLiquidPortal != PreDataChanged_Backup.m_hSurroundingLiquidPortal )
	{
		CLiquidPortal_InnerLiquidEffect *pLiquidEffect = (CLiquidPortal_InnerLiquidEffect *)g_pScreenSpaceEffects->GetScreenSpaceEffect( "LiquidPortal_InnerLiquid" );
		if( pLiquidEffect )
		{
			C_Func_LiquidPortal *pSurroundingPortal = m_hSurroundingLiquidPortal.Get();
			if( pSurroundingPortal != NULL )
			{
				C_Func_LiquidPortal *pOldSurroundingPortal = PreDataChanged_Backup.m_hSurroundingLiquidPortal.Get();
				if( pOldSurroundingPortal != pSurroundingPortal->m_hLinkedPortal.Get() )
				{
					pLiquidEffect->m_pImmersionPortal = pSurroundingPortal;
					pLiquidEffect->m_bFadeBackToReality = false;
				}
				else
				{
					pLiquidEffect->m_bFadeBackToReality = true;
					pLiquidEffect->m_fFadeBackTimeLeft = pLiquidEffect->s_fFadeBackEffectTime;
				}
			}
			else
			{
				pLiquidEffect->m_pImmersionPortal = NULL;
				pLiquidEffect->m_bFadeBackToReality = false;
			}
		}		
	}

	DetectAndHandlePortalTeleportation();

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		InitInvulnerableMaterial();
	}
	else
	{
		if ( m_iOldTeam != GetTeamNumber() || m_iOldDisguiseTeam != m_Shared.GetDisguiseTeam() )
		{
			InitInvulnerableMaterial();
			m_bUpdatePartyHat = true;
		}

		UpdateWearables();
	}

	CTFWeaponBase *pActiveWpn = GetActiveTFWeapon();
	if ( pActiveWpn )
	{
		if ( m_hOldActiveWeapon.Get() == NULL ||
			pActiveWpn != m_hOldActiveWeapon.Get() ||
			m_iOldPlayerClass != m_PlayerClass.GetClassIndex() )
		{
			pActiveWpn->SetViewModel();

			if ( ShouldDrawThisPlayer() )
			{
				m_Shared.UpdateCritBoostEffect();
			}
		}
	}

	// Check for full health and remove decals.
	if ( ( m_iHealth > m_iOldHealth && m_iHealth >= GetMaxHealth() ) || m_Shared.IsInvulnerable() )
	{
		// If we were just fully healed, remove all decals
		RemoveAllDecals();
	}

	// Detect class changes
	if ( m_iOldPlayerClass != m_PlayerClass.GetClassIndex() )
	{
		OnPlayerClassChange();
	}

	bool bJustSpawned = false;

	if ( m_iOldSpawnCounter != m_iSpawnCounter )
	{
		ClientPlayerRespawn();

		bJustSpawned = true;
		m_bUpdatePartyHat = true;
	}

	if ( m_bSaveMeParity != m_bOldSaveMeParity )
	{
		// Player has triggered a save me command
		if ( m_Shared.InCond( TF_COND_BURNING ) )
			CreateSaveMeEffect( TF_CALL_BURNING );
		else if ( m_Shared.InCond( TF_COND_BLEEDING ) )
			CreateSaveMeEffect( TF_CALL_BLEEDING );
		else if ( GetHealth() < 0.25 * GetPlayerClass()->GetMaxHealth() )
			CreateSaveMeEffect( TF_CALL_HEALTH );
		else
			CreateSaveMeEffect( TF_CALL_MEDIC );
	}

	UpdateTypingBubble();

	if ( m_Shared.InCond( TF_COND_BURNING ) && !m_pBurningSound )
	{
		StartBurningSound();
	}

	// See if we should show or hide nemesis icon for this player
	bool bShouldDisplayNemesisIcon = ShouldShowNemesisIcon();
	if ( bShouldDisplayNemesisIcon != m_bIsDisplayingNemesisIcon )
	{
		ShowNemesisIcon( bShouldDisplayNemesisIcon );
	}

	m_Shared.OnDataChanged();

	
	if ( m_bDisguised != m_Shared.InCond( TF_COND_DISGUISED ) )
	{
		m_flDisguiseEndEffectStartTime = max( m_flDisguiseEndEffectStartTime, gpGlobals->curtime );
	}

	int nNewWaterLevel = GetWaterLevel();

	if ( nNewWaterLevel != m_nOldWaterLevel )
	{
		if ( ( m_nOldWaterLevel == WL_NotInWater ) && ( nNewWaterLevel > WL_NotInWater ) )
		{
			// Set when we do a transition to/from partially in water to completely out
			m_flWaterEntryTime = gpGlobals->curtime;
		}

		// If player is now up to his eyes in water and has entered the water very recently (not just bobbing eyes in and out), play a bubble effect.
		if ( ( nNewWaterLevel == WL_Eyes ) && ( gpGlobals->curtime - m_flWaterEntryTime ) < 0.5f ) 
		{
			CNewParticleEffect *pEffect = ParticleProp()->Create( "water_playerdive", PATTACH_ABSORIGIN_FOLLOW );
			ParticleProp()->AddControlPoint( pEffect, 1, NULL, PATTACH_WORLDORIGIN, NULL, WorldSpaceCenter() );
		}		
		// If player was up to his eyes in water and is now out to waist level or less, play a water drip effect
		else if ( m_nOldWaterLevel == WL_Eyes && ( nNewWaterLevel < WL_Eyes ) && !bJustSpawned )
		{
			CNewParticleEffect *pWaterExitEffect = ParticleProp()->Create( "water_playeremerge", PATTACH_ABSORIGIN_FOLLOW );
			ParticleProp()->AddControlPoint( pWaterExitEffect, 1, this, PATTACH_ABSORIGIN_FOLLOW );
			m_bWaterExitEffectActive = true;
		}
	}

	if ( IsLocalPlayer() )
	{
		if ( updateType == DATA_UPDATE_CREATED )
		{
			SetupHeadLabelMaterials();
			GetClientVoiceMgr()->SetHeadLabelOffset( 50 );
		}

		if ( m_iOldTeam != GetTeamNumber() )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "localplayer_changeteam" );
			if ( event )
			{
				gameeventmanager->FireEventClientSide( event );
			}
		}

		if ( !IsPlayerClass(m_iOldClass) )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "localplayer_changeclass" );
			if ( event )
			{
				event->SetInt( "updateType", updateType );
				gameeventmanager->FireEventClientSide( event );
			}
		}

		if ( m_iOldClass == TF_CLASS_SPY && 
		   ( m_bDisguised != m_Shared.InCond( TF_COND_DISGUISED ) || m_iOldDisguiseClass != m_Shared.GetDisguiseClass() ) )
		{
			IGameEvent *event = gameeventmanager->CreateEvent( "localplayer_changedisguise" );
			if ( event )
			{
				event->SetBool( "disguised", m_Shared.InCond( TF_COND_DISGUISED ) );
				gameeventmanager->FireEventClientSide( event );
			}
		}

		// If our metal amount changed, send a game event
		int iCurrentMetal = GetAmmoCount( TF_AMMO_METAL );	

		if ( iCurrentMetal != m_iPreviousMetal )
		{
			//msg
			IGameEvent *event = gameeventmanager->CreateEvent( "player_account_changed" );
			if ( event )
			{
				event->SetInt( "old_account", m_iPreviousMetal );
				event->SetInt( "new_account", iCurrentMetal );
				gameeventmanager->FireEventClientSide( event );
			}

			m_iPreviousMetal = iCurrentMetal;
		}
	}

	// Some time in this network transmit we changed the size of the object array.
	// recalc the whole thing and update the hud
	if ( m_bUpdateObjectHudState )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "building_info_changed" );
		if ( event )
		{
			event->SetInt( "building_type", -1 );
			event->SetInt( "object_mode", OBJECT_MODE_NONE );
			gameeventmanager->FireEventClientSide( event );
		}
	
		m_bUpdateObjectHudState = false;
	}
}

extern ConVar cl_first_person_uses_world_model;
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed );

	if ( m_flHeadScale != 1.0f )
	{
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
		BuildNeckScaleTransformations( this, m_flHeadScale );
		BuildBigHeadTransformation( this, pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed, m_flHeadScale );
	}

	if ( m_flTorsoScale != 1.0f )
	{
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
		BuildTorsoScaleTransformations( this, m_flTorsoScale );
	}

	if ( m_flHandScale != 1.0f )
	{
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
		BuildHandScaleTransformations( this, m_flHandScale );
	}

	if ( ( cl_first_person_uses_world_model.GetBool() || ( GetActiveWeapon() == NULL ) ) || ( tf_taunt_first_person.GetBool() && m_Shared.InCond( TF_COND_TAUNTING ) ) )
		BuildFirstPersonMeathookTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed, "bip_head" );
}

//CalcView() gets called between OnPreDataChanged() and OnDataChanged(), and these changes need to be known about in both before CalcView() gets called, and if CalcView() doesn't get called
bool C_TFPlayer::DetectAndHandlePortalTeleportation( void )
{
	if( m_bPortalledMessagePending )
	{
		m_bPortalledMessagePending = false;

		//C_Prop_Portal *pOldPortal = PreDataChanged_Backup.m_hPortalEnvironment.Get();
		//Assert( pOldPortal );
		//if( pOldPortal )
		{
			Vector ptNewPosition = GetNetworkOrigin();

			UTIL_Portal_PointTransform( m_PendingPortalMatrix, PortalEyeInterpolation.m_vEyePosition_Interpolated, PortalEyeInterpolation.m_vEyePosition_Interpolated );
			UTIL_Portal_PointTransform( m_PendingPortalMatrix, PortalEyeInterpolation.m_vEyePosition_Uninterpolated, PortalEyeInterpolation.m_vEyePosition_Uninterpolated );

			PortalEyeInterpolation.m_bEyePositionIsInterpolating = true;

			UTIL_Portal_AngleTransform( m_PendingPortalMatrix, m_qEyeAngles_LastCalcView, m_angEyeAngles );
			m_angEyeAngles.x = AngleNormalize( m_angEyeAngles.x );
			m_angEyeAngles.y = AngleNormalize( m_angEyeAngles.y );
			m_angEyeAngles.z = AngleNormalize( m_angEyeAngles.z );
			m_iv_angEyeAngles.Reset(); //copies from m_angEyeAngles

			if( engine->IsPlayingDemo() )
			{				
				pl.v_angle = m_angEyeAngles;		
				engine->SetViewAngles( pl.v_angle );
			}

			engine->ResetDemoInterpolation();
			if( IsLocalPlayer() ) 
			{
				//DevMsg( "FPT: %.2f %.2f %.2f\n", m_angEyeAngles.x, m_angEyeAngles.y, m_angEyeAngles.z );
				SetLocalAngles( m_angEyeAngles );
			}

			m_PlayerAnimState->Teleport ( &ptNewPosition, &GetNetworkAngles(), this );

			// Reorient last facing direction to fix pops in view model lag
			for ( int i = 0; i < MAX_VIEWMODELS; i++ )
			{
				CBaseViewModel *vm = GetViewModel( i );
				if ( !vm )
					continue;

				UTIL_Portal_VectorTransform( m_PendingPortalMatrix, vm->m_vecLastFacing, vm->m_vecLastFacing );
			}
		}
		m_bPortalledMessagePending = false;
	}

	return false;
}

void C_TFPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );

	/*if ( m_iSpawnInterpCounter != m_iSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();
		m_iSpawnInterpCounterCache = m_iSpawnInterpCounter;
	}*/

	BaseClass::PostDataUpdate( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::InitInvulnerableMaterial( void )
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalPlayer )
		return;

	const char *pszMaterial = NULL;

		int iVisibleTeam = GetTeamNumber();
		// if this player is disguised and on the other team, use disguise team
		if ( m_Shared.InCond( TF_COND_DISGUISED ) && IsEnemyPlayer() )
		{
			iVisibleTeam = m_Shared.GetDisguiseTeam();
		}

		switch ( iVisibleTeam )
		{
		case TF_TEAM_RED:
			pszMaterial = "models/effects/invulnfx_red.vmt";
			break;
		case TF_TEAM_BLUE:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		case TF_TEAM_GREEN:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		case TF_TEAM_YELLOW:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		default:
			break;
		}

	if ( pszMaterial )
	{
		m_InvulnerableMaterial.Init( pszMaterial, TEXTURE_GROUP_CLIENT_EFFECTS );
	}
	else
	{
		m_InvulnerableMaterial.Shutdown();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void C_TFPlayer::OverrideView( CViewSetup *pSetup )
{
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		// adnan
		if(pWeapon->OverrideViewAngles()) 
		{
			// use the useAngles!
				// override with the angles the server sends to us as useAngles
				// use the useAngles only if we're holding and rotating with the grav gun
			pSetup->angles = m_vecUseAngles;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::StartBurningSound( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if ( !m_pBurningSound )
	{
		CLocalPlayerFilter filter;
		m_pBurningSound = controller.SoundCreate( filter, entindex(), "Player.OnFire" );
	}

	controller.Play( m_pBurningSound, 0.0, 100 );
	controller.SoundChangeVolume( m_pBurningSound, 1.0, 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::StopBurningSound( void )
{
	if ( m_pBurningSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pBurningSound );
		m_pBurningSound = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::GetGlowEffectColor( byte *r, byte *g, byte *b, byte *a )
{
	switch ( GetTeamNumber() )
	{
		case TF_TEAM_BLUE:
			*r = 0.49f; *g = 0.66f; *b = 0.7699971f;
			break;

		case TF_TEAM_RED:
			*r = 0.74f; *g = 0.23f; *b = 0.23f;
			break;

		case TF_TEAM_GREEN:
			*r = 0.03f; *g = 0.68f; *b = 0;
			break;

		case TF_TEAM_YELLOW:
			*r = 1.0f; *g = 0.62f; *b = 0;
			break;

		default:
			*r = 0.76f; *g = 0.76f; *b = 0.76f;
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateRecentlyTeleportedEffect( void )
{
	if ( m_Shared.ShouldShowRecentlyTeleported() )
	{
		if ( !m_pTeleporterEffect )
		{
			int iTeam = GetTeamNumber();
			if ( m_Shared.InCond( TF_COND_DISGUISED ) )
			{
				iTeam = m_Shared.GetDisguiseTeam();
			}

			const char *pszEffect = ConstructTeamParticle( "player_recent_teleport_%s", iTeam );

			if ( pszEffect )
			{
				m_pTeleporterEffect = ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
			}
		}
	}
	else
	{
		if ( m_pTeleporterEffect )
		{
			ParticleProp()->StopEmission( m_pTeleporterEffect );
			m_pTeleporterEffect = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Kill viewmodel particle effects
//-----------------------------------------------------------------------------
void C_TFPlayer::StopViewModelParticles( C_BaseEntity *pEntity )
{
	if ( pEntity && pEntity->ParticleProp() )
	{
		pEntity->ParticleProp()->StopParticlesInvolving( pEntity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::OnPlayerClassChange( void )
{
	// Init the anim movement vars
	m_PlayerAnimState->SetRunSpeed( GetPlayerClass()->GetMaxSpeed() );
	m_PlayerAnimState->SetWalkSpeed( GetPlayerClass()->GetMaxSpeed() * 0.5 );

	if ( IsLocalPlayer() )
	{
		// Execute the class cfg
		char szCommand[128];
		Q_snprintf( szCommand, sizeof( szCommand ), "exec %s.cfg\n", GetPlayerClass()->GetName() );
		engine->ExecuteClientCmd( szCommand );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::InitPhonemeMappings()
{
	CStudioHdr *pStudio = GetModelPtr();
	if ( pStudio )
	{
		char szBasename[MAX_PATH];
		Q_StripExtension( pStudio->pszName(), szBasename, sizeof( szBasename ) );
		char szExpressionName[MAX_PATH];
		Q_snprintf( szExpressionName, sizeof( szExpressionName ), "%s/phonemes/phonemes", szBasename );
		if ( FindSceneFile( szExpressionName ) )
		{
			SetupMappings( szExpressionName );	
		}
		else
		{
			BaseClass::InitPhonemeMappings();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::ResetFlexWeights( CStudioHdr *pStudioHdr )
{
	if ( !pStudioHdr || pStudioHdr->numflexdesc() == 0 )
		return;

	// Reset the flex weights to their starting position.
	LocalFlexController_t iController;
	for ( iController = LocalFlexController_t(0); iController < pStudioHdr->numflexcontrollers(); ++iController )
	{
		SetFlexWeight( iController, 0.0f );
	}

	// Reset the prediction interpolation values.
	m_iv_flexWeight.Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CStudioHdr *C_TFPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	// Reset attachments
	DestroyBoneAttachments();

	// Initialize the gibs.
	InitPlayerGibs();

	InitializePoseParams();

	// Init flexes, cancel any scenes we're playing
	ClearSceneEvents( NULL, false );

	// Reset the flex weights.
	ResetFlexWeights( hdr );

	// Reset the players animation states, gestures
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->OnNewModel();
	}

	if ( hdr )
	{
		InitPhonemeMappings();
	}

	if ( IsPlayerClass( TF_CLASS_SPY ) )
	{
		m_iSpyMaskBodygroup = FindBodygroupByName( "spyMask" );
	}
	else
	{
		m_iSpyMaskBodygroup = -1;
	}

	m_bUpdatePartyHat = true;

	return hdr;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdatePartyHat( void )
{
	if ( TFGameRules() && ( TFGameRules()->IsBirthday() || TFGameRules()->IsLFBirthday() ) && !IsLocalPlayer() && IsAlive() && 
		GetTeamNumber() >= FIRST_GAME_TEAM && !IsPlayerClass(TF_CLASS_UNDEFINED) )
	{
		if ( m_hPartyHat )
		{
			m_hPartyHat->Release();
		}

		m_hPartyHat = C_PlayerAttachedModel::Create( BDAY_HAT_MODEL, this, LookupAttachment("partyhat"), vec3_origin, PAM_PERMANENT, 0 );

		// C_PlayerAttachedModel::Create can return NULL!
		if ( m_hPartyHat )
		{
			int iVisibleTeam = GetTeamNumber();
			if ( m_Shared.InCond( TF_COND_DISGUISED ) && IsEnemyPlayer() )
			{
				iVisibleTeam = m_Shared.GetDisguiseTeam();
			}
			m_hPartyHat->m_nSkin = iVisibleTeam - 2;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Is this player an enemy to the local player
//-----------------------------------------------------------------------------
bool C_TFPlayer::IsEnemyPlayer( void )
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();

	if ( !pLocalPlayer )
		return false;

	switch ( pLocalPlayer->GetTeamNumber() )
	{
	case TF_TEAM_RED:
		return ( GetTeamNumber() == TF_TEAM_BLUE || GetTeamNumber() == TF_TEAM_GREEN || GetTeamNumber() == TF_TEAM_YELLOW );

	case TF_TEAM_BLUE:
		return ( GetTeamNumber() == TF_TEAM_RED || GetTeamNumber() == TF_TEAM_GREEN || GetTeamNumber() == TF_TEAM_YELLOW );

	case TF_TEAM_GREEN:
		return ( GetTeamNumber() == TF_TEAM_RED || GetTeamNumber() == TF_TEAM_BLUE || GetTeamNumber() == TF_TEAM_YELLOW );

	case TF_TEAM_YELLOW:
		return ( GetTeamNumber() == TF_TEAM_RED || GetTeamNumber() == TF_TEAM_BLUE || GetTeamNumber() == TF_TEAM_GREEN );

	default:
		break;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Displays a nemesis icon on this player to the local player
//-----------------------------------------------------------------------------
void C_TFPlayer::ShowNemesisIcon( bool bShow )
{
	if ( bShow )
	{
		const char *pszEffect = ConstructTeamParticle( "particle_nemesis_%s", GetTeamNumber() );
		ParticleProp()->Create( pszEffect, PATTACH_POINT_FOLLOW, "head" );
	}
	else
	{
		// stop effects for both team colors (to make sure we remove effects in event of team change)
		ParticleProp()->StopParticlesNamed( "particle_nemesis_red", true );
		ParticleProp()->StopParticlesNamed( "particle_nemesis_blue", true );
	}
	m_bIsDisplayingNemesisIcon = bShow;
}

#define	TF_TAUNT_PITCH	0
#define TF_TAUNT_YAW	1
#define TF_TAUNT_DIST	2

#define TF_TAUNT_MAXYAW		135
#define TF_TAUNT_MINYAW		-135
#define TF_TAUNT_MAXPITCH	90
#define TF_TAUNT_MINPITCH	0
#define TF_TAUNT_IDEALLAG	4.0f

static Vector TF_TAUNTCAM_HULL_MIN( -9.0f, -9.0f, -9.0f );
static Vector TF_TAUNTCAM_HULL_MAX( 9.0f, 9.0f, 9.0f );

static ConVar tf_tauntcam_yaw( "tf_tauntcam_yaw", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
static ConVar tf_tauntcam_pitch( "tf_tauntcam_pitch", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
static ConVar tf_tauntcam_dist( "tf_tauntcam_dist", "150", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

ConVar setcamerathird("setcamerathird", "0", 0);

extern ConVar cam_idealdist;
extern ConVar cam_idealdistright;
extern ConVar cam_idealdistup;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::TurnOnTauntCam( void )
{
	if ( !IsLocalPlayer() )
		return;

	if ( tf_taunt_first_person.GetBool() )
	{
		ThirdPersonSwitch( false );
		return;
	}

	// Already in third person?
	if ( g_ThirdPersonManager.WantToUseGameThirdPerson() )
		return;

	// Save the old view angles.
	/*engine->GetViewAngles( m_angTauntEngViewAngles );
	prediction->GetViewAngles( m_angTauntPredViewAngles );*/

	float flTauntCamDist = tf_tauntcam_dist.GetFloat();
	if ( m_iTauntItemDefIndex != 0 )
	{
		CEconItemDefinition *pItemDef = GetItemSchema()->GetItemDefinition( m_iTauntItemDefIndex );
		if ( pItemDef && pItemDef->camera_dist_up )
		{
			flTauntCamDist += pItemDef->camera_dist_up;
		}
	}

	// If we're already in taunt cam just reset the distance.
	if ( m_bWasTaunting )
	{
		g_ThirdPersonManager.SetDesiredCameraOffset( Vector( flTauntCamDist, 0.0f, 0.0f ) );
		return;
	}

	m_TauntCameraData.m_flPitch = tf_tauntcam_pitch.GetFloat();
	m_TauntCameraData.m_flYaw =  tf_tauntcam_yaw.GetFloat();
	m_TauntCameraData.m_flDist = flTauntCamDist;
	m_TauntCameraData.m_flLag = 4.0f;
	m_TauntCameraData.m_vecHullMin.Init( -9.0f, -9.0f, -9.0f );
	m_TauntCameraData.m_vecHullMax.Init( 9.0f, 9.0f, 9.0f );

	QAngle vecCameraOffset( tf_tauntcam_pitch.GetFloat(), tf_tauntcam_yaw.GetFloat(), flTauntCamDist );

	g_ThirdPersonManager.SetDesiredCameraOffset( Vector( flTauntCamDist, 0.0f, 0.0f ) );
	g_ThirdPersonManager.SetOverridingThirdPerson( true );
	::input->CAM_ToThirdPerson();
	ThirdPersonSwitch( true );

	::input->CAM_SetCameraThirdData( &m_TauntCameraData, vecCameraOffset );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::TurnOffTauntCam( void )
{
	m_bWasTaunting = false;
	m_flTauntOffTime = 0.0f;

	if ( !IsLocalPlayer() )
		return;	

	/*Vector vecOffset = g_ThirdPersonManager.GetCameraOffsetAngles();

	tf_tauntcam_pitch.SetValue( vecOffset[PITCH] - m_angTauntPredViewAngles[PITCH] );
	tf_tauntcam_yaw.SetValue( vecOffset[YAW] - m_angTauntPredViewAngles[YAW] );*/

	g_ThirdPersonManager.SetOverridingThirdPerson( false );
	::input->CAM_SetCameraThirdData( NULL, vec3_angle );

	if ( g_ThirdPersonManager.WantToUseGameThirdPerson() )
	{
		ThirdPersonSwitch( true );
		return;
	}

	::input->CAM_ToFirstPerson();

	// Reset the old view angles.
	/*engine->SetViewAngles( m_angTauntEngViewAngles );
	prediction->SetViewAngles( m_angTauntPredViewAngles );*/

	// Force the feet to line up with the view direction post taunt.
	m_PlayerAnimState->m_bForceAimYaw = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::HandleTaunting( void )
{
	// Clear the taunt slot.
	if ( ( !m_bWasTaunting || m_flTauntOffTime != 0.0f ) && (
		m_Shared.InCond( TF_COND_TAUNTING ) ||
		m_Shared.IsLoser() ||
		m_Shared.IsControlStunned() ||
		m_nForceTauntCam || 
		m_Shared.InCond( TF_COND_HALLOWEEN_BOMB_HEAD ) ||
		m_Shared.InCond( TF_COND_HALLOWEEN_GIANT ) ||
		m_Shared.InCond( TF_COND_HALLOWEEN_TINY ) ||
		m_Shared.InCond( TF_COND_HALLOWEEN_GHOST_MODE ) ) )
	{
		// Handle the camera for the local player.
		TurnOnTauntCam();

		m_bWasTaunting = true;
		m_flTauntOffTime = 0.0f;
	}

	if ( m_bWasTaunting && m_flTauntOffTime == 0.0f && (
		!m_Shared.InCond( TF_COND_TAUNTING ) &&
		!m_Shared.IsLoser() &&
		!m_Shared.IsControlStunned() &&
		!m_nForceTauntCam &&
		!m_Shared.InCond( TF_COND_PHASE ) &&
		!m_Shared.InCond( TF_COND_HALLOWEEN_BOMB_HEAD ) &&
		!m_Shared.InCond( TF_COND_HALLOWEEN_THRILLER ) &&
		!m_Shared.InCond( TF_COND_HALLOWEEN_GIANT ) &&
		!m_Shared.InCond( TF_COND_HALLOWEEN_TINY ) &&
		!m_Shared.InCond( TF_COND_HALLOWEEN_GHOST_MODE ) ) )
	{
		m_flTauntOffTime = gpGlobals->curtime;

		// Clear the vcd slot.
		m_PlayerAnimState->ResetGestureSlot( GESTURE_SLOT_VCD );
	}

	TauntCamInterpolation();
}

//---------------------------------------------------------------------------- -
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::TauntCamInterpolation( void )
{
	if ( m_flTauntOffTime != 0.0f )
	{
		// Pull the camera back in over the course of half a second.
		float flDist = RemapValClamped( gpGlobals->curtime - m_flTauntOffTime, 0.0f, 0.4f, tf_tauntcam_dist.GetFloat(), 0.0f );

		if ( flDist == 0.0f || !m_bWasTaunting || !IsAlive() || g_ThirdPersonManager.WantToUseGameThirdPerson() )
		{
			// Snap the camera back into first person.
			TurnOffTauntCam();
		}
		else
		{
			g_ThirdPersonManager.SetDesiredCameraOffset( Vector( flDist, 0.0f, 0.0f ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::ThirdPersonSwitch( bool bThirdPerson )
{
	BaseClass::ThirdPersonSwitch( bThirdPerson );

	if ( bThirdPerson )
	{
		if ( g_ThirdPersonManager.WantToUseGameThirdPerson() )
		{
			vec_t vecOffset[3] = { 0.0f, 0.0f, 0.0f };
			string_t strOffset = cl_thirdperson_offset.GetString();
			if ( strOffset != NULL_STRING )
				UTIL_StringToVector( vecOffset, strOffset );

			Vector vecCamera( TF_CAMERA_DIST + vecOffset[0], TF_CAMERA_DIST_RIGHT + vecOffset[1], TF_CAMERA_DIST_UP + vecOffset[2] );

			// Flip the angle if viewmodels are flipped.
			if ( cl_flipviewmodels.GetBool() )
				vecCamera.y *= -1.0f;

			g_ThirdPersonManager.SetDesiredCameraOffset( vecCamera );
		}
	}

	// Update any effects affected by camera mode.
	m_Shared.UpdateCritBoostEffect();
	UpdateOverhealEffect();

	if ( GetViewModel() )
		GetViewModel()->UpdateVisibility();

	if ( m_hItem )
		m_hItem->UpdateVisibility();
}

void C_TFPlayer::CalcMinViewmodelOffset( void )
{
	for ( int i = 0; i < MAX_VIEWMODELS; i++ )
	{
		C_TFViewModel *vm = dynamic_cast<C_TFViewModel *>( GetViewModel( i ) );
		if ( !vm )
			continue;

		vm->CalcMinViewmodelOffset( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_TFPlayer::CanLightCigarette( void )
{
	// Start smoke if we're not invisible or disguised
	if ( IsPlayerClass( TF_CLASS_SPY ) && IsAlive() &&						// only on spy model
		( !m_Shared.InCond( TF_COND_DISGUISED ) || !IsEnemyPlayer() ) &&	// disguise doesn't show for teammates
		GetPercentInvisible() <= 0 &&										// don't start if invis
		!InFirstPersonView() && 											// don't show in first person view
		!m_Shared.InCond( TF_COND_DISGUISED_AS_DISPENSER ) &&				// don't show if we're a dispenser
		!TFGameRules()->IsTFCAllowed() &&									// don't show if tfc content is allowed
		!TFGameRules()->IsMvMModelsAllowed() )								// don't show if whe are using bot models
		return true;

	return false;
}

void C_TFPlayer::ClientThink()
{
	// Pass on through to the base class.
	BaseClass::ClientThink();

	FixTeleportationRoll();

	UpdateIDTarget();

	UpdateLookAt();

	// Handle invisibility.
	m_Shared.InvisibilityThink();

	m_Shared.ConditionThink();

	m_Shared.ClientShieldChargeThink();
	m_Shared.ClientDemoBuffThink();

	// Clear our healer, it'll be reset by the medigun client think if we're being healed
	m_hHealer = NULL;

	if ( CanLightCigarette() )
	{
		if ( !m_bCigaretteSmokeActive )
		{
			int iSmokeAttachment = LookupAttachment( "cig_smoke" );
			ParticleProp()->Create( "cig_smoke", PATTACH_POINT_FOLLOW, iSmokeAttachment );
			m_bCigaretteSmokeActive = true;
		}
	}
	else	// stop the smoke otherwise if its active
	{
		if ( m_bCigaretteSmokeActive )
		{
			ParticleProp()->StopParticlesNamed( "cig_smoke", false );
			m_bCigaretteSmokeActive = false;
		}
	}

	if ( m_bWaterExitEffectActive && !IsAlive() )
	{
		ParticleProp()->StopParticlesNamed( "water_playeremerge", false );
		m_bWaterExitEffectActive = false;
	}

	if ( m_bUpdatePartyHat )
	{
		UpdatePartyHat();
		m_bUpdatePartyHat = false;
	}

	if ( m_pSaveMeEffect )
	{
		// Kill the effect if either
		// a) the player is dead
		// b) the enemy disguised spy is now invisible

		if ( !IsAlive() || ( m_Shared.InCond( TF_COND_DISGUISED ) && IsEnemyPlayer() && ( GetPercentInvisible() > 0 ) ) )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_pSaveMeEffect );
			m_pSaveMeEffect = NULL;
		}
	}

	if ( lfe_enable_team_glow.GetBool() )
	{
		if ( IsAlive() && !IsLocalPlayer() && InSameTeam( GetLocalTFPlayer() ) && TFGameRules()->IsAnyCoOp() )
		{
			SetClientSideGlowEnabled( true );
			if ( GetHealth() < 0.75 * GetPlayerClass()->GetMaxHealth() && GetHealth() > 0.5 * GetPlayerClass()->GetMaxHealth() )
			{
				GetGlowObject()->SetColor( Vector(128, 128, 0) );
				GetGlowObject()->SetAlpha( 0.3f );
			}
			else if ( GetHealth() < 0.5 * GetPlayerClass()->GetMaxHealth() && GetHealth() > 0.25 * GetPlayerClass()->GetMaxHealth() )
			{
				GetGlowObject()->SetColor( Vector(128, 110, 0) );
				GetGlowObject()->SetAlpha( 0.3f );
			}
			else if ( GetHealth() < 0.25 * GetPlayerClass()->GetMaxHealth() )
			{
				GetGlowObject()->SetColor( Vector(128, 0, 0) );
				GetGlowObject()->SetAlpha( 0.3f );
			}
			else
			{
				GetGlowObject()->SetColor( Vector(0, 128, 0) );
				GetGlowObject()->SetAlpha( 0.3f );
			}
		}

		if ( ( !IsAlive() || IsPlayerDead() ) && !IsLocalPlayer() || IsAlive() && !IsLocalPlayer() && !InSameTeam( GetLocalTFPlayer() ) )
		{
			SetClientSideGlowEnabled( false );
			DestroyGlowEffect();
		}
	}
	else if ( TFGameRules()->IsInfectionMode() && GetTeamNumber() == TF_TEAM_BLUE )
	{
		if ( IsAlive() && !IsLocalPlayer() && !InSameTeam( GetLocalTFPlayer() ) )
		{
			SetClientSideGlowEnabled( true );
			GetGlowObject()->SetColor(Vector(128, 0, 0));
			GetGlowObject()->SetAlpha( 0.3f );
		}
		else
		{
			SetClientSideGlowEnabled( false );
			DestroyGlowEffect();
		}
	}
	else
	{
		if ( !IsLocalPlayer() && InSameTeam( GetLocalTFPlayer() ) )
		{
			SetClientSideGlowEnabled( false );
			DestroyGlowEffect();
		}
	}

	if ( IsAlive() && lfe_muzzlelight.GetBool() )
	{
		C_TFViewModel *pViewModel = dynamic_cast<C_TFViewModel *>( GetViewModel() );
		if ( m_Shared.IsInvulnerable() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = WorldSpaceCenter();

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 10; dl->color.g = 255; dl->color.b = 30; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 255; dl->color.g = 255; dl->color.b = 30; dl->style = 0;
				break;
			}

			dl->radius = 200.f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}

		if ( m_Shared.IsCritBoosted() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_Shared.m_hCritEffectHost.Get() )
				dl->origin = m_Shared.m_hCritEffectHost.Get()->GetAbsOrigin();

			if ( !ShouldDrawThisPlayer() )
			{
				if ( pViewModel )
					pViewModel->GetAttachment( pViewModel->LookupAttachment( "effect_hand_r" ), dl->origin );
			}

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 94; dl->color.g = 8; dl->color.b = 5; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 80; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 6; dl->color.g = 100; dl->color.b = 21; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 28; dl->color.g = 28; dl->color.b = 9; dl->style = 0;
			}

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}
		else if ( m_Shared.IsMiniCritBoosted() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_Shared.m_hCritEffectHost.Get() )
				dl->origin = m_Shared.m_hCritEffectHost.Get()->GetAbsOrigin();

			if ( !ShouldDrawThisPlayer() )
			{
				if ( pViewModel )
					pViewModel->GetAttachment( pViewModel->LookupAttachment( "effect_hand_r" ), dl->origin );
			}

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 94; dl->color.g = 8; dl->color.b = 5; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 80; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 100; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 28; dl->color.g = 28; dl->color.b = 9; dl->style = 0;
			}

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}
		else if ( m_Shared.InCond( TF_COND_SODAPOPPER_HYPE ) )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_Shared.m_hCritEffectHost.Get() )
				dl->origin = m_Shared.m_hCritEffectHost.Get()->GetAbsOrigin();

			if ( !ShouldDrawThisPlayer() )
			{
				if ( pViewModel )
					pViewModel->GetAttachment( pViewModel->LookupAttachment( "effect_hand_r" ), dl->origin );
			}

			dl->color.r = 50; dl->color.g = 2; dl->color.b = 50; dl->style = 0;

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}

		if ( m_Shared.InCond( TF_COND_BURNING ) )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_PLAYER_BRIGHT + index );
			dl->origin = WorldSpaceCenter();

			if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
			{
				dl->color.r = 230; dl->color.g = 65; dl->color.b = 245;
			}
			else
			{
				dl->color.r = 255; dl->color.g = 100; dl->color.b = 10;
			}

			dl->radius = 200.f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}

		if ( m_Shared.InCond( TF_COND_TAUNTING ) )
		{
			if ( m_iTauntItemDefIndex == 477 )
			{
				dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_PLAYER_BRIGHT + index );
				dl->origin = WorldSpaceCenter();
				dl->color.r = 255; dl->color.g = 255; dl->color.b = 255;
				dl->radius = 256.f;
				dl->decay = 512.0f;
				dl->die = gpGlobals->curtime + 0.01f;
			}
			else if ( m_iTauntItemDefIndex == 1015 )
			{
				dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_PLAYER_BRIGHT + index );
				dl->origin = WorldSpaceCenter();
				dl->color.r = 255; dl->color.g = 100; dl->color.b = 10;
				dl->radius = 256.f;
				dl->decay = 512.0f;
				dl->die = gpGlobals->curtime + 0.01f;
			}
		}
	}

	if ( ( !IsAlive() || IsPlayerDead() ) && IsLocalPlayer() )
	{
		if ( GetTeamNumber() != TEAM_SPECTATOR && GetObserverMode() != OBS_MODE_IN_EYE )
		{
			CTFViewModel *vm = dynamic_cast<CTFViewModel*>(GetViewModel ( 0 ) );
			if ( vm )
			{
				for ( int i = 0; i < MAX_VIEWMODELS; i++ )
				{
					vm->RemoveViewmodelAddon( i );
				}
			}
		}
	}

}

void C_TFPlayer::FixTeleportationRoll( void )
{
	if( IsInAVehicle() ) //HL2 compatibility fix. do absolutely nothing to the view in vehicles
		return;

	if( !IsLocalPlayer() )
		return;

	// Normalize roll from odd portal transitions
	QAngle vAbsAngles = EyeAngles();


	Vector vCurrentForward, vCurrentRight, vCurrentUp;
	AngleVectors( vAbsAngles, &vCurrentForward, &vCurrentRight, &vCurrentUp );

	if ( vAbsAngles[ROLL] == 0.0f )
	{
		m_fReorientationRate = 0.0f;
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
		return;
	}

	bool bForcePitchReorient = ( vAbsAngles[ROLL] > 175.0f && vCurrentForward.z > 0.99f );
	bool bOnGround = ( GetGroundEntity() != NULL );

	if ( bForcePitchReorient )
	{
		m_fReorientationRate = REORIENTATION_RATE * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
	}
	else
	{
		// Don't reorient in air if they don't want to
		if ( !cl_reorient_in_air.GetBool() && !bOnGround )
		{
			g_bUpsideDown = ( vCurrentUp.z < 0.0f );
			return;
		}
	}

	if ( vCurrentUp.z < 0.75f )
	{
		m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;

		// Upright faster if on the ground
		float fMaxReorientationRate = REORIENTATION_RATE * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
		if ( m_fReorientationRate > fMaxReorientationRate )
			m_fReorientationRate = fMaxReorientationRate;
	}
	else
	{
		if ( m_fReorientationRate > REORIENTATION_RATE * 0.5f )
		{
			m_fReorientationRate -= gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if ( m_fReorientationRate < REORIENTATION_RATE * 0.5f )
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		}
		else if ( m_fReorientationRate < REORIENTATION_RATE * 0.5f )
		{
			m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if ( m_fReorientationRate > REORIENTATION_RATE * 0.5f )
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		}
	}

	if ( !m_bPitchReorientation && !bForcePitchReorient )
	{
		// Randomize which way we roll if we're completely upside down
		if ( vAbsAngles[ROLL] == 180.0f && RandomInt( 0, 1 ) == 1 )
		{
			vAbsAngles[ROLL] = -180.0f;
		}

		if ( vAbsAngles[ROLL] < 0.0f )
		{
			vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ROLL] > 0.0f )
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles( vAbsAngles );
		}
		else if ( vAbsAngles[ROLL] > 0.0f )
		{
			vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ROLL] < 0.0f )
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles( vAbsAngles );
			m_angEyeAngles = vAbsAngles;
			m_iv_angEyeAngles.Reset();
		}
	}
	else
	{
		if ( vAbsAngles[ROLL] != 0.0f )
		{
			if ( vCurrentUp.z < 0.2f )
			{
				float fDegrees = gpGlobals->frametime * m_fReorientationRate;
				if ( vCurrentForward.z > 0.0f )
				{
					fDegrees = -fDegrees;
				}

				// Rotate around the right axis
				VMatrix mAxisAngleRot = SetupMatrixAxisRot( vCurrentRight, fDegrees );

				vCurrentUp = mAxisAngleRot.VMul3x3( vCurrentUp );
				vCurrentForward = mAxisAngleRot.VMul3x3( vCurrentForward );

				VectorAngles( vCurrentForward, vCurrentUp, vAbsAngles );

				engine->SetViewAngles( vAbsAngles );
				m_angEyeAngles = vAbsAngles;
				m_iv_angEyeAngles.Reset();
			}
			else
			{
				if ( vAbsAngles[ROLL] < 0.0f )
				{
					vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ROLL] > 0.0f )
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles( vAbsAngles );
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
				else if ( vAbsAngles[ROLL] > 0.0f )
				{
					vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ROLL] < 0.0f )
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles( vAbsAngles );
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
			}
		}
	}

	// Keep track of if we're upside down for look control
	vAbsAngles = EyeAngles();
	AngleVectors( vAbsAngles, NULL, NULL, &vCurrentUp );

	if ( bForcePitchReorient )
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
	else
		g_bUpsideDown = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateLookAt( void )
{
	bool bFoundViewTarget = false;

	Vector vForward;
	AngleVectors( GetLocalAngles(), &vForward );

	Vector vMyOrigin =  GetAbsOrigin();

	Vector vecLookAtTarget = vec3_origin;

	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );
		if ( !pEnt || !pEnt->IsPlayer() )
			continue;

		if ( !pEnt->IsAlive() )
			continue;

		if ( pEnt == this )
			continue;

		Vector vDir = pEnt->GetAbsOrigin() - vMyOrigin;

		if ( vDir.Length() > 300 ) 
			continue;

		VectorNormalize( vDir );

		if ( DotProduct( vForward, vDir ) < 0.0f )
			continue;

		vecLookAtTarget = pEnt->EyePosition();
		bFoundViewTarget = true;
		break;
	}

	if ( bFoundViewTarget == false )
	{
		// no target, look forward
		vecLookAtTarget = GetAbsOrigin() + vForward * 512;
	}

	// orient eyes
	m_viewtarget = vecLookAtTarget;

	// blinking
	if ( m_blinkTimer.IsElapsed() )
	{
		m_blinktoggle = !m_blinktoggle;
		m_blinkTimer.Start( RandomFloat( 1.5f, 4.0f ) );
	}

	/*
	// Figure out where we want to look in world space.
	QAngle desiredAngles;
	Vector to = vecLookAtTarget - EyePosition();
	VectorAngles( to, desiredAngles );

	// Figure out where our body is facing in world space.
	QAngle bodyAngles( 0, 0, 0 );
	bodyAngles[YAW] = GetLocalAngles()[YAW];

	float flBodyYawDiff = bodyAngles[YAW] - m_flLastBodyYaw;
	m_flLastBodyYaw = bodyAngles[YAW];

	// Set the head's yaw.
	float desired = AngleNormalize( desiredAngles[YAW] - bodyAngles[YAW] );
	desired = clamp( -desired, m_headYawMin, m_headYawMax );
	m_flCurrentHeadYaw = ApproachAngle( desired, m_flCurrentHeadYaw, 130 * gpGlobals->frametime );

	// Counterrotate the head from the body rotation so it doesn't rotate past its target.
	m_flCurrentHeadYaw = AngleNormalize( m_flCurrentHeadYaw - flBodyYawDiff );

	SetPoseParameter( m_headYawPoseParam, m_flCurrentHeadYaw );

	// Set the head's yaw.
	desired = AngleNormalize( desiredAngles[PITCH] );
	desired = clamp( desired, m_headPitchMin, m_headPitchMax );

	m_flCurrentHeadPitch = ApproachAngle( -desired, m_flCurrentHeadPitch, 130 * gpGlobals->frametime );
	m_flCurrentHeadPitch = AngleNormalize( m_flCurrentHeadPitch );
	SetPoseParameter( m_headPitchPoseParam, m_flCurrentHeadPitch );
	*/
}


//-----------------------------------------------------------------------------
// Purpose: Try to steer away from any players and objects we might interpenetrate
//-----------------------------------------------------------------------------
#define TF_AVOID_MAX_RADIUS_SQR		5184.0f			// Based on player extents and max buildable extents.
#define TF_OO_AVOID_MAX_RADIUS_SQR	0.00019f

ConVar tf_max_separation_force ( "tf_max_separation_force", "256", FCVAR_DEVELOPMENTONLY );

extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

void C_TFPlayer::AvoidPlayers( CUserCmd *pCmd )
{
	// Turn off the avoid player code.
	if ( !tf_avoidteammates.GetBool() || !tf_avoidteammates_pushaway.GetBool() )
		return;

	// Don't test if the player doesn't exist.
	if ( this == NULL )
		return;

	// Don't test if the playeris dead.
	if ( !IsAlive() )
		return;

	C_Team *pTeam = ( C_Team * )GetTeam();
	if ( !pTeam )
		return;

	// Up vector.
	static Vector vecUp( 0.0f, 0.0f, 1.0f );

	Vector vecTFPlayerCenter = GetAbsOrigin();
	Vector vecTFPlayerMin = GetPlayerMins();
	Vector vecTFPlayerMax = GetPlayerMaxs();
	float flZHeight = vecTFPlayerMax.z - vecTFPlayerMin.z;
	vecTFPlayerCenter.z += 0.5f * flZHeight;
	VectorAdd( vecTFPlayerMin, vecTFPlayerCenter, vecTFPlayerMin );
	VectorAdd( vecTFPlayerMax, vecTFPlayerCenter, vecTFPlayerMax );

	// Find an intersecting player or object.
	int nAvoidPlayerCount = 0;
	C_TFPlayer *pAvoidPlayerList[MAX_PLAYERS];

	C_TFPlayer *pIntersectPlayer = NULL;
	CBaseObject *pIntersectObject = NULL;
	C_AI_BaseNPC *pIntersectNPC = NULL;
	float flAvoidRadius = 0.0f;

	Vector vecAvoidCenter, vecAvoidMin, vecAvoidMax;
	for ( int i = 0; i < pTeam->GetNumPlayers(); ++i )
	{
		C_TFPlayer *pAvoidPlayer = static_cast< C_TFPlayer * >( pTeam->GetPlayer( i ) );
		if ( pAvoidPlayer == NULL )
			continue;
		// Is the avoid player me?
		if ( pAvoidPlayer == this )
			continue;

		// Save as list to check against for objects.
		pAvoidPlayerList[nAvoidPlayerCount] = pAvoidPlayer;
		++nAvoidPlayerCount;

		// Check to see if the avoid player is dormant.
		if ( pAvoidPlayer->IsDormant() )
			continue;

		// Is the avoid player solid?
		if ( pAvoidPlayer->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
			continue;

		Vector t1, t2;

		vecAvoidCenter = pAvoidPlayer->GetAbsOrigin();
		vecAvoidMin = pAvoidPlayer->GetPlayerMins();
		vecAvoidMax = pAvoidPlayer->GetPlayerMaxs();
		flZHeight = vecAvoidMax.z - vecAvoidMin.z;
		vecAvoidCenter.z += 0.5f * flZHeight;
		VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
		VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

		if ( IsBoxIntersectingBox( vecTFPlayerMin, vecTFPlayerMax, vecAvoidMin, vecAvoidMax ) )
		{
			// Need to avoid this player.
			if ( !pIntersectPlayer )
			{
				pIntersectPlayer = pAvoidPlayer;
				break;
			}
		}
	}

	// We didn't find a player - look for objects to avoid.
	if ( !pIntersectPlayer )
	{
		for ( int iPlayer = 0; iPlayer < nAvoidPlayerCount; ++iPlayer )
		{	
			// Stop when we found an intersecting object.
			if ( pIntersectObject )
				break;

			C_TFTeam *pTeam = (C_TFTeam*)GetTeam();

			for ( int iObject = 0; iObject < pTeam->GetNumObjects(); ++iObject )
			{
				CBaseObject *pAvoidObject = pTeam->GetObject( iObject );
				if ( !pAvoidObject )
					continue;

				// Check to see if the object is dormant.
				if ( pAvoidObject->IsDormant() )
					continue;

				// Is the object solid.
				if ( pAvoidObject->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
					continue;

				// If we shouldn't avoid it, see if we intersect it.
				if ( pAvoidObject->ShouldPlayersAvoid() )
				{
					vecAvoidCenter = pAvoidObject->WorldSpaceCenter();
					vecAvoidMin = pAvoidObject->WorldAlignMins();
					vecAvoidMax = pAvoidObject->WorldAlignMaxs();
					VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
					VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

					if ( IsBoxIntersectingBox( vecTFPlayerMin, vecTFPlayerMax, vecAvoidMin, vecAvoidMax ) )
					{
						// Need to avoid this object.
						pIntersectObject = pAvoidObject;
						break;
					}
				}
			}

			if ( !pIntersectObject )
			{
				// Stop when we found an intersecting npc.
				if ( pIntersectNPC )
					break;

				for ( int iNPC = 0; iNPC < pTeam->GetNumNPCs(); ++iNPC )
				{
					C_AI_BaseNPC *pAvoidNPC = pTeam->GetNPC( iNPC );
					if ( !pAvoidNPC )
						continue;

					// Check to see if the object is dormant.
					if ( pAvoidNPC->IsDormant() )
						continue;

					// Is the object solid.
					if ( pAvoidNPC->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
						continue;

					vecAvoidCenter = pAvoidNPC->WorldSpaceCenter();
					vecAvoidMin = pAvoidNPC->WorldAlignMins();
					vecAvoidMax = pAvoidNPC->WorldAlignMaxs();
					VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
					VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

					if ( IsBoxIntersectingBox( vecTFPlayerMin, vecTFPlayerMax, vecAvoidMin, vecAvoidMax ) )
					{
						// Need to avoid this object.
						pIntersectNPC = pAvoidNPC;
						break;
					}
				}
			}
		}
	}

	// Anything to avoid?
	if ( !pIntersectPlayer && !pIntersectObject && !pIntersectNPC )
	{
		m_Shared.SetSeparation( false );
		m_Shared.SetSeparationVelocity( vec3_origin );
		return;
	}

	// Calculate the push strength and direction.
	Vector vecDelta;

	// Avoid a player - they have precedence.
	if ( pIntersectPlayer )
	{
		VectorSubtract( pIntersectPlayer->WorldSpaceCenter(), vecTFPlayerCenter, vecDelta );

		Vector vRad = pIntersectPlayer->WorldAlignMaxs() - pIntersectPlayer->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}
	else if ( pIntersectNPC )
	{
		VectorSubtract( pIntersectNPC->WorldSpaceCenter(), vecTFPlayerCenter, vecDelta );

		Vector vRad = pIntersectNPC->WorldAlignMaxs() - pIntersectNPC->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}
	else	// Avoid an object.
	{
		VectorSubtract( pIntersectObject->WorldSpaceCenter(), vecTFPlayerCenter, vecDelta );

		Vector vRad = pIntersectObject->WorldAlignMaxs() - pIntersectObject->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}

	float flPushStrength = RemapValClamped( vecDelta.Length(), flAvoidRadius, 0, 0, tf_max_separation_force.GetInt() ); //flPushScale;

	//Msg( "PushScale = %f\n", flPushStrength );

	// Check to see if we have enough push strength to make a difference.
	if ( flPushStrength < 0.01f )
		return;

	Vector vecPush;
	if ( GetAbsVelocity().Length2DSqr() > 0.1f )
	{
		Vector vecVelocity = GetAbsVelocity();
		vecVelocity.z = 0.0f;
		CrossProduct( vecUp, vecVelocity, vecPush );
		VectorNormalize( vecPush );
	}
	else
	{
		// We are not moving, but we're still intersecting.
		QAngle angView = pCmd->viewangles;
		angView.x = 0.0f;
		AngleVectors( angView, NULL, &vecPush, NULL );
	}

	// Move away from the other player/object.
	Vector vecSeparationVelocity;
	if ( vecDelta.Dot( vecPush ) < 0 )
	{
		vecSeparationVelocity = vecPush * flPushStrength;
	}
	else
	{
		vecSeparationVelocity = vecPush * -flPushStrength;
	}

	// Don't allow the max push speed to be greater than the max player speed.
	float flMaxPlayerSpeed = MaxSpeed();
	float flCropFraction = 1.33333333f;

	if ( ( GetFlags() & FL_DUCKING ) && ( GetGroundEntity() != NULL ) )
	{	
		flMaxPlayerSpeed *= flCropFraction;
	}	

	float flMaxPlayerSpeedSqr = flMaxPlayerSpeed * flMaxPlayerSpeed;

	if ( vecSeparationVelocity.LengthSqr() > flMaxPlayerSpeedSqr )
	{
		vecSeparationVelocity.NormalizeInPlace();
		VectorScale( vecSeparationVelocity, flMaxPlayerSpeed, vecSeparationVelocity );
	}

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;
	Vector currentdir;
	Vector rightdir;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );

	Vector vDirection = vecSeparationVelocity;

	VectorNormalize( vDirection );

	float fwd = currentdir.Dot( vDirection );
	float rt = rightdir.Dot( vDirection );

	float forward = fwd * flPushStrength;
	float side = rt * flPushStrength;

	//Msg( "fwd: %f - rt: %f - forward: %f - side: %f\n", fwd, rt, forward, side );

	m_Shared.SetSeparation( true );
	m_Shared.SetSeparationVelocity( vecSeparationVelocity );

	pCmd->forwardmove	+= forward;
	pCmd->sidemove		+= side;

	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}

	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}

	float flScale = min( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "Pforwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInputSampleTime - 
//			*pCmd - 
//-----------------------------------------------------------------------------
bool C_TFPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{	
	static QAngle angMoveAngle( 0.0f, 0.0f, 0.0f );
	
	bool bNoTaunt = true;
	if ( m_Shared.InCond( TF_COND_TAUNTING ) )
	{
		int nOldButtons = pCmd->buttons;
		pCmd->buttons = 0;
		pCmd->weaponselect = 0;

		if ( ( nOldButtons & IN_ATTACK ) )
			pCmd->buttons |= IN_ATTACK;

		if ( ( nOldButtons & IN_ATTACK2 ) )
			pCmd->buttons |= IN_ATTACK2;

		if ( ( nOldButtons & IN_ATTACK3 ) )
			pCmd->buttons |= IN_ATTACK3;

		if ( ( nOldButtons & IN_USE_ACTION ) )
			pCmd->buttons |= IN_USE_ACTION;

		if ( ( nOldButtons & IN_TAUNT ) )
			pCmd->buttons |= IN_TAUNT;

		if ( ( nOldButtons & IN_FORWARD ) )
			pCmd->buttons |= IN_FORWARD;

		if ( ( nOldButtons & IN_BACK ) )
			pCmd->buttons |= IN_BACK;

		if ( ( nOldButtons & IN_MOVELEFT ) )
			pCmd->buttons |= IN_MOVELEFT;

		if ( ( nOldButtons & IN_MOVERIGHT ) )
			pCmd->buttons |= IN_MOVERIGHT;

		if ( ( nOldButtons & IN_USE ) )
			pCmd->buttons |= IN_USE;

		VectorCopy( angMoveAngle, pCmd->viewangles );
		bNoTaunt = false;

		if ( !CanMoveDuringTaunt() )
		{
			pCmd->forwardmove = 0.0f;
			pCmd->sidemove = 0.0f;
			pCmd->upmove = 0.0f;
		}
	}	
	else if ( m_Shared.InCond( TF_COND_PHASE ) )
	{
		pCmd->weaponselect = 0;
		int nOldButtons = pCmd->buttons;
		pCmd->buttons = 0;

		// Scout can jump and duck while phased
		if ( ( nOldButtons & IN_JUMP ) )
			pCmd->buttons |= IN_JUMP;

		if ( ( nOldButtons & IN_DUCK ) )
			pCmd->buttons |= IN_DUCK;

		if ( ( nOldButtons & IN_USE_ACTION ) )
			pCmd->buttons |= IN_USE_ACTION;

		if ( ( nOldButtons & IN_USE ) )
			pCmd->buttons |= IN_USE;
	}
	else if ( IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
	{
		pCmd->forwardmove = 0.0f;
		pCmd->sidemove = 0.0f;
		pCmd->upmove = 0.0f;

		int nOldButtons = pCmd->buttons;
		pCmd->buttons = 0;

		if ( ( nOldButtons & IN_ATTACK ) )
			pCmd->buttons |= IN_ATTACK;

		if ( ( nOldButtons & IN_ATTACK2 ) )
			pCmd->buttons |= IN_ATTACK2;

		if ( ( nOldButtons & IN_ATTACK3 ) )
			pCmd->buttons |= IN_ATTACK3;

		if ( ( nOldButtons & IN_USE_ACTION ) )
			pCmd->buttons |= IN_USE_ACTION;

		if ( ( nOldButtons & IN_USE ) )
			pCmd->buttons |= IN_USE;

		VectorCopy( pCmd->viewangles, angMoveAngle );
	}
	else
	{
		VectorCopy( pCmd->viewangles, angMoveAngle );
	}

	// HACK: We're using an unused bit in buttons var to set the typing status based on whether player's chat panel is open.
	if ( GetTFChatHud() && GetTFChatHud()->GetMessageMode() != MM_NONE )
	{
		pCmd->buttons |= IN_TYPING;
	}

	BaseClass::CreateMove( flInputSampleTime, pCmd );

	AvoidPlayers( pCmd );

	return bNoTaunt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( IsLocalPlayer() )
	{
		if ( !prediction->IsFirstTimePredicted() )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector C_TFPlayer::GetObserverCamOrigin( void )
{
	if ( !IsAlive() )
	{
		if ( m_hFirstGib )
		{
			IPhysicsObject *pPhysicsObject = m_hFirstGib->VPhysicsGetObject();
			if( pPhysicsObject )
			{
				Vector vecMassCenter = pPhysicsObject->GetMassCenterLocalSpace();
				Vector vecWorld;
				m_hFirstGib->CollisionProp()->CollisionToWorldSpace( vecMassCenter, &vecWorld );
				return (vecWorld);
			}
			return m_hFirstGib->GetRenderOrigin();
		}

		IRagdoll *pRagdoll = GetRepresentativeRagdoll();
		if ( pRagdoll )
			return pRagdoll->GetRagdollOrigin();
	}

	return BaseClass::GetObserverCamOrigin();	
}

//-----------------------------------------------------------------------------
// Purpose: Consider the viewer and other factors when determining resulting
// invisibility
//-----------------------------------------------------------------------------
float C_TFPlayer::GetEffectiveInvisibilityLevel( void )
{
	float flPercentInvisible = GetPercentInvisible();

	// If this is a teammate of the local player or viewer is observer,
	// dont go above a certain max invis
	if ( !IsEnemyPlayer() )
	{
		float flMax = tf_teammate_max_invis.GetFloat();
		if ( flPercentInvisible > flMax )
		{
			flPercentInvisible = flMax;
		}
	}
	else
	{
		// If this player just killed me, show them slightly
		// less than full invis in the deathcam and freezecam

		C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();

		if ( pLocalPlayer )
		{
			int iObserverMode = pLocalPlayer->GetObserverMode();

			if ( ( iObserverMode == OBS_MODE_FREEZECAM || iObserverMode == OBS_MODE_DEATHCAM ) && 
				pLocalPlayer->GetObserverTarget() == this )
			{
				float flMax = tf_teammate_max_invis.GetFloat();
				if ( flPercentInvisible > flMax )
				{
					flPercentInvisible = flMax;
				}
			}
		}
	}

	return flPercentInvisible;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::DrawModel( int flags )
{
	// If we're a dead player with a fresh ragdoll, don't draw
	if ( m_nRenderFX == kRenderFxRagdoll )
		return 0;

	if (GetSpectatorTarget() == entindex() && GetSpectatorMode() == OBS_MODE_IN_EYE)
	{
		return 0;
	}

	// Don't draw the model at all if we're fully invisible
	if ( GetEffectiveInvisibilityLevel() >= 1.0f )
	{
		if ( m_hPartyHat && ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 90 ) && !m_hPartyHat->IsEffectActive( EF_NODRAW ) )
		{
			m_hPartyHat->SetEffects( EF_NODRAW );
		}
		return 0;
	}
	else
	{
		if ( m_hPartyHat && ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 90 ) && m_hPartyHat->IsEffectActive( EF_NODRAW ) )
		{
			m_hPartyHat->RemoveEffects( EF_NODRAW );
		}
	}

	CMatRenderContextPtr pRenderContext( materials );
	bool bDoEffect = false;

	float flAmountToChop = 0.0;
	if ( m_Shared.InCond( TF_COND_DISGUISING ) )
	{
		flAmountToChop = ( gpGlobals->curtime - m_flDisguiseEffectStartTime ) *
			( 1.0 / TF_TIME_TO_DISGUISE );
	}
	else
		if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		{
			float flETime = gpGlobals->curtime - m_flDisguiseEffectStartTime;
			if ( ( flETime > 0.0 ) && ( flETime < TF_TIME_TO_SHOW_DISGUISED_FINISHED_EFFECT ) )
			{
				flAmountToChop = 1.0 - ( flETime * ( 1.0/TF_TIME_TO_SHOW_DISGUISED_FINISHED_EFFECT ) );
			}
		}

	bDoEffect = ( flAmountToChop > 0.0 ) && ( ! IsLocalPlayer() );
#if ( SHOW_DISGUISE_EFFECT == 0  )
	bDoEffect = false;
#endif
	bDoEffect = false;
	if ( bDoEffect )
	{
		Vector vMyOrigin =  GetAbsOrigin();
		BoxDeformation_t mybox;
		mybox.m_ClampMins = vMyOrigin - Vector(100,100,100);
		mybox.m_ClampMaxes = vMyOrigin + Vector(500,500,72 * ( 1 - flAmountToChop ) );
		pRenderContext->PushDeformation( &mybox );
	}

	if( IsLocalPlayer() )
	{
		if ( !C_BasePlayer::ShouldDrawLocalPlayer() )
		{
			if ( !g_pPortalRender->IsRenderingPortal() )
				return 0;

			if( (g_pPortalRender->GetViewRecursionLevel() == 1) && (m_iForceNoDrawInPortalSurface != -1) ) //CPortalRender::s_iRenderingPortalView )
				return 0;
		}
	}

	int ret = BaseClass::DrawModel( flags );

	if ( bDoEffect )
		pRenderContext->PopDeformation();
	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::ProcessMuzzleFlashEvent()
{
	CBasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	// Reenable when the weapons have muzzle flash attachments in the right spot.
	bool bInToolRecordingMode = ToolsEnabled() && clienttools->IsInRecordingMode();
	if ( this == pLocalPlayer && !bInToolRecordingMode )
		return; // don't show own world muzzle flash for localplayer

	if ( pLocalPlayer && pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		// also don't show in 1st person spec mode
		if ( pLocalPlayer->GetObserverTarget() == this )
			return;
	}

	C_TFWeaponBase *pWeapon = m_Shared.GetActiveTFWeapon();
	if ( !pWeapon )
		return;

	pWeapon->ProcessMuzzleFlashEvent();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::SetForcedIDTarget( int iTarget )
{
	m_iForcedIDTarget = iTarget;
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's targetid entity
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateIDTarget()
{
	if ( !IsLocalPlayer() )
		return;

	// don't show IDs if mp_fadetoblack is on
	if ( GetTeamNumber() > TEAM_SPECTATOR && mp_fadetoblack.GetBool() && !IsAlive() )
	{
		m_iIDEntIndex = 0;
		return;
	}

	if ( m_iForcedIDTarget )
	{
		m_iIDEntIndex = m_iForcedIDTarget;
		return;
	}

	// If we're in deathcam, ID our killer
	if ( (GetObserverMode() == OBS_MODE_DEATHCAM || GetObserverMode() == OBS_MODE_CHASE) && GetObserverTarget() && GetObserverTarget() != GetLocalTFPlayer() )
	{
		m_iIDEntIndex = GetObserverTarget()->entindex();
		return;
	}

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), MAX_TRACE_LENGTH, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );

	Ray_t ray;
	ray.Init( vecStart, vecEnd );

	// If we're in observer mode, ignore our observer target. Otherwise, ignore ourselves.
	if ( IsObserver() )
	{
		UTIL_Portal_TraceRay( ray, MASK_SOLID, GetObserverTarget(), COLLISION_GROUP_VEHICLE, &tr );
	}
	else
	{
		UTIL_Portal_TraceRay( ray, MASK_SOLID, this, COLLISION_GROUP_VEHICLE, &tr );
	}

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && ( pEntity != this ) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Display appropriate hints for the target we're looking at
//-----------------------------------------------------------------------------
void C_TFPlayer::DisplaysHintsForTarget( C_BaseEntity *pTarget )
{
	// If the entity provides hints, ask them if they have one for this player
	ITargetIDProvidesHint *pHintInterface = dynamic_cast<ITargetIDProvidesHint*>(pTarget);
	if ( pHintInterface )
	{
		pHintInterface->DisplayHintTo( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::GetRenderTeamNumber( void )
{
	return m_nSkin;
}

static Vector WALL_MIN(-WALL_OFFSET,-WALL_OFFSET,-WALL_OFFSET);
static Vector WALL_MAX(WALL_OFFSET,WALL_OFFSET,WALL_OFFSET);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::CalcDeathCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	CBaseEntity	* killer = GetObserverTarget();

	// Swing to face our killer within half the death anim time
	float interpolation = ( gpGlobals->curtime - m_flDeathTime ) / (TF_DEATH_ANIMATION_TIME * 0.5);
	interpolation = clamp( interpolation, 0.0f, 1.0f );
	interpolation = SimpleSpline( interpolation );

	m_flObserverChaseDistance += gpGlobals->frametime*48.0f;
	m_flObserverChaseDistance = clamp(m_flObserverChaseDistance, CHASE_CAM_DISTANCE_MIN, CHASE_CAM_DISTANCE_MAX);

	QAngle aForward = eyeAngles = EyeAngles();
	Vector origin = EyePosition();			

	IRagdoll *pRagdoll = GetRepresentativeRagdoll();
	if ( pRagdoll )
	{
		origin = pRagdoll->GetRagdollOrigin();
		origin.z += VEC_DEAD_VIEWHEIGHT.z; // look over ragdoll, not through
	}

	if ( killer && (killer != this) ) 
	{
		Vector vKiller = killer->EyePosition() - origin;
		QAngle aKiller; VectorAngles( vKiller, aKiller );
		InterpolateAngles( aForward, aKiller, eyeAngles, interpolation );
	};

	Vector vForward; AngleVectors( eyeAngles, &vForward );

	VectorNormalize( vForward );

	VectorMA( origin, -m_flObserverChaseDistance, vForward, eyeOrigin );

	trace_t trace; // clip against world
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if (trace.fraction < 1.0)
	{
		eyeOrigin = trace.endpos;
		m_flObserverChaseDistance = VectorLength(origin - eyeOrigin);
	}

	fov = GetFOV();
}

extern ConVar spec_freeze_traveltime;
extern ConVar spec_freeze_time;

//-----------------------------------------------------------------------------
// Purpose: Calculate the view for the player while he's in freeze frame observer mode
//-----------------------------------------------------------------------------
void C_TFPlayer::CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	C_BaseEntity *pTarget = GetObserverTarget();
	if ( !pTarget )
	{
		CalcDeathCamView( eyeOrigin, eyeAngles, fov );
		return;
	}

	// Zoom towards our target
	float flCurTime = ( gpGlobals->curtime - m_flFreezeFrameStartTime );
	float flBlendPerc = clamp( flCurTime / spec_freeze_traveltime.GetFloat(), 0.f, 1.f );
	flBlendPerc = SimpleSpline( flBlendPerc );

	Vector vecCamDesired = pTarget->GetObserverCamOrigin();	// Returns ragdoll origin if they're ragdolled
	VectorAdd( vecCamDesired, GetChaseCamViewOffset( pTarget ), vecCamDesired );
	Vector vecCamTarget = vecCamDesired;
	if ( pTarget->IsAlive() )
	{
		// Look at their chest, not their head
		Vector maxs;

		// Obviously you can't apply player height to NPCs.
		if ( pTarget->IsNPC() )
		{
			maxs = pTarget->WorldAlignMaxs();
		}
		else
		{
			maxs = pTarget->GetBaseAnimating() ? VEC_HULL_MAX_SCALED( pTarget->GetBaseAnimating() ) : VEC_HULL_MAX;
		}

		vecCamTarget.z -= ( maxs.z * 0.5 );
	}
	else
	{
		vecCamTarget.z += pTarget->GetBaseAnimating() ? VEC_DEAD_VIEWHEIGHT_SCALED( pTarget->GetBaseAnimating() ).z : VEC_DEAD_VIEWHEIGHT.z;	// look over ragdoll, not through
	}

	// Figure out a view position in front of the target
	Vector vecEyeOnPlane = eyeOrigin;
	vecEyeOnPlane.z = vecCamTarget.z;
	Vector vecTargetPos = vecCamTarget;
	Vector vecToTarget = vecTargetPos - vecEyeOnPlane;
	VectorNormalize( vecToTarget );

	// Stop a few units away from the target, and shift up to be at the same height
	vecTargetPos = vecCamTarget - ( vecToTarget * m_flFreezeFrameDistance );
	float flEyePosZ = pTarget->EyePosition().z;
	vecTargetPos.z = flEyePosZ + m_flFreezeZOffset;

	// Now trace out from the target, so that we're put in front of any walls
	trace_t trace;
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( vecCamTarget, vecTargetPos, WALL_MIN, WALL_MAX, MASK_SOLID, pTarget, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();
	if ( trace.fraction < 1.0 )
	{
		// The camera's going to be really close to the target. So we don't end up
		// looking at someone's chest, aim close freezecams at the target's eyes.
		vecTargetPos = trace.endpos;
		vecCamTarget = vecCamDesired;

		// To stop all close in views looking up at character's chins, move the view up.
		vecTargetPos.z += fabs( vecCamTarget.z - vecTargetPos.z ) * 0.85;
		C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull( vecCamTarget, vecTargetPos, WALL_MIN, WALL_MAX, MASK_SOLID, pTarget, COLLISION_GROUP_NONE, &trace );
		C_BaseEntity::PopEnableAbsRecomputations();
		vecTargetPos = trace.endpos;
	}

	// Look directly at the target
	vecToTarget = vecCamTarget - vecTargetPos;
	VectorNormalize( vecToTarget );
	VectorAngles( vecToTarget, eyeAngles );

	VectorLerp( m_vecFreezeFrameStart, vecTargetPos, flBlendPerc, eyeOrigin );

	if ( flCurTime >= spec_freeze_traveltime.GetFloat() && !m_bSentFreezeFrame )
	{
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "freezecam_started" );
		if ( pEvent )
		{
			gameeventmanager->FireEventClientSide( pEvent );
		}

		m_bSentFreezeFrame = true;
		view->FreezeFrame( spec_freeze_time.GetFloat() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Do nothing multiplayer_animstate takes care of animation.
// Input  : playerAnim - 
//-----------------------------------------------------------------------------
void C_TFPlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	return;
}

float C_TFPlayer::GetMinFOV() const
{
	// Min FOV for Sniper Rifle
	return 20;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const QAngle& C_TFPlayer::EyeAngles()
{
	if ( IsLocalPlayer() && g_nKillCamMode == OBS_MODE_NONE )
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		return m_angEyeAngles;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &color - 
//-----------------------------------------------------------------------------
void C_TFPlayer::GetTeamColor( Color &color )
{
	color[3] = 255;

	switch (GetTeamNumber())
	{
		case TF_TEAM_RED:
			color[0] = 159;
			color[1] = 55;
			color[2] = 34;
			break;
		case TF_TEAM_BLUE:
			color[0] = 76;
			color[1] = 109;
			color[2] = 129;
			break;
		case TF_TEAM_GREEN:
			color[0] = 59;
			color[1] = 120;
			color[2] = 55;
			break;
		case TF_TEAM_YELLOW:
			color[0] = 145;
			color[1] = 145;
			color[2] = 55;
			break;
		default:
			color[0] = 255;
			color[1] = 255;
			color[2] = 255;
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bCopyEntity - 
// Output : C_BaseAnimating *
//-----------------------------------------------------------------------------
C_BaseAnimating *C_TFPlayer::BecomeRagdollOnClient()
{
	// Let the C_TFRagdoll take care of this.
	return NULL;
}

void C_TFPlayer::UpdatePortalEyeInterpolation( void )
{
#ifdef ENABLE_PORTAL_EYE_INTERPOLATION_CODE
	//PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;
	if( PortalEyeInterpolation.m_bUpdatePosition_FreeMove )
	{
		PortalEyeInterpolation.m_bUpdatePosition_FreeMove = false;

		C_Prop_Portal *pOldPortal = PreDataChanged_Backup.m_hPortalEnvironment.Get();
		if( pOldPortal )
		{
			UTIL_Portal_PointTransform( pOldPortal->MatrixThisToLinked(), PortalEyeInterpolation.m_vEyePosition_Interpolated, PortalEyeInterpolation.m_vEyePosition_Interpolated );
			//PortalEyeInterpolation.m_vEyePosition_Interpolated = pOldPortal->m_matrixThisToLinked * PortalEyeInterpolation.m_vEyePosition_Interpolated;

			//Vector vForward;
			//m_hPortalEnvironment.Get()->GetVectors( &vForward, NULL, NULL );

			PortalEyeInterpolation.m_vEyePosition_Interpolated = EyeFootPosition();

			PortalEyeInterpolation.m_bEyePositionIsInterpolating = true;
		}
	}

	if( IsInAVehicle() )
		PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;

	if( !PortalEyeInterpolation.m_bEyePositionIsInterpolating )
	{
		PortalEyeInterpolation.m_vEyePosition_Uninterpolated = EyeFootPosition();
		PortalEyeInterpolation.m_vEyePosition_Interpolated = PortalEyeInterpolation.m_vEyePosition_Uninterpolated;
		return;
	}

	Vector vThisFrameUninterpolatedPosition = EyeFootPosition();

	//find offset between this and last frame's uninterpolated movement, and apply this as freebie movement to the interpolated position
	PortalEyeInterpolation.m_vEyePosition_Interpolated += (vThisFrameUninterpolatedPosition - PortalEyeInterpolation.m_vEyePosition_Uninterpolated);
	PortalEyeInterpolation.m_vEyePosition_Uninterpolated = vThisFrameUninterpolatedPosition;

	Vector vDiff = vThisFrameUninterpolatedPosition - PortalEyeInterpolation.m_vEyePosition_Interpolated;
	float fLength = vDiff.Length();
	float fFollowSpeed = gpGlobals->frametime * 100.0f;
	const float fMaxDiff = 150.0f;
	if( fLength > fMaxDiff )
	{
		//camera lagging too far behind, give it a speed boost to bring it within maximum range
		fFollowSpeed = fLength - fMaxDiff;
	}
	else if( fLength < fFollowSpeed )
	{
		//final move
		PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;
		PortalEyeInterpolation.m_vEyePosition_Interpolated = vThisFrameUninterpolatedPosition;
		return;
	}

	if ( fLength > 0.001f )
	{
		vDiff *= (fFollowSpeed/fLength);
		PortalEyeInterpolation.m_vEyePosition_Interpolated += vDiff;
	}
	else
	{
		PortalEyeInterpolation.m_vEyePosition_Interpolated = vThisFrameUninterpolatedPosition;
	}



#else
	PortalEyeInterpolation.m_vEyePosition_Interpolated = BaseClass::EyePosition();
#endif
}

Vector C_TFPlayer::EyePosition()
{
	return PortalEyeInterpolation.m_vEyePosition_Interpolated;  
}

Vector C_TFPlayer::EyeFootPosition( const QAngle &qEyeAngles )
{
	//interpolate between feet and normal eye position based on view roll (gets us wall/ceiling & ceiling/ceiling teleportations without an eye position pop)
	float fFootInterp = fabs(qEyeAngles[ROLL]) * ((1.0f/180.0f) * 0.75f); //0 when facing straight up, 0.75 when facing straight down
	return (BaseClass::EyePosition() - (fFootInterp * m_vecViewOffset)); //TODO: Find a good Up vector for this rolled player and interpolate along actual eye/foot axis
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : IRagdoll*
//-----------------------------------------------------------------------------
IRagdoll* C_TFPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_TFRagdoll *pRagdoll = static_cast<C_TFRagdoll*>( m_hRagdoll.Get() );
		if ( !pRagdoll )
			return NULL;

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

void C_TFPlayer::PlayerPortalled( C_Prop_Portal *pEnteredPortal )
{
	if( pEnteredPortal )
	{
		m_bPortalledMessagePending = true;
		m_PendingPortalMatrix = pEnteredPortal->MatrixThisToLinked();

		if( IsLocalPlayer() )
			g_pPortalRender->EnteredPortal( pEnteredPortal );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::InitPlayerGibs( void )
{
	// Clear out the gib list and create a new one.
	m_aGibs.Purge();
	BuildGibList( m_aGibs, GetModelIndex(), 1.0f, COLLISION_GROUP_NONE );

	if ( TFGameRules() && ( TFGameRules()->IsBirthdayOrPyroVision() || TFGameRules()->IsLFBirthday() ) )
	{
		for ( int i = 0; i < m_aGibs.Count(); i++ )
		{
			if ( RandomFloat(0,1) < 0.75 )
			{
				Q_strncpy( m_aGibs[i].modelName, g_pszBDayGibs[ RandomInt(0,ARRAYSIZE(g_pszBDayGibs)-1) ] , sizeof(m_aGibs[i].modelName) );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecOrigin - 
//			&vecVelocity - 
//			&vecImpactVelocity - 
//-----------------------------------------------------------------------------
void C_TFPlayer::CreatePlayerGibs( const Vector &vecOrigin, const Vector &vecVelocity, float flImpactScale, bool bBurning, bool bHeadGib )
{
	// Make sure we have Gibs to create.
	if ( m_aGibs.Count() == 0 )
		return;

	AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );

	Vector vecBreakVelocity = vecVelocity;
	vecBreakVelocity.z += tf_playergib_forceup.GetFloat();
	VectorNormalize( vecBreakVelocity );
	vecBreakVelocity *= tf_playergib_force.GetFloat();

	// Cap the impulse.
	float flSpeed = vecBreakVelocity.Length();
	if ( flSpeed > tf_playergib_maxspeed.GetFloat() )
	{
		VectorScale( vecBreakVelocity, tf_playergib_maxspeed.GetFloat() / flSpeed, vecBreakVelocity );
	}

	breakablepropparams_t breakParams( vecOrigin, GetRenderAngles(), vecBreakVelocity, angularImpulse );
	breakParams.impactEnergyScale = 1.0f;

	// Break up the player.
	m_hSpawnedGibs.Purge();

	if ( bHeadGib )
	{
		if ( !UTIL_IsLowViolence() && !IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) ) // Pyrovision check
		{
			CUtlVector<breakmodel_t> list;
			const int iClassIdx = GetPlayerClass()->GetClassIndex();
			for ( int i=0; i<m_aGibs.Count(); ++i )
			{
				breakmodel_t breakModel = m_aGibs[i];
				if ( !V_strcmp( breakModel.modelName, TFGameRules() && TFGameRules()->IsMvMModelsAllowed() ? g_pszBotHeadGibs[iClassIdx] : g_pszHeadGibs[iClassIdx] ) )
					list.AddToHead( breakModel );
			}

			m_hFirstGib = CreateGibsFromList( list, GetModelIndex(), NULL, breakParams, this, -1, false, true, &m_hSpawnedGibs, bBurning );
			if ( m_hFirstGib )
			{
				Vector velocity, impulse;
				IPhysicsObject *pPhys = m_hFirstGib->VPhysicsGetObject();
				if ( pPhys )
				{
					pPhys->GetVelocity( &velocity, &impulse );
					impulse.x *= 6.0f;
					pPhys->AddVelocity( &velocity, &impulse );
				}
			}
		}
	}
	else
	{
		m_hFirstGib = CreateGibsFromList( m_aGibs, GetModelIndex(), NULL, breakParams, this, -1, false, true, &m_hSpawnedGibs, bBurning );
	}

	// Gib skin numbers don't match player skin numbers so we gotta fix it up here.
	for ( int i = 0; i < m_hSpawnedGibs.Count(); i++ )
	{
		C_BaseAnimating *pGib = static_cast<C_BaseAnimating *>( m_hSpawnedGibs[i].Get() );

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				pGib->m_nSkin = 0;
				break;
			case TF_TEAM_BLUE:
				pGib->m_nSkin = 1;
				break;
			default:
				pGib->m_nSkin = 0;
				break;
			}
	}

	DropPartyHat( breakParams, vecBreakVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::DropPartyHat( breakablepropparams_t &breakParams, Vector &vecBreakVelocity )
{
	if ( m_hPartyHat )
	{
		breakmodel_t breakModel;
		Q_strncpy( breakModel.modelName, BDAY_HAT_MODEL, sizeof(breakModel.modelName) );
		breakModel.health = 1;
		breakModel.fadeTime = RandomFloat(5,10);
		breakModel.fadeMinDist = 0.0f;
		breakModel.fadeMaxDist = 0.0f;
		breakModel.burstScale = breakParams.defBurstScale;
		breakModel.collisionGroup = COLLISION_GROUP_DEBRIS;
		breakModel.isRagdoll = false;
		breakModel.isMotionDisabled = false;
		breakModel.placementName[0] = 0;
		breakModel.placementIsBone = false;
		breakModel.offset = GetAbsOrigin() - m_hPartyHat->GetAbsOrigin();
		BreakModelCreateSingle( this, &breakModel, m_hPartyHat->GetAbsOrigin(), m_hPartyHat->GetAbsAngles(), vecBreakVelocity, breakParams.angularVelocity, m_hPartyHat->m_nSkin, breakParams );

		m_hPartyHat->Release();
	}
}

//-----------------------------------------------------------------------------
// Purpose: How many buildables does this player own
//-----------------------------------------------------------------------------
int	C_TFPlayer::GetObjectCount( void )
{
	return m_aObjects.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get a specific buildable that this player owns
//-----------------------------------------------------------------------------
C_BaseObject *C_TFPlayer::GetObject( int index )
{
	return m_aObjects[index].Get();
}

//-----------------------------------------------------------------------------
// Purpose: Get a specific buildable that this player owns
//-----------------------------------------------------------------------------
C_BaseObject *C_TFPlayer::GetObjectOfType( int iObjectType, int iObjectMode )
{
	int iCount = m_aObjects.Count();

	for ( int i=0;i<iCount;i++ )
	{
		C_BaseObject *pObj = m_aObjects[i].Get();

		if ( !pObj )
			continue;

		if ( pObj->IsDormant() || pObj->IsMarkedForDeletion() )
			continue;

		if ( pObj->GetType() == iObjectType && pObj->GetObjectMode() == iObjectMode )
		{
			return pObj;
		}
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_TFPlayer::ShouldCollide( int collisionGroup, int contentsMask ) const
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

float C_TFPlayer::GetPercentInvisible( void )
{
	return m_Shared.GetPercentInvisible();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::GetSkin()
{
	C_TFPlayer *pLocalPlayer = GetLocalTFPlayer();
	if ( !pLocalPlayer )
		return 0;

	int nSkin;

	int iVisibleTeam = GetTeamNumber();

	// if this player is disguised and on the other team, use disguise team
	if ( m_Shared.InCond( TF_COND_DISGUISED ) && IsEnemyPlayer() )
	{
		iVisibleTeam = m_Shared.GetDisguiseTeam();
	}

	switch ( iVisibleTeam )
	{
	case TF_TEAM_RED:
		nSkin = 0;
		break;

	case TF_TEAM_BLUE:
		nSkin = 1;
		break;

	default:
		nSkin = 1;
		break;
	}

	int nZombie = 0;
	CALL_ATTRIB_HOOK_INT( nZombie, zombiezombiezombiezombie );
	if ( nZombie != 0 )
	{
		if ( IsPlayerClass( TF_CLASS_SPY ) )
			nSkin += 22;
		else
			nSkin += 4;
	}

	// 3 and 4 are invulnerable
	if ( m_Shared.IsInvulnerable() && ( !m_Shared.InCond( TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE ) || gpGlobals->curtime - m_flLastDamageTime < 2.0f ) )
	{
		nSkin += 2;
	}

	return nSkin;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iClass - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_TFPlayer::IsPlayerClass( int iClass )
{
	C_TFPlayerClass *pClass = GetPlayerClass();
	if ( !pClass )
		return false;

	return ( pClass->GetClassIndex() == iClass );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::IsPlayerNPCClass( void )
{
	return ( GetPlayerClass()->GetClassIndex() >= TF_CLASS_COMBINE );
}

//-----------------------------------------------------------------------------
// Purpose: Don't take damage decals while stealthed
//-----------------------------------------------------------------------------
void C_TFPlayer::AddDecal( const Vector& rayStart, const Vector& rayEnd,
							const Vector& decalCenter, int hitbox, int decalIndex, bool doTrace, trace_t& tr, int maxLODToDecal )
{
	if ( m_Shared.IsStealthed() )
		return;

	if ( m_Shared.InCond( TF_COND_DISGUISED ) )
		return;

	if ( m_Shared.IsInvulnerable() )
	{ 
		Vector vecDir = rayEnd - rayStart;
		VectorNormalize(vecDir);
		g_pEffects->Ricochet( rayEnd - (vecDir * 8), -vecDir );
		return;
	}

	if ( m_Shared.InCond( TF_COND_PHASE ) )
		return;

	// don't decal from inside the player
	if ( tr.startsolid )
		return;

	BaseClass::AddDecal( rayStart, rayEnd, decalCenter, hitbox, decalIndex, doTrace, tr, maxLODToDecal );
}

//-----------------------------------------------------------------------------
// Called every time the player respawns
//-----------------------------------------------------------------------------
void C_TFPlayer::ClientPlayerRespawn( void )
{
	if ( IsLocalPlayer() )
	{
		// Reset the camera.
		m_bWasTaunting = false;
		HandleTaunting();

		IGameEvent * event = gameeventmanager->CreateEvent( "localplayer_respawn" );
		if ( event )
		{
			gameeventmanager->FireEventClientSide( event );
		}

		ResetToneMapping(1.0);

		// Release the duck toggle key
		KeyUp( &in_ducktoggle, NULL ); 

		LoadInventory();

		/*CTFWearableDemoShield *pShield = GetEquippedDemoShield( this );
		if ( pShield )
		{
			CTFViewModel *pVM = dynamic_cast<CTFViewModel*>( GetViewModel( 0 ) );
			if ( pVM )
			{
				const char *pszModel = NULL;
				if ( pShield->HasItemDefinition() && pShield->GetItem() )
					pszModel = pShield->GetItem()->GetPlayerDisplayModel( GetPlayerClass()->GetClassIndex() );

				if ( pszModel && pszModel[0] != '\0' )
				{
					pVM->UpdateViewmodelAddon( pszModel );
				}
			}
		}*/
	}

	// Reset rage
	m_Shared.ResetRageSystem();

	// Delete monoculus
	UpdateDemomanEyeEffect( 0 );

	m_hFirstGib = NULL;
	m_hSpawnedGibs.Purge();

	ThirdPersonSwitch( false );

	// Update min. viewmodel
	CalcMinViewmodelOffset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::CreateSaveMeEffect( MedicCallerType eType )
{
	// Don't create them for the local player
	if ( !ShouldDrawThisPlayer() )
		return;

	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();

	// Only show the bubble to teammates and players who are on the our disguise team.
	if ( !InSameTeam( pLocalPlayer ) && !( m_Shared.InCond( TF_COND_DISGUISED ) && m_Shared.GetDisguiseTeam() == pLocalPlayer->GetTeamNumber() ) )
		return;

	if ( m_pSaveMeEffect )
	{
		ParticleProp()->StopEmission( m_pSaveMeEffect );
		m_pSaveMeEffect = NULL;
	}

	const char *pszEffect = "speech_mediccall";

	switch ( eType )
	{
	case TF_CALL_MEDIC:
		pszEffect = "speech_mediccall";
		break;

	case TF_CALL_BURNING:
		pszEffect = "speech_mediccall"; // extra particle on top?
		break;

	case TF_CALL_HEALTH:
		pszEffect = "speech_medichurt";
		break;

	case TF_CALL_BLEEDING:
		pszEffect = "speech_mediccall"; // extra particle on top?
		break;

	case TF_CALL_AUTO:
		pszEffect = "speech_mediccall";
		break;

	case TF_CALL_REVIVE_EASY:
		pszEffect = "speech_revivecall";
		break;

	case TF_CALL_REVIVE_MEDIUM:
		pszEffect = "speech_revivecall_medium";
		break;

	case TF_CALL_REVIVE_HARD:
		pszEffect = "speech_revivecall_hard";
		break;

	default:
		pszEffect = "speech_mediccall";
		break;
	}

	m_pSaveMeEffect = ParticleProp()->Create( pszEffect, PATTACH_POINT_FOLLOW, "head" );
	if ( m_pSaveMeEffect )
	{
		// Set "redness" of the bubble based on player's health.
		float flHealthRatio = clamp( (float)GetHealth() / (float)GetMaxHealth(), 0.0f, 1.0f );
		m_pSaveMeEffect->SetControlPoint( 1, Vector( flHealthRatio ) );
	}

	// If the local player is a medic, add this player to our list of medic callers
	if ( pLocalPlayer && pLocalPlayer->IsPlayerClass( TF_CLASS_MEDIC ) && pLocalPlayer->IsAlive() )
	{
		Vector vecPos;
		if ( GetAttachmentLocal( LookupAttachment( "head" ), vecPos ) )
		{
			vecPos += Vector(0,0,18);	// Particle effect is 18 units above the attachment
			CTFMedicCallerPanel::AddMedicCaller( this, 5.0, vecPos, eType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::CreateTauntWithMeEffect( void )
{
	// Don't create them for the local player
	if ( !ShouldDrawThisPlayer() )
		return;

	if ( m_pTauntWithMeEffect )
		return;

	const char *pszEffect = "speech_taunt_all";

	switch ( GetTeamNumber() )
	{
	case TF_TEAM_RED:
		pszEffect = "speech_taunt_red";
		break;

	case TF_TEAM_BLUE:
		pszEffect = "speech_taunt_blue";
		break;

	default:
		pszEffect = "speech_taunt_all";
		break;
	}

	m_pTauntWithMeEffect = ParticleProp()->Create( pszEffect, PATTACH_POINT_FOLLOW, "head" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::StopTauntWithMeEffect( void )
{
	if ( m_pTauntWithMeEffect )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_pTauntWithMeEffect );
		m_pTauntWithMeEffect = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::IsOverridingViewmodel( void )
{
	C_TFPlayer *pPlayer = this;
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pLocalPlayer && pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE && 
		 pLocalPlayer->GetObserverTarget() && pLocalPlayer->GetObserverTarget()->IsPlayer() )
	{
		pPlayer = assert_cast<C_TFPlayer*>(pLocalPlayer->GetObserverTarget());
	}

	if ( pPlayer->m_Shared.IsInvulnerable() && !pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE ) )
		return true;

	return BaseClass::IsOverridingViewmodel();
}

//-----------------------------------------------------------------------------
// Purpose: Draw my viewmodel in some special way
//-----------------------------------------------------------------------------
int	C_TFPlayer::DrawOverriddenViewmodel( C_BaseViewModel *pViewmodel, int flags )
{
	int ret = 0;

	C_TFPlayer *pPlayer = this;
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pLocalPlayer && pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE && 
		pLocalPlayer->GetObserverTarget() && pLocalPlayer->GetObserverTarget()->IsPlayer() )
	{
		pPlayer = assert_cast<C_TFPlayer*>(pLocalPlayer->GetObserverTarget());
	}

	if ( pPlayer->m_Shared.IsInvulnerable() && !pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE ) )
	{
		// Force the invulnerable material
		modelrender->ForcedMaterialOverride( *pPlayer->GetInvulnMaterialRef() );

		C_ViewmodelAttachmentModel *pVMAddon = dynamic_cast<C_ViewmodelAttachmentModel *>( pViewmodel );
		if ( pVMAddon )
			ret = pVMAddon->DrawOverriddenViewmodel( flags );
		else
			ret = pViewmodel->DrawOverriddenViewmodel( flags );

		modelrender->ForcedMaterialOverride( NULL );
	}

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::SetHealer( C_TFPlayer *pHealer, float flChargeLevel )
{
	// We may be getting healed by multiple healers. Show the healer
	// who's got the highest charge level.
	if ( m_hHealer )
	{
		if ( m_flHealerChargeLevel > flChargeLevel )
			return;
	}

	m_hHealer = pHealer;
	m_flHealerChargeLevel = flChargeLevel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::CanShowClassMenu( void )
{
	return ( GetTeamNumber() > LAST_SHARED_TEAM );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::InitializePoseParams( void )
{
	/*
	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );
	*/

	CStudioHdr *hdr = GetModelPtr();
	Assert( hdr );
	if ( !hdr )
		return;

	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector C_TFPlayer::GetChaseCamViewOffset( CBaseEntity *target )
{
	if ( target->IsBaseObject() )
		return Vector(0,0,64);

	if ( target->IsNPC() )
	{
		if ( target->IsAlive() )
		{
			// NPC eye height varies so use GetViewOffset.
			float flEyeHeight = target->GetViewOffset().z;
			return Vector( 0, 0, flEyeHeight );
		}
		else
		{
			// Assume it's ragdoll.
			return VEC_DEAD_VIEWHEIGHT;
		}
	}

	return BaseClass::GetChaseCamViewOffset( target );
}

//-----------------------------------------------------------------------------
// Purpose: Called from PostDataUpdate to update the model index
//-----------------------------------------------------------------------------
void C_TFPlayer::ValidateModelIndex( void )
{
	if ( m_Shared.InCond( TF_COND_DISGUISED_AS_DISPENSER ) && IsEnemyPlayer() && GetGroundEntity() && IsDucked() )
	{
		m_nModelIndex = modelinfo->GetModelIndex( "models/buildables/dispenser_light.mdl" );

		if ( GetLocalPlayer() != this )
			SetAbsAngles( vec3_angle );
	}
	else if ( m_Shared.InCond( TF_COND_DISGUISED ) && IsEnemyPlayer() )
	{
		TFPlayerClassData_t *pData = GetPlayerClassData( m_Shared.GetDisguiseClass() );

		if ( Q_stricmp( m_iszCustomModel, "" ) )
			m_nModelIndex = modelinfo->GetModelIndex( m_iszCustomModel );
		else
			m_nModelIndex = modelinfo->GetModelIndex( pData->GetModelName() );
	}
	else
	{
		C_TFPlayerClass *pClass = GetPlayerClass();
		if ( pClass )
		{
			if ( Q_stricmp( m_iszCustomModel, "" ) )
				m_nModelIndex = modelinfo->GetModelIndex( m_iszCustomModel );
			else
				m_nModelIndex = modelinfo->GetModelIndex( pClass->GetModelName() );
		}
	}

	if ( m_iSpyMaskBodygroup > -1 && GetModelPtr() != NULL )
	{
		SetBodygroup( m_iSpyMaskBodygroup, ( m_Shared.InCond( TF_COND_DISGUISED ) && !IsEnemyPlayer() ) );
	}

	BaseClass::ValidateModelIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Simulate the player for this frame
//-----------------------------------------------------------------------------
void C_TFPlayer::Simulate( void )
{
	//Frame updates
	if ( IsLocalPlayer() )
	{
		//Update the flashlight
		Flashlight();
	}

	// TF doesn't do step sounds based on velocity, instead using anim events
	// So we deliberately skip over the base player simulate, which calls them.
	BaseClass::BaseClass::Simulate();
}

//-----------------------------------------------------------------------------
// Purpose: send reliable stream overflow
//-----------------------------------------------------------------------------
void C_TFPlayer::LoadInventory( void )
{
	for ( int iClass = TF_FIRST_NORMAL_CLASS; iClass <= TF_LAST_NORMAL_CLASS; iClass++ )
	{
		for ( int iSlot = 0; iSlot < LOADOUT_POSITION_TAUNT2; iSlot++ )
		{
			if ( iSlot == LOADOUT_POSITION_UTILITY )
				continue;

			int iPreset = GetTFInventory()->GetWeaponPreset( iClass, iSlot );
			char szCmd[64];
			Q_snprintf( szCmd, sizeof( szCmd ), "weaponpresetclass %d %d %d;", iClass, iSlot, iPreset );
			engine->ExecuteClientCmd( szCmd );
		}
	}
}

void C_TFPlayer::EditInventory( int iSlot, int iWeapon )
{
	int iClass = GetPlayerClass()->GetClassIndex();
	GetTFInventory()->SetWeaponPreset( iClass, iSlot, iWeapon );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	if ( event == 7001 )
	{
		// Force a footstep sound
		m_flStepSoundTime = 0;
		Vector vel;
		EstimateAbsVelocity( vel );
		UpdateStepSound( GetGroundSurface(), GetAbsOrigin(), vel );

		bool bInWater = ( enginetrace->GetPointContents(origin) & CONTENTS_WATER );
		if( bInWater )
		{
			//run splash
			CEffectData data;

			//trace up from foot position to the water surface
			trace_t tr;
			Vector vecTrace(0,0,1024);
			UTIL_TraceLine( origin, origin + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );
			if ( tr.fractionleftsolid )
			{
				data.m_vOrigin = origin + (vecTrace * tr.fractionleftsolid);
			}
			else
			{
				data.m_vOrigin = origin;
			}
			
			data.m_vNormal = Vector( 0,0,1 );
			data.m_flScale = random->RandomFloat( 6.0f, 8.0f );
			DispatchEffect( "watersplash", data );
		}
	}
	else if ( event == AE_CL_BODYGROUP_SET_VALUE_CMODEL_WPN )
	{
		C_TFWeaponBase *pTFWeapon = GetActiveTFWeapon();
		if ( pTFWeapon )
		{
			C_BaseAnimating *vm = pTFWeapon->GetAppropriateWorldOrViewModel();
			if ( vm )
			{
				vm->FireEvent( origin, angles, AE_CL_BODYGROUP_SET_VALUE, options );
			}
		}
	}
	else if ( event == AE_WPN_HIDE )
	{
		for ( int i = 0; i < MAX_WEAPONS; i++ )
		{
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetWeapon( i );
			if ( pWeapon )
			{
				int nDisableHide = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nDisableHide, disable_weapon_hiding_for_animations );
				if ( nDisableHide == 0 )
					pWeapon->SetWeaponVisible( false );
			}
		}
	}
	else if ( event == AE_WPN_UNHIDE )
	{
		for ( int i = 0; i < MAX_WEAPONS; i++ )
		{
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetWeapon( i );
			if ( pWeapon )
			{
				pWeapon->SetWeaponVisible( true );
			}
		}
	}
	else if ( event == AE_WPN_PLAYWPNSOUND )
	{
		if ( GetActiveWeapon() )
		{
			WeaponSound_t nWeaponSound = EMPTY;
			GetWeaponSoundFromString( options );
			if ( nWeaponSound != EMPTY )
			{
				GetActiveWeapon()->WeaponSound( nWeaponSound );
			}
		}
	}
	else if ( event == AE_TAUNT_ENABLE_MOVE )
	{
		m_bAllowMoveDuringTaunt = true;
	}
	else if ( event == AE_TAUNT_DISABLE_MOVE )
	{
		m_bAllowMoveDuringTaunt = false;
	}
	else if ( event == AE_TAUNT_ADD_ATTRIBUTE )
	{
		//Usage: AE_TAUNT_ADD_ATTRIBUTE <frame> <attr_name> <attr_value> <duration>
	}
	else if ( event == TF_AE_CIGARETTE_THROW )
	{
		CEffectData data;
		int iAttach = LookupAttachment( options );
		GetAttachment( iAttach, data.m_vOrigin, data.m_vAngles );

		data.m_vAngles = GetRenderAngles();

		data.m_hEntity = ClientEntityList().EntIndexToHandle( entindex() );
		DispatchEffect( "TF_ThrowCigarette", data );
		return;
	}
	else
	{
		BaseClass::FireEvent( origin, angles, event, options );
	}
}

// Shadows

ConVar cl_blobbyshadows( "cl_blobbyshadows", "0", FCVAR_CLIENTDLL );

ShadowType_t C_TFPlayer::ShadowCastType( void ) 
{
	// Removed the GetPercentInvisible - should be taken care off in BindProxy now.
	if ( !IsVisible() /*|| GetPercentInvisible() > 0.0f*/ )
		return SHADOWS_NONE;

	if ( IsEffectActive(EF_NODRAW | EF_NOSHADOW) )
		return SHADOWS_NONE;

	// If in ragdoll mode.
	if ( m_nRenderFX == kRenderFxRagdoll )
		return SHADOWS_NONE;

	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();

	// if we're first person spectating this player
	if ( pLocalPlayer && 
		pLocalPlayer->GetObserverTarget() == this &&
		pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		return SHADOWS_NONE;		
	}

	if( cl_blobbyshadows.GetBool() )
		return SHADOWS_SIMPLE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}

float g_flFattenAmt = 4;
void C_TFPlayer::GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType )
{
	if ( shadowType == SHADOWS_SIMPLE )
	{
		// Don't let the render bounds change when we're using blobby shadows, or else the shadow
		// will pop and stretch.
		mins = CollisionProp()->OBBMins();
		maxs = CollisionProp()->OBBMaxs();
	}
	else
	{
		GetRenderBounds( mins, maxs );

		// We do this because the normal bbox calculations don't take pose params into account, and 
		// the rotation of the guy's upper torso can place his gun a ways out of his bbox, and 
		// the shadow will get cut off as he rotates.
		//
		// Thus, we give it some padding here.
		mins -= Vector( g_flFattenAmt, g_flFattenAmt, 0 );
		maxs += Vector( g_flFattenAmt, g_flFattenAmt, 0 );
	}
}


void C_TFPlayer::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	// TODO POSTSHIP - this hack/fix goes hand-in-hand with a fix in CalcSequenceBoundingBoxes in utils/studiomdl/simplify.cpp.
	// When we enable the fix in CalcSequenceBoundingBoxes, we can get rid of this.
	//
	// What we're doing right here is making sure it only uses the bbox for our lower-body sequences since,
	// with the current animations and the bug in CalcSequenceBoundingBoxes, are WAY bigger than they need to be.
	C_BaseAnimating::GetRenderBounds( theMins, theMaxs );
}


bool C_TFPlayer::GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const
{ 
	if ( shadowType == SHADOWS_SIMPLE )
	{
		// Blobby shadows should sit directly underneath us.
		pDirection->Init( 0, 0, -1 );
		return true;
	}
	else
	{
		return BaseClass::GetShadowCastDirection( pDirection, shadowType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether this player is the nemesis of the local player
//-----------------------------------------------------------------------------
bool C_TFPlayer::IsNemesisOfLocalPlayer()
{
	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( pLocalPlayer )
	{
		// return whether this player is dominating the local player
		return m_Shared.IsPlayerDominated( pLocalPlayer->entindex() );
	}		
	return false;
}

extern ConVar tf_tournament_hide_domination_icons;
//-----------------------------------------------------------------------------
// Purpose: Returns whether we should show the nemesis icon for this player
//-----------------------------------------------------------------------------
bool C_TFPlayer::ShouldShowNemesisIcon()
{
	// we should show the nemesis effect on this player if he is the nemesis of the local player,
	// and is not dead, cloaked or disguised
	if ( IsNemesisOfLocalPlayer() && g_PR && g_PR->IsConnected( entindex() ) )
	{
		bool bStealthed = m_Shared.IsStealthed();
		bool bDisguised = m_Shared.InCond( TF_COND_DISGUISED );
		bool bTournamentHide = TFGameRules()->IsInTournamentMode() && tf_tournament_hide_domination_icons.GetBool();
		if ( IsAlive() && !bStealthed && !bDisguised && !bTournamentHide )
			return true;
	}
	return false;
}

bool C_TFPlayer::IsWeaponLowered( void )
{
	CTFWeaponBase *pWeapon = GetActiveTFWeapon();

	if ( !pWeapon )
		return false;

	CTFGameRules *pRules = TFGameRules();

	// Lower losing team's weapons in bonus round
	if ( ( pRules->State_Get() == GR_STATE_TEAM_WIN ) && ( pRules->GetWinningTeam() != GetTeamNumber() ) )
		return true;

	// Hide all view models after the game is over
	if ( pRules->State_Get() == GR_STATE_GAME_OVER )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	switch ( event->GetType() )
	{
	case CChoreoEvent::SEQUENCE:
	case CChoreoEvent::GESTURE:
		return StartGestureSceneEvent( info, scene, event, actor, pTarget );
	default:
		return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::StartGestureSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	// Get the (gesture) sequence.
	info->m_nSequence = LookupSequence( event->GetParameters() );
	if ( info->m_nSequence < 0 )
		return false;

	// Player the (gesture) sequence.
	m_PlayerAnimState->AddVCDSequenceToGestureSlot( GESTURE_SLOT_VCD, info->m_nSequence );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::GetNumActivePipebombs( void )
{
	if ( IsPlayerClass( TF_CLASS_DEMOMAN ) )
	{
		CTFPipebombLauncher *pWeapon = dynamic_cast < CTFPipebombLauncher*>( Weapon_OwnsThisID( TF_WEAPON_PIPEBOMBLAUNCHER ) );

		if ( pWeapon )
		{
			return pWeapon->GetPipeBombCount();
		}
	}

	return 0;
}

bool C_TFPlayer::IsAllowedToSwitchWeapons( void )
{
	if ( IsWeaponLowered() == true )
		return false;

	if ( m_Shared.InCond( TF_COND_STUNNED ) )
		return false;

	return BaseClass::IsAllowedToSwitchWeapons();
}

IMaterial *C_TFPlayer::GetHeadLabelMaterial( void )
{
	if ( g_pHeadLabelMaterial[0] == NULL )
		SetupHeadLabelMaterials();

	switch ( GetTeamNumber() )
	{
		case TF_TEAM_RED:
			return g_pHeadLabelMaterial[TF_PLAYER_HEAD_LABEL_RED];
			break;

		case TF_TEAM_BLUE:
			return g_pHeadLabelMaterial[TF_PLAYER_HEAD_LABEL_BLUE];
			break;

		default:
			return g_pHeadLabelMaterial[TF_PLAYER_HEAD_LABEL_RED];
			break;
	}

	return BaseClass::GetHeadLabelMaterial();
}

void SetupHeadLabelMaterials( void )
{
	for ( int i = 0; i < (TF_TEAM_COUNT - 2); i++ )
	{
		if ( g_pHeadLabelMaterial[i] )
		{
			g_pHeadLabelMaterial[i]->DecrementReferenceCount();
			g_pHeadLabelMaterial[i] = NULL;
		}

		g_pHeadLabelMaterial[i] = materials->FindMaterial( pszHeadLabelNames[i], TEXTURE_GROUP_VGUI );
		if ( g_pHeadLabelMaterial[i] )
		{
			g_pHeadLabelMaterial[i]->IncrementReferenceCount();
		}
	}
}

void C_TFPlayer::ComputeFxBlend( void )
{
	BaseClass::ComputeFxBlend();

	float flInvisible = GetPercentInvisible();
	if ( flInvisible != 0.0f )
	{
		// Tell our shadow
		ClientShadowHandle_t hShadow = GetShadowHandle();
		if ( hShadow != CLIENTSHADOW_INVALID_HANDLE )
		{
			g_pClientShadowMgr->SetFalloffBias( hShadow, flInvisible * 255 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	DetectAndHandlePortalTeleportation();
	//if( DetectAndHandlePortalTeleportation() )
	//	DevMsg( "Teleported within OnDataChanged\n" );

	m_iForceNoDrawInPortalSurface = -1;
	bool bEyeTransform_Backup = m_bEyePositionIsTransformedByPortal;
	m_bEyePositionIsTransformedByPortal = false; //assume it's not transformed until it provably is
	UpdatePortalEyeInterpolation();

	QAngle qEyeAngleBackup = EyeAngles();
	Vector ptEyePositionBackup = EyePosition();
	C_Prop_Portal *pPortalBackup = m_hPortalEnvironment.Get();

	HandleTaunting();

	IClientVehicle *pVehicle; 
	pVehicle = GetVehicle();

	if ( !pVehicle )
	{
		if ( IsObserver() )
		{
			CalcObserverView( eyeOrigin, eyeAngles, fov );
		}
		else
		{
			CalcPlayerView( eyeOrigin, eyeAngles, fov );
			if( m_hPortalEnvironment.Get() != NULL )
			{
				//time for hax
				m_bEyePositionIsTransformedByPortal = bEyeTransform_Backup;
				CalcPortalView( eyeOrigin, eyeAngles );
			}
		}
	}
	else
	{
		CalcVehicleView( pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov );
	}

	m_qEyeAngles_LastCalcView = qEyeAngleBackup;
	m_ptEyePosition_LastCalcView = ptEyePositionBackup;
	m_pPortalEnvironment_LastCalcView = pPortalBackup;

	BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );
}

void C_TFPlayer::CalcPortalView( Vector &eyeOrigin, QAngle &eyeAngles )
{
	//although we already ran CalcPlayerView which already did these copies, they also fudge these numbers in ways we don't like, so recopy
	VectorCopy( EyePosition(), eyeOrigin );
	VectorCopy( EyeAngles(), eyeAngles );

	//Re-apply the screenshake (we just stomped it)
	vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );

	C_Prop_Portal *pPortal = m_hPortalEnvironment.Get();
	assert( pPortal );

	C_Prop_Portal *pRemotePortal = pPortal->m_hLinkedPortal;
	if( !pRemotePortal )
	{
		return; //no hacks possible/necessary
	}

	Vector ptPortalCenter;
	Vector vPortalForward;

	ptPortalCenter = pPortal->GetNetworkOrigin();
	pPortal->GetVectors( &vPortalForward, NULL, NULL );
	float fPortalPlaneDist = vPortalForward.Dot( ptPortalCenter );

	bool bOverrideSpecialEffects = false; //sometimes to get the best effect we need to kill other effects that are simply for cleanliness

	float fEyeDist = vPortalForward.Dot( eyeOrigin ) - fPortalPlaneDist;
	bool bTransformEye = false;
	if( fEyeDist < 0.0f ) //eye behind portal
	{
		if( pPortal->m_PortalSimulator.EntityIsInPortalHole( this ) ) //player standing in portal
		{
			bTransformEye = true;
		}
		else if( vPortalForward.z < -0.01f ) //there's a weird case where the player is ducking below a ceiling portal. As they unduck their eye moves beyond the portal before the code detects that they're in the portal hole.
		{
			Vector ptPlayerOrigin = GetAbsOrigin();
			float fOriginDist = vPortalForward.Dot( ptPlayerOrigin ) - fPortalPlaneDist;

			if( fOriginDist > 0.0f )
			{
				float fInvTotalDist = 1.0f / (fOriginDist - fEyeDist); //fEyeDist is negative
				Vector ptPlaneIntersection = (eyeOrigin * fOriginDist * fInvTotalDist) - (ptPlayerOrigin * fEyeDist * fInvTotalDist);
				Assert( fabs( vPortalForward.Dot( ptPlaneIntersection ) - fPortalPlaneDist ) < 0.01f );

				Vector vIntersectionTest = ptPlaneIntersection - ptPortalCenter;

				Vector vPortalRight, vPortalUp;
				pPortal->GetVectors( NULL, &vPortalRight, &vPortalUp );

				if( (vIntersectionTest.Dot( vPortalRight ) <= PORTAL_HALF_WIDTH) &&
					(vIntersectionTest.Dot( vPortalUp ) <= PORTAL_HALF_HEIGHT) )
				{
					bTransformEye = true;
				}
			}
		}		
	}

	if( bTransformEye )
	{
		m_bEyePositionIsTransformedByPortal = true;

		//DevMsg( 2, "transforming portal view from <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		VMatrix matThisToLinked = pPortal->MatrixThisToLinked();
		UTIL_Portal_PointTransform( matThisToLinked, eyeOrigin, eyeOrigin );
		UTIL_Portal_AngleTransform( matThisToLinked, eyeAngles, eyeAngles );

		//DevMsg( 2, "transforming portal view to   <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		if ( IsToolRecording() )
		{
			static EntityTeleportedRecordingState_t state;

			KeyValues *msg = new KeyValues( "entity_teleported" );
			msg->SetPtr( "state", &state );
			state.m_bTeleported = false;
			state.m_bViewOverride = true;
			state.m_vecTo = eyeOrigin;
			state.m_qaTo = eyeAngles;
			MatrixInvert( matThisToLinked.As3x4(), state.m_teleportMatrix );

			// Post a message back to all IToolSystems
			Assert( (int)GetToolHandle() != 0 );
			ToolFramework_PostToolMessage( GetToolHandle(), msg );

			msg->deleteThis();
		}

		bOverrideSpecialEffects = true;
	}
	else
	{
		m_bEyePositionIsTransformedByPortal = false;
	}

	if( bOverrideSpecialEffects )
	{		
		m_iForceNoDrawInPortalSurface = ((pRemotePortal->m_bIsPortal2)?(2):(1));
		pRemotePortal->m_fStaticAmount = 0.0f;
	}
}

bool LocalPlayerIsCloseToPortal( void )
{
	return C_TFPlayer::GetLocalTFPlayer()->IsCloseToPortal();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::ForceUpdateObjectHudState( void )
{
	m_bUpdateObjectHudState = true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the weapon passed in would occupy a slot already occupied by the carrier
// Input  : *pWeapon - weapon to test for
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_TFPlayer::Weapon_SlotOccupied( CBaseCombatWeapon *pWeapon )
{
	if ( pWeapon == NULL )
		return false;

	//Check to see if there's a resident weapon already in this slot
	if ( Weapon_GetSlot( pWeapon->GetSlot() ) == NULL )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the weapon (if any) in the requested slot
// Input  : slot - which slot to poll
//-----------------------------------------------------------------------------
CBaseCombatWeapon *C_TFPlayer::Weapon_GetSlot( int slot ) const
{
	int	targetSlot = slot;

	// Check for that slot being occupied already
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( GetWeapon(i) != NULL )
		{
			// If the slots match, it's already occupied
			if ( GetWeapon(i)->GetSlot() == targetSlot )
				return GetWeapon(i);
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::FireGameEvent( IGameEvent *event )
{
	if ( V_strcmp( event->GetName(), "localplayer_changeteam" ) == 0 )
	{
		if ( !IsLocalPlayer() )
		{
			// Update any effects affected by disguise.
			m_Shared.UpdateCritBoostEffect();
			UpdateOverhealEffect();
			UpdateRecentlyTeleportedEffect();
		}
	}
	else
	{
		m_Shared.FireGameEvent( event );
		BaseClass::FireGameEvent( event );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateTypingBubble( void )
{
	// Don't show the bubble for local player since they don't need it.
	if ( IsLocalPlayer() )
		return;

	if ( m_bTyping && IsAlive() && ( !m_Shared.IsStealthed() || !IsEnemyPlayer() ) )
	{
		if ( !m_pTypingEffect )
		{
			m_pTypingEffect = ParticleProp()->Create( "speech_typing", PATTACH_POINT_FOLLOW, "head" );
		}
	}
	else
	{
		if ( m_pTypingEffect )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_pTypingEffect );
			m_pTypingEffect = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateOverhealEffect( void )
{
	bool bShouldShow = true;

	if ( !m_Shared.InCond( TF_COND_HEALTH_OVERHEALED ) )
	{
		bShouldShow = false;
	}
	else if ( InFirstPersonView() )
	{
		bShouldShow = false;
	}
	else if ( IsEnemyPlayer() )
	{
		if ( m_Shared.IsStealthed() || m_Shared.InCond( TF_COND_DISGUISED ) )
		{
			// Don't give away cloaked and disguised spies.
			bShouldShow = false;
		}
	}

	if ( bShouldShow )
	{
		if ( !m_pOverhealEffect )
		{
			const char *pszEffect = ConstructTeamParticle( "overhealedplayer_%s_pluses", GetTeamNumber() );
			m_pOverhealEffect = ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
		}
	}
	else
	{
		if ( m_pOverhealEffect )
		{
			ParticleProp()->StopEmission( m_pOverhealEffect );
			m_pOverhealEffect = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateDemomanEyeEffect( int iDecapCount )
{
	if ( m_pDemoEyeEffect )
	{
		ParticleProp()->StopEmission( m_pDemoEyeEffect );
		m_pDemoEyeEffect = NULL;
	}

	if ( iDecapCount == 0 )
		return;

	iDecapCount = Min( iDecapCount, 4 );
	switch (iDecapCount)
	{
		case 1:
			m_pDemoEyeEffect = ParticleProp()->Create( "eye_powerup_green_lvl_1", PATTACH_POINT_FOLLOW, "eyeglow_L" );
			break;
		case 2:
			m_pDemoEyeEffect = ParticleProp()->Create( "eye_powerup_green_lvl_2", PATTACH_POINT_FOLLOW, "eyeglow_L" );
			break;
		case 3:
			m_pDemoEyeEffect = ParticleProp()->Create( "eye_powerup_green_lvl_3", PATTACH_POINT_FOLLOW, "eyeglow_L" );
			break;
		case 4:
			m_pDemoEyeEffect = ParticleProp()->Create( "eye_powerup_green_lvl_4", PATTACH_POINT_FOLLOW, "eyeglow_L" );
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFPlayer::UpdateRuneIcon( bool bHasRune )
{
	if ( bHasRune )
	{
		if ( !m_pRuneEffect && !IsLocalPlayer() )
			m_pRuneEffect = ParticleProp()->Create( GetPowerupIconName( m_Shared.GetCarryingRuneType(), GetTeamNumber() ), PATTACH_POINT_FOLLOW, "partyhat" );
	}
	else
	{
		if ( m_pRuneEffect )
		{
			ParticleProp()->StopEmission( m_pRuneEffect );
			m_pRuneEffect = NULL;
		}
	}
}

static void cc_tf_crashclient()
{
	C_TFPlayer *pPlayer = NULL;
	pPlayer->ComputeFxBlend();
}
static ConCommand tf_crashclient( "tf_crashclient", cc_tf_crashclient, "Crashes this client for testing.", FCVAR_DEVELOPMENTONLY );

#include "c_obj_sentrygun.h"


static void cc_tf_debugsentrydmg()
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	pPlayer->UpdateIDTarget();
	int iTarget = pPlayer->GetIDTarget();
	if ( iTarget > 0 )
	{
		C_BaseEntity *pEnt = cl_entitylist->GetEnt( iTarget );

		C_ObjectSentrygun *pSentry = dynamic_cast< C_ObjectSentrygun * >( pEnt );

		if ( pSentry )
		{
			pSentry->DebugDamageParticles();
		}
	}
}
static ConCommand tf_debugsentrydamage( "tf_debugsentrydamage", cc_tf_debugsentrydmg, "", FCVAR_DEVELOPMENTONLY );

vgui::IImage* GetDefaultAvatarImage( C_BasePlayer *pPlayer )
{
	if ( pPlayer )
	{
		switch ( pPlayer->GetTeamNumber() )
		{
		case TF_TEAM_RED:
		{
			static vgui::IImage *pRedAvatar = scheme()->GetImage( "../vgui/avatar_default_red", true );
			return pRedAvatar;
		}
		case TF_TEAM_BLUE:
		{
			static vgui::IImage *pBlueAvatar = scheme()->GetImage( "../vgui/avatar_default_blue", true );
			return pBlueAvatar;
		}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void C_TFPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_hLadder = NULL;
}

//-----------------------------------------------------------------------------
// Should this object receive shadows?
//-----------------------------------------------------------------------------
bool C_TFPlayer::ShouldReceiveProjectedTextures( int flags )
{
	Assert( flags & SHADOW_FLAGS_PROJECTED_TEXTURE_TYPE_MASK );

	if ( IsEffectActive( EF_NODRAW ) )
		 return false;

	if( flags & SHADOW_FLAGS_FLASHLIGHT )
	{
		return true;
	}

	return BaseClass::ShouldReceiveProjectedTextures( flags );
}

void C_TFPlayer::UpdateFlashlight()
{
	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pPlayer )
		return;

	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		if (!m_pFlashlight)
		{
			// Turned on the headlight; create it.
			m_pFlashlight = new CFlashlightEffect(index);

			if (!m_pFlashlight)
				return;

			m_pFlashlight->TurnOn();
		}

		Vector vecForward, vecRight, vecUp;
		EyeVectors( &vecForward, &vecRight, &vecUp );

		// mimics l4d flashlight offset.
		//flashlight origin is the player pos if a weapon isn't detected.
		Vector vecOrigin = EyePosition();
		QAngle angFlashlightAngle = EyeAngles();

		int iAttachment = pPlayer->LookupAttachment( "camera" );

		C_TFViewModel *pViewModel = dynamic_cast<C_TFViewModel *>( GetViewModel() );
		if ( !ShouldDrawThisPlayer() )
		{
			if ( GetActiveTFWeapon() && GetActiveTFWeapon()->IsMeleeWeapon() )
			{
				Vector aimFwd;
				AngleVectors( angFlashlightAngle, &aimFwd );
				vecOrigin += aimFwd * ( VEC_HULL_MAX ).Length2D();
			}
			else
			{
				if ( pViewModel )
				{
					pViewModel->GetAttachment( pViewModel->LookupAttachment( "muzzle" ), vecOrigin, angFlashlightAngle );
					iAttachment = pViewModel->LookupAttachment( "muzzle" );
				}
			}
		}
		else
		{
			Vector aimFwd;
			AngleVectors( angFlashlightAngle, &aimFwd );
			vecOrigin += aimFwd * ( VEC_HULL_MAX ).Length2D();
		}

		AngleVectors( angFlashlightAngle, &vecForward, &vecRight, &vecUp );

		m_pFlashlight->UpdateLight( vecOrigin, vecForward, vecRight, vecUp, TF_FLASHLIGHT_DISTANCE );

		/*if ( ShouldDrawThisPlayer() )
		{
			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + (vecForward * 100), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			BeamInfo_t beamInfo;

			if ( !m_pFlashlightBeam )
			{
				beamInfo.m_nType		= TE_BEAMPOINTS;
				beamInfo.m_vecStart		= tr.startpos;
				beamInfo.m_vecEnd		= tr.endpos;
				//beamInfo.m_nStartAttachment = iAttachment;
				beamInfo.m_pszModelName = "sprites/glow_test02.vmt"; 
				beamInfo.m_pszHaloName	= "sprites/light_glow03.vmt";
				beamInfo.m_flHaloScale	= 3.0f;
				beamInfo.m_flWidth		= 13.0f;
				beamInfo.m_flEndWidth	= 10.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude	= 0;
				beamInfo.m_flBrightness = 20.0f;
				beamInfo.m_flSpeed		= 0.0f;
				beamInfo.m_nStartFrame	= 0.0;
				beamInfo.m_flFrameRate	= 0.0;
				beamInfo.m_flRed		= 255.0;
				beamInfo.m_flGreen		= 255.0;
				beamInfo.m_flBlue		= 255.0;
				beamInfo.m_nSegments	= 10.0f;
				beamInfo.m_bRenderable	= true;
				beamInfo.m_flLife		= 0.5;
				beamInfo.m_nFlags		= FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

				m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
			}
			else
			{
				beamInfo.m_vecStart		= tr.startpos;
				beamInfo.m_vecEnd		= tr.endpos;
				beamInfo.m_flRed		= 255.0;
				beamInfo.m_flGreen		= 255.0;
				beamInfo.m_flBlue		= 255.0;
				beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );
			}
		}*/
	}
	else
	{
		// Turned off the flashlight; delete it.
		if ( m_pFlashlight )
		{
			delete m_pFlashlight;
			m_pFlashlight = NULL;
		}

		/*if ( m_pFlashlightBeam )
		{
			m_pFlashlightBeam->flags	= 0;
			m_pFlashlightBeam->die		= gpGlobals->curtime - 1;
			delete m_pFlashlightBeam;
			m_pFlashlightBeam = NULL;
		}*/
	}

	//BaseClass::UpdateFlashlight();
}

//-----------------------------------------------------------------------------
// Purpose: Creates player flashlight if it's alive
//-----------------------------------------------------------------------------
void C_TFPlayer::Flashlight( void )
{
	UpdateFlashlight();

	BaseClass::Flashlight();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_TFPlayer::GetVisionFilterFlags( bool bWeaponsCheck )
{
	int nVisionFlag = TF_VISION_FILTER_NONE;

	if ( bWeaponsCheck && IsLocalPlayer() )
	{
		int nAttribVisionFlag = TF_VISION_FILTER_PYRO;
		for ( int i = 0; i < MAX_WEAPONS; i++ )
		{
			CTFWeaponBase *pWeapon = (CTFWeaponBase *)GetWeapon( i );
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWeapon, nAttribVisionFlag, vision_opt_in_flags );
		}

		for ( int i = 0; i < GetNumWearables(); i++ )
		{
			CEconWearable *pWearable = GetWearable( i );
			CALL_ATTRIB_HOOK_INT_ON_OTHER( pWearable, nAttribVisionFlag, vision_opt_in_flags );
		}

		if ( nAttribVisionFlag == 1 )
			nVisionFlag |= TF_VISION_FILTER_PYRO;
		if ( nAttribVisionFlag == 2 )
			nVisionFlag |= TF_VISION_FILTER_HALLOWEEN;
		if ( nAttribVisionFlag == 4 )
			nVisionFlag |= TF_VISION_FILTER_ROME;
	}

	if ( cl_pyrovision.GetBool() )
		nVisionFlag |= TF_VISION_FILTER_PYRO;

	if ( TFGameRules()->IsHolidayActive( kHoliday_Halloween ) )
		nVisionFlag |= TF_VISION_FILTER_HALLOWEEN;
	//else
	//	nVisionFlag &= ~TF_VISION_FILTER_HALLOWEEN;

	/*if ( TFGameRules()->IsMannVsMachineMode() && somethingrome )
		nVisionFlag |= TF_VISION_FILTER_ROME;*/

	if ( cl_romevision.GetBool() )
		nVisionFlag |= TF_VISION_FILTER_ROME;

	return nVisionFlag;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_TFPlayer::HasVisionFilterFlags( int nFlags, bool bWeaponsCheck )
{
	return ( GetVisionFilterFlags( bWeaponsCheck ) & nFlags ) == nFlags; 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::CalculateVisionUsingCurrentFlags( void )
{
	BaseClass::CalculateVisionUsingCurrentFlags();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFPlayer::OnAchievementAchieved( int iAchievement )
{
	if ( !IsAlive() )
		return;

	EmitSound( "Achievement.Earned" );

	/*CBaseAchievement *pAchievement = engine->GetAchievementMgr()->GetAchievementByID( iAchievement );
	if ( pAchievement )
	{
		
	}*/

	switch ( iAchievement )
	{
		// hard
	case ACHIEVEMENT_LFE_BEAT_HL2:
	case ACHIEVEMENT_LFE_BEAT_HL2EP1:
	case ACHIEVEMENT_LFE_BEAT_HL2EP2:
	case ACHIEVEMENT_LFE_BEAT_HLS:
	case ACHIEVEMENT_LFE_BEAT_P1:
		EmitSound( "Quest.StatusTickExpertCompletePDA" );
		ParticleProp()->Create( "contract_completed_bonus", PATTACH_POINT_FOLLOW, "partyhat" );
		break;

	// medium
	case ACHIEVEMENT_LFE_KILL_COMBINE:
	case ACHIEVEMENT_LFE_KILL_ZOMBIE:
	case ACHIEVEMENT_LFE_KILL_ANTLION:
		EmitSound( "Quest.StatusTickAdvancedCompletePDA" );
		ParticleProp()->Create( "contract_completed_primary", PATTACH_POINT_FOLLOW, "partyhat" );
		break;

	// easy
	default:
		EmitSound( "Quest.StatusTickNoviceCompletePDA" );
		break;
	}
}