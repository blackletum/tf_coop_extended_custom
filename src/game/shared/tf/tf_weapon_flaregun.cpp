//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_flaregun.h"
#include "tf_projectile_flare.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#include "soundent.h"
#include "tf_gamestats.h"
#endif

CREATE_SIMPLE_WEAPON_TABLE( TFFlareGun, tf_weapon_flaregun )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFFlareGun::CTFFlareGun()
{
	m_bReloadsSingly = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFFlareGun::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFFlareGun::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFFlareGun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFFlareGun::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFFlareGun::PrimaryAttack( void )
{
	// Cannot fire underwater
	if ( GetPlayerOwner() && GetPlayerOwner()->GetWaterLevel() == 3 )
	{
		WeaponSound( EMPTY );
		return;
	}

	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 )
	{
		// Check for ammunition.
		if ( m_iClip1 <= 0 && UsesClipsForAmmo1() )
			return;

		// Get the player owning the weapon.
		CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
		if ( !pPlayer )
			return;

		if ( !CanAttack() )
			return;

		CalcIsAttackCritical();

#ifndef CLIENT_DLL
		pPlayer->RemoveInvisibility();
		pPlayer->RemoveDisguise();
		pPlayer->SpeakWeaponFire();
		CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.5, pPlayer, SOUNDENT_CHANNEL_WEAPON );
#endif

		// Set the weapon mode.
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

		SendWeaponAnim( ACT_VM_PRIMARYATTACK );

		pPlayer->SetAnimation( PLAYER_ATTACK1 );

		CBaseEntity *pProjectile = FireProjectile( pPlayer );
		if ( pProjectile )
		{
#ifdef GAME_DLL
			CTFProjectile_Flare *pFlare = (CTFProjectile_Flare*)pProjectile;
			AddFlare( pFlare );
			m_iFlareCount = m_Flares.Count();
#endif
		}

		m_flLastFireTime = gpGlobals->curtime;

		// Set next attack times.
		float flFireDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
		CALL_ATTRIB_HOOK_FLOAT( flFireDelay, mult_postfiredelay );

		if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
			flFireDelay /= 2;

		if ( pPlayer->m_Shared.InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
			flFireDelay /= 1.5;

		m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;

		// Don't push out secondary attack, because our secondary fire
		// systems are all separate from primary fire (sniper zooming, demoman pipebomb detonating, etc)
		//m_flNextSecondaryAttack = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;

		// Set the idle animation times based on the sequence duration, so that we play full fire animations
		// that last longer than the refire rate may allow.
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		AbortReload();
	}
	else
	{
		BaseClass::PrimaryAttack();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFFlareGun::SecondaryAttack( void )
{
	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );
	if ( iType == 1 )
	{
		if ( !CanAttack() )
			return;

		if ( m_iFlareCount )
		{
			// Get a valid player.
			CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
			if ( !pPlayer )
				return;

			for ( int i = 0; i < m_Flares.Count(); i++ )
			{
				CTFProjectile_Flare *pTemp = m_Flares[i];
				if ( pTemp )
				{
					//This guy will die soon enough.
					if ( pTemp->IsEffectActive( EF_NODRAW ) )
						continue;

#ifdef GAME_DLL
					pTemp->Detonate( true );
#endif
				}
			}
		}
	}
	else
	{
		BaseClass::SecondaryAttack();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFFlareGun::WeaponReset( void )
{
	BaseClass::WeaponReset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFFlareGun::AddFlare( CTFProjectile_Flare *pFlare )
{
	FlareHandle hHandle;
	hHandle = pFlare;
	m_Flares.AddToTail( hHandle );
}


//-----------------------------------------------------------------------------
// Purpose: If a flare has been removed, remove it from our list
//-----------------------------------------------------------------------------
void CTFFlareGun::DeathNotice( CBaseEntity *pVictim )
{
	Assert( dynamic_cast<CTFProjectile_Flare*>( pVictim ) );

	FlareHandle hHandle;
	hHandle = (CTFProjectile_Flare*)pVictim;
	m_Flares.FindAndRemove( hHandle );

	m_iFlareCount = m_Flares.Count();
}

