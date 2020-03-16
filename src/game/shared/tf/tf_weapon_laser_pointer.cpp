//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_laser_pointer.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "tf_gamerules.h"
#include "baseobject_shared.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "c_obj_sentrygun.h"
// Server specific.
#else
#include "tf_player.h"
#include "ai_basenpc.h"
#include "tf_obj_sentrygun.h"
#endif

//=============================================================================
//
// Weapon Laser Pointer tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFLaserPointer, tf_weapon_laser_pointer )

//=============================================================================
//
// Weapon Laser Pointer functions.
//
CTFLaserPointer::CTFLaserPointer()
{
#ifdef GAME_DLL
	m_hGun = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLaserPointer::PrimaryAttack( void )
{
	if ( !CanAttack() )
		return;

#ifdef GAME_DLL
	if ( !m_hGun )
		return;

	if ( m_flNextPrimaryAttack < gpGlobals->curtime && m_hGun->GetState() == SENTRY_STATE_WRANGLED )
	{
		m_hGun->SetShouldFire( true );

		// input buffer
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.05f;
	}
#endif
	SendWeaponAnim( ACT_ITEM1_VM_RELOAD );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLaserPointer::SecondaryAttack( void )
{
	if ( !CanAttack() )
		return;

#ifdef GAME_DLL
	if ( !m_hGun )
		return;

	if ( m_flNextSecondaryAttack <= gpGlobals->curtime && m_hGun->GetState() == SENTRY_STATE_WRANGLED )
	{
		int iUpgradeLevel = m_hGun->GetUpgradeLevel();

		if ( iUpgradeLevel == 3 )
		{
			m_hGun->FireRocket();

			// Rockets fire slightly faster wrangled
			m_flNextSecondaryAttack = gpGlobals->curtime + 2.5;
		}
	}
#endif 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFLaserPointer::Deploy( void )
{
#ifdef GAME_DLL
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( pOwner )
	{
		for ( int i = 0; i < pOwner->GetObjectCount(); i++ )
		{
			CBaseObject *pObject = pOwner->GetObject( i );

			if ( pObject->GetType() == OBJ_SENTRYGUN )
			{
				m_hGun = dynamic_cast< CObjectSentrygun * > ( pObject );
			}
		}
	}
#endif

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLaserPointer::ItemPostFrame( void )
{
#ifdef GAME_DLL
	if ( m_hGun )
	{
		//TODO: Find a better way to determine if we can wrangle
		if ( !m_hGun->IsRedeploying() && !m_hGun->IsBuilding() && !m_hGun->IsUpgrading() && !m_hGun->HasSapper() )
		{
			m_hGun->SetState( SENTRY_STATE_WRANGLED );
		}

		if ( m_hGun->GetState() == SENTRY_STATE_WRANGLED )
		{
			UpdateLaserDot();
		}
	}
#endif

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFLaserPointer::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef GAME_DLL
	if ( m_hGun )
	{
		if ( m_hGun->GetState() == SENTRY_STATE_WRANGLED )
		{
			m_hGun->OnStopWrangling();
			m_hGun->SetShouldFire( false );
		}
	}
	m_hGun = NULL;
#endif

	return BaseClass::Holster( pSwitchingTo );
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFLaserPointer::UpdateLaserDot( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

	m_hGun->StudioFrameAdvance( );

	if ( !pOwner || !pOwner->IsAlive() )
	{
		m_hGun->OnStopWrangling();
		m_hGun->SetShouldFire( false );
		return;
	}

	trace_t tr;
	Vector vecStart, vecEnd, vecForward;
	pOwner->EyeVectors( &vecForward );

	vecStart = pOwner->EyePosition();
	vecEnd = vecStart + ( vecForward * MAX_TRACE_LENGTH );

	CTraceFilterIgnoreTeammatesAndTeamObjects *pFilter = new CTraceFilterIgnoreTeammatesAndTeamObjects( this, COLLISION_GROUP_NONE, GetTeamNumber() );

	// First pass to find where we are looking
	Ray_t rayLaser;
	rayLaser.Init( vecStart, vecEnd );
	UTIL_Portal_TraceRay( rayLaser, MASK_SOLID, pFilter, &tr );

	vecStart = m_hGun->EyePosition();

	CProp_Portal *pPortal = pOwner->FInViewConeThroughPortal( tr.m_pEnt );
	if ( pPortal )
		UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vecEnd, vecEnd );

	// If we're looking at a player fix our position to the centermass
	if ( tr.DidHitNonWorldEntity() && tr.m_pEnt && tr.m_pEnt->IsCombatCharacter() )
	{
		vecEnd = m_hGun->GetEnemyAimPosition( tr.m_pEnt );

		// Second pass to make sure the sentry can actually see the person we're targeting 
		UTIL_Portal_TraceRay( rayLaser, MASK_SOLID, pFilter, &tr );

		if ( tr.DidHitNonWorldEntity() && tr.m_pEnt && tr.m_pEnt->IsCombatCharacter() )
		{
			m_hGun->SetEnemy( tr.m_pEnt );
		}
		else
		{
			m_hGun->SetEnemy( NULL );
			vecEnd = tr.endpos;
		}
	}
	else
	{
		// We're not locked on to a player so make sure the laser doesn't clip through walls
		m_hGun->SetEnemy( NULL );

		// Second pass
		Ray_t rayLaser; rayLaser.Init( vecStart, tr.endpos );
		UTIL_Portal_TraceRay( rayLaser, MASK_SOLID, pFilter, &tr );

		vecEnd = tr.endpos;
	}

	m_hGun->SetEndVector( vecEnd );

	// Adjust sentry angles 
	vecForward = vecEnd - vecStart;
	m_hGun->UpdateSentryAngles( vecForward );
}
#endif