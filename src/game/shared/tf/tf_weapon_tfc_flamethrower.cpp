//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_tfc_flamethrower.h"
#include "tf_tfc_flame.h"
// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "soundent.h"
#include "tf_gamerules.h"
#include "ilagcompensationmanager.h"
#include "ai_basenpc.h"
#include "props.h"
#endif

//=============================================================================
//
// Weapon TFC Flamethrower tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFFlamethrowerTFC, tf_weapon_tfc_flamethrower )

//=============================================================================
//
// Weapon TFC Flamethrower functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFFlamethrowerTFC::CTFFlamethrowerTFC()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFFlamethrowerTFC::FireProjectile( CTFPlayer *pPlayer )
{
	EmitSound( "Weapon_FlameThrower_TFC.Single" );

	// Server only - create the fireball.
#ifdef GAME_DLL
	Vector vecSrc;
	QAngle angForward;
	Vector vecOffset( -23.5f, 10.0f, -23.5f );
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		vecOffset.z = 12.5f;
	}

	GetProjectileFireSetup( pPlayer, vecOffset, &vecSrc, &angForward, false, false );
		
	CTFCFlame *pProjectile = CTFCFlame::Create( this, vecSrc, angForward, pPlayer, pPlayer );
	if ( pProjectile )
	{
		pProjectile->SetCritical( IsCurrentAttackACrit() );
		pProjectile->SetDamage( GetProjectileDamage() );
	}

#endif

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
	
	RemoveAmmo( pPlayer );

	DoFireEffects();

	UpdatePunchAngles( pPlayer );

#ifdef GAME_DLL
	return pProjectile;
#endif
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFFlamethrowerTFC::PrimaryAttack()
{
	m_bReloadedThroughAnimEvent = false;

	BaseClass::BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFFlamethrowerTFC::SecondaryAttack()
{
	BaseClass::BaseClass::SecondaryAttack();
}