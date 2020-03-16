//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_grenadelauncher.h"
#include "in_buttons.h"
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#else
#include "tf_player.h"
#include "tf_gamestats.h"
#endif

//=============================================================================
//
// Weapon Grenade Launcher tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFGrenadeLauncher, DT_WeaponGrenadeLauncher )

BEGIN_NETWORK_TABLE( CTFGrenadeLauncher, DT_WeaponGrenadeLauncher )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_flDetonateTime ) ),
#else
	SendPropTime( SENDINFO( m_flDetonateTime ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFGrenadeLauncher )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_flDetonateTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_grenadelauncher, CTFGrenadeLauncher );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenadelauncher );

//=============================================================================

#define TF_GRENADE_LAUNCER_VEL 1200

//=============================================================================
//
// Weapon Grenade Launcher functions.
//

CTFGrenadeLauncher::CTFGrenadeLauncher()
{
	m_bReloadsSingly = true;
}

CTFGrenadeLauncher::~CTFGrenadeLauncher()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::Spawn( void )
{
	m_iAltFireHint = HINT_ALTFIRE_GRENADELAUNCHER;
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFGrenadeLauncher::GetWeaponID( void ) const
{
	return TF_WEAPON_GRENADELAUNCHER; 
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we holster
//-----------------------------------------------------------------------------
bool CTFGrenadeLauncher::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_flDetonateTime = 0;
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we deploy
//-----------------------------------------------------------------------------
bool CTFGrenadeLauncher::Deploy( void )
{
	m_flDetonateTime = 0;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGrenadeLauncher::GetMaxClip1( void ) const
{
	return BaseClass::GetMaxClip1();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFGrenadeLauncher::GetDefaultClip1( void ) const
{
	return BaseClass::GetDefaultClip1();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::PrimaryAttack( void )
{
	// Check for ammunition.
	if ( m_iClip1 <= 0 && m_iClip1 != -1 )
		return;

	// Are we capable of firing again?
	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return;

	if ( !CanAttack() )
	{
		m_flDetonateTime = 0;
		return;
	}

	if ( CanCharge() )
	{
		if ( m_flDetonateTime <= 0 )
		{
			// Set the weapon mode.
			m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

			// save that we had the attack button down
			m_flDetonateTime = gpGlobals->curtime;

			SendWeaponAnim( ACT_VM_PULLBACK );
		}
		else
		{
			float flTotalChargeTime = gpGlobals->curtime - m_flDetonateTime;
			if ( flTotalChargeTime >= GetChargeMaxTime() )
			{
				LaunchGrenade();
			}
		}
	}
	else
	{
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
		
		LaunchGrenade();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::SecondaryAttack( void )
{
	BaseClass::SecondaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFGrenadeLauncher::Reload( void )
{
	if ( CanCharge() )
	{
		if ( m_flDetonateTime > 0 )
			return false;
	}

	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::WeaponIdle( void )
{
	if ( CanCharge() )
	{
		if ( m_flDetonateTime > 0 && m_iClip1 > 0 )
		{
			LaunchGrenade();
		}
		else
		{
			BaseClass::WeaponIdle();
		}
	}
	else
	{
		BaseClass::WeaponIdle();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::WeaponReset( void )
{
	BaseClass::WeaponIdle();

	m_flDetonateTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFGrenadeLauncher::FireProjectileInternal( CTFPlayer *pPlayer )
{
	CTFWeaponBaseGrenadeProj *pGrenade = (CTFWeaponBaseGrenadeProj *)FireProjectile( pPlayer );
	if ( pGrenade )
	{
		/*if ( GetDetonateMode() == TF_GL_MODE_FIZZLE )
			pGrenade->m_bFizzle = true;*/

#ifdef GAME_DLL
		if ( CanCharge() )
		{
			float flTotalChargeTime = gpGlobals->curtime - m_flDetonateTime;
			if ( flTotalChargeTime >= GetChargeMaxTime() )
				pGrenade->Detonate();
			else
				pGrenade->SetDetonateTimerLength( GetChargeMaxTime() - ( flTotalChargeTime ) );
		}
#endif
	}
	return pGrenade;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::LaunchGrenade( void )
{
	// Get the player owning the weapon.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	CalcIsAttackCritical();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	FireProjectileInternal( pPlayer );

#if !defined( CLIENT_DLL ) 
	pPlayer->SpeakWeaponFire();
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif

	// Set next attack times.
	float flDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flDelay, mult_postfiredelay );
	m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	// Check the reload mode and behave appropriately.
	if ( m_bReloadsSingly )
		m_iReloadMode.Set( TF_RELOAD_START );

	m_flDetonateTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::Precache( void )
{
	PrecacheParticleSystem( "loose_cannon_sparks" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeLauncher::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( CanCharge() )
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if ( pOwner && !( pOwner->m_nButtons & IN_ATTACK ) )
		{
			if ( m_flDetonateTime > 0 && m_iClip1 > 0 )
			{
				LaunchGrenade();
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFGrenadeLauncher::GetProjectileSpeed( void )
{
	float flVelocity = TF_GRENADE_LAUNCER_VEL;

	if ( CanCharge() )
		flVelocity = RemapValClamped( ( gpGlobals->curtime - m_flDetonateTime ), 0.0f, GetChargeMaxTime(), TF_PIPEBOMB_MIN_CHARGE_VEL, TF_PIPEBOMB_MAX_CHARGE_VEL );

	CALL_ATTRIB_HOOK_FLOAT( flVelocity, mult_projectile_range );
	return flVelocity;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFGrenadeLauncher::GetChargeMaxTime( void )
{
	float flMaxChargeTime = 1.0f;

	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		return flMaxChargeTime / 1.5f;

	return flMaxChargeTime;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFGrenadeLauncher::GetDetonateMode( void ) const
{
	int nDetonateMode = 0;
	CALL_ATTRIB_HOOK_INT( nDetonateMode, set_detonate_mode );
	return nDetonateMode;
}

//-----------------------------------------------------------------------------
// Purpose: donk cannon
//-----------------------------------------------------------------------------
bool CTFGrenadeLauncher::CanCharge( void )
{
	int iMortarMode = 0;
	CALL_ATTRIB_HOOK_INT( iMortarMode, grenade_launcher_mortar_mode );
	return ( iMortarMode > 0 );
}

//=============================================================================

CREATE_SIMPLE_WEAPON_TABLE( TFCannon, tf_weapon_cannon )

//=============================================================================
//
// Weapon Cannon functions.
//

CTFCannon::CTFCannon()
{
	m_bReloadsSingly = true;
}

CTFCannon::~CTFCannon()
{
}