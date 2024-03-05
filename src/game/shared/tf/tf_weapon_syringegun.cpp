//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_syringegun.h"
#include "tf_fx_shared.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Syringe Gun tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFSyringeGun, tf_weapon_syringegun_medic )
CREATE_SIMPLE_WEAPON_TABLE( TFCrossBow, tf_weapon_crossbow )

//=============================================================================
//
// Weapon SyringeGun functions.
//
void CTFSyringeGun::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	PrecacheTeamParticles( "nailtrails_medic_%s" );
	PrecacheTeamParticles( "nailtrails_medic_%s_crit" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCrossBow::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCrossBow::SecondaryAttack( void )
{
	int iAttribModCrossbowAddZoom = 0;
	CALL_ATTRIB_HOOK_INT(iAttribModCrossbowAddZoom, mod_crossbow_secondaryfire);
	CTFPlayer *pOwner = ToTFPlayer(GetOwner());
	if (!pOwner){
		DevMsg("CTFCrossBow has no owner yet is trying to secondary fire! \n");
		return;
	}
		


	if (iAttribModCrossbowAddZoom == 1 && m_flNextSecondaryAttack <= gpGlobals->curtime){
		if (pOwner->m_Shared.InCond(TF_COND_ZOOMED))
		{
			ZoomOut();
			pOwner->m_Shared.RemoveCond(TF_COND_AIMING);
		}
		else{
			ZoomIn();
			pOwner->m_Shared.AddCond(TF_COND_AIMING);
		}
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
	}

	BaseClass::SecondaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCrossBow::WeaponRegenerate( void )
{
	BaseClass::WeaponRegenerate();
}

bool CTFCrossBow::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if( !m_bReloadedThroughAnimEvent )
	{
		float flFireDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
		CALL_ATTRIB_HOOK_FLOAT( flFireDelay, mult_postfiredelay );

		float flReloadTime = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeReload;
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, mult_reload_time );
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, mult_reload_time_hidden );
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, fast_reload );

		float flDelay = flReloadTime + flFireDelay;
		if ( flDelay > GetWeaponIdleTime() )
		{
			SetWeaponIdleTime( flDelay );
			m_flNextPrimaryAttack = flDelay;
		}

		IncrementAmmo();
	}
	CTFPlayer *pOwner = ToTFPlayer(GetOwner());
	if (!pOwner){
		DevMsg("CTFCrossBow has no owner yet is trying to holster! \n");
	}
	if (pOwner->m_Shared.InCond(TF_COND_ZOOMED))
	{
		ZoomOut();
		pOwner->m_Shared.RemoveCond(TF_COND_AIMING);
	}
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCrossBow::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
}
