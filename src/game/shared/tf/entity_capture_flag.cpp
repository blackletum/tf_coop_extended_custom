//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: CTF Flag.
//
//=============================================================================//
#include "cbase.h"
#include "entity_capture_flag.h"
#include "tf_gamerules.h"
#include "tf_shareddefs.h"

#ifdef CLIENT_DLL
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui/IScheme.h>
#include "hudelement.h"
#include "iclientmode.h"
#include "hud_numericdisplay.h"
#include "tf_imagepanel.h"
#include "c_tf_player.h"
#include "c_tf_team.h"
#include "tf_hud_objectivestatus.h"
#include "view.h"
#include "glow_outline_effect.h"
#include "c_ai_basenpc.h"

extern CUtlVector<int>	g_Flags;

ConVar cl_flag_return_size( "cl_flag_return_size", "20", FCVAR_CHEAT );

#else
#include "tf_player.h"
#include "tf_team.h"
#include "tf_objective_resource.h"
#include "tf_gamestats.h"
#include "func_respawnroom.h"
#include "datacache/imdlcache.h"
#include "func_respawnflag.h"
#include "func_flagdetectionzone.h"
#include "ai_basenpc.h"

extern ConVar tf_flag_caps_per_round;

ConVar cl_flag_return_height( "cl_flag_return_height", "82", FCVAR_CHEAT );

#endif


#ifdef CLIENT_DLL

static void RecvProxy_IsDisabled( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CCaptureFlag *pFlag = (CCaptureFlag *) pStruct;
	bool bIsDisabled = ( pData->m_Value.m_Int > 0 );

	if ( pFlag )
	{
		pFlag->SetDisabled( bIsDisabled ); 
	}
}

static void RecvProxy_FlagStatus( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CCaptureFlag *pFlag = (CCaptureFlag *) pStruct;

	if ( pFlag )
	{
		pFlag->UpdateGlowEffect();
		pFlag->m_nFlagStatus = pData->m_Value.m_Int;
	}
}


#else
CON_COMMAND_F( lfe_flag_attachment_position, "Set flags position & angles.\n\tFormat: lfe_flag_attachment_position <X> <Y> <Z> <Pitch> <Yaw> <Roll>", FCVAR_CHEAT )
{
	if ( args.ArgC() < 3 )
	{
		Msg( "Too few parameters.  lfe_flag_attachment_position <bot name> <X> <Y> <Z> <Pitch> <Yaw> <Roll>\n" );
		return;
	}

	Vector vecPos( atof(args[1]), atof(args[2]), atof(args[3]) );
	QAngle vecAng( atof(args[4]), atof(args[5]), atof(args[6]) );

	for ( int i=0; i<ICaptureFlagAutoList::AutoList().Count(); ++i )
	{
		CCaptureFlag *pFlag = static_cast<CCaptureFlag *>( ICaptureFlagAutoList::AutoList()[i] );
		if ( pFlag )
		{
			pFlag->SetLocalOrigin( vecPos );
			pFlag->SetLocalAngles( vecAng );
		}
	}
}
#endif

//=============================================================================
//
// CTF Flag tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( CaptureFlag, DT_CaptureFlag )

BEGIN_NETWORK_TABLE( CCaptureFlag, DT_CaptureFlag )

#ifdef GAME_DLL
	SendPropBool( SENDINFO( m_bDisabled ) ),
	SendPropInt( SENDINFO( m_nGameType ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nFlagStatus ), 3, SPROP_UNSIGNED ),
	SendPropTime( SENDINFO( m_flResetTime ) ),
	SendPropTime( SENDINFO( m_flNeutralTime ) ),
	SendPropTime( SENDINFO( m_flMaxResetTime ) ),
	SendPropEHandle( SENDINFO( m_hPrevOwner ) ),
#else
	RecvPropInt( RECVINFO( m_bDisabled ), 0, RecvProxy_IsDisabled ),
	RecvPropInt( RECVINFO( m_nGameType ) ),
	RecvPropInt( RECVINFO( m_nFlagStatus ), 0, RecvProxy_FlagStatus ),
	RecvPropTime( RECVINFO( m_flResetTime ) ),
	RecvPropTime( RECVINFO( m_flNeutralTime ) ),
	RecvPropTime( RECVINFO( m_flMaxResetTime ) ),
	RecvPropEHandle( RECVINFO( m_hPrevOwner ) ),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC( CCaptureFlag )

	// Keyfields.
	DEFINE_KEYFIELD( m_flResetTime, FIELD_INTEGER, "ReturnTime"),

	DEFINE_KEYFIELD( m_nGameType, FIELD_INTEGER, "GameType" ),
		
	DEFINE_KEYFIELD( m_szModel, FIELD_STRING, "flag_model" ),
	DEFINE_KEYFIELD( m_szHudIcon, FIELD_STRING, "flag_icon" ),
	DEFINE_KEYFIELD( m_szPaperEffect, FIELD_STRING, "flag_paper" ),
	DEFINE_KEYFIELD( m_szTrailEffect, FIELD_STRING, "flag_trail" ),
	DEFINE_KEYFIELD( m_nUseTrailEffect, FIELD_INTEGER, "trail_effect" ),
	DEFINE_KEYFIELD( m_bVisibleWhenDisabled, FIELD_BOOLEAN, "VisibleWhenDisabled" ),
	
#ifdef GAME_DLL
	// Inputs.
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RoundActivate", InputRoundActivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceReset", InputForceReset ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceResetAndDisableSilent", InputForceResetAndDisableSilent ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceResetSlient", InputForceResetSilent ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetReturnTime", InputSetReturnTime ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceGlowDisabled", InputForceGlowDisabled ),

	// Outputs.
	DEFINE_OUTPUT( m_outputOnReturn, "OnReturn" ),
	DEFINE_OUTPUT( m_outputOnPickUp, "OnPickUp" ),
	DEFINE_OUTPUT( m_outputOnPickUpTeam1, "OnPickupTeam1" ),
	DEFINE_OUTPUT( m_outputOnPickUpTeam2, "OnPickupTeam2" ),
	DEFINE_OUTPUT( m_outputOnPickUpTeam3, "OnPickupTeam3" ),
	DEFINE_OUTPUT( m_outputOnPickUpTeam4, "OnPickupTeam4" ),
	DEFINE_OUTPUT( m_outputOnDrop, "OnDrop" ),
	DEFINE_OUTPUT( m_outputOnCapture, "OnCapture" ),
	DEFINE_OUTPUT( m_outputOnCapTeam1, "OnCapTeam1"),
	DEFINE_OUTPUT( m_outputOnCapTeam2, "OnCapTeam2" ),
	DEFINE_OUTPUT( m_outputOnCapTeam3, "OnCapTeam3" ),
	DEFINE_OUTPUT( m_outputOnCapTeam4, "OnCapTeam4" ),
	DEFINE_OUTPUT( m_outputOnTouchSameTeam, "OnTouchSameTeam"),

#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS( item_teamflag, CCaptureFlag );

#ifdef GAME_DLL
IMPLEMENT_AUTO_LIST( ICaptureFlagAutoList );
#endif

//=============================================================================
//
// CTF Flag functions.
//

