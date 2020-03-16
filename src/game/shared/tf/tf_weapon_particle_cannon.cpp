//====== Copyright © 1996-2020, Valve Corporation, All rights reserved. =======
//
// TF Particle Cannon
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_particle_cannon.h"
#include "tf_fx_shared.h"
#include "tf_weaponbase_rocket.h"
#include "tf_projectile_energy_ball.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "soundent.h"
#include "tf_gamerules.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#include "ai_basenpc.h"
#include "props.h"
#endif


//=============================================================================
//
// Weapon Particle Cannon tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFParticleCannon, DT_ParticleCannon )

BEGIN_NETWORK_TABLE( CTFParticleCannon, DT_ParticleCannon )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_flChargeBeginTime ) ),
#else
	SendPropTime( SENDINFO( m_flChargeBeginTime ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFParticleCannon )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_flChargeBeginTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_particle_cannon, CTFParticleCannon );
PRECACHE_WEAPON_REGISTER( tf_weapon_particle_cannon );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CTFParticleCannon::CTFParticleCannon()
{
	m_bReloadsSingly = true;

	m_flChargeBeginTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CTFParticleCannon::~CTFParticleCannon()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( "drg_cow_idle" );
	PrecacheParticleSystem( "drg_cow_idle_blue" );
	PrecacheParticleSystem( "drg_cow_muzzleflash_normal" );
	PrecacheParticleSystem( "drg_cow_muzzleflash_normal_blue" );
	PrecacheParticleSystem( "drg_cow_muzzleflash_charged" );
	PrecacheParticleSystem( "drg_cow_muzzleflash_charged_blue" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::PrimaryAttack( void )
{
	if ( ( m_flEnergy < Energy_GetMaxEnergy() ) )
		Reload();

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::SecondaryAttack( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();

	if ( !CanAttack() )
	{
		m_flChargeBeginTime = 0.0f;
		return;
	}

	if ( ( m_flEnergy < 4.0f ) )
	{
		m_flChargeBeginTime = 0.0f;
		Reload();
		return;
	}

	if ( m_flChargeBeginTime <= 0 )
	{
		// Set the weapon mode.
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

		// save that we had the attack button down
		m_flChargeBeginTime = gpGlobals->curtime;

#ifdef CLIENT_DLL
		CreateChargeEffect();
#endif

		WeaponSound( SPECIAL1 );

		if ( pPlayer )
		{
			pPlayer->m_Shared.AddCond( TF_COND_AIMING );
			pPlayer->TeamFortress_SetSpeed();

#ifdef GAME_DLL
			pPlayer->RemoveInvisibility();
			pPlayer->RemoveDisguise();
			CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif
		}

		// Set next attack times.
		float flDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
		m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

#if GAME_DLL
		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON );
#endif

		SendWeaponAnim( ACT_PRIMARY_VM_PRIMARYATTACK_3 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFParticleCannon::FireProjectile( CTFPlayer *pPlayer )
{
	return BaseClass::FireProjectile( pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::ItemPostFrame( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	BaseClass::ItemPostFrame();

	if ( m_flChargeBeginTime > 0 )
	{
		float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;
		if ( flTotalChargeTime >= GetChargeMaxTime() )
		{
#ifdef GAME_DLL
			FireChargedShot();
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we holster
//-----------------------------------------------------------------------------
bool CTFParticleCannon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_flChargeBeginTime = 0.0f;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Reset the charge when we deploy
//-----------------------------------------------------------------------------
bool CTFParticleCannon::Deploy( void )
{
	m_flChargeBeginTime = 0.0f;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::WeaponReset( void )
{
	m_flChargeBeginTime = 0.0f;
	BaseClass::WeaponReset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFParticleCannon::GetChargeBeginTime( void )
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFParticleCannon::GetChargeMaxTime( void )
{
	float flTime = 1.60f;
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( pPlayer && pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		return flTime / 1.5f;

	return flTime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFParticleCannon::Energy_FullyCharged( void ) const
{
	return BaseClass::Energy_FullyCharged();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFParticleCannon::Energy_GetShotCost( void ) const
{
	return 1.0f;
}

// -----------------------------------------------------------------------------
// Purpose: Returns override from item schema if there is one.
// -----------------------------------------------------------------------------
const char *CTFParticleCannon::GetShootSound( int iIndex ) const
{
	return BaseClass::GetShootSound( iIndex );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::PlayWeaponShootSound( void )
{
	WeaponSound( SINGLE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFParticleCannon::OwnerCanTaunt( void )
{
	if ( m_flChargeBeginTime > 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFParticleCannon::GetProjectileSpeed( void )
{
	return 1100.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFParticleCannon::GetProjectileGravity( void )
{
	return 0.0f;
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
const char *CTFParticleCannon::GetMuzzleFlashParticleEffect( void )
{
	const char *pszEffect = "drg_cow_muzzleflash_normal";
	if ( GetTeamNumber() != TF_TEAM_RED )
		pszEffect = "drg_cow_muzzleflash_normal_blue";

	if ( m_flChargeBeginTime > 0 )
	{
		pszEffect = "drg_cow_muzzleflash_charged";
		if ( GetTeamNumber() != TF_TEAM_RED )
			pszEffect = "drg_cow_muzzleflash_charged_blue";
	}

	if ( Q_strlen( pszEffect ) > 0 )
		return pszEffect;

	return BaseClass::GetMuzzleFlashParticleEffect();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::FireChargedShot( void )
{
	// Get the player owning the weapon.
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	CalcIsAttackCritical();

	WeaponSound( BURST );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	pPlayer->m_Shared.RemoveCond( TF_COND_AIMING );
	pPlayer->TeamFortress_SetSpeed();

	CTFProjectile_EnergyBall *pProjectile = static_cast<CTFProjectile_EnergyBall *>( FireProjectile( pPlayer ) );
	if ( pProjectile )
		pProjectile->SetChargedShot( true );

#ifdef GAME_DLL
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

	m_flChargeBeginTime = 0.0f;
	Energy_DrainEnergy( 3.0f );
}
#else
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
void CTFParticleCannon::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::CreateMuzzleFlashEffects( C_BaseEntity *pAttachEnt, int nIndex )
{
	BaseClass::CreateMuzzleFlashEffects( pAttachEnt, nIndex );

	// Don't do backblast effects in first person
	C_TFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner->IsLocalPlayer() )
		return;

	ParticleProp()->Create( "rocketbackblast", PATTACH_POINT_FOLLOW, "backblast" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFParticleCannon::CreateChargeEffect( void )
{
	ProcessMuzzleFlashEvent();
}
#endif