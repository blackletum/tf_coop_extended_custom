//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_tfc_crowbar.h"
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
// Weapon Crowbar tables.
//
CREATE_SIMPLE_WEAPON_TABLE( TFCCrowbar, tf_weapon_tfc_crowbar )
CREATE_SIMPLE_WEAPON_TABLE( TFCUmbrella, tf_weapon_tfc_umbrella )

//=============================================================================
//
// Weapon Crowbar functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFCCrowbar::CTFCCrowbar()
{
}
void CTFCCrowbar::OnSwingHit(trace_t &trace)
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if (pPlayer == nullptr)
		return;

	// Do Damage.
	DoMeleeDamage(trace.m_pEnt, trace);

	// Hit sound - immediate.
	if (trace.m_pEnt)
	{
		if (trace.m_pEnt->IsPlayer())
		{
			WeaponSound(MELEE_HIT);
		}
		else if (trace.m_pEnt->IsNPC())
		{
				WeaponSound(MELEE_HIT);
		}
		else
		{
			WeaponSound(MELEE_HIT_WORLD);
		}
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
		// Don't impact trace friendly players or objects
		if (!pPlayer->InSameTeam(trace.m_pEnt))
		{
#if defined( CLIENT_DLL )
			UTIL_ImpactTrace(&trace, DMG_CLUB);
#endif
			m_bConnected = true;
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFCUmbrella::CTFCUmbrella()
{
}
