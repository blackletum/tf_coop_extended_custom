//====== Copyright � 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose:
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_jar_gas.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "datacache/imdlcache.h"
#include "effect_dispatch_data.h"
#include "engine/IEngineSound.h"
#include "tf_gamerules.h"
#include "eventlist.h"
#include "tf_viewmodel.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "particles_new.h"
// Server specific.
#else
#include "tf_player.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "tf_fx.h"
#include "tf_gamestats.h"
#endif

#define GAS_MODEL "models/weapons/c_models/c_gascan/c_gascan.mdl"

#define TF_JAR_DAMAGE 0.0f
#define TF_JAR_VEL 1000.0f
#define TF_JAR_GRAV 1.0f

extern ConVar tf_grenade_show_radius;
//=============================================================================
//
// TF Projectile Jar Gas tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_JarGas, DT_TFProjectile_JarGas )

BEGIN_NETWORK_TABLE( CTFProjectile_JarGas, DT_TFProjectile_JarGas )
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFProjectile_JarGas )
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_projectile_jar_gas, CTFProjectile_JarGas );
PRECACHE_REGISTER( tf_projectile_jar_gas );

#ifdef GAME_DLL
CTFProjectile_JarGas *CTFProjectile_JarGas::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo )
{
	CTFProjectile_JarGas *pJar = static_cast<CTFProjectile_JarGas *>( CBaseEntity::CreateNoSpawn( "tf_projectile_jar_gas", vecOrigin, vecAngles, pOwner ) );
	if ( pJar )
	{
		// Set scorer.
		pJar->SetScorer( pScorer );

		// Set firing weapon.
		pJar->SetLauncher( pWeapon );

		DispatchSpawn( pJar );

		pJar->InitGrenade( vecVelocity, angVelocity, pOwner, pWeapon );

		pJar->ApplyLocalAngularVelocityImpulse( angVelocity );
	}

	return pJar;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_JarGas::Precache( void )
{
	PrecacheModel( GAS_MODEL );
	PrecacheParticleSystem( "peejar_impact_gas" );

	BaseClass::Precache();
}
#endif

//=============================================================================
//
// Weapon PISS tables.
//


IMPLEMENT_NETWORKCLASS_ALIASED( TFJarGas, DT_TFWeaponJarGas )

BEGIN_NETWORK_TABLE( CTFJarGas, DT_TFWeaponJarGas )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFJarGas )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_jar_gas, CTFJarGas );
PRECACHE_WEAPON_REGISTER( tf_weapon_jar_gas );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFJarGas )
END_DATADESC()
#endif

CTFJarGas::CTFJarGas()
{
	m_flEffectBarRegenTime = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFJarGas::Precache( void )
{
	BaseClass::Precache();
	PrecacheParticleSystem( "peejar_drips_gas" );
}

// ---------------------------------------------------------------------------- -
// Purpose: 
//-----------------------------------------------------------------------------
void CTFJarGas::PrimaryAttack( void )
{
	// Get the player owning the weapon.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
		return;

	if ( !HasAmmo() )
		return;

	CalcIsAttackCritical();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	FireProjectile( pPlayer );

#if !defined( CLIENT_DLL ) 
	pPlayer->SpeakWeaponFire( MP_CONCEPT_JARATE_LAUNCH );
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif

	// Set next attack times.
	float flDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flDelay, mult_postfiredelay );
	m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flEffectBarRegenTime = gpGlobals->curtime + InternalGetEffectBarRechargeTime();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJarGas::GetProjectileDamage( void )
{
	return TF_JAR_DAMAGE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJarGas::GetProjectileSpeed( void )
{
	return TF_JAR_VEL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFJarGas::GetProjectileGravity( void )
{
	return TF_JAR_GRAV;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFJarGas::CalcIsAttackCriticalHelper( void )
{
	// No random critical hits.
	return false;
}