CCaptureFlag::CCaptureFlag()
{
#ifdef CLIENT_DLL
	m_pPaperTrailEffect = NULL;
#else
	m_hReturnIcon = NULL;
	m_pGlowTrail = NULL;
#endif	
	UseClientSideAnimation();
	m_nUseTrailEffect = 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CCaptureFlag::GetItemID( void )
{
	return TF_ITEM_CAPTURE_FLAG;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CCaptureFlag::GetTrailEffect( int iTeamNum, char *buf , size_t buflen )
{
	const char *szTrailEffect = "flagtrail";
	if ( m_szTrailEffect != NULL_STRING )
		szTrailEffect = STRING( m_szTrailEffect );

	Q_snprintf( buf, buflen, "effects/%s_%s.vmt", szTrailEffect, g_aTeamNamesShort[ iTeamNum ] );

	return buf;
}

//-----------------------------------------------------------------------------
// Purpose: Precache the model and sounds.
//-----------------------------------------------------------------------------
void CCaptureFlag::Precache( void )
{
	// Flag model
	if (m_szModel == NULL_STRING)
	{
		m_szModel = MAKE_STRING(TF_FLAG_MODEL);
	}

	// Paper trail particle effect
	if (m_szPaperEffect == NULL_STRING)
	{
		m_szPaperEffect = MAKE_STRING("player_intel_papertrail");
	}

	// Team colored trail
	if (m_szTrailEffect == NULL_STRING)
	{
		m_szTrailEffect = MAKE_STRING("flagtrail");
	}

	PrecacheModel(STRING(m_szModel));

	PrecacheScriptSound( TF_CTF_FLAGSPAWN ); // Should be Resource.Flagspawn
	PrecacheScriptSound( TF_CTF_ENEMY_STOLEN );
	PrecacheScriptSound( TF_CTF_ENEMY_DROPPED );
	PrecacheScriptSound( TF_CTF_ENEMY_CAPTURED );
	PrecacheScriptSound( TF_CTF_ENEMY_RETURNED );
	PrecacheScriptSound( TF_CTF_TEAM_STOLEN );
	PrecacheScriptSound( TF_CTF_TEAM_DROPPED );
	PrecacheScriptSound( TF_CTF_TEAM_CAPTURED );
	PrecacheScriptSound( TF_CTF_TEAM_RETURNED );

	PrecacheScriptSound( TF_AD_CAPTURED_SOUND );
	PrecacheScriptSound( TF_AD_ENEMY_STOLEN );
	PrecacheScriptSound( TF_AD_ENEMY_DROPPED );
	PrecacheScriptSound( TF_AD_ENEMY_CAPTURED );
	PrecacheScriptSound( TF_AD_ENEMY_RETURNED );
	PrecacheScriptSound( TF_AD_TEAM_STOLEN );
	PrecacheScriptSound( TF_AD_TEAM_DROPPED );
	PrecacheScriptSound( TF_AD_TEAM_CAPTURED );
	PrecacheScriptSound( TF_AD_TEAM_RETURNED );

	PrecacheScriptSound( TF_INVADE_ENEMY_STOLEN );
	PrecacheScriptSound( TF_INVADE_ENEMY_DROPPED );
	PrecacheScriptSound( TF_INVADE_ENEMY_CAPTURED );
	PrecacheScriptSound( TF_INVADE_TEAM_STOLEN );
	PrecacheScriptSound( TF_INVADE_TEAM_DROPPED );
	PrecacheScriptSound( TF_INVADE_TEAM_CAPTURED );
	PrecacheScriptSound( TF_INVADE_FLAG_RETURNED );

	PrecacheScriptSound( TF_SD_ENEMY_STOLEN );
	PrecacheScriptSound( TF_SD_ENEMY_DROPPED );
	PrecacheScriptSound( TF_SD_ENEMY_CAPTURED );
	PrecacheScriptSound( TF_SD_RETURNED );
	PrecacheScriptSound( TF_SD_TEAM_STOLEN );
	PrecacheScriptSound( TF_SD_TEAM_DROPPED );
	PrecacheScriptSound( TF_SD_TEAM_CAPTURED );

	PrecacheScriptSound( TF_SD_2014_RED_CAPTURED );
	PrecacheScriptSound( TF_SD_2014_BLU_CAPTURED );
	PrecacheScriptSound( TF_SD_2014_ENEMY_DROPPED );
	PrecacheScriptSound( TF_SD_2014_RETURNED );
	PrecacheScriptSound( TF_SD_2014_TEAM_STOLEN );
	PrecacheScriptSound( TF_SD_2014_TEAM_DROPPED );

	PrecacheParticleSystem( "player_intel_trail_blue" );
	PrecacheParticleSystem( "player_intel_trail_red" );


	// Team colored trail
	char tempChar[ 512 ];
	PrecacheModel( GetTrailEffect( TF_TEAM_RED, tempChar, sizeof( tempChar ) ) );
	PrecacheModel( GetTrailEffect( TF_TEAM_BLUE, tempChar, sizeof( tempChar ) ) );

}

#ifndef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCaptureFlag::OnPreDataChanged( DataUpdateType_t updateType )
{
	m_nOldFlagStatus = m_nFlagStatus;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCaptureFlag::OnDataChanged( DataUpdateType_t updateType )
{
	if ( m_nOldFlagStatus != m_nFlagStatus )
	{
		UpdateGlowEffect();
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "flagstatus_update" );
		if ( pEvent )
		{
			gameeventmanager->FireEventClientSide( pEvent );
		}
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::Spawn( void )
{
	// Precache the model and sounds.  Set the flag model.
	Precache();

	if ( m_szHudIcon == NULL_STRING )
		m_szHudIcon = MAKE_STRING("../hud/objectives_flagpanel_carried");

	SetModel( STRING(m_szModel) );
	
	// Set the flag solid and the size for touching.
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetSize( vec3_origin, vec3_origin );

	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds( true, 24 );

	// use the initial dynamic prop "m_bStartDisabled" setting to set our own m_bDisabled flag
#ifdef GAME_DLL
	m_bDisabled = m_bStartDisabled;
	m_bStartDisabled = false;

	// Don't allow the intelligence to fade.
	m_flFadeScale = 0.0f;
#else
	m_bDisabled = false;
#endif

	// Base class spawn.
	BaseClass::Spawn();

	AddFlag( FL_OBJECT );

#ifdef GAME_DLL
	// Save the starting position, so we can reset the flag later if need be.
	m_vecResetPos = GetAbsOrigin();
	m_vecResetAng = GetAbsAngles();

	SetFlagStatus( TF_FLAGINFO_NONE );
	ResetFlagReturnTime();
	ResetFlagNeutralTime();

	m_bAllowOwnerPickup = true;
	m_hPrevOwner = NULL;

	m_bCaptured = false;

	AddGlowEffect();
#else

	// add this element if it isn't already in the list
	if ( g_Flags.Find( entindex() ) == -1 )
	{
		g_Flags.AddToTail( entindex() );
	}

#endif

	if ( m_bDisabled )
	{
		SetDisabled( true );
	}
	else
	{
		SetDisabled( false );
	}
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Manage glow effect
//-----------------------------------------------------------------------------
void CCaptureFlag::UpdateGlowEffect( void )
{
	if ( !GetGlowObject() )
		return;

	Vector vecColor;
	TFGameRules()->GetTeamGlowColor( GetTeamNumber(), vecColor.x, vecColor.y, vecColor.z );
	GetGlowObject()->SetColor( vecColor );
}
#endif

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCaptureFlag::Activate( void )
{
	BaseClass::Activate();

	m_iOriginalTeam = GetTeamNumber();

	m_nSkin = GetIntelSkin(GetTeamNumber());
}
#endif

int CCaptureFlag::GetIntelSkin(int iTeamNum, bool bPickupSkin)
{
	switch (iTeamNum)
	{
	case TF_TEAM_RED:
		return bPickupSkin ? 3 : 0;
		break;
	case TF_TEAM_BLUE:
		return bPickupSkin ? 4 : 1;
		break;
	default:
		return bPickupSkin ? 5 : 2;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset the flag position state.
//-----------------------------------------------------------------------------
void CCaptureFlag::Reset( void )
{
#ifdef GAME_DLL
	// Set the flag position.
	SetAbsOrigin( m_vecResetPos );
	SetAbsAngles( m_vecResetAng );

	// No longer dropped, if it was.
	SetFlagStatus( TF_FLAGINFO_NONE );
	ResetFlagReturnTime();
	ResetFlagNeutralTime();

	m_bAllowOwnerPickup = true;
	m_hPrevOwner = NULL;

	if ( m_nGameType == TF_FLAGTYPE_INVADE )
	{
		ChangeTeam( m_iOriginalTeam );
	}

	m_nSkin = GetIntelSkin(GetTeamNumber());
	SetMoveType( MOVETYPE_NONE );
#endif 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCaptureFlag::ResetMessage( void )
{
#ifdef GAME_DLL
	if ( m_nGameType == TF_FLAGTYPE_CTF )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam == GetTeamNumber() )
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_CTF_ENEMY_RETURNED );

				TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_YOUR_FLAG_RETURNED );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_CTF_TEAM_RETURNED );

				TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_ENEMY_FLAG_RETURNED );
			}
		}

		// Returned sound
		CPASAttenuationFilter filter( this, TF_CTF_FLAGSPAWN );
		EmitSound( filter, entindex(), TF_CTF_FLAGSPAWN );
	}
	else if ( m_nGameType == TF_FLAGTYPE_ATTACK_DEFEND )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam == GetTeamNumber() )
			{
				TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_AD_FlagReturned" );

				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_TEAM_RETURNED );
			}
			else
			{
				TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_AD_FlagReturned" );

				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_ENEMY_RETURNED );
			}
		}
	}
	else if ( m_nGameType == TF_FLAGTYPE_INVADE )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_Invade_FlagReturned" );

			CTeamRecipientFilter filter( iTeam, true );
			EmitSound( filter, entindex(), TF_INVADE_FLAG_RETURNED );
		}
	}
	else if ( m_nGameType == TF_FLAGTYPE_SPECIAL_DELIVERY )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
		CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_SD_RETURNED );
		}
	}

	// Output.
	m_outputOnReturn.FireOutput( this, this );

	DestroyReturnIcon();
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::FlagTouch( CBaseEntity *pOther )
{
	// Is the flag enabled?
	if ( IsDisabled() )
		return;

	// The the touch from a live player.
	if ( !pOther->IsCombatCharacter() || !pOther->IsAlive() )
		return;

#ifdef GAME_DLL
	// Don't let the person who threw this flag pick it up until it hits the ground.
	// This way we can throw the flag to people, but not touch it as soon as we throw it ourselves
	if(  m_hPrevOwner.Get() && m_hPrevOwner.Get() == pOther && m_bAllowOwnerPickup == false )
		return;

	if ( InSameTeam( pOther ) )
		m_outputOnTouchSameTeam.FireOutput( this, this );
#endif

	// Does my team own this flag? If so, no touch.
	if ( m_nGameType == TF_FLAGTYPE_CTF && InSameTeam( pOther ) )
		return;

	if ( ( m_nGameType == TF_FLAGTYPE_ATTACK_DEFEND || m_nGameType == TF_FLAGTYPE_TERRITORY_CONTROL ) &&
		   !InSameTeam( pOther ) )
	{
		return;
	}

	if ( m_nGameType == TF_FLAGTYPE_INVADE && GetTeamNumber() != TEAM_UNASSIGNED )
	{
		if ( !InSameTeam( pOther ) )
		{
			return;
		}
	}

	// Can't pickup flags during WaitingForPlayers
	if ( TFGameRules()->IsInWaitingForPlayers() )
		return;

	// Get the touching player.
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( pPlayer )
	{
		// Is the touching player about to teleport?
		if ( pPlayer->m_Shared.InCond( TF_COND_SELECTED_TO_TELEPORT ) )
			return;

		// Don't let invulnerable players pickup flags
		if ( pPlayer->m_Shared.IsInvulnerable() )
			return;
		
		// Don't let bonked players pickup flags
		if ( pPlayer->m_Shared.InCond( TF_COND_PHASE ) )
			return;

	#ifdef GAME_DLL
		if ( PointInRespawnRoom(pPlayer,pPlayer->WorldSpaceCenter()) )
			return;
	#endif

		// Do not allow the player to pick up multiple flags
		if ( pPlayer->HasTheFlag() )
			return;

		// Pick up the flag.
		PickUp( pPlayer, true );
	}

	// Get the touching npc.
	CAI_BaseNPC *pNPC = pOther->MyNPCPointer();
	if ( pNPC )
	{
		// Don't let this npc with these conds pickup flags
		if ( pNPC->InCond( TF_COND_SELECTED_TO_TELEPORT ) || pNPC->IsInvulnerable() || pNPC->InCond( TF_COND_PHASE ) )
			return;

	/*#ifdef GAME_DLL
		if ( PointInRespawnRoom(pPlayer,pPlayer->WorldSpaceCenter()) )
			return;
	#endif*/

		// Do not allow the npc to pick up multiple flags
		if ( pNPC->HasTheFlag() )
			return;

		// Pick up the flag.
		PickUp( pNPC, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::PickUp( CBaseEntity *pOwner, bool bInvisible )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pOwner );
	CAI_BaseNPC *pNPC = pOwner->MyNPCPointer();

	// Is the flag enabled?
	if ( IsDisabled() )
		return;

	if ( !TFGameRules()->FlagsMayBeCapped() )
		return;

#ifdef GAME_DLL
	if ( !m_bAllowOwnerPickup )
	{
		if ( m_hPrevOwner.Get() && m_hPrevOwner.Get() == pOwner ) 
		{
			return;
		}
	}
#endif

	// Check whether we have a weapon that's prohibiting us from picking the flag up
	if ( pTFPlayer )
	{
		if ( !pTFPlayer->IsAllowedToPickUpFlag() )
			return;
	}

	if ( pNPC )
	{
		if ( !pNPC->IsAllowedToPickUpFlag() )
			return;
	}

	// Call into the base class pickup.
	BaseClass::PickUp( pOwner, false );

	m_nSkin = GetIntelSkin(GetTeamNumber(), true);

	if ( pTFPlayer )
		pTFPlayer->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>( pOwner );
	pAnimating->AddGlowEffect();

	// Update the parent to set the correct place on the model to attach the flag.
	int iAttachment = pNPC ? pAnimating->LookupAttachment( "chest" ) : pAnimating->LookupAttachment( "flag" );

	if( iAttachment != -1 )
	{
		SetParent( pOwner, iAttachment );
		if ( pNPC )
		{
			if ( FClassnameIs( pNPC, "npc_citizen" ) || FClassnameIs( pNPC, "npc_metropolice" ) || FClassnameIs( pNPC, "npc_alyx" ) || FClassnameIs( pNPC, "npc_eli" ) || FClassnameIs( pNPC, "npc_kleiner" ) || FClassnameIs( pNPC, "npc_magnusson" ) || FClassnameIs( pNPC, "npc_barney" ) || FClassnameIs( pNPC, "npc_gman" ) || FClassnameIs( pNPC, "npc_breen" ) || FClassnameIs( pNPC, "npc_zombie" ) || FClassnameIs( pNPC, "npc_zombine" ) )
			{
				Vector vecPos( -12, 0, -2 );
				QAngle vecAng( 0, 90, -85 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_combine_s" ) )
			{
				Vector vecPos( -12, 0, 45 );
				QAngle vecAng( 0, 90, -80 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_antlion" ) )
			{
				Vector vecPos( -25, 0, 40 );
				QAngle vecAng( -100, 0, 0 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_zombie_torso" ) || FClassnameIs( pNPC, "npc_fastzombie_torso" ) )
			{
				Vector vecPos( -18, 0, 0 );
				QAngle vecAng( 0, 90, 120 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_fastzombie" ) )
			{
				Vector vecPos( -4, 0, -10 );
				QAngle vecAng( 30, -30, 200 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_poisonzombie" ) )
			{
				Vector vecPos( -14, 0, -2 );
				QAngle vecAng( 0, 90, -100 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_headcrab" ) || FClassnameIs( pNPC, "npc_headcrab_fast" ) || FClassnameIs( pNPC, "npc_headcrab_poison" ) || FClassnameIs( pNPC, "monster_headcrab" ) )
			{
				Vector vecPos( -5, 0, 18 );
				QAngle vecAng( 0, 90, -10 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_hunter" ) )
			{
				Vector vecPos( 2, -9, 80 );
				QAngle vecAng( 0, 130, -80 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "monster_zombie" ) )
			{
				Vector vecPos( -12, 0, 40 );
				QAngle vecAng( 0, 90, -85 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "monster_scientist" ) || FClassnameIs( pNPC, "monster_barney" ) || FClassnameIs( pNPC, "monster_gman" ) )
			{
				Vector vecPos( -12, 0, 50 );
				QAngle vecAng( 0, 90, -85 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else if ( FClassnameIs( pNPC, "npc_conscript" ) || FClassnameIs( pNPC, "npc_conscriptred" ) )
			{
				Vector vecPos( -8, 0, 45 );
				QAngle vecAng( 0, 90, -85 );
				SetLocalAngles( vecAng );
				SetLocalOrigin( vecPos );
			}
			else
			{
				SetLocalOrigin( vec3_origin );
				SetLocalAngles( vec3_angle );
			}
		}
		else
		{
			SetLocalOrigin( vec3_origin );
			SetLocalAngles( vec3_angle );
		}
	}

	// Remove the player's disguse if they're a spy
	if ( pTFPlayer )
	{
		if ( pTFPlayer->GetPlayerClass()->GetClassIndex() == TF_CLASS_SPY )
		{
			if ( pTFPlayer->m_Shared.InCond( TF_COND_DISGUISED ) ||
				pTFPlayer->m_Shared.InCond( TF_COND_DISGUISING ))
			{
				pTFPlayer->m_Shared.RemoveDisguise();
			}
		}
	}

	// Remove the touch function.
	SetTouch( NULL );

	m_hPrevOwner = pOwner;
	m_bAllowOwnerPickup = true;

	if ( m_nGameType == TF_FLAGTYPE_CTF )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_CTF_ENEMY_STOLEN );

				TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_YOUR_FLAG_TAKEN );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_CTF_TEAM_STOLEN );

				// exclude the guy who just picked it up
				if ( pTFPlayer )
					filter.RemoveRecipient( pTFPlayer );

				TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_ENEMY_FLAG_TAKEN );
			}
		}
	}
	else if ( m_nGameType == TF_FLAGTYPE_ATTACK_DEFEND )
	{
		// Handle messages to the screen.
		if ( pTFPlayer )
			TFTeamMgr()->PlayerCenterPrint( pTFPlayer, "#TF_AD_TakeFlagToPoint" );

		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_ENEMY_STOLEN );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_TEAM_STOLEN );
			}
		}
	}
	else if ( m_nGameType == TF_FLAGTYPE_INVADE )
	{
		// Handle messages to the screen.
		if ( pTFPlayer )
		{
			TFTeamMgr()->PlayerCenterPrint( pTFPlayer, "#TF_Invade_PlayerPickup" );
			TFTeamMgr()->PlayerTeamCenterPrint( pTFPlayer, "#TF_Invade_PlayerTeamPickup" );
		}

		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_Invade_OtherTeamPickup" );

				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_INVADE_ENEMY_STOLEN );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_INVADE_TEAM_STOLEN );
			}
		}

		// set the flag's team to match the player's team
		ChangeTeam( pOwner->GetTeamNumber() );
		m_nSkin = ( GetTeamNumber() == TEAM_UNASSIGNED ) ? 2 : (GetTeamNumber() - 2);
	}
	else if ( m_nGameType == TF_FLAGTYPE_SPECIAL_DELIVERY )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_SD_ENEMY_STOLEN );

			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_SD_TEAM_STOLEN );
			}
		}
	}

	SetFlagStatus( TF_FLAGINFO_STOLEN );
	ResetFlagReturnTime();
	ResetFlagNeutralTime();

	IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_flag_event" );
	if ( event )
	{
		event->SetInt( "player", /*pTFPlayer ? pTFPlayer->GetUserID() :*/ pOwner->entindex() );
		event->SetString( "player_name", pOwner->GetClassname() );
		event->SetInt( "team", GetTeamNumber() );
		event->SetInt( "eventtype", TF_FLAGEVENT_PICKUP );
		event->SetInt( "priority", 8 );

		gameeventmanager->FireEvent( event );
	}


	if ( FStrEq( STRING( m_szModel ), "models/props_td/atom_bomb.mdl" ) )
	{
		if ( pTFPlayer )
		{
			pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_FLAGPICKUP );

			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					if ( pTFPlayer->IsMiniBoss() )
						TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_HAS_BOMB, iTeam );
					else
						TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_BOMB_PICKUP, iTeam );
				}
			}
		}

		if ( pNPC )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					if ( pNPC->IsMiniBoss() )
						TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_GIANT_HAS_BOMB, iTeam );
					else
						TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_BOMB_PICKUP, iTeam );
				}
			}
		}
	}

	// Output.
	m_outputOnPickUp.FireOutput( pOwner, this );

	switch ( pOwner->GetTeamNumber() )
	{
	case TF_TEAM_RED:
		m_outputOnPickUpTeam1.FireOutput( this, this );
		break;

	case TF_TEAM_BLUE:
		m_outputOnPickUpTeam2.FireOutput( this, this );
		break;
	}

	DestroyReturnIcon();

	HandleFlagPickedUpInDetectionZone( pOwner );
