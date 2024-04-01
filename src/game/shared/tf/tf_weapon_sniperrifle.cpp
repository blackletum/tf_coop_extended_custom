//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Sniper Rifle
//
//=============================================================================//
#include "cbase.h" 
#include "tf_fx_shared.h"
#include "tf_weapon_sniperrifle.h"
#include "in_buttons.h"

#include "portal_util_shared.h"

// Client specific.
#ifdef CLIENT_DLL
#include "view.h"
#include "beamdraw.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include "vgui_controls/Controls.h"
#include "hud_crosshair.h"
#include "functionproxy.h"
#include "materialsystem/imaterialvar.h"
#include "toolframework_client.h"
#include "input.h"
#include "c_tf_viewmodeladdon.h"

// For TFGameRules() and Player resources
#include "tf_gamerules.h"
#include "c_tf_playerresource.h"

// forward declarations
void ToolFramework_RecordMaterialParams( IMaterial *pMaterial );

ConVar tf_sniper_fullcharge_bell( "tf_sniper_fullcharge_bell", "0", FCVAR_ARCHIVE );
#endif

#define TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC	50.0f
#define TF_WEAPON_SNIPERRIFLE_UNCHARGE_PER_SEC	75.0f
#define	TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN		50.0f
#define TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX		150.0f
#define TF_WEAPON_SNIPERRIFLE_RELOAD_TIME		1.5f
#define TF_WEAPON_SNIPERRIFLE_ZOOM_TIME			0.3f

#define TF_WEAPON_SNIPERRIFLE_NO_CRIT_AFTER_ZOOM_TIME	0.2f

#define SNIPER_DOT_SPRITE_RED		"effects/sniperdot_red.vmt"
#define SNIPER_DOT_SPRITE_BLUE		"effects/sniperdot_blue.vmt"

//=============================================================================
//
// Weapon Sniper Rifles tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFSniperRifle, DT_TFSniperRifle )

