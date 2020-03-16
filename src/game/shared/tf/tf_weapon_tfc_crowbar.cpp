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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFCUmbrella::CTFCUmbrella()
{
}