#endif
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::CreateReturnIcon( void )
{
	m_hReturnIcon = CBaseEntity::Create( "item_teamflag_return_icon", GetAbsOrigin() + Vector(0,0,cl_flag_return_height.GetFloat()), vec3_angle, this );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::DestroyReturnIcon( void )
{
	if ( m_hReturnIcon.Get() )
	{
		UTIL_Remove( m_hReturnIcon );
		m_hReturnIcon = NULL;
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::Capture( CBaseEntity *pOwner, int nCapturePoint )
{
	// Is the flag enabled?
	if ( IsDisabled() )
		return;

#ifdef GAME_DLL
	CTFPlayer *pTFPlayer = ToTFPlayer( pOwner );
	CAI_BaseNPC *pNPC = pOwner->MyNPCPointer();

	if ( m_nGameType == TF_FLAGTYPE_CTF )
	{
		bool bNotify = true;

		// don't play any sounds if this is going to win the round for one of the teams (victory sound will be played instead)
		if ( tf_flag_caps_per_round.GetInt() > 0 )
		{
			int nCaps = TFTeamMgr()->GetFlagCaptures( pOwner->GetTeamNumber() );

			if ( ( nCaps >= 0 ) && ( tf_flag_caps_per_round.GetInt() - nCaps <= 1 ) )
			{
				// this cap is going to win, so don't play a sound
				bNotify = false;
			}
		}

		if ( bNotify )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_CTF_ENEMY_CAPTURED );

					TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_YOUR_FLAG_CAPTURED );
				}
				else
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_CTF_TEAM_CAPTURED );

					TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_ENEMY_FLAG_CAPTURED );
				}
			}
		}

		// Give temp crit boost to capper's team.
		if ( TFGameRules() )
		{
			TFGameRules()->HandleCTFCaptureBonus( pOwner->GetTeamNumber() );
		}

		// Reward the player
		if ( pTFPlayer )
			CTF_GameStats.Event_PlayerCapturedPoint( pTFPlayer );

		// Reward the team
		if ( tf_flag_caps_per_round.GetInt() > 0 )
		{
			TFTeamMgr()->IncrementFlagCaptures( pOwner->GetTeamNumber() );
		}
		else
		{
			TFTeamMgr()->AddTeamScore( pOwner->GetTeamNumber(), TF_CTF_CAPTURED_TEAM_FRAGS );
		}
	}
	else if ( m_nGameType == TF_FLAGTYPE_ATTACK_DEFEND )
	{
		char szNumber[64];
		Q_snprintf( szNumber, sizeof(szNumber), "%d", nCapturePoint );

		// Handle messages to the screen.
		/*if ( pTFPlayer )
		{
			TFTeamMgr()->PlayerCenterPrint( pTFPlayer, "#TF_AD_YouSecuredPoint", szNumber );
			TFTeamMgr()->PlayerTeamCenterPrint( pTFPlayer, "TF_AD_AttackersSecuredPoint", szNumber );
		}*/

		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				//TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_AD_AttackersSecuredPoint" );

				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_ENEMY_CAPTURED );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_AD_TEAM_CAPTURED );
			}
		}

		// Capture sound
		CBroadcastRecipientFilter filter;
		EmitSound( filter, entindex(), TF_AD_CAPTURED_SOUND );

		// Reward the player
		if ( pTFPlayer )
			pTFPlayer->IncrementFragCount( TF_AD_CAPTURED_FRAGS );

		// TFTODO:: Reward the team	
	}
	else if ( m_nGameType == TF_FLAGTYPE_INVADE )
	{
		// Handle messages to the screen.
		if ( pTFPlayer )
		{
			TFTeamMgr()->PlayerCenterPrint( pTFPlayer, "#TF_Invade_PlayerCapture" );
			TFTeamMgr()->PlayerTeamCenterPrint( pTFPlayer, "#TF_Invade_PlayerTeamCapture" );
		}

		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_Invade_OtherTeamCapture" );

				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_INVADE_ENEMY_CAPTURED );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_INVADE_TEAM_CAPTURED );
			}
		}

		// Reward the player
		if ( pTFPlayer )
			pTFPlayer->IncrementFragCount( TF_INVADE_CAPTURED_FRAGS );

		// Reward the team
		TFTeamMgr()->AddTeamScore( pOwner->GetTeamNumber(), TF_INVADE_CAPTURED_TEAM_FRAGS );

		// This was added by request for the "push" map -danielmm8888
		if (tf_flag_caps_per_round.GetInt() > 0)
		{
			TFTeamMgr()->IncrementFlagCaptures(pOwner->GetTeamNumber());
		}

	}
	else if ( m_nGameType == TF_FLAGTYPE_SPECIAL_DELIVERY )
	{
		for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
		{
			if ( iTeam != pOwner->GetTeamNumber() )
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_SD_ENEMY_CAPTURED );
			}
			else
			{
				CTeamRecipientFilter filter( iTeam, true );
				EmitSound( filter, entindex(), TF_SD_TEAM_CAPTURED );
			}
		}

		// Reward the player
		if ( pTFPlayer )
			pTFPlayer->IncrementFragCount( TF_INVADE_CAPTURED_FRAGS );
	}

	IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_flag_event" );
	if ( event )
	{
		event->SetInt( "player", /*pTFPlayer ? pTFPlayer->GetUserID() :*/ pOwner->entindex() );
		event->SetString( "player_name", pOwner->GetClassname() );
		event->SetInt( "team", GetTeamNumber() );
		event->SetInt( "eventtype", TF_FLAGEVENT_CAPTURE );
		event->SetInt( "priority", 9 );
		gameeventmanager->FireEvent( event );
	}

	SetFlagStatus( TF_FLAGINFO_NONE );
	ResetFlagReturnTime();
	ResetFlagNeutralTime();

	HandleFlagCapturedInDetectionZone( pOwner );		
	HandleFlagDroppedInDetectionZone( pOwner );

	// Reset the flag.
	BaseClass::Drop( pOwner, true );

	Reset();

	if ( pTFPlayer )
	{
		pTFPlayer->TeamFortress_SetSpeed();
		pTFPlayer->SpeakConceptIfAllowed( MP_CONCEPT_FLAGCAPTURED );
		pTFPlayer->RemoveGlowEffect();
	}

	if ( pNPC )
		pNPC->RemoveGlowEffect();

	// Output.
	m_outputOnCapture.FireOutput( pOwner, this );

	switch ( pOwner->GetTeamNumber() )
	{
	case TF_TEAM_RED:
		m_outputOnCapTeam1.FireOutput( this, this );
		break;

	case TF_TEAM_BLUE:
		m_outputOnCapTeam2.FireOutput( this, this );
		break;
	}

	m_bCaptured = true;
	SetNextThink( gpGlobals->curtime + TF_FLAG_THINK_TIME );

	if ( TFGameRules()->InStalemate() )
	{
		if ( !pTFPlayer )
			return;

		// whoever capped the flag is the winner, give them enough caps to win
		CTFTeam *pTeam = pTFPlayer->GetTFTeam();
		if ( !pTeam )
			return;

		// if we still need more caps to trigger a win, give them to us
		if ( pTeam->GetFlagCaptures() < tf_flag_caps_per_round.GetInt() )
		{
			pTeam->SetFlagCaptures( tf_flag_caps_per_round.GetInt() );
		}	
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose: A player drops the flag.
//-----------------------------------------------------------------------------
void CCaptureFlag::Drop( CBaseEntity *pOwner, bool bVisible,  bool bThrown /*= false*/, bool bMessage /*= true*/ )
{
	CTFPlayer *pTFPlayer = ToTFPlayer( pOwner );

	// Is the flag enabled?
	if ( IsDisabled() )
		return;

	// Call into the base class drop.
	BaseClass::Drop( pOwner, bVisible );

	if ( pTFPlayer )
		pTFPlayer->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	if ( pTFPlayer )
		pTFPlayer->RemoveGlowEffect();

	CAI_BaseNPC *pNPC = pOwner->MyNPCPointer();
	if ( pNPC )
		pNPC->RemoveGlowEffect();

	if ( bThrown )
	{
		m_bAllowOwnerPickup = false;
		m_flOwnerPickupTime = gpGlobals->curtime + TF_FLAG_OWNER_PICKUP_TIME;
	}

	Vector vecStart = GetAbsOrigin();
	Vector vecEnd = vecStart;
	vecEnd.z -= 8000.0f;
	trace_t trace;
	UTIL_TraceHull( vecStart, vecEnd, WorldAlignMins(), WorldAlignMaxs(), MASK_SOLID, this, COLLISION_GROUP_DEBRIS, &trace );
	SetAbsOrigin( trace.endpos );

	if ( m_nGameType == TF_FLAGTYPE_CTF )
	{
		if ( bMessage )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_CTF_ENEMY_DROPPED );

					TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_YOUR_FLAG_DROPPED );
				}
				else
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_CTF_TEAM_DROPPED );

					TFGameRules()->SendHudNotification( filter, HUD_NOTIFY_ENEMY_FLAG_DROPPED );
				}
			}
		}

		SetFlagReturnIn( TF_CTF_RESET_TIME );
	}
	else if ( m_nGameType == TF_FLAGTYPE_INVADE )
	{
		if ( bMessage )
		{
			// Handle messages to the screen.
			if ( pTFPlayer )
			{
				TFTeamMgr()->PlayerCenterPrint( pTFPlayer, "#TF_Invade_PlayerFlagDrop" );
				TFTeamMgr()->PlayerTeamCenterPrint( pTFPlayer, "#TF_Invade_FlagDrop" );
			}

			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_Invade_FlagDrop" );

					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_INVADE_ENEMY_DROPPED );
				}
				else
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_INVADE_TEAM_DROPPED );
				}
			}
		}

		SetFlagReturnIn( TF_INVADE_RESET_TIME );
		SetFlagNeutralIn( TF_INVADE_NEUTRAL_TIME );
	}
	else if ( m_nGameType == TF_FLAGTYPE_ATTACK_DEFEND )
	{
		if ( bMessage )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_AD_ENEMY_DROPPED );
				}
				else
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_AD_TEAM_DROPPED );
				}
			}
		}

		SetFlagReturnIn( TF_AD_RESET_TIME );
	}
	else if ( m_nGameType == TF_FLAGTYPE_SPECIAL_DELIVERY )
	{
		if ( bMessage )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_SD_ENEMY_DROPPED );
				}
				else
				{
					CTeamRecipientFilter filter( iTeam, true );
					EmitSound( filter, entindex(), TF_SD_TEAM_DROPPED );
				}
			}
		}

		SetFlagReturnIn( TF_CTF_RESET_TIME );
	}

	if ( FStrEq( STRING( m_szModel ), "models/props_td/atom_bomb.mdl" ) )
	{
		if ( pTFPlayer )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_BOMB_DROPPED, iTeam );
				}
			}
		}

		if ( pNPC )
		{
			for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
			{
				if ( iTeam != pOwner->GetTeamNumber() )
				{
					TFGameRules()->HaveAllPlayersSpeakConceptIfAllowed( MP_CONCEPT_MVM_BOMB_DROPPED, iTeam );
				}
			}
		}
	}

	// Reset the flag's angles.
	SetAbsAngles( m_vecResetAng );

	// Reset the touch function.
	SetTouch( &CCaptureFlag::FlagTouch );

	SetFlagStatus( TF_FLAGINFO_DROPPED );

	// Output.
	m_outputOnDrop.FireOutput( pOwner, this );

	CreateReturnIcon();	

	HandleFlagDroppedInDetectionZone( pOwner );	

	if ( PointInRespawnFlagZone( GetAbsOrigin() ) )
	{
		Reset();
		ResetMessage();
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CCaptureFlag::IsDropped( void )
{ 
	return ( m_nFlagStatus == TF_FLAGINFO_DROPPED ); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CCaptureFlag::IsHome( void )
{ 
	return ( m_nFlagStatus == TF_FLAGINFO_NONE ); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CCaptureFlag::IsStolen( void )
{ 
	return ( m_nFlagStatus == TF_FLAGINFO_STOLEN ); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CCaptureFlag::IsDisabled( void )
{
	return m_bDisabled;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::SetDisabled( bool bDisabled )
{
	m_bDisabled = bDisabled;

	if ( bDisabled )
	{
		if (!m_bVisibleWhenDisabled)
			AddEffects(EF_NODRAW);

		SetTouch( NULL );
		SetThink( NULL );

#ifdef GAME_DLL
		RemoveGlowEffect();
#endif
	}
	else
	{
		RemoveEffects( EF_NODRAW );

		SetTouch( &CCaptureFlag::FlagTouch );
		SetThink( &CCaptureFlag::Think );
		SetNextThink( gpGlobals->curtime );

#ifdef GAME_DLL
		AddGlowEffect();
#endif
	}
}

//-----------------------------------------------------------------------------------------------
// GAME DLL Functions
//-----------------------------------------------------------------------------------------------
#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::Think( void )
{
	// Is the flag enabled?
	if ( IsDisabled() )
		return;

	if ( !TFGameRules()->FlagsMayBeCapped() )
	{
		SetNextThink( gpGlobals->curtime + TF_FLAG_THINK_TIME );
		return;
	}

	if ( m_bCaptured )
	{
		m_bCaptured = false;
		SetTouch( &CCaptureFlag::FlagTouch );
	}

	ManageSpriteTrail();

	if ( IsDropped() )
	{
		if ( !m_bAllowOwnerPickup )
		{
			if ( m_flOwnerPickupTime && gpGlobals->curtime > m_flOwnerPickupTime )
			{
				m_bAllowOwnerPickup = true;
			}
		}

		if ( m_nGameType == TF_FLAGTYPE_INVADE )
		{
			if ( m_flResetTime && gpGlobals->curtime > m_flResetTime )
			{
				Reset();
				ResetMessage();
			}
			else if ( m_flNeutralTime && gpGlobals->curtime > m_flNeutralTime )
			{
				for ( int iTeam = TF_TEAM_RED; iTeam < TF_TEAM_COUNT; ++iTeam )
				{
					TFTeamMgr()->TeamCenterPrint( iTeam, "#TF_Invade_FlagNeutral" );
				}

				// reset the team to the original team setting (when it spawned)
				ChangeTeam( m_iOriginalTeam );
				m_nSkin = ( GetTeamNumber() == TEAM_UNASSIGNED ) ? 2 : (GetTeamNumber() - 2);

				ResetFlagNeutralTime();
			}
		}
		else
		{
			if ( m_flResetTime && gpGlobals->curtime > m_flResetTime )
			{
				Reset();
				ResetMessage();
			}
		}
		m_nSkin = GetIntelSkin( GetTeamNumber() );
	}

	SetNextThink( gpGlobals->curtime + TF_FLAG_THINK_TIME );
}

//-----------------------------------------------------------------------------
// Purpose: Handles the team colored trail sprites
//-----------------------------------------------------------------------------
void CCaptureFlag::ManageSpriteTrail( void )
{
	if ( m_nFlagStatus == TF_FLAGINFO_STOLEN )
	{
		if ( ( m_nUseTrailEffect == 1 || m_nUseTrailEffect == 3 ) )
		{
			if ( GetPrevOwner() /*&& GetPrevOwner() != CBasePlayer::GetLocalPlayer() */ )
			{
				char pEffectName[512];
				if ( GetPrevOwner() )
				{
					if ( GetPrevOwner()->IsPlayer() )
					{
						CTFPlayer *pPlayer = ToTFPlayer( GetPrevOwner() );
						if ( pPlayer && ( pPlayer->GetAbsVelocity().Length() >= pPlayer->MaxSpeed() * 0.5f ) )
						{
							if ( m_pGlowTrail == NULL )
							{
								GetTrailEffect( pPlayer->GetTeamNumber(), pEffectName, sizeof( pEffectName ) );
								m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( pEffectName, GetLocalOrigin(), false );
								m_pGlowTrail->FollowEntity( this );
								m_pGlowTrail->SetTransparency( kRenderTransAlpha, -1, -1, -1, 96, 0 );
								m_pGlowTrail->SetBrightness( 96 );
								m_pGlowTrail->SetStartWidth( 32.0 );
								m_pGlowTrail->SetTextureResolution( 0.010416667 );
								m_pGlowTrail->SetLifeTime( 0.69999999 );
								m_pGlowTrail->SetTransmit( false );
								m_pGlowTrail->SetParent( this );
							}
						}
					}
					else
					{
						if( GetPrevOwner()->GetAbsVelocity() != vec3_origin )
						{
							if ( m_pGlowTrail == NULL )
							{
								GetTrailEffect( GetPrevOwner()->GetTeamNumber(), pEffectName, sizeof( pEffectName ) );
								m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( pEffectName, GetLocalOrigin(), false );
								m_pGlowTrail->FollowEntity( this );
								m_pGlowTrail->SetTransparency( kRenderTransAlpha, -1, -1, -1, 96, 0 );
								m_pGlowTrail->SetBrightness( 96 );
								m_pGlowTrail->SetStartWidth( 32.0 );
								m_pGlowTrail->SetTextureResolution( 0.010416667 );
								m_pGlowTrail->SetLifeTime( 0.69999999 );
								m_pGlowTrail->SetTransmit( false );
								m_pGlowTrail->SetParent( this );
							}
						}
					}
				}
			}
		}
	}
	else
	{
		if ( m_pGlowTrail )
		{
			m_pGlowTrail->Remove();
			m_pGlowTrail = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the flag status
//-----------------------------------------------------------------------------
void CCaptureFlag::SetFlagStatus( int iStatus )
{ 
	MDLCACHE_CRITICAL_SECTION();

	m_nFlagStatus = iStatus; 

	switch ( m_nFlagStatus )
	{
	case TF_FLAGINFO_NONE:
	case TF_FLAGINFO_DROPPED:
		ResetSequence( LookupSequence("spin") );	// set spin animation if it's not being held
		break;
	case TF_FLAGINFO_STOLEN:
		ResetSequence( LookupSequence("idle") );	// set idle animation if it is being held
		break;
	default:
		AssertOnce( false );	// invalid stats
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::InputEnable( inputdata_t &inputdata )
{
	SetDisabled( false );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptureFlag::InputDisable( inputdata_t &inputdata )
{
	SetDisabled( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCaptureFlag::InputRoundActivate( inputdata_t &inputdata )
{
	CTFPlayer *pPlayer = ToTFPlayer( m_hPrevOwner.Get() );
	// If the player has a capture flag, drop it.
	if ( pPlayer && pPlayer->HasItem() && ( pPlayer->GetItem() == this ) )
		Drop( pPlayer, true, false, false );

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( m_hPrevOwner.Get() );
	if ( pNPC && pNPC->HasItem() && ( pNPC->GetItem() == this ) )
		Drop( pNPC, true, false, false );

	Reset();
}

void CCaptureFlag::InputForceDrop( inputdata_t &inputdata )
{
	CTFPlayer *pPlayer = ToTFPlayer( m_hPrevOwner.Get() );
	// If the player has a capture flag, drop it.
	if ( pPlayer && pPlayer->HasItem() && ( pPlayer->GetItem() == this ) )
		Drop( pPlayer, true, false, true );

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( m_hPrevOwner.Get() );
	if ( pNPC && pNPC->HasItem() && ( pNPC->GetItem() == this ) )
		Drop( pNPC, true, false, true );
}

void CCaptureFlag::InputForceReset( inputdata_t &inputdata )
{
	CTFPlayer *pPlayer = ToTFPlayer( m_hPrevOwner.Get() );
	// If the player has a capture flag, drop it.
	if ( pPlayer && pPlayer->HasItem() && ( pPlayer->GetItem() == this ) )
		Drop( pPlayer, true, false, true );

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( m_hPrevOwner.Get() );
	if ( pNPC && pNPC->HasItem() && ( pNPC->GetItem() == this ) )
		Drop( pNPC, true, false, true );

	Reset();
	ResetMessage();
}

void CCaptureFlag::InputForceResetAndDisableSilent( inputdata_t &inputdata )
{
	CTFPlayer *pPlayer = ToTFPlayer( m_hPrevOwner.Get() );
	// If the player has a capture flag, drop it.
	if ( pPlayer && pPlayer->HasItem() && ( pPlayer->GetItem() == this ) )
		Drop( pPlayer, true, false, false );

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( m_hPrevOwner.Get() );
	if ( pNPC && pNPC->HasItem() && ( pNPC->GetItem() == this ) )
		Drop( pNPC, true, false, false );

	SetDisabled( true );
}

void CCaptureFlag::InputForceResetSilent( inputdata_t &inputdata )
{
	CTFPlayer *pPlayer = ToTFPlayer( m_hPrevOwner.Get() );
	// If the player has a capture flag, drop it.
	if ( pPlayer && pPlayer->HasItem() && ( pPlayer->GetItem() == this ) )
		Drop( pPlayer, true, false, false );

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>( m_hPrevOwner.Get() );
	if ( pNPC && pNPC->HasItem() && ( pNPC->GetItem() == this ) )
		Drop( pNPC, true, false, false );

	Reset();
}

void CCaptureFlag::InputSetReturnTime( inputdata_t &inputdata )
{
	SetFlagReturnIn( inputdata.value.Int() );
}

void CCaptureFlag::InputForceGlowDisabled( inputdata_t &inputdata )
{
	RemoveGlowEffect();
}

//-----------------------------------------------------------------------------
// Purpose: Always transmitted to clients
//-----------------------------------------------------------------------------
int CCaptureFlag::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}


#else

float CCaptureFlag::GetReturnProgress()
{
	float flEventTime = max( m_flResetTime.m_Value, m_flNeutralTime.m_Value );

	return ( 1.0 - ( ( flEventTime - gpGlobals->curtime ) / m_flMaxResetTime ) );
}


void CCaptureFlag::Simulate( void )
{
	BaseClass::Simulate();
	ManageTrailEffects();
}

void CCaptureFlag::ManageTrailEffects( void )
{
	if ( m_nFlagStatus == TF_FLAGINFO_STOLEN )
	{
		if (!m_nUseTrailEffect)
			return;

		if ( GetPrevOwner() && (m_nUseTrailEffect == 1 || m_nUseTrailEffect == 2) )
		{
			CTFPlayer *pPlayer = ToTFPlayer( GetPrevOwner() );
			CAI_BaseNPC *pNPC = GetPrevOwner()->MyNPCPointer();
			if ( pPlayer && ( pPlayer->GetAbsVelocity().Length() >= pPlayer->MaxSpeed() * 0.5f ) )
			{
				if ( m_pPaperTrailEffect == NULL )
					m_pPaperTrailEffect = ParticleProp()->Create( m_szPaperEffect, PATTACH_ABSORIGIN_FOLLOW );
			}
			else if ( pNPC && ( pNPC->GetAbsVelocity() != pNPC->GetAbsOrigin() ) )
			{
				if ( m_pPaperTrailEffect == NULL )
					m_pPaperTrailEffect = ParticleProp()->Create( m_szPaperEffect, PATTACH_ABSORIGIN_FOLLOW );
			}
			else
			{
				if ( m_pPaperTrailEffect )
				{
					ParticleProp()->StopEmission( m_pPaperTrailEffect );
					m_pPaperTrailEffect = NULL;
				}
			}
		}

	}
	else
	{
		if ( m_pPaperTrailEffect )
		{
			ParticleProp()->StopEmission( m_pPaperTrailEffect );
			m_pPaperTrailEffect = NULL;
		}
	}
}


#endif


LINK_ENTITY_TO_CLASS( item_teamflag_return_icon, CCaptureFlagReturnIcon );

IMPLEMENT_NETWORKCLASS_ALIASED( CaptureFlagReturnIcon, DT_CaptureFlagReturnIcon )

BEGIN_NETWORK_TABLE( CCaptureFlagReturnIcon, DT_CaptureFlagReturnIcon )
END_NETWORK_TABLE()

CCaptureFlagReturnIcon::CCaptureFlagReturnIcon()
{
#ifdef CLIENT_DLL
	m_pReturnProgressMaterial_Empty = NULL;
	m_pReturnProgressMaterial_Full = NULL;
#endif
}

#ifdef GAME_DLL

void CCaptureFlagReturnIcon::Spawn( void )
{
	BaseClass::Spawn();

	UTIL_SetSize( this, Vector(-8,-8,-8), Vector(8,8,8) );

	CollisionProp()->SetCollisionBounds( Vector( -50, -50, -50 ), Vector( 50, 50, 50 ) );
}

int CCaptureFlagReturnIcon::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_PVSCHECK );
}
#endif

#ifdef CLIENT_DLL

// This defines the properties of the 8 circle segments
// in the circular progress bar.
progress_segment_t Segments[8] = 
{
	{ 0.125, 0.5, 0.0, 1.0, 0.0, 1, 0 },
	{ 0.25,	 1.0, 0.0, 1.0, 0.5, 0, 1 },
	{ 0.375, 1.0, 0.5, 1.0, 1.0, 0, 1 },
	{ 0.50,	 1.0, 1.0, 0.5, 1.0, -1, 0 },
	{ 0.625, 0.5, 1.0, 0.0, 1.0, -1, 0 },
	{ 0.75,	 0.0, 1.0, 0.0, 0.5, 0, -1 },
	{ 0.875, 0.0, 0.5, 0.0, 0.0, 0, -1 },
	{ 1.0,	 0.0, 0.0, 0.5, 0.0, 1, 0 },
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
RenderGroup_t CCaptureFlagReturnIcon::GetRenderGroup( void ) 
{	
	return RENDER_GROUP_TRANSLUCENT_ENTITY;	
}

void CCaptureFlagReturnIcon::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	theMins.Init( -20, -20, -20 );
	theMaxs.Init(  20,  20,  20 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCaptureFlagReturnIcon::DrawModel( int flags )
{
	int nRetVal = BaseClass::DrawModel( flags );
	
	DrawReturnProgressBar();

	return nRetVal;
}

//-----------------------------------------------------------------------------
// Purpose: Draw progress bar above the flag indicating when it will return
//-----------------------------------------------------------------------------
void CCaptureFlagReturnIcon::DrawReturnProgressBar( void )
{
	CCaptureFlag *pFlag = dynamic_cast< CCaptureFlag * > ( GetOwnerEntity() );

	if ( !pFlag )
		return;

	// Don't draw if this flag is not going to reset
	if ( pFlag->GetMaxResetTime() <= 0 )
		return;

	if ( !TFGameRules()->FlagsMayBeCapped() )
		return;

	if ( !m_pReturnProgressMaterial_Full )
	{
		m_pReturnProgressMaterial_Full = materials->FindMaterial( "VGUI/flagtime_full", TEXTURE_GROUP_VGUI );
	}

	if ( !m_pReturnProgressMaterial_Empty )
	{
		m_pReturnProgressMaterial_Empty = materials->FindMaterial( "VGUI/flagtime_empty", TEXTURE_GROUP_VGUI );
	}

	if ( !m_pReturnProgressMaterial_Full || !m_pReturnProgressMaterial_Empty )
	{
		return;
	}

	CMatRenderContextPtr pRenderContext( materials );

	Vector vOrigin = GetAbsOrigin();
	QAngle vAngle = vec3_angle;

	// Align it towards the viewer
	Vector vUp = CurrentViewUp();
	Vector vRight = CurrentViewRight();
	if ( fabs( vRight.z ) > 0.95 )	// don't draw it edge-on
		return;

	vRight.z = 0;
	VectorNormalize( vRight );

	float flSize = cl_flag_return_size.GetFloat();

	unsigned char ubColor[4];
	ubColor[3] = 255;

	switch( pFlag->GetTeamNumber() )
	{
	case TF_TEAM_RED:
		ubColor[0] = 255;
		ubColor[1] = 0;
		ubColor[2] = 0;
		break;
	case TF_TEAM_BLUE:
		ubColor[0] = 0;
		ubColor[1] = 0;
		ubColor[2] = 255;
		break;
	default:
		ubColor[0] = 100;
		ubColor[1] = 100;
		ubColor[2] = 100;
		break;
	}

	// First we draw a quad of a complete icon, background
	CMeshBuilder meshBuilder;

	pRenderContext->Bind( m_pReturnProgressMaterial_Empty );
	IMesh *pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Color4ubv( ubColor );
	meshBuilder.TexCoord2f( 0,0,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv( ubColor );
	meshBuilder.TexCoord2f( 0,1,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv( ubColor );
	meshBuilder.TexCoord2f( 0,1,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * flSize) + (vUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv( ubColor );
	meshBuilder.TexCoord2f( 0,0,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -flSize) + (vUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();

	pMesh->Draw();

	float flProgress = pFlag->GetReturnProgress();

	pRenderContext->Bind( m_pReturnProgressMaterial_Full );
	pMesh = pRenderContext->GetDynamicMesh();

	vRight *= flSize * 2;
	vUp *= flSize * -2;

	// Next we're drawing the circular progress bar, in 8 segments
	// For each segment, we calculate the vertex position that will draw
	// the slice.
	int i;
	for ( i=0;i<8;i++ )
	{
		if ( flProgress < Segments[i].maxProgress )
		{
			CMeshBuilder meshBuilder_Full;

			meshBuilder_Full.Begin( pMesh, MATERIAL_TRIANGLES, 3 );

			// vert 0 is ( 0.5, 0.5 )
			meshBuilder_Full.Color4ubv( ubColor );
			meshBuilder_Full.TexCoord2f( 0, 0.5, 0.5 );
			meshBuilder_Full.Position3fv( vOrigin.Base() );
			meshBuilder_Full.AdvanceVertex();

			// Internal progress is the progress through this particular slice
			float internalProgress = RemapVal( flProgress, Segments[i].maxProgress - 0.125, Segments[i].maxProgress, 0.0, 1.0 );
			internalProgress = clamp( internalProgress, 0.0, 1.0 );

			// Calculate the x,y of the moving vertex based on internal progress
			float swipe_x = Segments[i].vert2x - ( 1.0 - internalProgress ) * 0.5 * Segments[i].swipe_dir_x;
			float swipe_y = Segments[i].vert2y - ( 1.0 - internalProgress ) * 0.5 * Segments[i].swipe_dir_y;

			// vert 1 is calculated from progress
			meshBuilder_Full.Color4ubv( ubColor );
			meshBuilder_Full.TexCoord2f( 0, swipe_x, swipe_y );
			meshBuilder_Full.Position3fv( (vOrigin + (vRight * ( swipe_x - 0.5 ) ) + (vUp *( swipe_y - 0.5 ) ) ).Base() );
			meshBuilder_Full.AdvanceVertex();

			// vert 2 is ( Segments[i].vert1x, Segments[i].vert1y )
			meshBuilder_Full.Color4ubv( ubColor );
			meshBuilder_Full.TexCoord2f( 0, Segments[i].vert2x, Segments[i].vert2y );
			meshBuilder_Full.Position3fv( (vOrigin + (vRight * ( Segments[i].vert2x - 0.5 ) ) + (vUp *( Segments[i].vert2y - 0.5 ) ) ).Base() );
			meshBuilder_Full.AdvanceVertex();

			meshBuilder_Full.End();

			pMesh->Draw();
		}
	}
}

#endif