BEGIN_NETWORK_TABLE_NOBASE( CTFSniperRifle, DT_SniperRifleLocalData )
#if !defined( CLIENT_DLL )
	SendPropFloat( SENDINFO(m_flChargedDamage), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#else
	RecvPropFloat( RECVINFO(m_flChargedDamage) ),
#endif
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE( CTFSniperRifle, DT_TFSniperRifle )
#if !defined( CLIENT_DLL )
	SendPropDataTable( "SniperRifleLocalData", 0, &REFERENCE_SEND_TABLE( DT_SniperRifleLocalData ), SendProxy_SendLocalWeaponDataTable ),
#else
	RecvPropDataTable( "SniperRifleLocalData", 0, 0, &REFERENCE_RECV_TABLE( DT_SniperRifleLocalData ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFSniperRifle )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_flUnzoomTime, FIELD_FLOAT, 0 ),
	DEFINE_PRED_FIELD( m_flRezoomTime, FIELD_FLOAT, 0 ),
	DEFINE_PRED_FIELD( m_bRezoomAfterShot, FIELD_BOOLEAN, 0 ),
	DEFINE_PRED_FIELD( m_flChargedDamage, FIELD_FLOAT, 0 ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_sniperrifle, CTFSniperRifle );
PRECACHE_WEAPON_REGISTER( tf_weapon_sniperrifle );

//=============================================================================
//
// Weapon Sniper Rifles funcions.
//

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFSniperRifle::CTFSniperRifle()
{
// Server specific.
#ifdef GAME_DLL
	m_hSniperDot = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CTFSniperRifle::~CTFSniperRifle()
{
// Server specific.
#ifdef GAME_DLL
	DestroySniperDot();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::Spawn()
{
	m_iAltFireHint = HINT_ALTFIRE_SNIPERRIFLE;
	BaseClass::Spawn();

	ResetTimers();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::Precache()
{
	BaseClass::Precache();
	PrecacheModel( SNIPER_DOT_SPRITE_RED );
	PrecacheModel( SNIPER_DOT_SPRITE_BLUE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::ResetTimers( void )
{
	m_flUnzoomTime = -1;
	m_flRezoomTime = -1;
	m_bRezoomAfterShot = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSniperRifle::Reload( void )
{
	// We currently don't reload.
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFSniperRifle::CanHolster( void ) const
{
 	CTFPlayer *pPlayer = GetTFPlayerOwner();
 	if ( pPlayer )
	{
		// don't allow us to holster this weapon if we're in the process of zooming and 
		// we've just fired the weapon (next primary attack is only 1.5 seconds after firing)
		if ( ( pPlayer->GetFOV() < pPlayer->GetDefaultFOV() ) && ( m_flNextPrimaryAttack > gpGlobals->curtime ) )
		{
			return false;
		}
	}

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSniperRifle::Holster( CBaseCombatWeapon *pSwitchingTo )
{
// Server specific.
#ifdef GAME_DLL
	// Destroy the sniper dot.
	DestroySniperDot();
#else
	m_bDinged = false;
#endif

	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( pPlayer && pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
	{
		ZoomOut();
	}

	m_flChargedDamage = 0.0f;
	ResetTimers();

	return BaseClass::Holster( pSwitchingTo );
}

void CTFSniperRifle::WeaponReset( void )
{
	BaseClass::WeaponReset();

	ZoomOut();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::HandleZooms( void )
{
	// Get the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	int bBlockZoom = 0;
	CALL_ATTRIB_HOOK_INT( bBlockZoom, unimplemented_mod_sniper_no_charge );
	if ( bBlockZoom > 0 )
		return;

	// Handle the zoom when taunting.
	if ( pPlayer->m_Shared.InCond( TF_COND_TAUNTING ) )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_AIMING ) )
		{
			ToggleZoom();
		}

		//Don't rezoom in the middle of a taunt.
		ResetTimers();
	}

	if ( m_flUnzoomTime > 0 && gpGlobals->curtime > m_flUnzoomTime )
	{
		if ( m_bRezoomAfterShot )
		{
			ZoomOutIn();
			m_bRezoomAfterShot = false;
		}
		else
		{
			ZoomOut();
		}

		m_flUnzoomTime = -1;
	}

	if ( m_flRezoomTime > 0 )
	{
		if ( gpGlobals->curtime > m_flRezoomTime )
		{
            ZoomIn();
			m_flRezoomTime = -1;
		}
	}

	if ( ( pPlayer->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		// If we're in the process of rezooming, just cancel it
		if ( m_flRezoomTime > 0 || m_flUnzoomTime > 0 )
		{
			// Prevent them from rezooming in less time than they would have
			m_flNextSecondaryAttack = m_flRezoomTime + TF_WEAPON_SNIPERRIFLE_ZOOM_TIME;
			m_flRezoomTime = -1;
		}
		else
		{
			Zoom();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::ItemPostFrame( void )
{
	// If we're lowered, we're not allowed to fire
	if ( m_bLowered )
		return;

	// Get the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
	{
		if ( IsZoomed() )
		{
			ToggleZoom();
		}
		return;
	}

	HandleZooms();

#ifdef GAME_DLL
	// Update the sniper dot position if we have one
	if ( m_hSniperDot )
	{
		UpdateSniperDot();
	}
#endif

	// Start charging when we're zoomed in, and allowed to fire
	if ( pPlayer->m_Shared.IsJumping() )
	{
		// Unzoom if we're jumping
		if ( IsZoomed() )
		{
			ToggleZoom();
		}

		m_flChargedDamage = 0.0f;
		m_bRezoomAfterShot = false;
	}
	else if ( m_flNextSecondaryAttack <= gpGlobals->curtime )
	{
		// Don't start charging in the time just after a shot before we unzoom to play rack anim.
		if ( pPlayer->m_Shared.InCond( TF_COND_AIMING ) && !m_bRezoomAfterShot )
		{
			m_flChargedDamage = min( m_flChargedDamage + gpGlobals->frametime * SniperRifleChargeRateMod(), TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX );

#ifdef CLIENT_DLL
			if ( m_flChargedDamage >= TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX && !m_bDinged )
			{
				m_bDinged = true;
				if ( tf_sniper_fullcharge_bell.GetBool() )
				{
					CSingleUserRecipientFilter filter( pPlayer );
					C_BaseEntity::EmitSound( filter, pPlayer->entindex(), "TFPlayer.Recharged" );
				}
			}
#endif
		}
		else
		{
			m_flChargedDamage = max( 0.0f, m_flChargedDamage - gpGlobals->frametime * TF_WEAPON_SNIPERRIFLE_UNCHARGE_PER_SEC );
		}
	}

	// Fire.
	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		Fire( pPlayer );
	}

	// Idle.
	if ( !( ( pPlayer->m_nButtons & IN_ATTACK) || ( pPlayer->m_nButtons & IN_ATTACK2 ) ) )
	{
		// No fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFSniperRifle::Lower( void )
{
	if ( BaseClass::Lower() )
	{
		if ( IsZoomed() )
		{
			ToggleZoom();
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack.
//-----------------------------------------------------------------------------
void CTFSniperRifle::Zoom( void )
{
	// Don't allow the player to zoom in while jumping
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->m_Shared.IsJumping() )
	{
		if ( pPlayer->GetFOV() >= 75 )
			return;
	}

	ToggleZoom();

	// at least 0.1 seconds from now, but don't stomp a previous value
	m_flNextPrimaryAttack = max( m_flNextPrimaryAttack, gpGlobals->curtime + 0.1 );
	m_flNextSecondaryAttack = gpGlobals->curtime + TF_WEAPON_SNIPERRIFLE_ZOOM_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::ZoomOutIn( void )
{
	ZoomOut();

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->ShouldAutoRezoom() )
	{
		m_flRezoomTime = gpGlobals->curtime + 0.9;
	}
	else
	{
		m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::ZoomIn( void )
{
	// Start aiming.
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( !pPlayer )
		return;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	BaseClass::ZoomIn();

	pPlayer->m_Shared.AddCond( TF_COND_AIMING );
	pPlayer->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	// Create the sniper dot.
	CreateSniperDot();
	pPlayer->ClearExpression();
#endif
}

bool CTFSniperRifle::IsZoomed( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( pPlayer )
	{
		return pPlayer->m_Shared.InCond( TF_COND_ZOOMED );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::ZoomOut( void )
{
	BaseClass::ZoomOut();

	// Stop aiming
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( !pPlayer )
		return;

	pPlayer->m_Shared.RemoveCond( TF_COND_AIMING );
	pPlayer->TeamFortress_SetSpeed();

#ifdef GAME_DLL
	// Destroy the sniper dot.
	DestroySniperDot();
	pPlayer->ClearExpression();
#else
	m_bDinged = false;
#endif

	// if we are thinking about zooming, cancel it
	m_flUnzoomTime = -1;
	m_flRezoomTime = -1;
	m_bRezoomAfterShot = false;
	m_flChargedDamage = 0.0f;	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::Fire( CTFPlayer *pPlayer )
{
	// Check the ammo.  We don't use clip ammo, check the primary ammo type.
	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		HandleFireOnEmpty();
		return;
	}

	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return;

	// Fire the sniper shot.
	PrimaryAttack();

	if ( IsZoomed() )
	{
		// If we have more bullets, zoom out, play the bolt animation and zoom back in
		if( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) > 0 )
		{
			SetRezoom( true, 0.5f );	// zoom out in 0.5 seconds, then rezoom
		}
		else	
		{
			//just zoom out
			SetRezoom( false, 0.5f );	// just zoom out in 0.5 seconds
		}
	}
	else
	{
		// Prevent primary fire preventing zooms
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}

	m_flChargedDamage = 0.0f;

#ifdef GAME_DLL
	if ( m_hSniperDot )
	{
		m_hSniperDot->ResetChargeTime();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifle::SetRezoom( bool bRezoom, float flDelay )
{
	m_flUnzoomTime = gpGlobals->curtime + flDelay;

	m_bRezoomAfterShot = bRezoom;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CTFSniperRifle::GetProjectileDamage( void )
{
	// Uncharged? Min damage.
	float flDamage = max( m_flChargedDamage, TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN );

	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( pPlayer && ( pPlayer->m_Shared.InCond( TF_COND_RUNE_STRENGTH ) || pPlayer->m_Shared.InCond( TF_COND_RUNE_PRECISION ) ) )
		flDamage *= 2;

	return flDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFSniperRifle::GetDamageType( void ) const
{
	// Only do hit location damage if we're zoomed
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	int bSniperHeadshotNoScope = 0;
	CALL_ATTRIB_HOOK_INT(bSniperHeadshotNoScope, sniper_crit_no_scope); 
	int bSniperNoCritNotFullCharged = 0;
	CALL_ATTRIB_HOOK_INT(bSniperNoCritNotFullCharged, sniper_no_headshot_without_full_charge);
	if ( pPlayer && (pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) || bSniperHeadshotNoScope != 0)  )
		return BaseClass::GetDamageType();

	return ( BaseClass::GetDamageType() & ~DMG_USE_HITLOCATIONS );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::CreateSniperDot( void )
{
// Server specific.
#ifdef GAME_DLL

	// Check to see if we have already been created?
	if ( m_hSniperDot )
		return;

	// Get the owning player (make sure we have one).
	CBaseCombatCharacter *pPlayer = GetOwner();
	if ( !pPlayer )
		return;

	// Create the sniper dot, but do not make it visible yet.
	m_hSniperDot = CSniperDot::Create( GetAbsOrigin(), pPlayer, true );
	m_hSniperDot->ChangeTeam( pPlayer->GetTeamNumber() );

#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::DestroySniperDot( void )
{
// Server specific.
#ifdef GAME_DLL

	// Destroy the sniper dot.
	if ( m_hSniperDot )
	{
		UTIL_Remove( m_hSniperDot );
		m_hSniperDot = NULL;
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifle::UpdateSniperDot( void )
{
// Server specific.
#ifdef GAME_DLL

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// Get the start and endpoints.
	Vector vecMuzzlePos = pPlayer->Weapon_ShootPosition();
	Vector forward;
	pPlayer->EyeVectors( &forward );
	Vector vecEndPos = vecMuzzlePos + ( forward * MAX_TRACE_LENGTH );

	trace_t	trace;

#if defined( PORTAL )
	g_bBulletPortalTrace = true;
	Ray_t rayLaser; rayLaser.Init( vecMuzzlePos, vecEndPos );
	UTIL_Portal_TraceRay( rayLaser, (MASK_SHOT & ~CONTENTS_WINDOW), GetOwner(), COLLISION_GROUP_NONE, &trace );
	g_bBulletPortalTrace = false;
#else
	UTIL_TraceLine( vecMuzzlePos, vecEndPos, ( MASK_SHOT & ~CONTENTS_WINDOW ), GetOwner(), COLLISION_GROUP_NONE, &trace );
#endif

	// Update the sniper dot.
	if ( m_hSniperDot )
	{
		CBaseEntity *pEntity = NULL;
		if ( trace.DidHitNonWorldEntity() )
		{
			pEntity = trace.m_pEnt;
			if ( !pEntity || !pEntity->m_takedamage )
			{
				pEntity = NULL;
			}
		}

		m_hSniperDot->Update( pEntity, trace.endpos, trace.plane.normal );
	}

#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSniperRifle::CanFireCriticalShot( bool bIsHeadshot )
{
	// can only fire a crit shot if this is a headshot
	if ( !bIsHeadshot )
		return false;

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 ) 
		return false;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer )
	{
		int iCritNoScope = 0;
		CALL_ATTRIB_HOOK_INT( iCritNoScope, sniper_crit_no_scope );
		if ( iCritNoScope != 1 )
		{
			// no crits if they're not zoomed
			if ( pPlayer->GetFOV() >= pPlayer->GetDefaultFOV() )
				return false;

			// no crits for 0.2 seconds after starting to zoom
			if ( ( gpGlobals->curtime - pPlayer->GetFOVTime() ) < TF_WEAPON_SNIPERRIFLE_NO_CRIT_AFTER_ZOOM_TIME )
				return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
float CTFSniperRifle::GetJarateTime( void )
{
	float flJarateDuration = 0;
	CALL_ATTRIB_HOOK_FLOAT( flJarateDuration, jarate_duration );
	if ( flJarateDuration > 0.0 )
		return RemapValClamped( m_flChargedDamage, TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN, TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX, 2.0, flJarateDuration );

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
float CTFSniperRifle::SniperRifleChargeRateMod( void )
{
	float flChargePerSec = TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC;
	CALL_ATTRIB_HOOK_FLOAT( flChargePerSec, mult_sniper_charge_per_sec );
	return flChargePerSec;
}

//=============================================================================
//
// Client specific functions.
//
#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFSniperRifle::GetHUDDamagePerc( void )
{
	return (m_flChargedDamage / TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX);
}

//-----------------------------------------------------------------------------
// Returns the sniper chargeup from 0 to 1
//-----------------------------------------------------------------------------
class CProxySniperRifleCharge : public CResultProxy
{
public:
	void OnBind( void *pC_BaseEntity );
};

void CProxySniperRifleCharge::OnBind( void *pC_BaseEntity )
{
	Assert( m_pResult );

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();

	if ( GetSpectatorTarget() != 0 && GetSpectatorMode() == OBS_MODE_IN_EYE )
	{
		pPlayer = (C_TFPlayer *)UTIL_PlayerByIndex( GetSpectatorTarget() );
	}

	if ( pPlayer )
	{
		CTFSniperRifle *pWeapon = assert_cast<CTFSniperRifle*>(pPlayer->GetActiveTFWeapon());
		if ( pWeapon )
		{
			float flChargeValue = ( ( 1.0 - pWeapon->GetHUDDamagePerc() ) * 0.8 ) + 0.6;

			VMatrix mat, temp;

			Vector2D center( 0.5, 0.5 );
			MatrixBuildTranslation( mat, -center.x, -center.y, 0.0f );

			// scale
			{
				Vector2D scale( 1.0f, 0.25f );
				MatrixBuildScale( temp, scale.x, scale.y, 1.0f );
				MatrixMultiply( temp, mat, mat );
			}

			MatrixBuildTranslation( temp, center.x, center.y, 0.0f );
			MatrixMultiply( temp, mat, mat );

			// translation
			{
				Vector2D translation( 0.0f, flChargeValue );
				MatrixBuildTranslation( temp, translation.x, translation.y, 0.0f );
				MatrixMultiply( temp, mat, mat );
			}

			m_pResult->SetMatrixValue( mat );
		}
	}

	if ( ToolsEnabled() )
	{
		ToolFramework_RecordMaterialParams( GetMaterial() );
	}
}

EXPOSE_INTERFACE( CProxySniperRifleCharge, IMaterialProxy, "SniperRifleCharge" IMATERIAL_PROXY_INTERFACE_VERSION );
#endif

//=============================================================================
//
// Laser Dot functions.
//

IMPLEMENT_NETWORKCLASS_ALIASED( SniperDot, DT_SniperDot )

BEGIN_NETWORK_TABLE( CSniperDot, DT_SniperDot )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flChargeStartTime ) ),
#else
	SendPropTime( SENDINFO( m_flChargeStartTime ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( env_sniperdot, CSniperDot );

BEGIN_DATADESC( CSniperDot )
DEFINE_FIELD( m_vecSurfaceNormal,	FIELD_VECTOR ),
DEFINE_FIELD( m_hTargetEnt,			FIELD_EHANDLE ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CSniperDot::CSniperDot( void )
{
	m_vecSurfaceNormal.Init();
	m_hTargetEnt = NULL;

#ifdef CLIENT_DLL
	m_hSpriteMaterial = NULL;
#endif

	ResetChargeTime();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CSniperDot::~CSniperDot( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CSniperDot
//-----------------------------------------------------------------------------
CSniperDot *CSniperDot::Create( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
// Client specific.
#ifdef CLIENT_DLL

	return NULL;

// Server specific.
#else

	// Create the sniper dot entity.
	CSniperDot *pDot = static_cast<CSniperDot*>( CBaseEntity::Create( "env_sniperdot", origin, QAngle( 0.0f, 0.0f, 0.0f ) ) );
	if ( !pDot )
		return NULL;

	//Create the graphic
	pDot->SetMoveType( MOVETYPE_NONE );
	pDot->AddSolidFlags( FSOLID_NOT_SOLID );
	pDot->AddEffects( EF_NOSHADOW );
	UTIL_SetSize( pDot, -Vector( 4.0f, 4.0f, 4.0f ), Vector( 4.0f, 4.0f, 4.0f ) );

	// Set owner.
	pDot->SetOwnerEntity( pOwner );

	// Force updates even though we don't have a model.
	pDot->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	return pDot;

#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSniperDot::Update( CBaseEntity *pTarget, const Vector &vecOrigin, const Vector &vecNormal )
{
	SetAbsOrigin( vecOrigin );
	m_vecSurfaceNormal = vecNormal;
	m_hTargetEnt = pTarget;
}

//=============================================================================
//
// Client specific functions.
//
#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
// TFTODO: Make the sniper dot get brighter the more damage it will do.
//-----------------------------------------------------------------------------
int CSniperDot::DrawModel( int flags )
{
	// Get the owning player.
	C_TFPlayer *pPlayer = ToTFPlayer( GetOwnerEntity() );
	if ( !pPlayer )
		return -1;

	// Get the sprite rendering position.
	Vector vecEndPos;

	float flSize = 6.0;

	if ( !pPlayer->IsDormant() )
	{
		Vector vecAttachment, vecDir;
		QAngle angles;

		float flDist = MAX_TRACE_LENGTH;

		// Always draw the dot in front of our faces when in first-person.
		if ( pPlayer->IsLocalPlayer() )
		{
			// Take our view position and orientation
			vecAttachment = CurrentViewOrigin();
			vecDir = CurrentViewForward();

			// Clamp the forward distance for the sniper's firstperson
			flDist = 384;

			flSize = 2.0;
		}
		else
		{
			// Take the owning player eye position and direction.
			vecAttachment = pPlayer->EyePosition();
			QAngle angles = pPlayer->EyeAngles();
			AngleVectors( angles, &vecDir );
		}

		trace_t tr;
		Ray_t rayLaser; rayLaser.Init( vecAttachment, vecAttachment + ( vecDir * flDist ) );
		UTIL_Portal_TraceRay( rayLaser, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

		// Backup off the hit plane, towards the source
		vecEndPos = tr.endpos + vecDir * -4;
	}
	else
	{
		// Just use our position if we can't predict it otherwise.
		vecEndPos = GetAbsOrigin();
	}

	// Draw our laser dot in space.
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_hSpriteMaterial, this );

	float flChargePerSec = TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC;

	CTFSniperRifle *pSniperRifle = dynamic_cast<CTFSniperRifle*>( pPlayer->Weapon_GetSlot( LOADOUT_POSITION_PRIMARY ) );
	if (pSniperRifle){
		flChargePerSec = pSniperRifle->SniperRifleChargeRateMod();
	}

	float flLifeTime = gpGlobals->curtime - m_flChargeStartTime;
	float flStrength = RemapValClamped( flLifeTime, 0.0, TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX / flChargePerSec, 0.1, 1.0 );
	
	color32 innercolor = { 255, 255, 255, 255 };
	color32 outercolor = { 255, 255, 255, 128 };

	DrawSprite( vecEndPos, flSize, flSize, outercolor );
	DrawSprite( vecEndPos, flSize * flStrength, flSize * flStrength, innercolor );

	// Successful.
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSniperDot::ShouldDraw( void )			
{
	if ( IsEffectActive( EF_NODRAW ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSniperDot::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
			switch (GetTeamNumber())
			{
			case TF_TEAM_RED:
				m_hSpriteMaterial.Init(SNIPER_DOT_SPRITE_RED, TEXTURE_GROUP_CLIENT_EFFECTS);
				break;
			case TF_TEAM_BLUE:
				m_hSpriteMaterial.Init(SNIPER_DOT_SPRITE_BLUE, TEXTURE_GROUP_CLIENT_EFFECTS);
				break;
			}
	}
}

#endif

//=============================================================================
//
// Weapon Sniper Rifles tables.
//

CREATE_SIMPLE_WEAPON_TABLE( TFSniperRifleClassic, tf_weapon_sniperrifle_classic )

//=============================================================================
//
// Weapon Sniper Rifles funcions.
//

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFSniperRifleClassic::CTFSniperRifleClassic()
{
// Server specific.
#ifdef GAME_DLL
	m_hSniperDot = NULL;
#else
	m_pChargeBeam = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CTFSniperRifleClassic::~CTFSniperRifleClassic()
{
// Server specific.
#ifdef GAME_DLL
	DestroySniperDot();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::Spawn()
{
	m_iAltFireHint = HINT_ALTFIRE_SNIPERRIFLE;
	BaseClass::Spawn();

	ResetTimers();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::Precache()
{
	BaseClass::Precache();

	PrecacheTeamParticles( "tfc_sniper_charge_%s" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSniperRifleClassic::Holster( CBaseCombatWeapon *pSwitchingTo )
{
// Server specific.
#ifdef GAME_DLL
	// Destroy the sniper dot.
	DestroySniperDot();
#else
	m_bDinged = false;

	if ( m_pChargeBeam )
	{
		ParticleProp()->StopEmission( m_pChargeBeam );
		m_pChargeBeam = NULL;
	}
#endif

	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( pPlayer )
	{
		pPlayer->m_Shared.RemoveCond( TF_COND_AIMING );
		pPlayer->TeamFortress_SetSpeed();

		if ( pPlayer->m_Shared.InCond( TF_COND_ZOOMED ) )
			ZoomOut();
	}

	m_flChargedDamage = 0.0f;
	ResetTimers();

	return BaseClass::Holster( pSwitchingTo );
}

void CTFSniperRifleClassic::WeaponReset( void )
{
	BaseClass::WeaponReset();

	ZoomOut();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::HandleZooms( void )
{
	// Get the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	int bBlockZoom = 0;
	CALL_ATTRIB_HOOK_INT( bBlockZoom, unimplemented_mod_sniper_no_charge );
	if ( bBlockZoom > 0 )
		return;

	// Handle the zoom when taunting.
	if ( pPlayer->m_Shared.InCond( TF_COND_TAUNTING ) )
	{
		if ( pPlayer->m_Shared.InCond( TF_COND_AIMING ) )
		{
			ToggleZoom();
		}

		//Don't rezoom in the middle of a taunt.
		ResetTimers();
	}

	if ( m_flUnzoomTime > 0 && gpGlobals->curtime > m_flUnzoomTime )
	{
		if ( m_bRezoomAfterShot )
		{
			ZoomOutIn();
			m_bRezoomAfterShot = false;
		}
		else
		{
			ZoomOut();
		}

		m_flUnzoomTime = -1;
	}

	if ( m_flRezoomTime > 0 )
	{
		if ( gpGlobals->curtime > m_flRezoomTime )
		{
            ZoomIn();
			m_flRezoomTime = -1;
		}
	}

	if ( ( pPlayer->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		// If we're in the process of rezooming, just cancel it
		if ( m_flRezoomTime > 0 || m_flUnzoomTime > 0 )
		{
			// Prevent them from rezooming in less time than they would have
			m_flNextSecondaryAttack = m_flRezoomTime + TF_WEAPON_SNIPERRIFLE_ZOOM_TIME;
			m_flRezoomTime = -1;
		}
		else
		{
			Zoom();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::ItemPostFrame( void )
{
	// If we're lowered, we're not allowed to fire
	if ( m_bLowered )
		return;

	// Get the owning player.
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
	{
		if ( IsZoomed() )
		{
			ToggleZoom();
		}
		return;
	}

	HandleZooms();

#ifdef GAME_DLL
	// Update the sniper dot position if we have one
	if ( m_hSniperDot )
	{
		UpdateSniperDot();
	}
#endif

	// Start charging when we're allowed to fire
	if ( ( pPlayer->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		// Don't start charging in the time just after a shot before we unzoom to play rack anim.
		if ( !m_bRezoomAfterShot )
		{
			m_flChargedDamage = min( m_flChargedDamage + gpGlobals->frametime * SniperRifleChargeRateMod(), TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX );
			pPlayer->m_Shared.AddCond( TF_COND_AIMING );
			pPlayer->TeamFortress_SetSpeed();
#ifdef CLIENT_DLL
			if ( m_flChargedDamage >= TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX && !m_bDinged )
			{
				m_bDinged = true;
				if ( tf_sniper_fullcharge_bell.GetBool() )
				{
					CSingleUserRecipientFilter filter( pPlayer );
					C_BaseEntity::EmitSound( filter, pPlayer->entindex(), "TFPlayer.Recharged" );
				}
			}
#else
			// Create the sniper dot.
			CreateSniperDot();
			pPlayer->ClearExpression();
#endif
		}
		else
		{
			m_flChargedDamage = max( 0.0f, m_flChargedDamage - gpGlobals->frametime * TF_WEAPON_SNIPERRIFLE_UNCHARGE_PER_SEC );
		}
	}

	if( pPlayer->m_afButtonReleased & IN_ATTACK || pPlayer->GetActiveTFWeapon() != this ) 
	{
		 Fire( pPlayer );
		 pPlayer->m_Shared.RemoveCond( TF_COND_AIMING );
		 pPlayer->TeamFortress_SetSpeed();
#ifdef GAME_DLL
		// Destroy the sniper dot.
		DestroySniperDot();
		pPlayer->ClearExpression();
#endif
	}

	// Idle.
	if ( !( ( pPlayer->m_nButtons & IN_ATTACK) || ( pPlayer->m_nButtons & IN_ATTACK2 ) ) )
	{
		// No fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFSniperRifleClassic::Lower( void )
{
	if ( BaseClass::Lower() )
	{
		if ( IsZoomed() )
		{
			ToggleZoom();
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack.
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::Zoom( void )
{
	// Don't allow the player to zoom in while jumping
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->m_Shared.IsJumping() )
	{
		if ( pPlayer->GetFOV() >= 75 )
			return;
	}

	ToggleZoom();

	// at least 0.1 seconds from now, but don't stomp a previous value
	m_flNextPrimaryAttack = max( m_flNextPrimaryAttack, gpGlobals->curtime + 0.1 );
	m_flNextSecondaryAttack = gpGlobals->curtime + TF_WEAPON_SNIPERRIFLE_ZOOM_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::ZoomIn( void )
{
	// Start aiming.
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( !pPlayer )
		return;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	BaseClass::BaseClass::ZoomIn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSniperRifleClassic::ZoomOut( void )
{
	BaseClass::BaseClass::ZoomOut();

	// Stop aiming
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( !pPlayer )
		return;

	// if we are thinking about zooming, cancel it
	m_flUnzoomTime = -1;
	m_flRezoomTime = -1;
	m_bRezoomAfterShot = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFSniperRifleClassic::GetDamageType( void ) const
{
	int iDmgType = BaseClass::GetDamageType() & ~DMG_USE_HITLOCATIONS;

	// Only do hit location damage if we're fully charged
	if ( m_flChargedDamage == TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX )
		iDmgType |= DMG_ALWAYSGIB;

	return iDmgType;
}

#ifdef CLIENT_DLL
void CTFSniperRifleClassic::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	ManageChargeBeam();
}

void CTFSniperRifleClassic::ManageChargeBeam( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	C_ViewmodelAttachmentModel *pAttachment = GetViewmodelAddon();
	if ( !pAttachment )
		return;

	/*if ( ( pOwner->m_nButtons & IN_ATTACK ) )
	{
		if ( !m_pChargeBeam )
		{
			const char *pszEffectName = ConstructTeamParticle( "tfc_sniper_charge_%s", pOwner->GetTeamNumber() );
			m_pChargeBeam = pAttachment->ParticleProp()->Create( pszEffectName, PATTACH_POINT_FOLLOW, "laser" );
		}
	}
	else
	{
		if ( m_pChargeBeam )
		{
			ParticleProp()->StopEmission( m_pChargeBeam );
			m_pChargeBeam = NULL;
		}
	}*/
}
#endif

//=============================================================================
//
// Weapon Sniper Rifles Decap tables.
//

CREATE_SIMPLE_WEAPON_TABLE( TFSniperRifleDecap, tf_weapon_sniperrifle_decap )

//=============================================================================
//
// Weapon Sniper Rifles Decap funcions.
//

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFSniperRifleDecap::CTFSniperRifleDecap()
{
// Server specific.
#ifdef GAME_DLL
	m_hSniperDot = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CTFSniperRifleDecap::~CTFSniperRifleDecap()
{
// Server specific.
#ifdef GAME_DLL
	DestroySniperDot();
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
void CTFSniperRifleDecap::OnPlayerKill( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	CTFPlayer *pKiller = ToTFPlayer( info.GetAttacker() );
	if ( !pKiller )
		return;

	if ( pKiller != GetOwner() )
		return;

	if ( !pVictim )
		return;

	if ( info.GetDamageCustom() == TF_DMG_CUSTOM_HEADSHOT )
	{
		int iHeadCount = pKiller->m_Shared.GetDecapitationCount() + 1;
		pKiller->m_Shared.SetDecapitationCount( iHeadCount );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
float CTFSniperRifleDecap::SniperRifleChargeRateMod( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if  (pOwner == nullptr )
		return BaseClass::SniperRifleChargeRateMod();

	return ( TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC * ( 0.5 + Min( ( pOwner->m_Shared.GetDecapitationCount() ), 6 ) * 0.25 ) );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
int CTFSniperRifleDecap::GetCount( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return 0;
	
	return pOwner->m_Shared.GetDecapitationCount();
}
#endif