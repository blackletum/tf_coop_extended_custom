//====== Copyright ? 1996-2020, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_slap.h"


// Server specific.
#if !defined( CLIENT_DLL )
#include "tf_player.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
// Client specific.
#else
#include "c_tf_player.h"
#endif

//=============================================================================
//
// Weapon SLAP tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFSlap, tf_weapon_slap )

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSlap::CTFSlap()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFSlap::~CTFSlap()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSlap::PrimaryAttack()
{
	BaseClass::PrimaryAttack();
	Slap();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFSlap::Slap()
{
	// Get the current player.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CanAttack() )
		return;
	
	// Set the weapon usage mode - primary, secondary.
	m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;
	m_bConnected = false;

	CalcIsAttackCritical();
	CalcIsAttackMiniCritical();

	// Set next attack times.
	float flFireDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flFireDelay, mult_postfiredelay );

	if ( pPlayer->m_Shared.InCond( TF_COND_RUNE_HASTE ) )
		flFireDelay /= 2;

	if ( pPlayer->m_Shared.InCond( TF_COND_HALLOWEEN_SPEED_BOOST ) )
		flFireDelay /= 1.5;

	m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;

	SetWeaponIdleTime( m_flNextPrimaryAttack + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeIdleEmpty );

	m_flSmackTime = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flSmackDelay;

	m_bCurrentAttackIsMiniCrit = pPlayer->m_Shared.GetNextMeleeCrit() != kCritType_None;

#ifdef GAME_DLL
	CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACritical() );
#endif
}

void CTFSlap::OnEntityHit( CBaseEntity *pEntity, CTakeDamageInfo *pInfo )
{
	BaseClass::OnEntityHit( pEntity, pInfo );

	CTFPlayer *pOwner = ToTFPlayer( GetPlayerOwner() );
	if ( !pOwner )
		return;

	if ( !pEntity )
		return;

	/*IGameEvent *eventfish = gameeventmanager->CreateEvent( "fish_notice" );
	if ( eventfish )
	{
		event->SetInt( "userid", pOwner->GetUserID() );
		event->SetInt( "victim_index", pEntity->entindex() );
		event->SetInt( "attacker", pOwner ? pOwner->GetUserID() : 0 );
		event->SetInt( "attacker_index", pOwner->entindex() );
		event->SetString( "attacker_name", pOwner ? pOwner->GetClassname() : NULL );
		event->SetInt( "attacker_team", pOwner ? pOwner->GetTeamNumber() : 0 );
		event->SetString( "weapon", "hot_hand" );
		event->SetInt( "weaponid", GetWeaponID() );
		event->SetString( "weapon_logclassname", "hot_hand" );
		event->SetInt( "damagebits", pInfo->GetDamageType() );
		event->SetInt( "customkill", pInfo->GetDamageCustom() );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted

		gameeventmanager->FireEvent( event );
	}*/
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
// ----------------------------------------------------------------------------
void CTFSlap::OnPlayerKill( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	CTFPlayer *pOwner = ToTFPlayer( info.GetAttacker() );
	if ( pOwner == NULL )
		return;

	if ( pOwner != GetOwner() )
		return;

	if ( !pVictim )
		return;

	/*IGameEvent *eventfish = gameeventmanager->CreateEvent( "fish_notice" );
	if ( eventfish )
	{
		event->SetInt( "userid", pOwner->GetUserID() );
		event->SetInt( "victim_index", pVictim->entindex() );
		event->SetInt( "attacker", pOwner ? pOwner->GetUserID() : 0 );
		event->SetInt( "attacker_index", pOwner->entindex() );
		event->SetString( "attacker_name", pOwner ? pOwner->GetClassname() : NULL );
		event->SetInt( "attacker_team", pOwner ? pOwner->GetTeamNumber() : 0 );
		event->SetString( "weapon", "hot_hand" );
		event->SetInt( "weaponid", GetWeaponID() );
		event->SetString( "weapon_logclassname", "hot_hand" );
		event->SetInt( "damagebits", pInfo->GetDamageType() );
		event->SetInt( "customkill", TF_DMG_CUSTOM_SLAP_KILL );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted

		gameeventmanager->FireEvent( event );
	}*/
}
#endif