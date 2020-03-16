//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_zombie_claw.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Zombie Claw tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFZombieClaw, tf_weapon_zombie_claw )

//=============================================================================
//
// Weapon Zombie Claw functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFZombieClaw::CTFZombieClaw()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFZombieClaw::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "NPC_FastZombie.LeapAttack" );
	PrecacheScriptSound( "NPC_FastZombie.Scream" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFZombieClaw::SecondaryAttack( void )
{
	BaseClass::SecondaryAttack();

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( !pOwner->IsPlayerClass( TF_CLASS_ZOMBIEFAST ) )
		return;

	if ( !pOwner->GetAmmoCount( m_iPrimaryAmmoType ) )
		return;

#ifdef GAME_DLL
	if ( pOwner->GetGroundEntity() != NULL )
	{
		Vector vecPushDir = pOwner->EyeDirection3D();
		QAngle angPushDir = pOwner->EyeAngles();
		angPushDir[PITCH] = min( -25, angPushDir[PITCH] );
		AngleVectors( angPushDir, &vecPushDir );
		pOwner->SetGroundEntity( NULL );
		pOwner->SetAbsVelocity( vecPushDir * 600 );
		pOwner->m_Shared.AddCond( LFE_COND_ZOMBIE_LEAP );
		pOwner->EmitSound( "NPC_FastZombie.Scream" );
		EmitSound( "NPC_FastZombie.LeapAttack" );
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		StartEffectBarRegen();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFZombieClaw::HasChargeBar( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return false;

	return pOwner->IsPlayerClass( TF_CLASS_ZOMBIEFAST );
}
